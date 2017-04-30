/*
 * speed_control.c
 *
 *  Created on: 30 apr 2017
 *      Author: osannolik
 */

#include "speed_control.h"
#include "utils.h"

#include "calmeas.h"

CALMEAS_SYMBOL(float, p_spd_gain_kp, 0.4f, "");
CALMEAS_SYMBOL(float, p_spd_gain_ki, 0.4f, "");
CALMEAS_SYMBOL(float, p_spd_integral_max_abs, 30.0f, "");
CALMEAS_SYMBOL(float, p_spd_u_rate_max, 0.1f, "");

static float integral;
static rate_limit_t rlim_spdctrl;

float spdctrl_controller(const float speed, const float speed_setpoint, const float dt)
{

  const float error = speed_setpoint - speed;

  integral = saturatef(integral + dt * p_spd_gain_ki * error,
                       -p_spd_integral_max_abs,
                       p_spd_integral_max_abs);

  float u = p_spd_gain_kp * error + integral;

  if (speed_setpoint >= 0) {
    u = MAX(u, 0.0f);
  } else {
    u = MIN(u, 0.0f);
  }

  u = rate_limit(&rlim_spdctrl, u, -p_spd_u_rate_max, p_spd_u_rate_max);

  return u;
}

void spdctrl_reset(void)
{
  integral = 0.0f;
}

int spdctrl_init(void)
{
  rate_limit_reset(&rlim_spdctrl, 0.0f);
  spdctrl_reset();

  return 0;
}
