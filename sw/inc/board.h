/*
 * board.h
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "stm32f4xx_hal.h"

#define BOARD_GATE_DRIVER_PIN    GPIO_PIN_3
#define BOARD_GATE_DRIVER_PORT   GPIOA
#define BOARD_GATE_DRIVER_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()

#define BOARD_BUTTON_PIN      GPIO_PIN_2
#define BOARD_BUTTON_PORT     GPIOB
#define BOARD_BUTTON_CLK_EN   __HAL_RCC_GPIOB_CLK_ENABLE()

int board_init(void);
void board_gate_driver_enable(void);
void board_gate_driver_disable(void);
uint8_t board_button_pressed(void);

#endif /* BOARD_H_ */
