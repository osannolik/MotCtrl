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
#include "debug.h"
#include "utils.h"
#include "calmeas.h"

#define I_HIST_LEN (200)

/* Measurements */
CALMEAS_SYMBOL(float,   m_bldc6s_duty_set, 0, "");
CALMEAS_SYMBOL(float,   m_bldc6s_duty_req, 0, "");
CALMEAS_SYMBOL(uint8_t, m_bldc6s_direction_req, DIR_NONE, "");
CALMEAS_SYMBOL(float,   m_bldc6s_current_ctrl_diff, 0.0f, "");
CALMEAS_SYMBOL(float,   m_bldc6s_i_tot, 0.0f, "");
CALMEAS_SYMBOL(float,   m_bldc6s_i_tot_avg, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(float,    p_bldc6s_sample_trigger, -3.0f /* 0.5f */, "");  // Offset to compensate for sample delay
CALMEAS_SYMBOL(uint32_t, p_bldc6s_i_tot_avg_len, 10, "");
CALMEAS_SYMBOL(uint8_t,  p_bldc6s_current_ctrl_enable, 0, "");
CALMEAS_SYMBOL(float,    p_bldc6s_current_ctrl_set_point_A, 0.0f, "");
CALMEAS_SYMBOL(float,    p_bldc6s_current_ctrl_gain, 0.0f, "");
CALMEAS_SYMBOL(float,    p_bldc6s_current_ctrl_max_delta, 0.0f, "");
CALMEAS_SYMBOL(float,    p_bldc6s_current_ctrl_max, 0.0f, "");
CALMEAS_SYMBOL(float,    p_bldc6s_duty_rate_max, 0.1f, "");


static rate_limit_t rlim_duty;
static volatile uint8_t current_step = STEP_OFF;
static bldc_cal_state_t cal_state = CAL_NOT_PERFORMED;
static uint8_t hall_commutation_enabled = false;

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

void bldc6s_allow_hall_commutation(const uint8_t onoff)
{
  hall_commutation_enabled = onoff;
}

void bldc6s_hall_commutation(uint8_t current_hall_state)
{
#if 1
  DBG_PAD1_TOGGLE;

  if (current_hall_state == 1u) {
    DBG_PAD2_SET;
    DBG_PAD2_RESET;
  }
#endif

  if (bldc6s_cal_state() == CAL_OK && hall_commutation_enabled == true) {
    bldc6s_commutation(m_bldc6s_direction_req, current_hall_state);
  }
}

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

static void direction_commutation(const float duty_req)
{
  static uint8_t prev_direction = DIR_NONE;

  uint8_t direction;

  if (duty_req > 0.0f) {
    direction = DIR_CW;
  } else if (duty_req < 0.0f) {
    direction = DIR_CCW;
  } else {
    direction = DIR_NONE;
  }

  if (prev_direction != direction) {
    m_bldc6s_direction_req = direction;
    bldc6s_commutation(direction, hall_get_state());
  }

  prev_direction = direction;
}

void bldc6s_request_duty_cycle(const float duty)
{
  if (bldc6s_cal_state() == CAL_OK) {
    m_bldc6s_duty_req = saturatef(duty, -100.0f, 100.0f);
  }
}

void bldc6s_period_by_period_handler(const float dt,
                                     float i_a, float i_b, float i_c,
                                     float emf_a, float emf_b, float emf_c)
{
  float i_tot;

  switch (current_step) {
    case STEP_1:
      i_tot = (i_c - i_a)*0.5f;
      break;
    case STEP_2:
      i_tot = (i_c - i_b)*0.5f;
      break;
    case STEP_3:
      i_tot = (i_a - i_b)*0.5f;
      break;
    case STEP_4:
      i_tot = (i_a - i_c)*0.5f;
      break;
    case STEP_5:
      i_tot = (i_b - i_c)*0.5f;
      break;
    case STEP_6:
      i_tot = (i_b - i_a)*0.5f;
      break;
    default:
      i_tot = 0.0f;
      break;
  }

  i_tot = MAX(i_tot, 0.0f);

  static float i_tot_hist[I_HIST_LEN];
  static uint32_t i_index = 0;

  i_tot_hist[i_index++] = i_tot;

  if (i_index >= p_bldc6s_i_tot_avg_len) {
    i_index = 0;
  }

  float i_tot_avg = 0;
  uint32_t i;
  for (i=0; i<p_bldc6s_i_tot_avg_len; i++) {
    i_tot_avg += i_tot_hist[i];
  }
  i_tot_avg = i_tot_avg/((float) p_bldc6s_i_tot_avg_len);

  float u_delta = 0;
  float tmp;

  if (p_bldc6s_current_ctrl_enable != 0) {
    m_bldc6s_current_ctrl_diff = p_bldc6s_current_ctrl_set_point_A - i_tot_avg;
    u_delta = p_bldc6s_current_ctrl_gain * m_bldc6s_current_ctrl_diff;

    u_delta = saturatef(u_delta, -p_bldc6s_current_ctrl_max_delta, p_bldc6s_current_ctrl_max_delta);

    tmp = saturatef(m_bldc6s_duty_set+u_delta, 5.0f, p_bldc6s_current_ctrl_max);
  } else {
    tmp = m_bldc6s_duty_req;
  }

  m_bldc6s_duty_set = rate_limit(&rlim_duty, tmp, -p_bldc6s_duty_rate_max, p_bldc6s_duty_rate_max);

  direction_commutation(m_bldc6s_duty_set);

  pwm_set_duty_gate_abc_perc(ABS(m_bldc6s_duty_set));

  pwm_set_sample_trigger_perc(p_bldc6s_sample_trigger);
  //pwm_set_sample_trigger_perc(p_bldc6s_sample_trigger * m_bldc6s_duty_set);

  m_bldc6s_i_tot = i_tot;
  m_bldc6s_i_tot_avg = i_tot_avg;
}

void bldc6s_hall_calibration_step(uint32_t period_ms)
{
  static uint8_t step;
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
      m_bldc6s_duty_req = 3.0f;
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
      m_bldc6s_duty_req = 3.0f;
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
      m_bldc6s_duty_req = 0.0f;
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

  rate_limit_reset(&rlim_duty, 0.0f);

  return 0;
}
