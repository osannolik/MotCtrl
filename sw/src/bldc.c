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

#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(float, m_bldc_set_duty, 0, "");
#define CALMEAS_TYPECODE_pos_direction_t   CALMEAS_TYPECODE_uint8_t
#define CALMEAS_MEMSEC_pos_direction_t     CALMEAS_MEMSEC_uint8_t
CALMEAS_SYMBOL(pos_direction_t, m_bldc_direction_req, DIR_NONE, "");

/* Parameters */
CALMEAS_SYMBOL(uint8_t, p_manual_step, 0, "");
CALMEAS_SYMBOL(uint32_t, p_openloop_commutation_delay_ms, 200, "");

static bldc_cal_state_t cal_state = CAL_NOT_PERFORMED;

static void (* const commutation_steps_ch3[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch3_off,
  /* Normal steps */
  [STEP_1] = pwm_ch3_pwm_afw,
  [STEP_2] = pwm_ch3_pwm_afw,
  [STEP_3] = pwm_ch3_off,
  [STEP_4] = pwm_ch3_sink,
  [STEP_5] = pwm_ch3_sink,
  [STEP_6] = pwm_ch3_off
};

static void (* const commutation_steps_ch2[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch2_off,
  /* Normal steps */
  [STEP_1] = pwm_ch2_off,
  [STEP_2] = pwm_ch2_sink,
  [STEP_3] = pwm_ch2_sink,
  [STEP_4] = pwm_ch2_off,
  [STEP_5] = pwm_ch2_pwm_afw,
  [STEP_6] = pwm_ch2_pwm_afw
};

static void (* const commutation_steps_ch1[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch1_off,
  /* Normal steps */
  [STEP_1] = pwm_ch1_sink,
  [STEP_2] = pwm_ch1_off,
  [STEP_3] = pwm_ch1_pwm_afw,
  [STEP_4] = pwm_ch1_pwm_afw,
  [STEP_5] = pwm_ch1_off,
  [STEP_6] = pwm_ch1_sink
};

static uint8_t bldc_hall_state_to_step_map[NUMBER_OF_DIRS][POS_NUMBER_OF_HALL_STATES] =
{
  [DIR_NONE] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
  [DIR_CW]   = {0u, 1u, 5u, 6u, 3u, 2u, 4u, 0u},
  [DIR_CCW]  = {0u, 5u, 3u, 4u, 1u, 6u, 2u, 0u}
};

#if (POS_HALL_SENSOR_OFFSET_DEG > 15)
#define commutation_steps_ch3_cal commutation_steps_ch3
#define commutation_steps_ch2_cal commutation_steps_ch2
#define commutation_steps_ch1_cal commutation_steps_ch1

static const uint8_t cal_step_to_commutation_step[NUMBER_OF_DIRS][NUMBER_OF_STEPS] =
{
  [DIR_NONE] = {STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF},
  [DIR_CW]   = {STEP_OFF, STEP_3,   STEP_4,   STEP_5,   STEP_6,   STEP_1,   STEP_2},
  [DIR_CCW]  = {STEP_OFF, STEP_5,   STEP_6,   STEP_1,   STEP_2,   STEP_3,   STEP_4}
};
#else
static void (* const commutation_steps_ch3_cal[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch3_off,
  /* Normal steps */
  [STEP_1] = pwm_ch3_pwm_afw,
  [STEP_2] = pwm_ch3_pwm_afw,
  [STEP_3] = pwm_ch3_pwm_afw,
  [STEP_4] = pwm_ch3_sink,
  [STEP_5] = pwm_ch3_sink,
  [STEP_6] = pwm_ch3_sink
};

static void (* const commutation_steps_ch2_cal[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch2_off,
  /* Normal steps */
  [STEP_1] = pwm_ch2_pwm_afw,
  [STEP_2] = pwm_ch2_sink,
  [STEP_3] = pwm_ch2_sink,
  [STEP_4] = pwm_ch2_sink,
  [STEP_5] = pwm_ch2_pwm_afw,
  [STEP_6] = pwm_ch2_pwm_afw
};

static void (* const commutation_steps_ch1_cal[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch1_off,
  /* Normal steps */
  [STEP_1] = pwm_ch1_sink,
  [STEP_2] = pwm_ch1_sink,
  [STEP_3] = pwm_ch1_pwm_afw,
  [STEP_4] = pwm_ch1_pwm_afw,
  [STEP_5] = pwm_ch1_pwm_afw,
  [STEP_6] = pwm_ch1_sink
};

static const uint8_t cal_step_to_commutation_step[NUMBER_OF_DIRS][NUMBER_OF_STEPS] =
{
  [DIR_NONE] = {STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF, STEP_OFF},
  [DIR_CW]   = {STEP_OFF, STEP_2,   STEP_3,   STEP_4,   STEP_5,   STEP_6,   STEP_1},
  [DIR_CCW]  = {STEP_OFF, STEP_5,   STEP_6,   STEP_1,   STEP_2,   STEP_3,   STEP_4}
};
#endif /* (POS_HALL_SENSOR_OFFSET_DEG > 15) */


static void hall_calibration_step(uint32_t period_ms)
{
  static int8_t step;
  static uint32_t delay_ms = 0u;
  const uint32_t rotation_delay_time_ms = 500u;
  const uint32_t probing_delay_time_ms = 500u;
  uint8_t hallstate;
  static float angle = 0.0f;

  switch (cal_state) {

    case CAL_NOT_PERFORMED:
      bldc_safe_state();
      step = 6u;
      cal_state = CAL_PRE_ROTATION;
      break;

    case CAL_PRE_ROTATION:
      commutation_steps_ch1_cal[step]();
      commutation_steps_ch2_cal[step]();
      commutation_steps_ch3_cal[step]();

      pwm_update_event();
      pwm_set_duty_perc(3.0f);
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
      commutation_steps_ch1_cal[step]();
      commutation_steps_ch2_cal[step]();
      commutation_steps_ch3_cal[step]();

      pwm_update_event();
      pwm_set_duty_perc(3.0f);
      if (delay_ms < probing_delay_time_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0u;

        hallstate = (int8_t) position_get_hall_state();

        position_map_hall_state_to_angle(hallstate, angle);

        bldc_hall_state_to_step_map[DIR_CW][hallstate]  = cal_step_to_commutation_step[DIR_CW][step];
        bldc_hall_state_to_step_map[DIR_CCW][hallstate] = cal_step_to_commutation_step[DIR_CCW][step];

        if (step >= 6) {
          cal_state = CAL_CHECK;
        }

        step++;
        angle += 60.0f;
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
  static pos_direction_t prev_dir = DIR_NONE;

  static uint8_t prev_p_manual_step = 0;

  static uint32_t delay_ms = 0u;
  static int8_t step;

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
        bldc_commutation(m_bldc_direction_req, position_get_hall_state());
      }
      prev_dir = m_bldc_direction_req;

      pwm_set_duty_perc(ABS(m_bldc_set_duty));

      break;

    /* Debugging modes */
    case MANUAL_STEP:
      board_gate_driver_enable();

      if (p_manual_step != prev_p_manual_step) {
        p_manual_step = MIN(p_manual_step, 6u);
        commutation_steps_ch1[p_manual_step]();
        commutation_steps_ch2[p_manual_step]();
        commutation_steps_ch3[p_manual_step]();
        pwm_update_event();
      }
      prev_p_manual_step = p_manual_step;

      pwm_set_duty_perc(ABS(m_bldc_set_duty));

      break;

    case OPEN_LOOP:
      board_gate_driver_enable();

      if (delay_ms < p_openloop_commutation_delay_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0;
        if (++step > 6) {
          step = 1;
        }
        commutation_steps_ch1[step]();
        commutation_steps_ch2[step]();
        commutation_steps_ch3[step]();
        pwm_update_event();
      }

      pwm_set_duty_perc(ABS(m_bldc_set_duty));

      break;

    default:
      bldc_safe_state();
      break;
  }
}

void bldc_safe_state(void)
{
  board_gate_driver_disable();
  pwm_set_duty_perc(0.0f);
  commutation_steps_ch1[STEP_OFF]();
  commutation_steps_ch2[STEP_OFF]();
  commutation_steps_ch3[STEP_OFF]();
  pwm_update_event();
}

void bldc_set_duty_cycle(float duty_req)
{
  m_bldc_set_duty = duty_req;
}

static void bldc_hall_commutation(uint8_t current_hall_state)
{
  if (RUNNING == modes_current_mode()) {
    bldc_commutation(m_bldc_direction_req, current_hall_state);
  }
}

int bldc_init(void)
{
  position_set_hall_commutation_indication_cb(bldc_hall_commutation);

#if 1
  uint8_t hall_state;
  for (hall_state = 0; hall_state < POS_NUMBER_OF_HALL_STATES; hall_state++) {
    bldc_hall_state_to_step_map[DIR_NONE][hall_state] = STEP_OFF;
    bldc_hall_state_to_step_map[DIR_CW][hall_state]   = STEP_OFF;
    bldc_hall_state_to_step_map[DIR_CCW][hall_state]  = STEP_OFF;
  }
#endif
  return 0;
}

void bldc_commutation(pos_direction_t direction, uint8_t current_hall_state)
{
  uint8_t next_step = bldc_hall_state_to_step_map[direction][current_hall_state];

  commutation_steps_ch1[next_step]();
  commutation_steps_ch2[next_step]();
  commutation_steps_ch3[next_step]();

  pwm_update_event();
}
