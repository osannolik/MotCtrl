/*
 * modes.h
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#ifndef MODES_H_
#define MODES_H_

#include "stm32f4xx_hal.h"

typedef enum {
  INACTIVE = 0u,
  BLDC_HALL_CALIBRATION,
  RUNNING
} modes_mode_t;

int modes_init(void);
void modes_step(uint32_t period_ms);
modes_mode_t modes_current_mode(void);

#endif /* MODES_H_ */
