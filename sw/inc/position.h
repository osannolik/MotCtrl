/*
 * position.h
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#ifndef POSITION_H_
#define POSITION_H_

#include "stm32f4xx_hal.h"

/* The hall sensor offset is here defined as the smallest angle between the sensor
 * position and the stator flux vectors that can be obtained by the 6-step BLDC modulation.
 * I.e. 30 degrees offset corresponds to the case where applying a stationary vector
 * will align the rotor to the center of two hall sensors state changes.
 * Another way of defining this is the angle between the phase-to-phase EMF zero crossing 
 * and the hall transition. 
 * Range 0-30.
 */
#define POS_HALL_SENSOR_OFFSET_DEG      (30)

/* When the offset is non-zero then it is required to delay the commutation in order
 * to get a stator flux vector that creates an angle to the rotor that varies from 120-60 deg.
 * The time of delay is nominally set to half the time the previous commutation cycle took.
 */
#define POS_HALL_COMMUTATION_DELAY_PERC (POS_HALL_SENSOR_OFFSET_DEG*0.5f/30.0f)

#define POS_NUMBER_OF_HALL_STATES       (8)

#define RPM_TO_DEGPS                    (6.0f)

typedef enum {
  DIR_CW   = 0,
  DIR_CCW  = 1,
  DIR_NONE = 2,
  NUMBER_OF_DIRS = 3
} pos_direction_t;

int position_init(void);
void position_set_hall_commutation_indication_cb(void (* callback)(uint8_t));
void position_hall_individual_states(uint8_t * const h1, uint8_t * const h2, uint8_t * const h3);
uint8_t position_get_hall_state(void);
void position_calculate_direction_map(void);
void position_map_hall_state_to_angle(const uint8_t hall_state, const float angle);
pos_direction_t position_get_direction(void);
float position_get_angle_est_deg(void);
float position_get_speed_est_erpm(void);
void position_angle_est_reset_to(const float angle_0);
float position_angle_est_update(const float period_s);
void position_speed_est_update(const float speed_raw);
float position_get_speed_raw_erpm(void);
void position_speed_est_reset(void);
float wrap_to_range(const float low, const float high, float x);

#endif /* POSITION_H_ */
