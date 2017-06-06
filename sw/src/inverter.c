/*
 * inverter.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "inverter.h"
#include "position.h"
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
CALMEAS_SYMBOL(float, m_ivtr_duty_set, 0, "");
CALMEAS_SYMBOL(float, m_ivtr_duty_req, 0, "");
CALMEAS_SYMBOL(uint8_t, m_ivtr_direction_req, DIR_NONE, "");
CALMEAS_SYMBOL(float, m_ivtr_i_a,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_b,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_c,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_a, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_b, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_c, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_current_ctrl_diff, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_tot, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_tot_avg, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(uint8_t,  p_ivtr_manual_step, 0, "");
CALMEAS_SYMBOL(uint32_t, p_ivtr_openloop_commutation_delay_ms, 200, "");
CALMEAS_SYMBOL(uint8_t,  p_ivtr_debug_output_sel, 0, "");
CALMEAS_SYMBOL(float,    p_ivtr_sample_trigger, -3.0f, "");  // Offset to compensate for sample delay
//CALMEAS_SYMBOL(float,    p_ivtr_sample_trigger, 0.5f, "");
CALMEAS_SYMBOL(uint8_t, p_ivtr_current_ctrl_enable, 0, "");
CALMEAS_SYMBOL(float, p_ivtr_current_ctrl_set_point_A, 0.0f, "");
CALMEAS_SYMBOL(float, p_ivtr_current_ctrl_gain, 0.0f, "");
CALMEAS_SYMBOL(float, p_ivtr_current_ctrl_max_delta, 0.0f, "");
CALMEAS_SYMBOL(float, p_ivtr_current_ctrl_max, 0.0f, "");
CALMEAS_SYMBOL(uint32_t, p_ivtr_i_tot_avg_len, 10, "");
CALMEAS_SYMBOL(float, p_ivtr_duty_rate_max, 0.1f, "");


#define I_HIST_LEN (200)

static rate_limit_t rlim_duty;

static void direction_commutation(const float duty_req)
{
  static uint8_t prev_direction = DIR_NONE;

  float direction;

  if (duty_req > 0.0f) {
    direction = DIR_CW;
  } else if (duty_req < 0.0f) {
    direction = DIR_CCW;
  } else {
    direction = DIR_NONE;
  }

  if (prev_direction != direction) {
    m_ivtr_direction_req = direction;
    bldc6s_commutation(direction, hall_get_state());
  }

  prev_direction = direction;
}

#include "recorder.h"

#define REC_POS_SAMPLES (2000)

float rec_pos_buffer[REC_POS_SAMPLES];
recorder_t rec_pos;

CALMEAS_SYMBOL(float, mdbg_rec_pos, 0.0, "");
CALMEAS_SYMBOL(uint8_t, pdbg_rec_start, 0, "");
CALMEAS_SYMBOL(uint8_t, pdbg_rec_play, 0, "");

void rec_pos_handler(void)
{
  static uint8_t pdbg_rec_start_prev = 0;

  if (pdbg_rec_start && (pdbg_rec_start != pdbg_rec_start_prev)) {
    rec_start(&rec_pos);
  }
  pdbg_rec_start_prev = pdbg_rec_start;

  static uint8_t pdbg_rec_play_prev = 0;
  if (pdbg_rec_play && (pdbg_rec_play != pdbg_rec_play_prev)) {
    rec_play(&rec_pos);
  }
  pdbg_rec_play_prev = pdbg_rec_play;

  rec_output(&rec_pos, &mdbg_rec_pos);
}

static void ivtr_period_by_period_handler(void)
{
  //DBG_PAD3_SET;

  m_ivtr_i_a = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_A));
  m_ivtr_i_b = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_B));
  m_ivtr_i_c = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_C));
  m_ivtr_emf_a = adc_get_measurement(ADC_EMF_A);
  m_ivtr_emf_b = adc_get_measurement(ADC_EMF_B);
  m_ivtr_emf_c = adc_get_measurement(ADC_EMF_C);

  position_update_angle_filter(1.0f/((float)PWM_FREQUENCY_HZ));

  rec_input(&rec_pos, position_get_angle());

  float i_tot;
  switch (bldc6s_current_step()) {
    case STEP_1:
      i_tot = (m_ivtr_i_c - m_ivtr_i_a)*0.5f;
      break;
    case STEP_2:
      i_tot = (m_ivtr_i_c - m_ivtr_i_b)*0.5f;
      break;
    case STEP_3:
      i_tot = (m_ivtr_i_a - m_ivtr_i_b)*0.5f;
      break;
    case STEP_4:
      i_tot = (m_ivtr_i_a - m_ivtr_i_c)*0.5f;
      break;
    case STEP_5:
      i_tot = (m_ivtr_i_b - m_ivtr_i_c)*0.5f;
      break;
    case STEP_6:
      i_tot = (m_ivtr_i_b - m_ivtr_i_a)*0.5f;
      break;
    default:
      i_tot = 0.0f;
      break;
  }

  i_tot = MAX(i_tot, 0.0f);

  static float i_tot_hist[I_HIST_LEN];
  static uint32_t i_index = 0;

  i_tot_hist[i_index++] = i_tot;

  if (i_index >= p_ivtr_i_tot_avg_len) {
    i_index = 0;
  }

  float i_tot_avg = 0;
  uint32_t i;
  for (i=0; i<p_ivtr_i_tot_avg_len; i++) {
    i_tot_avg += i_tot_hist[i];
  }
  i_tot_avg = i_tot_avg/((float) p_ivtr_i_tot_avg_len);

  float u_delta = 0;
  float tmp;

  if (p_ivtr_current_ctrl_enable != 0) {
    m_ivtr_current_ctrl_diff = p_ivtr_current_ctrl_set_point_A - i_tot_avg;
    u_delta = p_ivtr_current_ctrl_gain * m_ivtr_current_ctrl_diff;

    u_delta = saturatef(u_delta, -p_ivtr_current_ctrl_max_delta, p_ivtr_current_ctrl_max_delta);

    tmp = saturatef(m_ivtr_duty_set+u_delta, 5.0f, p_ivtr_current_ctrl_max);
  } else {
    tmp = m_ivtr_duty_req;
  }


  m_ivtr_duty_set = rate_limit(&rlim_duty, tmp, -p_ivtr_duty_rate_max, p_ivtr_duty_rate_max);

  pwm_set_sample_trigger_perc(p_ivtr_sample_trigger);

  //pwm_set_sample_trigger_perc(p_ivtr_sample_trigger * m_ivtr_duty_set);

  direction_commutation(m_ivtr_duty_set);
  
  pwm_set_duty_gate_abc_perc(ABS(m_ivtr_duty_set));


  uint16_t dac_output;

  if (p_ivtr_debug_output_sel <= ADC_EMF_C) {
    dac_output = adc_get_measurement_raw(p_ivtr_debug_output_sel);
  } else if (p_ivtr_debug_output_sel == ADC_EMF_C + 1) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * board_ix_ampere_to_volt(i_tot);
  } else if (p_ivtr_debug_output_sel == ADC_EMF_C + 2) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * 3.3f/100.0f * m_ivtr_duty_set;
  } else if (p_ivtr_debug_output_sel == ADC_EMF_C + 3) {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * 3.3f/p_ivtr_current_ctrl_max_delta * u_delta;
  } else {
    dac_output = EXT_DAC_LSB_PER_VOLTAGE * board_ix_ampere_to_volt(i_tot_avg);
  }

  m_ivtr_i_tot = i_tot;
  m_ivtr_i_tot_avg = i_tot_avg;

  ext_dac_set_value_raw(dac_output);

  //DBG_PAD3_RESET;
}

static void hall_commutation(uint8_t current_hall_state)
{
#if 1
  DBG_PAD1_TOGGLE;

  if (current_hall_state == 1u) {
    DBG_PAD2_SET;
    DBG_PAD2_RESET;
  }
#endif

  if (RUNNING == modes_current_mode()) {
    bldc6s_commutation(m_ivtr_direction_req, current_hall_state);
  }
}

void ivtr_step(uint32_t period_ms)
{
  static uint8_t prev_p_ivtr_manual_step = 0;

  static uint32_t delay_ms = 0u;
  static int8_t step;

  rec_pos_handler();

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

      if (p_ivtr_manual_step != prev_p_ivtr_manual_step) {
        p_ivtr_manual_step = MIN(p_ivtr_manual_step, 6u);
        bldc6s_set_commutation_step(p_ivtr_manual_step);
      }
      prev_p_ivtr_manual_step = p_ivtr_manual_step;

      break;

    case OPEN_LOOP:
      board_gate_driver_enable();

      if (delay_ms < p_ivtr_openloop_commutation_delay_ms) {
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
      ivtr_idle_state();
      break;
  }
}

void ivtr_idle_state(void)
{
  board_gate_driver_disable();

  bldc6s_set_commutation_step(STEP_OFF);

  pwm_set_duty_gate_abc_perc(0.0f);

  ivtr_request_duty_cycle(0.0f);
}

void ivtr_safe_state(void)
{
  pwm_disable_sample_trigger();
  ivtr_idle_state();
}

void ivtr_request_duty_cycle(float duty_req)
{
  m_ivtr_duty_req = duty_req;
}

int ivtr_init(void)
{
  rec_init(&rec_pos, rec_pos_buffer, REC_POS_SAMPLES);

  adc_set_new_samples_indication_cb(ivtr_period_by_period_handler);
  hall_set_commutation_indication_cb(hall_commutation);

  bldc6s_init();

  rate_limit_reset(&rlim_duty, 0.0f);

  return 0;
}
