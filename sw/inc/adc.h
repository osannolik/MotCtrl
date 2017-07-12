/*
 * adc.h
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32f4xx_hal.h"

#define ADC_VREF                    (3.3f)
#define ADC_VOLTAGE_PER_LSB         (ADC_VREF / 4095.0f) // 12 bit

#define ADC_REGCONV_SAMPLE_FREQ_HZ  (14000u)
#define ADC_REGCONV_PRESCALER       (4u)

#define ADC_I_A_PIN                 GPIO_PIN_1
#define ADC_I_A_PORT                GPIOC
#define ADC_I_A_GPIO_CLK_EN         __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_A_CH                  ADC_CHANNEL_11
#define ADC_I_A_RANK                (1u)

#define ADC_I_B_PIN                 GPIO_PIN_2
#define ADC_I_B_PORT                GPIOC
#define ADC_I_B_GPIO_CLK_EN         __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_B_CH                  ADC_CHANNEL_12
#define ADC_I_B_RANK                (1u)

#define ADC_I_C_PIN                 GPIO_PIN_3
#define ADC_I_C_PORT                GPIOC
#define ADC_I_C_GPIO_CLK_EN         __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_I_C_CH                  ADC_CHANNEL_13
#define ADC_I_C_RANK                (1u)

#define ADC_EMF_A_PIN               GPIO_PIN_0
#define ADC_EMF_A_PORT              GPIOA
#define ADC_EMF_A_GPIO_CLK_EN       __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_A_CH                ADC_CHANNEL_0
#define ADC_EMF_A_RANK              (2u)

#define ADC_EMF_B_PIN               GPIO_PIN_1
#define ADC_EMF_B_PORT              GPIOA
#define ADC_EMF_B_GPIO_CLK_EN       __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_B_CH                ADC_CHANNEL_1
#define ADC_EMF_B_RANK              (2u)

#define ADC_EMF_C_PIN               GPIO_PIN_2
#define ADC_EMF_C_PORT              GPIOA
#define ADC_EMF_C_GPIO_CLK_EN       __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_EMF_C_CH                ADC_CHANNEL_2
#define ADC_EMF_C_RANK              (2u)

#define ADC_BAT_SENSE_PIN           GPIO_PIN_0
#define ADC_BAT_SENSE_PORT          GPIOC
#define ADC_BAT_SENSE_GPIO_CLK_EN   __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_BAT_SENSE_CH            ADC_CHANNEL_10
#define ADC_BAT_SENSE_RANK          (1u)

#define ADC_BOARD_TEMP_PIN          GPIO_PIN_4
#define ADC_BOARD_TEMP_PORT         GPIOC
#define ADC_BOARD_TEMP_GPIO_CLK_EN  __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADC_BOARD_TEMP_CH           ADC_CHANNEL_14
#define ADC_BOARD_TEMP_RANK         (2u)

#define ADC_MCU_TEMP_RANK           (3u)

#define ADC_INJECTED_RANK_MAX       (4u)

#define ADC_NUMBER_OF_REGULAR_MEAS  (3u)
#define ADC_NUMBER_OF_INJECTED_ADC1 (2u)
#define ADC_NUMBER_OF_INJECTED_ADC2 (2u)
#define ADC_NUMBER_OF_INJECTED_ADC3 (2u)
#define ADC_NUMBER_OF_INJECTED_MEAS (ADC_NUMBER_OF_INJECTED_ADC1+ADC_NUMBER_OF_INJECTED_ADC2+ADC_NUMBER_OF_INJECTED_ADC3)
#define ADC_NUMBER_OF_MEAS          (ADC_NUMBER_OF_REGULAR_MEAS+ADC_NUMBER_OF_INJECTED_MEAS)

/* These are used as buffer indices.
 * Regular conversions need to be from 0 and upwards since the
 * DMA fills from index 0. Bad style, but convenient. */
typedef enum {
  // Regular channels
  ADC_BAT_SENSE = 0u,
  ADC_BOARD_TEMP,
  ADC_MCU_TEMP,
  // Injected channels
  ADC_I_A,
  ADC_I_B,
  ADC_I_C,
  ADC_EMF_A,
  ADC_EMF_B,
  ADC_EMF_C
} adc_measurement_t;

#define ADC_INJECCONV_IRQ_PRIO  (0x00u)

int adc_init();
float adc_get_measurement(adc_measurement_t m);
uint16_t adc_get_measurement_raw(adc_measurement_t m);
void adc_set_new_samples_indication_cb(void (* callback)(void));

#endif /* ADC_H_ */
