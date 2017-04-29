/*
 * bldc_6step.h
 *
 *  Created on: 29 apr 2017
 *      Author: osannolik
 */

#ifndef BLDC_6STEP_H_
#define BLDC_6STEP_H_

#include <stdint.h>
#include <stddef.h>
#include "position.h"

typedef enum {
  STEP_OFF        = 0,
  STEP_1          = 1,
  STEP_2          = 2,
  STEP_3          = 3,
  STEP_4          = 4,
  STEP_5          = 5,
  STEP_6          = 6,
  NUMBER_OF_STEPS = 7
} bldc_step_t;

typedef enum {
  CAL_NOT_PERFORMED = 0,
  CAL_PRE_ROTATION,
  CAL_PROBING,
  CAL_CHECK,
  CAL_OK,
  CAL_FAILED
} bldc_cal_state_t;

uint8_t bldc6s_current_step(void);
void bldc6s_commutation(const pos_direction_t direction, const uint8_t current_hall_state);
inline void bldc6s_set_commutation_step(const uint8_t step);
void bldc6s_hall_calibration_step(uint32_t period_ms);
bldc_cal_state_t bldc6s_cal_state(void);
int bldc6s_init(void);

#endif /* BLDC_6STEP_H_ */
