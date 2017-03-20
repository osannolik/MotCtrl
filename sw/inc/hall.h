/*
 * hall.h
 *
 *  Created on: 21 jan 2017
 *      Author: osannolik
 */

#ifndef HALL_H_
#define HALL_H_

#include "stm32f4xx_hal.h"

#define HALL_SENSOR_H1_PIN    GPIO_PIN_6
#define HALL_SENSOR_H1_PORT   GPIOB
#define HALL_SENSOR_H1_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_SENSOR_H2_PIN    GPIO_PIN_7
#define HALL_SENSOR_H2_PORT   GPIOB
#define HALL_SENSOR_H2_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_SENSOR_H3_PIN    GPIO_PIN_8
#define HALL_SENSOR_H3_PORT   GPIOB
#define HALL_SENSOR_H3_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_IRQ_PRIO         (0x03u)

int hall_init(void);
void hall_individual_states(uint8_t *h1, uint8_t *h2, uint8_t *h3);
void hall_start(void);
uint8_t hall_state(void);
void hall_set_commutation_indication_cb(void (* callback)(uint8_t));

#endif /* HALL_H_ */
