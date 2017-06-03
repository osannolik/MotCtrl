/*
 * position.h
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#ifndef POSITION_H_
#define POSITION_H_

#include <stdint.h>
#include <stddef.h>

#include "hall.h"

enum {
  SENSORLESS = 0,
  HALL = 1,
  ENCODER = 2
};

typedef struct {
  float speed_est;
  float angle_est;
  float speed_raw_per_commutation[HALL_NUMBER_OF_COMMUTATIONS];
  uint8_t commutation_counter;
} hall_filter_t;

int position_init(const uint8_t source_sensor);
float position_get_angle(void);
float position_get_angle_raw(void);
float position_get_speed(void);
float position_get_speed_raw(void);
float position_update_speed_filter(const float dt);
float position_update_angle_filter(const float dt);
float position_update_encoder_speed_filter(const float dt);
float position_update_hall_speed_filter(void);
float position_update_hall_angle_filter(const float dt);

#endif /* POSITION_H_ */
