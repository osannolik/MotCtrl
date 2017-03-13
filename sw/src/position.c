/*
 * position.c
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#include "position.h"
#include "hall.h"

static void (* commutation_indication_cb)(uint8_t hall_state) = NULL;
static float hall_state_to_angle_map_cw[POS_NUMBER_OF_HALL_STATES] = {0.0f};
static float hall_state_to_angle_map_ccw[POS_NUMBER_OF_HALL_STATES] = {0.0f};

static void position_hall_commutation(uint8_t hall_state);

int position_init(void)
{
  hall_set_commutation_indication_cb(position_hall_commutation);

  return 0;
}

static void position_hall_commutation(uint8_t hall_state)
{
  if (commutation_indication_cb != NULL) {
    commutation_indication_cb(hall_state);
  }
}

void position_set_hall_commutation_indication_cb(void (* callback)(uint8_t))
{
  commutation_indication_cb = callback;
}

uint8_t position_get_hall_state(void)
{
  return hall_state();
}

void position_map_hall_state_to_angle(uint8_t hall_state, float angle)
{
  if (hall_state < POS_NUMBER_OF_HALL_STATES) {
    hall_state_to_angle_map_cw[hall_state]  = angle + 30.0f;
    hall_state_to_angle_map_ccw[hall_state] = angle - 30.0f;
  }
}

float position_get_angle_est(void)
{
  return hall_state_to_angle_map_cw[hall_state()];
}

float position_get_speed_est(void)
{
  return 0.0;
}
