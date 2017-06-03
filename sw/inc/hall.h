/*
 * hall.h
 *
 *  Created on: 21 jan 2017
 *      Author: osannolik
 */

#ifndef HALL_H_
#define HALL_H_

#include "stm32f4xx_hal.h"

#define HALL_SENSOR_H1_PIN    GPIO_PIN_6
#define HALL_SENSOR_H1_PORT   GPIOB
#define HALL_SENSOR_H1_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_SENSOR_H2_PIN    GPIO_PIN_7
#define HALL_SENSOR_H2_PORT   GPIOB
#define HALL_SENSOR_H2_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_SENSOR_H3_PIN    GPIO_PIN_8
#define HALL_SENSOR_H3_PORT   GPIOB
#define HALL_SENSOR_H3_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define HALL_IRQ_PRIO         (0x03u)


/* The hall sensor offset is here defined as the smallest angle between the sensor
 * position and the stator flux vectors that can be obtained by the 6-step BLDC modulation.
 * I.e. 30 degrees offset corresponds to the case where applying a stationary vector
 * will align the rotor to the center of two hall sensors state changes.
 * Another way of defining this is the angle between the phase-to-phase EMF zero crossing 
 * and the hall transition. 
 * Range 0-30.
 */
#define HALL_SENSOR_OFFSET_DEG      (30)

/* When the offset is non-zero then it is required to delay the commutation in order
 * to get a stator flux vector that creates an angle to the rotor that varies from 120-60 deg.
 * The time of delay is nominally set to half the time the previous commutation cycle took.
 */
#define HALL_COMMUTATION_DELAY_PERC (HALL_SENSOR_OFFSET_DEG*0.5f/30.0f)

#define NUMBER_OF_HALL_STATES       (8)
#define HALL_NUMBER_OF_COMMUTATIONS (6)

enum {
  DIR_CW   = 0,
  DIR_CCW  = 1,
  DIR_NONE = 2,
  NUMBER_OF_DIRS = 3
};

typedef struct {
  uint8_t current;
  uint8_t previous;
} hall_state_t;


int hall_init(void);
uint8_t hall_get_state(void);
float hall_get_direction(void);
float hall_get_speed_raw_radps(void);
float hall_get_angle_raw_rad(void);
float hall_state_to_angle_deg(hall_state_t * const state);
void hall_calculate_direction_map(void);
void hall_map_state_to_angle(const uint8_t hall_state, const float angle);
void hall_set_commutation_indication_cb(void (* callback)(uint8_t));
void hall_set_state_change_indication_cb(void (* callback)(uint8_t));


#endif /* HALL_H_ */
