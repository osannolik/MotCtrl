/*
 * bldc_6step.c
 *
 *  Created on: 29 apr 2017
 *      Author: osannolik
 */

#include "bldc_6step.h"
#include "hall.h"
#include "inverter.h"
#include "pwm.h"

static volatile uint8_t current_step = STEP_OFF;

static bldc_cal_state_t cal_state = CAL_NOT_PERFORMED;

/* Notes on different inverter switching modes:
 * 
 * Assuming 6-step control of a BLDC motor:
 * 
 * 1. Independent/Complementary Unipolar Switching (2 quadrant operation)
 *    (one half is complementary while the other independent, "active freewheeling")
 *    + Lower switching losses
 *    + Lower bus ripple
 *    - Short time for sampling with low duty
 *    - Nasty EMF waveform
 * 
 * 2. Complementary Bipolar Switching (4 quadrant operation)
 *    + 4 quadrant
 *    + Sampling is easy
 *    + Lower ripple for sensing EMF on open phase
 *    - Worse bus ripple
 *    - Higher losses
 *
 * 3. Complementary Unipolar Switching (4 quadrant operation)
 *    + 4 quadrant
 *    + Lower switching losses
 *    + Lowest bus ripple
 *    + Effectively twice the freq (if centered and symmetric duty)
 *    - Sampling not as trivial
 *
 * TODO: Maybe implement 2 and 3? 
 */

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

#if 0
/* Complementary Bipolar Switching experiment */
static void (* const commutation_steps_ch3[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch3_off,
  /* Normal steps */
  [STEP_1] = pwm_ch3_complementary_pos, 
  [STEP_2] = pwm_ch3_complementary_pos, 
  [STEP_3] = pwm_ch3_off,
  [STEP_4] = pwm_ch3_complementary_neg, 
  [STEP_5] = pwm_ch3_complementary_neg, 
  [STEP_6] = pwm_ch3_off
};

static void (* const commutation_steps_ch2[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch2_off,
  /* Normal steps */
  [STEP_1] = pwm_ch2_off,
  [STEP_2] = pwm_ch2_complementary_neg, 
  [STEP_3] = pwm_ch2_complementary_neg, 
  [STEP_4] = pwm_ch2_off,
  [STEP_5] = pwm_ch2_complementary_pos, 
  [STEP_6] = pwm_ch2_complementary_pos, 
};

static void (* const commutation_steps_ch1[NUMBER_OF_STEPS])(void) =
{
  /* Use index 0 for gate off */
  [STEP_OFF] = pwm_ch1_off,
  /* Normal steps */
  [STEP_1] = pwm_ch1_complementary_neg, 
  [STEP_2] = pwm_ch1_off,
  [STEP_3] = pwm_ch1_complementary_pos, 
  [STEP_4] = pwm_ch1_complementary_pos, 
  [STEP_5] = pwm_ch1_off,
  [STEP_6] = pwm_ch1_complementary_neg, 
};
#endif

static uint8_t hall_state_to_step_map[NUMBER_OF_DIRS][NUMBER_OF_HALL_STATES] =
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

void bldc6s_commutation(const uint8_t direction, const uint8_t current_hall_state)
{
  uint8_t next_step = hall_state_to_step_map[direction][current_hall_state];

  bldc6s_set_commutation_step(next_step);
}

inline void bldc6s_set_commutation_step(const uint8_t step)
{
  commutation_steps_ch1[step]();
  commutation_steps_ch2[step]();
  commutation_steps_ch3[step]();

  pwm_update_event();

  current_step = step;
}

uint8_t bldc6s_current_step(void)
{
  return current_step;
}

void bldc6s_hall_calibration_step(uint32_t period_ms)
{
  static int8_t step;
  static uint32_t delay_ms = 0u;
  const uint32_t rotation_delay_time_ms = 500u;
  const uint32_t probing_delay_time_ms = 500u;
  uint8_t hallstate;
  static float angle = 0.0f;

  switch (cal_state) {

    case CAL_NOT_PERFORMED:
      ivtr_idle_state();
      step = 6u;
      cal_state = CAL_PRE_ROTATION;
      break;

    case CAL_PRE_ROTATION:
      bldc6s_set_commutation_step(step);
      ivtr_request_duty_cycle(3.0f);
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
      bldc6s_set_commutation_step(step);
      ivtr_request_duty_cycle(3.0f);
      if (delay_ms < probing_delay_time_ms) {
        delay_ms += period_ms;
      } else {
        delay_ms = 0u;

        hallstate = (int8_t) hall_get_state();

        hall_map_state_to_angle(hallstate, angle);

        hall_state_to_step_map[DIR_CW][hallstate]  = cal_step_to_commutation_step[DIR_CW][step];
        hall_state_to_step_map[DIR_CCW][hallstate] = cal_step_to_commutation_step[DIR_CCW][step];

        if (step >= 6) {
          cal_state = CAL_CHECK;
        }

        step++;
        angle += 60.0f;
      }
      break;

    case CAL_CHECK:
      ivtr_request_duty_cycle(0.0f);
      hall_calculate_direction_map();
      cal_state = CAL_OK;
      break;

    case CAL_OK:
      ivtr_idle_state();
      break;

    default:
      break;
  }
}

bldc_cal_state_t bldc6s_cal_state(void)
{
  return cal_state;
}

int bldc6s_init(void)
{
#if 1
  uint8_t hall_state;
  for (hall_state = 0; hall_state < NUMBER_OF_HALL_STATES; hall_state++) {
    hall_state_to_step_map[DIR_NONE][hall_state] = STEP_OFF;
    hall_state_to_step_map[DIR_CW][hall_state]   = STEP_OFF;
    hall_state_to_step_map[DIR_CCW][hall_state]  = STEP_OFF;
  }
#endif
  return 0;
}
