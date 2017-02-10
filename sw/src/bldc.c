/*
 * bldc.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "bldc.h"
#include "utils.h"
#include "modes.h"
#include "pwm.h"
#include "board.h"
#include "hall.h"

#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(float, m_bldc_set_duty, 0, "");
#define CALMEAS_TYPECODE_bldc_direction_t   CALMEAS_TYPECODE_uint8_t
#define CALMEAS_MEMSEC_bldc_direction_t     CALMEAS_MEMSEC_uint8_t
CALMEAS_SYMBOL(bldc_direction_t, m_bldc_direction_req, DIR_NONE, "");

static uint8_t bldc_hall_state_to_step_map[NUMBER_OF_DIRS][POS_NUMBER_OF_HALL_STATES];
static bldc_cal_state_t cal_state = CAL_NOT_PERFORMED;

static void (* const commutation_steps_ch3[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch3_off,
  /* Normal steps */
  [STEP_1] = pwm_ch3_sink,
  [STEP_2] = pwm_ch3_sink,
  [STEP_3] = pwm_ch3_off,
  [STEP_4] = pwm_ch3_pwm_afw,
  [STEP_5] = pwm_ch3_pwm_afw,
  [STEP_6] = pwm_ch3_off
};

static void (* const commutation_steps_ch2[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch2_off,
  /* Normal steps */
  [STEP_1] = pwm_ch2_pwm_afw,
  [STEP_2] = pwm_ch2_off,
  [STEP_3] = pwm_ch2_sink,
  [STEP_4] = pwm_ch2_sink,
  [STEP_5] = pwm_ch2_off,
  [STEP_6] = pwm_ch2_pwm_afw
};

static void (* const commutation_steps_ch1[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch1_off,
  /* Normal steps */
  [STEP_1] = pwm_ch1_off,
  [STEP_2] = pwm_ch1_pwm_afw,
  [STEP_3] = pwm_ch1_pwm_afw,
  [STEP_4] = pwm_ch1_off,
  [STEP_5] = pwm_ch1_sink,
  [STEP_6] = pwm_ch1_sink
};

static void hall_calibration_step(uint32_t period_ms)
{
  static uint8_t step;
  static uint32_t delay_ms = 0u;
  const uint32_t rotation_delay_time_ms = 500u;
  const uint32_t probing_delay_time_ms = 500u;
  uint8_t hallstate, step_cw, step_ccw;

  switch (cal_state) {

    case CAL_NOT_PERFORMED:
      bldc_safe_state();
      step = 6u;
      cal_state = CAL_PRE_ROTATION;
      break;

    case CAL_PRE_ROTATION:
      commutation_steps_ch1[step]();
      commutation_steps_ch2[step]();
      commutation_steps_ch3[step]();
      pwm_commutation_event();
      pwm_set_duty_perc(2.0f);
      if (delay_ms < rotation_delay_time_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0u;
        if (step == 1u) {
          cal_state = CAL_PROBING;
        } else {
          step--;
        }
      }
      break;

    case CAL_PROBING:
      commutation_steps_ch1[step]();
      commutation_steps_ch2[step]();
      commutation_steps_ch3[step]();
      pwm_commutation_event();
      pwm_set_duty_perc(2.0f);
      if (delay_ms < probing_delay_time_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0u;

        hallstate = hall_state();

        if (step == 6) {
          step_cw = 1;
        } else {
          step_cw = step + 1;
        }

        if (step == 1) {
          step_ccw = 6;
        } else {
          step_ccw = step - 1;
        }

        bldc_hall_state_to_step_map[DIR_CW][hallstate]  = step_cw;
        bldc_hall_state_to_step_map[DIR_CCW][hallstate] = step_ccw;

        if (step >= 6) {
          cal_state = CAL_CHECK;
        }

        step++;
      }
      break;

    case CAL_CHECK:
      cal_state = CAL_OK;
      break;

    case CAL_OK:
      bldc_safe_state();
      break;

    default:
      break;
  }
}

bldc_cal_state_t bldc_cal_state(void)
{
  return cal_state;
}

void bldc_step(uint32_t period_ms)
{
  static bldc_direction_t prev_dir = DIR_NONE;

  switch (modes_current_mode()) {

    case BLDC_HALL_CALIBRATION:
      board_gate_driver_enable();
      hall_calibration_step(period_ms);
      break;

    case RUNNING:
      board_gate_driver_enable();

      if (m_bldc_set_duty > 0.0f) {
        m_bldc_direction_req = DIR_CW;
      } else if (m_bldc_set_duty < 0.0f) {
        m_bldc_direction_req = DIR_CCW;
      } else {
        m_bldc_direction_req = DIR_NONE;
      }

      if (prev_dir != m_bldc_direction_req) {
        bldc_commutation(m_bldc_direction_req, hall_state());
      }
      prev_dir = m_bldc_direction_req;

      pwm_set_duty_perc(ABS(m_bldc_set_duty));
      break;

    default:
      bldc_safe_state();
      break;
  }
}

void bldc_safe_state(void)
{
  pwm_set_duty_perc(0.0f);
  commutation_steps_ch1[STEP_OFF]();
  commutation_steps_ch2[STEP_OFF]();
  commutation_steps_ch3[STEP_OFF]();
  pwm_commutation_event();
  board_gate_driver_disable();
}

void bldc_set_duty_cycle(float duty_req)
{
  m_bldc_set_duty = duty_req;
}

int bldc_init(void)
{
  uint8_t hall_state;
  for (hall_state = 0; hall_state < POS_NUMBER_OF_HALL_STATES; hall_state++) {
    bldc_hall_state_to_step_map[DIR_NONE][hall_state] = STEP_OFF;
    bldc_hall_state_to_step_map[DIR_CW][hall_state]   = STEP_OFF;
    bldc_hall_state_to_step_map[DIR_CCW][hall_state]  = STEP_OFF;
  }

  return 0;
}

void bldc_commutation(bldc_direction_t direction, uint8_t current_hall_state)
{
  uint8_t next_step = bldc_hall_state_to_step_map[direction][current_hall_state];

  commutation_steps_ch1[next_step]();
  commutation_steps_ch2[next_step]();
  commutation_steps_ch3[next_step]();

  pwm_commutation_event();
}

void bldc_hall_indication(uint8_t current_hall_state)
{
  if (RUNNING == modes_current_mode()) {
    bldc_commutation(m_bldc_direction_req, current_hall_state);
  }
}
