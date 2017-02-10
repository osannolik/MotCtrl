/*
 * board.c
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#include "board.h"

int board_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  BOARD_GATE_DRIVER_CLK_EN;
  BOARD_BUTTON_CLK_EN;

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull  = GPIO_NOPULL;
  GPIOinitstruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIOinitstruct.Pin   = BOARD_GATE_DRIVER_PIN;

  HAL_GPIO_Init(BOARD_GATE_DRIVER_PORT, &GPIOinitstruct);

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull  = GPIO_PULLUP;
  GPIOinitstruct.Mode  = GPIO_MODE_INPUT;
  GPIOinitstruct.Pin   = BOARD_BUTTON_PIN;

  HAL_GPIO_Init(BOARD_BUTTON_PORT, &GPIOinitstruct);

  board_gate_driver_disable();

  return 0;
}

void board_gate_driver_enable(void)
{
  HAL_GPIO_WritePin(BOARD_GATE_DRIVER_PORT, BOARD_GATE_DRIVER_PIN, GPIO_PIN_SET);
}

void board_gate_driver_disable(void)
{
  HAL_GPIO_WritePin(BOARD_GATE_DRIVER_PORT, BOARD_GATE_DRIVER_PIN, GPIO_PIN_RESET);
}

uint8_t board_button_pressed(void)
{
  return (uint8_t) (GPIO_PIN_RESET == HAL_GPIO_ReadPin(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN));
}
