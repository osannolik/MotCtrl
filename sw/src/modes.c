/*
 * modes.c
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#include "modes.h"
#include "board.h"
#include "bldc.h"

#include "calmeas.h"

//static modes_mode_t mode = INACTIVE;
#define CALMEAS_TYPECODE_modes_mode_t   CALMEAS_TYPECODE_uint8_t
#define CALMEAS_MEMSEC_modes_mode_t     CALMEAS_MEMSEC_uint8_t
CALMEAS_SYMBOL(modes_mode_t, mode, INACTIVE, "");

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
      if (board_button_pressed()) {
        delay_ms += period_ms;
        if (delay_ms >= button_hold_time_ms) {
          mode = MANUAL_STEP;
          delay_ms = 0u;
        }
      } else {
        delay_ms = 0u;
      }
      break;

    case MANUAL_STEP:
      if (board_button_pressed()) {
        delay_ms += period_ms;
        if (delay_ms >= button_hold_time_ms) {
          mode = OPEN_LOOP;
          delay_ms = 0u;
        }
      } else {
        delay_ms = 0u;
      }
      break;

    case OPEN_LOOP:
      if (board_button_pressed()) {
        delay_ms += period_ms;
        if (delay_ms >= button_hold_time_ms) {
          mode = RUNNING;
          delay_ms = 0u;
        }
      } else {
        delay_ms = 0u;
      }
      break;

    default:
      break; 
  }
}

modes_mode_t modes_current_mode(void)
{
  return mode;
}
