/*
 * crc8.h
 *
 *  Created on: 25 mar 2017
 *      Author: osannolik
 */

#ifndef CRC8_H_
#define CRC8_H_

#include "stm32f4xx_hal.h"

#define CRC8_8         (0)
#define CRC8_MAXIM     (1)
#define CRC8_SAE_J1850 (2)
#define CRC8_AUTOSAR   (3)
#define CRC8_CDMA2000  (4)

#define CRC8_USED_POLY CRC8_MAXIM

uint8_t crc8_block(const uint8_t *data, uint32_t len);
uint8_t crc8(uint8_t crc, const uint8_t *data, uint32_t len);

#endif /* CRC8_H_ */
