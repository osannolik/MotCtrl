/*
 * filter.c
 *
 *  Created on: 30 maj 2017
 *      Author: osannolik
 */

#include "filter.h"
#include "utils.h"

int filter_speed_pll_init(filter_speed_pll_t * const f, const float kp, const float ki)
{
  filter_speed_pll_reset(f, 0.0f, 0.0f);
  return filter_speed_pll_set_gains(f, kp, ki);
}

void filter_speed_pll_reset(filter_speed_pll_t * const f, const float angle_0, const float speed_0)
{
  f->angle_est = angle_0;
  f->speed_est = speed_0;
  f->integral_e_dt = 0.0f;
}

int filter_speed_pll_set_gains(filter_speed_pll_t * const f, const float kp, const float ki)
{
  if (kp<0.0f || ki<0.0f) {
    return -1;
  }

  f->kp = kp;
  f->ki = ki;

  return 0;
}

float filter_speed_pll_update(filter_speed_pll_t * const f, const float dt, float angle_meas)
{
  angle_meas = wrap_to_range_f(-PI, PI, angle_meas);

  const float e = wrap_to_range_f(-PI, PI, angle_meas - f->angle_est);

  f->integral_e_dt += e * dt;

  f->speed_est = f->kp * e + f->ki * f->integral_e_dt;

  f->angle_est = wrap_to_range_f(-PI, PI, f->angle_est + f->speed_est * dt);

  return f->speed_est;
}

float filter_speed_pll_get_speed(filter_speed_pll_t * const f)
{
  return f->speed_est;
}
