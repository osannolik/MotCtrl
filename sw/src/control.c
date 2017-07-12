/*
 * control.c
 *
 *  Created on: 30 apr 2017
 *      Author: osannolik
 */

#include "control.h"
#include "inverter.h"
#include "position.h"
#include "speed_control.h"
#include "modes.h"
#include "utils.h"

#include "calmeas.h"

CALMEAS_SYMBOL(float,   p_ctrl_speed_setpoint, 0.0f, "");
CALMEAS_SYMBOL(float,   p_ctrl_duty, 0.0f, "");
CALMEAS_SYMBOL(float,   p_ctrl_current, 0.0f, "");
CALMEAS_SYMBOL(uint8_t, p_ctrl_mode, CTRL_MODE_CURRENT, "");


void ctrl_step(const uint32_t period_ms)
{
  if (modes_current_mode() != RUNNING) {
    return;
  }

  float u = 0.0f;

  switch (p_ctrl_mode) {
    case CTRL_MODE_DUTY:
      u = saturatef(p_ctrl_duty, -100.0f, 100.0f);
      spdctrl_reset();
      ivtr_request_duty_cycle(u);
      break;

    case CTRL_MODE_SPEED:
      u = spdctrl_controller(position_get_speed_raw(),
                             p_ctrl_speed_setpoint,
                             (float) period_ms * 0.001f);
      ivtr_request_duty_cycle(u);
      break;

    case CTRL_MODE_CURRENT:
      u = p_ctrl_current;
      ivtr_request_current(0.0f, u);
      break;

    default:
      break;
  }
}

int ctrl_init(void)
{
  return 0;
}
