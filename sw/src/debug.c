/*
 * debug.c
 *
 *  Created on: 23 aug 2015
 *      Author: osannolik
 */

#include "debug.h"

int debug_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_NOPULL;
  GPIOinitstruct.Mode = GPIO_MODE_OUTPUT_PP;

#if DBG_USE_LED
  DBG_LED_CLK_EN;
  GPIOinitstruct.Pin = DBG_LED_PIN;
  HAL_GPIO_Init(DBG_LED_PORT, &GPIOinitstruct);
  DBG_LED_RESET;
#endif /* DBG_USE_LED */

#if DBG_USE_PAD1
  DBG_PAD1_CLK_EN;
  GPIOinitstruct.Pin = DBG_PAD1_PIN;
  HAL_GPIO_Init(DBG_PAD1_PORT, &GPIOinitstruct);
  DBG_PAD1_RESET;
#endif /* DBG_USE_PAD1 */

#if DBG_USE_PAD2
  DBG_PAD2_CLK_EN;
  GPIOinitstruct.Pin = DBG_PAD2_PIN;
  HAL_GPIO_Init(DBG_PAD2_PORT, &GPIOinitstruct);
  DBG_PAD2_RESET;
#endif /* DBG_USE_PAD2 */

#if DBG_USE_PAD3
  DBG_PAD3_CLK_EN;
  GPIOinitstruct.Pin = DBG_PAD3_PIN;
  HAL_GPIO_Init(DBG_PAD3_PORT, &GPIOinitstruct);
  DBG_PAD3_RESET;
#endif /* DBG_USE_PAD3 */

#if DBG_USE_PAD4
  DBG_PAD4_CLK_EN;
  GPIOinitstruct.Pin = DBG_PAD4_PIN;
  HAL_GPIO_Init(DBG_PAD4_PORT, &GPIOinitstruct);
  DBG_PAD4_RESET;
#endif /* DBG_USE_PAD4 */

#if DBG_USE_PAD5
  DBG_PAD5_CLK_EN;
  GPIOinitstruct.Pin = DBG_PAD5_PIN;
  HAL_GPIO_Init(DBG_PAD5_PORT, &GPIOinitstruct);
  DBG_PAD5_RESET;
#endif /* DBG_USE_PAD5 */

  return 0;
}
