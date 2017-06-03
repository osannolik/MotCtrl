/*
 * hall.c
 *
 *  Created on: 26 maj 2017
 *      Author: osannolik
 */

#include "hall.h"
#include "utils.h"
#include "calmeas.h"

#define NUMBER_OF_COMMUTATIONS (6u)
#define ANGLE_UNDEFINED (-1.0f)


static hall_state_t hall_state;
static void (* commutation_indication_cb)(uint8_t hall_state) = NULL;
static TIM_HandleTypeDef hall_timer;
static float hall_state_to_angle_map_cw[NUMBER_OF_HALL_STATES];
static float hall_state_to_angle_map_ccw[NUMBER_OF_HALL_STATES];
static uint8_t hall_state_to_direction[NUMBER_OF_HALL_STATES][NUMBER_OF_HALL_STATES];
static float speed_timer_resolution_s;
static float speed_raw_per_commutation[NUMBER_OF_COMMUTATIONS];


/* Measurements */
CALMEAS_SYMBOL_BY_ADDRESS(uint8_t, m_hall_state_current, &(hall_state.current),  "");
CALMEAS_SYMBOL_BY_ADDRESS(uint8_t, m_hall_state_previous, &(hall_state.previous),  "");
CALMEAS_SYMBOL(uint8_t, m_hall_direction, DIR_NONE, "");
CALMEAS_SYMBOL(float,   m_hall_angle_est_rad, 0.0f, "");
CALMEAS_SYMBOL(float,   m_hall_speed_est_erpm, 0.0f, "");
CALMEAS_SYMBOL(float,   m_hall_speed_raw_erpm, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(float, p_commutation_delay, HALL_COMMUTATION_DELAY_PERC, "");


static uint8_t hall_state_update(hall_state_t * const state);
static uint8_t direction(hall_state_t * const state);
static void hall_commutation(hall_state_t * const state);
static float speed_raw_erpm(const uint32_t speed_timer_cnt, const uint8_t direction);
static void hall_speed_est_update(hall_state_t * const state, const uint32_t speed_timer_cnt);


int hall_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  __HAL_RCC_TIM4_CLK_ENABLE();

  HALL_SENSOR_H1_CLK_EN;
  HALL_SENSOR_H2_CLK_EN;
  HALL_SENSOR_H3_CLK_EN;

  GPIOinitstruct.Mode      = GPIO_MODE_AF_PP;
  GPIOinitstruct.Pull      = GPIO_NOPULL;
  GPIOinitstruct.Speed     = GPIO_SPEED_HIGH;
  GPIOinitstruct.Alternate = GPIO_AF2_TIM4;

  GPIOinitstruct.Pin = HALL_SENSOR_H1_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H1_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = HALL_SENSOR_H2_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H2_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = HALL_SENSOR_H3_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H3_PORT, &GPIOinitstruct);


  /* Initialize TIM4 peripheral as follows:
   *
   * - Hall sensor inputs are connected to Ch1, Ch2, and Ch3.
   * - TI1 is xor of all three channels.
   * - Input capture IC1 is configured to capture at both edges of TI1.
   * - TI1F_ED = TI1 is set to trigger a reset of the timer.
   * - OC2 is configured to create a pulse delayed from the TRC = TI1F_ED event.
   * - Interrupt at input capture and delayed pulse event.
   *
   * This way it is possible to measure the time between two consecutive
   * hall sensor changes and thus to estimate the speed of the motor.
   * Also, it is possible to trigger the commutation of the BLDC based on
   * the IC (or delayed pulse) interrupt.
   *
   * Configuration:
   *   APB1 is the clock source = 2*APB1 (2*45 MHz)
   *   Using a prescaler of 225 and using all 16 bits yields:
   *   - Resolution = 225 / 90 MHz = 2.5 us
   *   - Time until overflow = 2^16 * 225 / 90 MHz = 0.16384 s
   * This allows for a speed down to 61 rpm before an overflow occurs.
   * At 10000 rpm, the resolution will be approx 2.5 us * (10000^2)/10 = 25 rpm
   */

  const uint32_t prescaler = 225u;

  hall_timer.Instance               = TIM4;
  hall_timer.Init.Period            = 0xFFFF;
  hall_timer.Init.Prescaler         = prescaler - 1;
  hall_timer.Init.ClockDivision     = 0;
  hall_timer.Init.CounterMode       = TIM_COUNTERMODE_UP;
  hall_timer.Init.RepetitionCounter = 0;

  speed_timer_resolution_s = ((float) prescaler) / ((float) 2.0f * HAL_RCC_GetPCLK1Freq());

  TIM_HallSensor_InitTypeDef TIMHallSensorInithandle;
  TIMHallSensorInithandle.Commutation_Delay = 1u;
  TIMHallSensorInithandle.IC1Filter         = 0x0Fu;
  TIMHallSensorInithandle.IC1Polarity       = TIM_ICPOLARITY_BOTHEDGE;
  TIMHallSensorInithandle.IC1Prescaler      = TIM_ICPSC_DIV1;
  HAL_TIMEx_HallSensor_Init(&hall_timer, &TIMHallSensorInithandle);

  HAL_TIMEx_HallSensor_Start_IT(&hall_timer);
  //__HAL_TIM_ENABLE_IT(&hall_timer, TIM_IT_CC2);

  hall_timer.Instance->CR1 |= 0x4u; // Only counter overflow/underflow generates an update interrupt
  __HAL_TIM_ENABLE_IT(&hall_timer, TIM_IT_UPDATE);

  NVIC_SetPriority(TIM4_IRQn, HALL_IRQ_PRIO);
  NVIC_EnableIRQ(TIM4_IRQn);


  hall_state.current = 0;
  hall_state.previous = 0;

  uint8_t i, j;
  for (i=0; i<NUMBER_OF_HALL_STATES; i++) {
    for (j=0; j<NUMBER_OF_HALL_STATES; j++) {
      hall_state_to_direction[i][j] = DIR_NONE;
      hall_state_to_direction[i][j] = DIR_NONE;
    }

    hall_state_to_angle_map_cw[i] = ANGLE_UNDEFINED;
    hall_state_to_angle_map_ccw[i] = ANGLE_UNDEFINED;
  }

  for (i=0; i<NUMBER_OF_COMMUTATIONS; i++) {
    speed_raw_per_commutation[i] = 0.0f;
  }

  return 0;
}

void TIM4_IRQHandler(void)
{
  /* Input capture (xor of hall sensor):
   * Determine current position and calculate speed! */
  if (__HAL_TIM_GET_FLAG(&hall_timer, TIM_FLAG_CC1) && \
      __HAL_TIM_GET_IT_SOURCE(&hall_timer, TIM_IT_CC1)) {
    __HAL_TIM_CLEAR_IT(&hall_timer, TIM_IT_CC1);

    hall_state_update(&hall_state);

    /* Prepare for commutation. Assume we need to wait a factor of the latest hall period */
    const uint32_t speed_timer_cnt = HAL_TIM_ReadCapturedValue(&hall_timer, TIM_CHANNEL_1); // Also clears IF
    hall_timer.Instance->CCR2 = (uint32_t) (((float) speed_timer_cnt) * p_commutation_delay);

    hall_speed_est_update(&hall_state, speed_timer_cnt);
    hall_angle_est_reset_to(hall_state_to_angle_deg(&hall_state));

    m_hall_speed_est_erpm = hall_get_speed_est_erpm();
    m_hall_direction      = direction(&hall_state);

    __HAL_TIM_ENABLE_IT(&hall_timer, TIM_IT_CC2);
  }

  /* Output compare is creating a pulse delayed from the input capture event:
   * Trigger a BLDC commutation! */
  if(__HAL_TIM_GET_FLAG(&hall_timer, TIM_FLAG_CC2) && \
     __HAL_TIM_GET_IT_SOURCE(&hall_timer, TIM_IT_CC2)) {
    __HAL_TIM_CLEAR_IT(&hall_timer, TIM_IT_CC2);

    hall_commutation(&hall_state);

    __HAL_TIM_DISABLE_IT(&hall_timer, TIM_IT_CC2);
  }

  /* Input capture timer has overflowed, assume speed is zero */
  if(__HAL_TIM_GET_FLAG(&hall_timer, TIM_FLAG_UPDATE) && \
     __HAL_TIM_GET_IT_SOURCE(&hall_timer, TIM_IT_UPDATE)) {
    __HAL_TIM_CLEAR_IT(&hall_timer, TIM_FLAG_UPDATE);

    hall_speed_est_reset_to(0.0f);
    m_hall_speed_est_erpm = hall_get_speed_est_erpm();
  }
}

static uint8_t hall_state_update(hall_state_t * const state)
{
  const uint8_t h1 = ((HALL_SENSOR_H1_PORT->IDR & HALL_SENSOR_H1_PIN) == HALL_SENSOR_H1_PIN);
  const uint8_t h2 = ((HALL_SENSOR_H2_PORT->IDR & HALL_SENSOR_H2_PIN) == HALL_SENSOR_H2_PIN);
  const uint8_t h3 = ((HALL_SENSOR_H3_PORT->IDR & HALL_SENSOR_H3_PIN) == HALL_SENSOR_H3_PIN);

  state->previous = state->current;
  state->current = ((h3 << 2) | (h2 << 1) | (h1));

  return state->current;
}

uint8_t hall_get_state(void)
{
  return hall_state.current;
}

uint8_t hall_get_direction(void)
{
  return direction(&hall_state);
}

static uint8_t direction(hall_state_t * const state)
{
  return hall_state_to_direction[state->previous][state->current];
}

static void hall_commutation(hall_state_t * const state)
{
  if (commutation_indication_cb != NULL) {
    commutation_indication_cb(state->current);
  }
}

static float speed_raw_erpm(const uint32_t speed_timer_cnt, const uint8_t direction)
{
  if (speed_timer_cnt == 0.0f) {
    return 0.0f;
  }

  float speed_raw = 10.0f / ((float) speed_timer_cnt * speed_timer_resolution_s);

  if (direction == DIR_CCW) {
    speed_raw = -speed_raw;
  }

  return speed_raw;
}

static void hall_speed_est_update(hall_state_t * const state, const uint32_t speed_timer_cnt)
{
  const uint8_t dir = direction(state);
  static uint8_t c = 0;

  if (dir != DIR_NONE) {
    m_hall_speed_raw_erpm = speed_raw_erpm(speed_timer_cnt, dir);
  } else {
    m_hall_speed_raw_erpm = 0.0f;
  }

  speed_raw_per_commutation[c] = m_hall_speed_raw_erpm;

  if (++c >= NUMBER_OF_COMMUTATIONS) {
    c = 0;
  }
}

float hall_get_speed_raw_erpm(void)
{
  return m_hall_speed_raw_erpm;
}

float hall_get_speed_est_erpm(void)
{
  float speed_est = 0.0f;

  uint8_t c;
  for (c=0; c<NUMBER_OF_COMMUTATIONS; c++) {
    speed_est += speed_raw_per_commutation[c];
  }

  return speed_est/((float) NUMBER_OF_COMMUTATIONS);
}

void hall_speed_est_reset_to(const float speed_0)
{
  uint8_t c = 0;
  for (c=0; c<NUMBER_OF_COMMUTATIONS; c++) {
    speed_raw_per_commutation[c] = speed_0;
  }
}

float hall_state_to_angle_deg(hall_state_t * const state) 
{
  float angle_raw;

  switch (direction(state)) {
    case DIR_CW:
      angle_raw = hall_state_to_angle_map_cw[state->current];
      break;

    case DIR_CCW:
      angle_raw = hall_state_to_angle_map_ccw[state->current];
      break;

    case DIR_NONE:
      angle_raw = 0.0f;
      break;

    default:
      angle_raw = 0.0f;
      break;
  }

  return angle_raw;
}

float hall_angle_est_update(const float period_s)
{
  m_hall_angle_est_rad += RPM_TO_RADPS(hall_get_speed_est_erpm()) * period_s;

  return m_hall_angle_est_rad;
}

float hall_get_angle_est_rad(void)
{
  return m_hall_angle_est_rad;
}

void hall_angle_est_reset_to(const float angle_0)
{
  m_hall_angle_est_rad = angle_0;
}

void hall_set_commutation_indication_cb(void (* callback)(uint8_t))
{
  commutation_indication_cb = callback;
}

void hall_calculate_direction_map(void)
{
  /* An increasing angle is defined as clockwise direction */

  float b;
  uint8_t h, h_next;

  for (h=0; h<NUMBER_OF_HALL_STATES; h++) {

    b = hall_state_to_angle_map_ccw[h];

    if (0.0f<=b && b<360.0f) {
      for (h_next=0; h_next<NUMBER_OF_HALL_STATES; h_next++) {
        if (hall_state_to_angle_map_cw[h_next] == b) {
          hall_state_to_direction[h][h_next] = DIR_CW;
          hall_state_to_direction[h_next][h] = DIR_CCW;
        }
      }
    }

  }
}

void hall_map_state_to_angle(const uint8_t hall_state, const float angle)
{
  if (hall_state < NUMBER_OF_HALL_STATES) {
    hall_state_to_angle_map_cw[hall_state]  = wrap_to_range_f(0.0f, 360.0f, angle + 0.0f);
    hall_state_to_angle_map_ccw[hall_state] = wrap_to_range_f(0.0f, 360.0f, angle + 60.0f);
  }
}
