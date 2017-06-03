/*
 * filter.h
 *
 *  Created on: 30 maj 2017
 *      Author: osannolik
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

typedef struct {
  float angle_est;
  float speed_est;
  float integral_e_dt;
  float kp;
  float ki;
} filter_speed_pll_t;

int filter_speed_pll_init(filter_speed_pll_t * const f, const float kp, const float ki);
void filter_speed_pll_reset(filter_speed_pll_t * const f, const float angle_0, const float speed_0);
int filter_speed_pll_set_gains(filter_speed_pll_t * const f, const float kp, const float ki);
float filter_speed_pll_update(filter_speed_pll_t * const f, const float dt, float angle_meas);
float filter_speed_pll_get_speed(filter_speed_pll_t * const f);


#endif /* FILTER_H_ */
