/*
 * modes.c
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#include "modes.h"
#include "board.h"
#include "bldc.h"

static modes_mode_t mode = INACTIVE;

int modes_init(void)
{
  mode = INACTIVE;

  return 0;
}

void modes_step(uint32_t period_ms)
{
  const uint32_t button_hold_time_ms = 3000u;
  static uint32_t delay_ms = 0u;

  switch (mode) {

    case INACTIVE:
      if (board_button_pressed()) {
        delay_ms += period_ms;
        if (delay_ms >= button_hold_time_ms) {
          mode = BLDC_HALL_CALIBRATION;
        }
      } else {
        delay_ms = 0u;
      }
      break;
    
    case BLDC_HALL_CALIBRATION:
      if (CAL_OK == bldc_cal_state()) {
        mode = RUNNING;
      }
      break;

    case RUNNING:
      break;

    default:
      break; 
  }
}

modes_mode_t modes_current_mode(void)
{
  return mode;
}
