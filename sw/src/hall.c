/*
 * hall.c
 *
 *  Created on: 21 jan 2017
 *      Author: osannolik
 */

#include "hall.h"
#include "bldc.h"

#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(uint8_t, m_hall1, 0, "");
CALMEAS_SYMBOL(uint8_t, m_hall2, 0, "");
CALMEAS_SYMBOL(uint8_t, m_hall3, 0, "");

int hall_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_NOPULL;
  GPIOinitstruct.Mode = GPIO_MODE_IT_RISING_FALLING;

  HALL_SENSOR_H1_CLK_EN;
  HALL_SENSOR_H2_CLK_EN;
  HALL_SENSOR_H3_CLK_EN;

  GPIOinitstruct.Pin = HALL_SENSOR_H1_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H1_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = HALL_SENSOR_H2_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H2_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = HALL_SENSOR_H3_PIN;
  HAL_GPIO_Init(HALL_SENSOR_H3_PORT, &GPIOinitstruct);

  hall_start();

  return 0;
}

void hall_start(void)
{
  NVIC_SetPriority(EXTI9_5_IRQn, HALL_EXI_IRQ_PRIO);
  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void hall_individual_states(uint8_t *h1, uint8_t *h2, uint8_t *h3)
{
  *h1 = (uint8_t) ((HALL_SENSOR_H1_PORT->IDR & HALL_SENSOR_H1_PIN) == HALL_SENSOR_H1_PIN);
  *h2 = (uint8_t) ((HALL_SENSOR_H2_PORT->IDR & HALL_SENSOR_H2_PIN) == HALL_SENSOR_H2_PIN);
  *h3 = (uint8_t) ((HALL_SENSOR_H3_PORT->IDR & HALL_SENSOR_H3_PIN) == HALL_SENSOR_H3_PIN);
}

static inline uint8_t form_hall_state(uint8_t h1, uint8_t h2, uint8_t h3) {
  return ((h3 << 2) | (h2 << 1) | (h1));
}

uint8_t hall_state(void)
{
  hall_individual_states(&m_hall1, &m_hall2, &m_hall3);
  return form_hall_state(m_hall1, m_hall2, m_hall3);
}

void EXTI9_5_IRQHandler(void)
{
  if(__HAL_GPIO_EXTI_GET_IT(HALL_SENSOR_H1_PIN) != RESET) {
    __HAL_GPIO_EXTI_CLEAR_IT(HALL_SENSOR_H1_PIN);
  } else if(__HAL_GPIO_EXTI_GET_IT(HALL_SENSOR_H2_PIN) != RESET) {
    __HAL_GPIO_EXTI_CLEAR_IT(HALL_SENSOR_H2_PIN);
  } else if(__HAL_GPIO_EXTI_GET_IT(HALL_SENSOR_H3_PIN) != RESET) {
    __HAL_GPIO_EXTI_CLEAR_IT(HALL_SENSOR_H3_PIN);
  }

  bldc_hall_indication(hall_state());
}
