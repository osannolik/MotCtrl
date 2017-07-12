/*
 * modes.c
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#include "modes.h"
#include "board.h"
#include "bldc_6step.h"
#include "foc.h"

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
#if 0
          mode = BLDC_HALL_CALIBRATION;
#else
          mode = FOC_SENSOR_ALIGNMENT;
#endif
        }
      } else {
        delay_ms = 0u;
      }
      break;

    case FOC_SENSOR_ALIGNMENT:
      switch (foc_sensor_alignment_state()) {
        case ALIGNMENT_OK:
          mode = RUNNING; break;
        case ALIGNMENT_FAILED:
          mode = INACTIVE; break;
        default: break;
      }
      break;

    case BLDC_HALL_CALIBRATION:
      switch (bldc6s_cal_state()) {
        case CAL_OK:
          mode = RUNNING; break;
        case CAL_FAILED:
          mode = INACTIVE; break;
        default: break;
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
