/*
 * bldc.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef BLDC_H_
#define BLDC_H_

#include "stm32f4xx_hal.h"

#define POS_NUMBER_OF_HALL_STATES (8)

typedef enum {
  DIR_CW   = 0,
  DIR_CCW  = 1,
  DIR_NONE = 2,
  NUMBER_OF_DIRS = 3
} bldc_direction_t;

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

void bldc_step(uint32_t period_ms);
void bldc_set_duty_cycle(float duty_req);
int bldc_init(void);
void bldc_safe_state(void);
bldc_cal_state_t bldc_cal_state(void);
void bldc_commutation(bldc_direction_t direction, uint8_t current_hall_state);
void bldc_hall_indication(uint8_t current_hall_state);

#endif /* BLDC_H_ */
