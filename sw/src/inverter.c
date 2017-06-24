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
CALMEAS_SYMBOL(float, m_ivtr_i_a,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_b,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_i_c,   0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_a, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_b, 0.0f, "");
CALMEAS_SYMBOL(float, m_ivtr_emf_c, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(uint8_t,  p_ivtr_manual_step, 0, "");
CALMEAS_SYMBOL(uint8_t,  p_ivtr_debug_output_sel, 0, "");

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

  const float dt = 1.0f/((float)PWM_FREQUENCY_HZ);

  m_ivtr_i_a = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_A));
  m_ivtr_i_b = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_B));
  m_ivtr_i_c = board_ix_volt_to_ampere(adc_get_measurement(ADC_I_C));
  m_ivtr_emf_a = adc_get_measurement(ADC_EMF_A);
  m_ivtr_emf_b = adc_get_measurement(ADC_EMF_B);
  m_ivtr_emf_c = adc_get_measurement(ADC_EMF_C);

#if 0
  position_update_angle_filter(dt);
#else
  position_update_speed_filter(dt);
#endif

  rec_input(&rec_pos, position_get_angle());

  bldc6s_period_by_period_handler(dt,
                                  m_ivtr_i_a, m_ivtr_i_b, m_ivtr_i_c,
                                  m_ivtr_emf_a, m_ivtr_emf_b, m_ivtr_emf_c);

  if (p_ivtr_debug_output_sel <= ADC_EMF_C) {
    ext_dac_set_value_raw(adc_get_measurement_raw((adc_measurement_t) p_ivtr_debug_output_sel));
  }

  //DBG_PAD3_RESET;
}

void ivtr_step(uint32_t period_ms)
{
  static uint8_t prev_p_ivtr_manual_step = 0;

  rec_pos_handler();

  switch (modes_current_mode()) {

    case BLDC_HALL_CALIBRATION:
      board_gate_driver_enable();
      bldc6s_hall_calibration_step(period_ms);
      break;

    case RUNNING:
      board_gate_driver_enable();
      bldc6s_allow_hall_commutation(true);
      break;

    /* Debugging modes */
    case MANUAL_STEP:
      board_gate_driver_enable();
      bldc6s_allow_hall_commutation(false);

      if (p_ivtr_manual_step != prev_p_ivtr_manual_step) {
        p_ivtr_manual_step = (uint8_t) MIN(p_ivtr_manual_step, 6u);
        bldc6s_set_commutation_step(p_ivtr_manual_step);
      }
      prev_p_ivtr_manual_step = p_ivtr_manual_step;

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
  bldc6s_allow_hall_commutation(false);
  ivtr_idle_state();
}

void ivtr_request_duty_cycle(float duty_req)
{
  bldc6s_request_duty_cycle(duty_req);
}

int ivtr_init(void)
{
  rec_init(&rec_pos, rec_pos_buffer, REC_POS_SAMPLES);

  adc_set_new_samples_indication_cb(ivtr_period_by_period_handler);
  hall_set_commutation_indication_cb(bldc6s_hall_commutation);

  bldc6s_init();

  return 0;
}
