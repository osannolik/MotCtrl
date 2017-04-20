/*
 * position.c
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#include "position.h"
#include "hall.h"

#include "calmeas.h"

#define NUMBER_OF_COMMUTATIONS (6u)
#define ANGLE_UNDEFINED (-1.0f)
 
/* Measurements */
CALMEAS_SYMBOL(uint8_t, m_pos_hall1, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hall2, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hall3, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hallstate, 0, "");
CALMEAS_SYMBOL(uint32_t, m_pos_speed_timer, 0, "");
#define CALMEAS_TYPECODE_pos_direction_t   CALMEAS_TYPECODE_uint8_t
#define CALMEAS_MEMSEC_pos_direction_t     CALMEAS_MEMSEC_uint8_t
CALMEAS_SYMBOL(pos_direction_t, m_pos_direction, DIR_NONE, "");
CALMEAS_SYMBOL(float, m_pos_angle_est_deg, 0.0f, "");
CALMEAS_SYMBOL(float, m_pos_speed_raw_erpm, 0.0f, "");
CALMEAS_SYMBOL(float, m_pos_speed_est_erpm, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(float, p_commutation_delay, POS_HALL_COMMUTATION_DELAY_PERC, "");


static void (* commutation_indication_cb)(uint8_t hall_state) = NULL;
static float hall_state_to_angle_map_cw[POS_NUMBER_OF_HALL_STATES];
static float hall_state_to_angle_map_ccw[POS_NUMBER_OF_HALL_STATES];
static pos_direction_t hall_state_to_direction[POS_NUMBER_OF_HALL_STATES][POS_NUMBER_OF_HALL_STATES];
static TIM_HandleTypeDef TIMhandle;
static uint8_t hallstate_prev = DIR_NONE;
static float speed_timer_resolution_s;
static float speed_raw_per_commutation[NUMBER_OF_COMMUTATIONS];

int position_init(void)
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

  TIMhandle.Instance               = TIM4;
  TIMhandle.Init.Period            = 0xFFFF;
  TIMhandle.Init.Prescaler         = prescaler - 1;
  TIMhandle.Init.ClockDivision     = 0;
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIMhandle.Init.RepetitionCounter = 0;

  speed_timer_resolution_s = ((float) prescaler) / ((float) 2.0f * HAL_RCC_GetPCLK1Freq());

  TIM_HallSensor_InitTypeDef TIMHallSensorInithandle;
  TIMHallSensorInithandle.Commutation_Delay = 1u;
  TIMHallSensorInithandle.IC1Filter         = 0x0Fu;
  TIMHallSensorInithandle.IC1Polarity       = TIM_ICPOLARITY_BOTHEDGE;
  TIMHallSensorInithandle.IC1Prescaler      = TIM_ICPSC_DIV1;
  HAL_TIMEx_HallSensor_Init(&TIMhandle, &TIMHallSensorInithandle);

  HAL_TIMEx_HallSensor_Start_IT(&TIMhandle);
  //__HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_CC2);

  TIMhandle.Instance->CR1 |= 0x4u; // Only counter overflow/underflow generates an update interrupt
  __HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_UPDATE);

  NVIC_SetPriority(TIM4_IRQn, HALL_IRQ_PRIO);
  NVIC_EnableIRQ(TIM4_IRQn);

  uint8_t i, j;
  for (i=0; i<POS_NUMBER_OF_HALL_STATES; i++) {
    for (j=0; j<POS_NUMBER_OF_HALL_STATES; j++) {
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

static void hall_commutation(void)
{
  if (commutation_indication_cb != NULL) {
    commutation_indication_cb(m_pos_hallstate);
  }
}

static float speed_raw_erpm(const uint32_t speed_timer, const pos_direction_t direction)
{
  if (speed_timer == 0.0f) {
    return 0.0f;
  }

  float speed_raw = 10.0f / ((float) speed_timer * speed_timer_resolution_s);

  if (direction == DIR_CCW) {
    speed_raw = -speed_raw;
  }

  return speed_raw;
}

static float angle_raw_deg(const uint8_t hall_state, const pos_direction_t direction) {
  float angle_raw = 0.0f;

  switch (direction) {
    case DIR_CW:
      angle_raw = hall_state_to_angle_map_cw[hall_state];
      break;

    case DIR_CCW:
      angle_raw = hall_state_to_angle_map_ccw[hall_state];
      break;
    case DIR_NONE:
      break;

    default:
      break;
  }

  return angle_raw;
}

static void angle_and_speed_update(TIM_HandleTypeDef * const htim)
{
  /* Prepare for commutation. Assume we need to wait a factor of the latest hall period */
  m_pos_speed_timer = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // Also clears IF
  htim->Instance->CCR2 = (uint32_t) (((float) m_pos_speed_timer) * p_commutation_delay);

  hallstate_prev = m_pos_hallstate;
  m_pos_hallstate = position_get_hall_state();

  m_pos_direction = position_get_direction();

  if (m_pos_direction != DIR_NONE) {
    m_pos_speed_raw_erpm = speed_raw_erpm(m_pos_speed_timer, m_pos_direction);
    position_angle_est_reset_to(angle_raw_deg(m_pos_hallstate, m_pos_direction));
  } else {
    m_pos_speed_raw_erpm = 0.0f;
  }

  position_speed_est_update(m_pos_speed_raw_erpm);

  m_pos_speed_est_erpm = position_get_speed_est_erpm();
}

void position_speed_est_reset(void)
{
  uint8_t c = 0;
  for (c=0; c<NUMBER_OF_COMMUTATIONS; c++) {
    speed_raw_per_commutation[c] = 0.0f;
  }
}

void position_speed_est_update(const float speed_raw)
{
  static uint8_t c = 0;

  speed_raw_per_commutation[c] = speed_raw;

  if (++c >= NUMBER_OF_COMMUTATIONS) {
    c = 0;
  }
}

void position_angle_est_reset_to(const float angle_0)
{
  m_pos_angle_est_deg = angle_0;
}

float position_angle_est_update(const float period_s)
{
  m_pos_angle_est_deg += position_get_speed_est_erpm() * RPM_TO_DEGPS * period_s ;

  return m_pos_angle_est_deg;
}

float position_get_angle_est_deg(void)
{
  return m_pos_angle_est_deg;
}

float position_get_speed_est_erpm(void)
{
  float speed_est = 0.0f;

  uint8_t c;
  for (c=0; c<NUMBER_OF_COMMUTATIONS; c++) {
    speed_est += speed_raw_per_commutation[c];
  }

  return speed_est/((float) NUMBER_OF_COMMUTATIONS);
}

void TIM4_IRQHandler(void)
{
  /* Input capture (xor of hall sensor):
   * Determine current position and calculate speed! */
  if (__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC1) && \
      __HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_CC1)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC1);

    angle_and_speed_update(&TIMhandle);

    __HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_CC2);
  }

  /* Output compare is creating a pulse delayed from the input capture event:
   * Trigger a BLDC commutation! */
  if(__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC2) && \
     __HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_CC2)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC2);

    hall_commutation();

    __HAL_TIM_DISABLE_IT(&TIMhandle, TIM_IT_CC2);
  }

  /* Input capture timer has overflowed, assume speed is zero */
  if(__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_UPDATE) && \
     __HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_UPDATE)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_FLAG_UPDATE);

    position_speed_est_reset();
    m_pos_speed_est_erpm = position_get_speed_est_erpm();
    m_pos_speed_raw_erpm = 0.0f;
  }
}

void position_set_hall_commutation_indication_cb(void (* callback)(uint8_t))
{
  commutation_indication_cb = callback;
}

void position_hall_individual_states(uint8_t * const h1, uint8_t * const h2, uint8_t * const h3)
{
  *h1 = (uint8_t) ((HALL_SENSOR_H1_PORT->IDR & HALL_SENSOR_H1_PIN) == HALL_SENSOR_H1_PIN);
  *h2 = (uint8_t) ((HALL_SENSOR_H2_PORT->IDR & HALL_SENSOR_H2_PIN) == HALL_SENSOR_H2_PIN);
  *h3 = (uint8_t) ((HALL_SENSOR_H3_PORT->IDR & HALL_SENSOR_H3_PIN) == HALL_SENSOR_H3_PIN);
}

static inline uint8_t form_hall_state(const uint8_t h1, const uint8_t h2, const uint8_t h3) {
  return ((h3 << 2) | (h2 << 1) | (h1));
}

uint8_t position_get_hall_state(void)
{
  position_hall_individual_states(&m_pos_hall1, &m_pos_hall2, &m_pos_hall3);
  return form_hall_state(m_pos_hall1, m_pos_hall2, m_pos_hall3);
}

float wrap_to_range(const float low, const float high, float x)
{
  /* Wrap x into interval [low, high) */
  /* Assumes high > low */

  const float range = high - low;

  if (range > 0.0f) {
    while (x >= high) {
      x -= range;
    }

    while (x < low) {
      x += range;
    }
  } else {
    x = low;
  }

  return x;
}

void position_calculate_direction_map(void)
{
  /* An increasing angle is defined as clockwise direction */

  float b;
  uint8_t h, h_next;

  for (h=0; h<POS_NUMBER_OF_HALL_STATES; h++) {

    b = hall_state_to_angle_map_ccw[h];

    if (0.0f<=b && b<360.0f) {
      for (h_next=0; h_next<POS_NUMBER_OF_HALL_STATES; h_next++) {
        if (hall_state_to_angle_map_cw[h_next] == b) {
          hall_state_to_direction[h][h_next] = DIR_CW;
          hall_state_to_direction[h_next][h] = DIR_CCW;
        }
      }
    }

  }
}

void position_map_hall_state_to_angle(const uint8_t hall_state, const float angle)
{
  if (hall_state < POS_NUMBER_OF_HALL_STATES) {
    hall_state_to_angle_map_cw[hall_state]  = wrap_to_range(0.0f, 360.0f, angle + 0.0f);
    hall_state_to_angle_map_ccw[hall_state] = wrap_to_range(0.0f, 360.0f, angle + 60.0f);
  }
}

pos_direction_t position_get_direction(void)
{
  return hall_state_to_direction[hallstate_prev][m_pos_hallstate];
}
