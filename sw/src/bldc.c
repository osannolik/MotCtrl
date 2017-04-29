/*
 * bldc.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "bldc.h"
#include "bldc_6step.h"
#include "utils.h"
#include "modes.h"
#include "pwm.h"
#include "adc.h"
#include "ext.h"
#include "board.h"

#include "debug.h"

#include "calmeas.h"


/* Measurements */
CALMEAS_SYMBOL(float, m_bldc_set_duty, 0, "");
#define CALMEAS_TYPECODE_pos_direction_t   CALMEAS_TYPECODE_uint8_t
#define CALMEAS_MEMSEC_pos_direction_t     CALMEAS_MEMSEC_uint8_t
CALMEAS_SYMBOL(pos_direction_t, m_bldc_direction_req, DIR_NONE, "");

/* Parameters */
CALMEAS_SYMBOL(uint8_t,  p_bldc_manual_step, 0, "");
CALMEAS_SYMBOL(uint32_t, p_bldc_openloop_commutation_delay_ms, 200, "");
CALMEAS_SYMBOL(uint8_t,  p_bldc_debug_output_sel, 0, "");
CALMEAS_SYMBOL(float,    p_bldc_sample_trigger, -3.0f, "");  // Offset to compensate for sample delay
//CALMEAS_SYMBOL(float,    p_bldc_sample_trigger, 0.5f, "");



CALMEAS_SYMBOL(float, m_bldc_i_a,   0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_i_b,   0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_i_c,   0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_emf_a, 0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_emf_b, 0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_emf_c, 0.0f, "");

CALMEAS_SYMBOL(uint8_t, p_bldc_current_ctrl_enable, 0, "");
CALMEAS_SYMBOL(float, p_bldc_current_ctrl_set_point_A, 0.0f, "");
CALMEAS_SYMBOL(float, p_bldc_current_ctrl_gain, 0.0f, "");
CALMEAS_SYMBOL(float, p_bldc_current_ctrl_max_delta, 0.0f, "");
CALMEAS_SYMBOL(float, p_bldc_current_ctrl_max, 0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_current_ctrl_diff, 0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_i_tot, 0.0f, "");
CALMEAS_SYMBOL(float, m_bldc_i_tot_avg, 0.0f, "");

CALMEAS_SYMBOL(uint32_t, p_bldc_i_tot_avg_len, 10, "");
#define I_HIST_LEN (200)

static void direction_commutation(const float duty_req)
{
  static pos_direction_t prev_direction = DIR_NONE;

  float direction;

  if (duty_req > 0.0f) {
    direction = DIR_CW;
  } else if (duty_req < 0.0f) {
    direction = DIR_CCW;
  } else {
    direction = DIR_NONE;
  }

  if (prev_direction != direction) {
    m_bldc_direction_req = direction;
    bldc6s_commutation(direction, position_get_hall_state());
  }

  prev_direction = direction;
}

static void bldc_period_by_period_handler(void)
{
  //DBG_PAD3_SET;

  m_bldc_i_a = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_A));
  m_bldc_i_b = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_B));
  m_bldc_i_c = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_C));
  m_bldc_emf_a = adc_get_measurement(ADC_EMF_A);
  m_bldc_emf_b = adc_get_measurement(ADC_EMF_B);
  m_bldc_emf_c = adc_get_measurement(ADC_EMF_C);


  (void) position_angle_est_update(1.0f/((float)PWM_FREQUENCY_HZ));


  float i_tot;
  switch (bldc6s_current_step()) {
    case STEP_1:
      i_tot = (m_bldc_i_c - m_bldc_i_a)*0.5f;
      break;
    case STEP_2:
      i_tot = (m_bldc_i_c - m_bldc_i_b)*0.5f;
      break;
    case STEP_3:
      i_tot = (m_bldc_i_a - m_bldc_i_b)*0.5f;
      break;
    case STEP_4:
      i_tot = (m_bldc_i_a - m_bldc_i_c)*0.5f;
      break;
    case STEP_5:
      i_tot = (m_bldc_i_b - m_bldc_i_c)*0.5f;
      break;
    case STEP_6:
      i_tot = (m_bldc_i_b - m_bldc_i_a)*0.5f;
      break;
    default:
      i_tot = 0.0f;
      break;
  }

  i_tot = MAX(i_tot, 0.0f);

  static float i_tot_hist[I_HIST_LEN];
  static uint32_t i_index = 0;

  i_tot_hist[i_index++] = i_tot;

  if (i_index >= p_bldc_i_tot_avg_len) {
    i_index = 0;
  }

  float i_tot_avg = 0;
  uint32_t i;
  for (i=0; i<p_bldc_i_tot_avg_len; i++) {
    i_tot_avg += i_tot_hist[i];
  }
  i_tot_avg = i_tot_avg/((float) p_bldc_i_tot_avg_len);

  float u_delta = 0;

  if (p_bldc_current_ctrl_enable != 0) {
    m_bldc_current_ctrl_diff = p_bldc_current_ctrl_set_point_A - i_tot_avg;
    u_delta = p_bldc_current_ctrl_gain * m_bldc_current_ctrl_diff;

    u_delta = saturatef(u_delta, -p_bldc_current_ctrl_max_delta, p_bldc_current_ctrl_max_delta);

    m_bldc_set_duty = saturatef(m_bldc_set_duty+u_delta, 5.0f, p_bldc_current_ctrl_max);
  }


  pwm_set_sample_trigger_perc(p_bldc_sample_trigger);

  //pwm_set_sample_trigger_perc(p_bldc_sample_trigger * m_bldc_set_duty);

  direction_commutation(m_bldc_set_duty);
  
  pwm_set_duty_gate_abc_perc(ABS(m_bldc_set_duty));


  uint16_t dac_output;

  if (p_bldc_debug_output_sel <= ADC_EMF_C) {
    dac_output = adc_get_measurement_raw(p_bldc_debug_output_sel);
  } else if (p_bldc_debug_output_sel == ADC_EMF_C + 1) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * board_ix_ampere_to_volt(i_tot);
  } else if (p_bldc_debug_output_sel == ADC_EMF_C + 2) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * 3.3f/100.0f * m_bldc_set_duty;
  } else if (p_bldc_debug_output_sel == ADC_EMF_C + 3) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * 3.3f/p_bldc_current_ctrl_max_delta * u_delta;
  } else {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * board_ix_ampere_to_volt(i_tot_avg);
  }

  m_bldc_i_tot = i_tot;
  m_bldc_i_tot_avg = i_tot_avg;

  ext_dac_set_value_raw(dac_output);

  //DBG_PAD3_RESET;
}

static void bldc_hall_commutation(uint8_t current_hall_state)
{
  DBG_PAD1_TOGGLE;

  if (current_hall_state == 1u) {
    DBG_PAD2_SET;
    DBG_PAD2_RESET;
  }

  if (RUNNING == modes_current_mode()) {
    bldc6s_commutation(m_bldc_direction_req, current_hall_state);
  }
}

void bldc_step(uint32_t period_ms)
{
  static uint8_t prev_p_bldc_manual_step = 0;

  static uint32_t delay_ms = 0u;
  static int8_t step;

  switch (modes_current_mode()) {

    case BLDC_HALL_CALIBRATION:
      board_gate_driver_enable();
      bldc6s_hall_calibration_step(period_ms);
      break;

    case RUNNING:
      board_gate_driver_enable();
      break;

    /* Debugging modes */
    case MANUAL_STEP:
      board_gate_driver_enable();

      if (p_bldc_manual_step != prev_p_bldc_manual_step) {
        p_bldc_manual_step = MIN(p_bldc_manual_step, 6u);
        bldc6s_set_commutation_step(p_bldc_manual_step);
      }
      prev_p_bldc_manual_step = p_bldc_manual_step;

      break;

    case OPEN_LOOP:
      board_gate_driver_enable();

      if (delay_ms < p_bldc_openloop_commutation_delay_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0;
        if (++step > 6) {
          step = 1;
        }
        bldc6s_set_commutation_step(step);
      }

      break;

    default:
      bldc_idle_state();
      break;
  }
}

void bldc_idle_state(void)
{
  board_gate_driver_disable();

  bldc6s_set_commutation_step(STEP_OFF);

  pwm_set_duty_gate_abc_perc(0.0f);

  bldc_request_duty_cycle(0.0f);
}

void bldc_safe_state(void)
{
  pwm_disable_sample_trigger();
  bldc_idle_state();
}

void bldc_request_duty_cycle(float duty_req)
{
  m_bldc_set_duty = duty_req;
}

int bldc_init(void)
{
  adc_set_new_samples_indication_cb(bldc_period_by_period_handler);
  position_set_hall_commutation_indication_cb(bldc_hall_commutation);

  bldc6s_init();

  return 0;
}
