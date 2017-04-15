/*
 * board.h
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "stm32f4xx_hal.h"

#define BOARD_GATE_DRIVER_PIN     GPIO_PIN_3
#define BOARD_GATE_DRIVER_PORT    GPIOA
#define BOARD_GATE_DRIVER_CLK_EN  __HAL_RCC_GPIOA_CLK_ENABLE()

#define BOARD_BUTTON_PIN          GPIO_PIN_2
#define BOARD_BUTTON_PORT         GPIOB
#define BOARD_BUTTON_CLK_EN       __HAL_RCC_GPIOB_CLK_ENABLE()

#define BOARD_V25                 (0.76f)   // MCU temp sensor: voltage at 25 degC
#define BOARD_AVG_SLOPE           (2.5e-3f) // MCU temp sensor: volts per degC

#define BOARD_R1                  (20000.0f)
#define BOARD_R2                  (1800.0f)
#define BOARD_BAT_SENSE_DIVIDER   ((BOARD_R2+BOARD_R1)/BOARD_R2) //approx 40/3.3
#define BOARD_R3                  (10000.0f)
#define BOARD_NTC                 (10000.0f)
#define BOARD_NTC_BETA            (3434.0f)

int board_init(void);
void board_step(const uint32_t period_ms);
void board_gate_driver_enable(void);
void board_gate_driver_disable(void);
uint8_t board_button_pressed(void);
float board_bat_voltage(void);
float board_temp_degC(void);
float board_mcu_temp_degC(void);

#endif /* BOARD_H_ */
