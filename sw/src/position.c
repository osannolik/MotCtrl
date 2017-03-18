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


  TIMhandle.Instance = TIM4;

  /* Initialize TIMx peripheral as follows:
   *   APB1 is the clock source
   *   90000000/(1/((2^-9)/1000)) = 176*/

  //period = 2*HAL_RCC_GetPCLK1Freq()/PWM_FREQUENCY_HZ - 1; /* TIM1 clock source 2*APB1 (2*45 MHz), since APB2 presc != 1 */

  TIMhandle.Init.Period            = 0xFFFF;
  TIMhandle.Init.Prescaler         = 176;
  TIMhandle.Init.ClockDivision     = 0;
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIMhandle.Init.RepetitionCounter = 0;
  HAL_TIM_IC_Init(&TIMhandle);

  HAL_TIM_ConfigTI1Input(&TIMhandle, TIM_TI1SELECTION_XORCOMBINATION);

  TIM_IC_InitTypeDef ICInithandle;
  ICInithandle.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE;
  ICInithandle.ICSelection = TIM_ICSELECTION_DIRECTTI;
  ICInithandle.ICPrescaler = TIM_ICPSC_DIV1;
  ICInithandle.ICFilter    = 0;
  HAL_TIM_IC_ConfigChannel(&TIMhandle, &ICInithandle, TIM_CHANNEL_1);

#if 0
  // Automatic reset does not seem to work?
  TIM_SlaveConfigTypeDef TIMSlaveConfighandle;
  TIMSlaveConfighandle.InputTrigger     = TIM_SLAVEMODE_RESET;
  TIMSlaveConfighandle.SlaveMode        = TIM_TS_TI1F_ED;
  TIMSlaveConfighandle.TriggerFilter    = 0u;
  TIMSlaveConfighandle.TriggerPolarity  = TIM_TRIGGERPOLARITY_BOTHEDGE;
  TIMSlaveConfighandle.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  HAL_TIM_SlaveConfigSynchronization(&TIMhandle, &TIMSlaveConfighandle);
#endif

  HAL_TIM_IC_Start_IT(&TIMhandle, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&TIMhandle);


  NVIC_SetPriority(TIM4_IRQn, HALL_EXI_IRQ_PRIO);
  NVIC_EnableIRQ(TIM4_IRQn);

  return 0;
}

#include "debug.h"

void TIM4_IRQHandler(void)
{
  /* xor of hall sensors changed */
  if (__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC1)) {

    m_pos_speed_timer = HAL_TIM_ReadCapturedValue(&TIMhandle, TIM_CHANNEL_1);
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC1);

    m_pos_hallstate = position_get_hall_state();

    if (m_pos_hallstate == 1u) {
      DBG_PAD2_SET;
    }

    if (commutation_indication_cb != NULL) {
      commutation_indication_cb(m_pos_hallstate);
    }

    DBG_PAD2_RESET;
  }

  /* Speed timer overflow */
  if (__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_UPDATE)) {
    __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_UPDATE);

    DBG_PAD3_TOGGLE;
  }

  HAL_TIM_IRQHandler(&TIMhandle);
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
