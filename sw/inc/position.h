/*
 * position.h
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#ifndef POSITION_H_
#define POSITION_H_

#include "stm32f4xx_hal.h"

#define POS_NUMBER_OF_HALL_STATES (8)

typedef enum {
  DIR_CW   = 0,
  DIR_CCW  = 1,
  DIR_NONE = 2,
  NUMBER_OF_DIRS = 3
} pos_direction_t;

int position_init(void);
void position_set_hall_commutation_indication_cb(void (* callback)(uint8_t));
void position_hall_individual_states(uint8_t *h1, uint8_t *h2, uint8_t *h3);
uint8_t position_get_hall_state(void);
void position_map_hall_state_to_angle(uint8_t hall_state, float angle);
float position_get_angle_est(void);
float position_get_speed_est(void);

#endif /* POSITION_H_ */
