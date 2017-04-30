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
CALMEAS_SYMBOL(uint8_t, p_ctrl_mode, CTRL_MODE_DUTY, "");


void ctrl_step(const uint32_t period_ms)
{
  if (modes_current_mode() != RUNNING) {
    return;
  }

  float u;

  switch (p_ctrl_mode) {
    case CTRL_MODE_DUTY:
      u = saturatef(p_ctrl_duty, -100.0f, 100.0f);
      spdctrl_reset();
      break;

    case CTRL_MODE_SPEED:
      u = spdctrl_controller(position_get_speed_raw_erpm(),
                             p_ctrl_speed_setpoint,
                             (float) period_ms * 0.001f);
      break;

    default:
      u = 0.0f;
      break;
  }

  ivtr_request_duty_cycle(u);
}

int ctrl_init(void)
{
  return 0;
}
