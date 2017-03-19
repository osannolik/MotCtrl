/*
 * position.c
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#include "position.h"
#include "hall.h"

#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(uint8_t, m_pos_hall1, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hall2, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hall3, 0, "");
CALMEAS_SYMBOL(uint8_t, m_pos_hallstate, 0, "");
CALMEAS_SYMBOL(uint32_t, m_pos_speed_timer, 0, "");

/* Parameters */
CALMEAS_SYMBOL(float, p_commutation_delay, POS_HALL_COMMUTATION_DELAY_PERC, "");


static void (* commutation_indication_cb)(uint8_t hall_state) = NULL;
static float hall_state_to_angle_map_cw[POS_NUMBER_OF_HALL_STATES] = {0.0f};
static float hall_state_to_angle_map_ccw[POS_NUMBER_OF_HALL_STATES] = {0.0f};
static TIM_HandleTypeDef TIMhandle;

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
   *   - Resolution = 225 * 90 MHz = 2.5 us
   *   - Time until overflow = 2^16 * 225 * 90 MHz = 0.16384 s
   * This allows for a speed down to 61 rpm before an overflow occurs.
   * At 10000 rpm, the resolution will be approx 2.5 us * (10000^2)/10 = 25 rpm
   */

  TIMhandle.Instance               = TIM4;
  TIMhandle.Init.Period            = 0xFFFF;
  TIMhandle.Init.Prescaler         = 225-1;
  TIMhandle.Init.ClockDivision     = 0;
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIMhandle.Init.RepetitionCounter = 0;

  TIM_HallSensor_InitTypeDef TIMHallSensorInithandle;
  TIMHallSensorInithandle.Commutation_Delay = 1u;
  TIMHallSensorInithandle.IC1Filter         = 0x0Fu;
  TIMHallSensorInithandle.IC1Polarity       = TIM_ICPOLARITY_BOTHEDGE;
  TIMHallSensorInithandle.IC1Prescaler      = TIM_ICPSC_DIV1;
  HAL_TIMEx_HallSensor_Init(&TIMhandle, &TIMHallSensorInithandle);

  HAL_TIMEx_HallSensor_Start_IT(&TIMhandle);
  __HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_CC2);
  //__HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_UPDATE);

  NVIC_SetPriority(TIM4_IRQn, HALL_EXI_IRQ_PRIO);
  NVIC_EnableIRQ(TIM4_IRQn);

  return 0;
}

#include "debug.h"
#include "ext.h"


#if 0
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  DBG_PAD1_TOGGLE;
}
#endif

static void hall_commutation(void)
{
  DBG_PAD2_TOGGLE;

  m_pos_hallstate = position_get_hall_state();

  if (m_pos_hallstate == 1u) {
    //DBG_PAD2_SET;
  }

  if (commutation_indication_cb != NULL) {
    commutation_indication_cb(m_pos_hallstate);
  }
}

static void hall_period_update(TIM_HandleTypeDef *htim)
{
  DBG_PAD3_TOGGLE;

  m_pos_speed_timer = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // Also clears IF

  htim->Instance->CCR2 = (uint32_t) (((float) m_pos_speed_timer) * p_commutation_delay);
}

void TIM4_IRQHandler(void)
{
  /* Input capture (xor of hall sensor):
   * Calculate speed! */
  if (__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC1) && \
      __HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_CC1)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC1);

    hall_period_update(&TIMhandle);
  }

  /* Output compare is creating a pulse delayed from the input capture event:
   * Trigger a BLDC commutation! */
  if(__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC2) && \
     __HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_CC2)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC2);

    hall_commutation();
  }
}

void position_set_hall_commutation_indication_cb(void (* callback)(uint8_t))
{
  commutation_indication_cb = callback;
}

void position_hall_individual_states(uint8_t *h1, uint8_t *h2, uint8_t *h3)
{
  *h1 = (uint8_t) ((HALL_SENSOR_H1_PORT->IDR & HALL_SENSOR_H1_PIN) == HALL_SENSOR_H1_PIN);
  *h2 = (uint8_t) ((HALL_SENSOR_H2_PORT->IDR & HALL_SENSOR_H2_PIN) == HALL_SENSOR_H2_PIN);
  *h3 = (uint8_t) ((HALL_SENSOR_H3_PORT->IDR & HALL_SENSOR_H3_PIN) == HALL_SENSOR_H3_PIN);
}

static inline uint8_t form_hall_state(uint8_t h1, uint8_t h2, uint8_t h3) {
  return ((h3 << 2) | (h2 << 1) | (h1));
}

uint8_t position_get_hall_state(void)
{
  position_hall_individual_states(&m_pos_hall1, &m_pos_hall2, &m_pos_hall3);
  return form_hall_state(m_pos_hall1, m_pos_hall2, m_pos_hall3);
}

void position_map_hall_state_to_angle(uint8_t hall_state, float angle)
{
  if (hall_state < POS_NUMBER_OF_HALL_STATES) {
    hall_state_to_angle_map_cw[hall_state]  = angle + 30.0f;
    hall_state_to_angle_map_ccw[hall_state] = angle - 30.0f;
  }
}

float position_get_angle_est(void)
{
  return hall_state_to_angle_map_cw[position_get_hall_state()];
}

float position_get_speed_est(void)
{
  return 0.0;
}
