/*
 * adc.h
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32f4xx_hal.h"

#define ADC_I_A_PIN         GPIO_PIN_1
#define ADC_I_A_PORT        GPIOC
#define ADC_I_A_GPIO_CLK_EN __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_A_CH          ADC_CHANNEL_11

#define ADC_I_B_PIN         GPIO_PIN_2
#define ADC_I_B_PORT        GPIOC
#define ADC_I_B_GPIO_CLK_EN __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_B_CH          ADC_CHANNEL_12

#define ADC_I_C_PIN         GPIO_PIN_3
#define ADC_I_C_PORT        GPIOC
#define ADC_I_C_GPIO_CLK_EN __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_C_CH          ADC_CHANNEL_13

#define ADC_EMF_A_PIN         GPIO_PIN_0
#define ADC_EMF_A_PORT        GPIOA
#define ADC_EMF_A_GPIO_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_A_CH          ADC_CHANNEL_0

#define ADC_EMF_B_PIN         GPIO_PIN_1
#define ADC_EMF_B_PORT        GPIOA
#define ADC_EMF_B_GPIO_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_B_CH          ADC_CHANNEL_1

#define ADC_EMF_C_PIN         GPIO_PIN_2
#define ADC_EMF_C_PORT        GPIOA
#define ADC_EMF_C_GPIO_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_C_CH          ADC_CHANNEL_2

#define ADC_INJECCONV_IRQ_PRIO  (0x00u)

int adc_init();

#endif /* ADC_H_ */
