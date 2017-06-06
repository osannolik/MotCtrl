/*
 * encoder.h
 *
 *  Created on: 26 maj 2017
 *      Author: osannolik
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "stm32f4xx_hal.h"
#include "utils.h"


#define ENCODER_A_PIN        GPIO_PIN_6
#define ENCODER_A_PORT       GPIOB
#define ENCODER_A_CLK_EN     __HAL_RCC_GPIOB_CLK_ENABLE()

#define ENCODER_B_PIN        GPIO_PIN_7
#define ENCODER_B_PORT       GPIOB
#define ENCODER_B_CLK_EN     __HAL_RCC_GPIOB_CLK_ENABLE()

#define ENCODER_INDEX_PIN    GPIO_PIN_8
#define ENCODER_INDEX_PORT   GPIOB
#define ENCODER_INDEX_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define ENCODER_IRQ_PRIO     (0x03u)

#define ENCODER_PPR          (2048)


int encoder_init(void);
float encoder_get_angle_rad(void);
float encoder_get_angle_deg(void);
void encoder_set_angle_rad(const float angle0);
void encoder_set_angle_deg(const float angle0);
uint32_t encoder_get_counter(void);
void encoder_set_counter(const uint32_t cnt0);
float encoder_get_speed_raw_radps(void);
float encoder_get_direction(void);

#endif /* ENCODER_H_ */
