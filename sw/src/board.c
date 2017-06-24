/*
 * board.c
 *
 *  Created on: 6 feb 2017
 *      Author: osannolik
 */

#include "math.h"
#include "board.h"
#include "utils.h"

#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(float, m_board_bat_v, 0.0f, "");
CALMEAS_SYMBOL(float, m_board_temp_degC, 0.0f, "");
CALMEAS_SYMBOL(float, m_board_mcu_temp_degC, 0.0f, "");

int board_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  BOARD_GATE_DRIVER_CLK_EN;
  BOARD_BUTTON_CLK_EN;

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull  = GPIO_NOPULL;
  GPIOinitstruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIOinitstruct.Pin   = BOARD_GATE_DRIVER_PIN;

  HAL_GPIO_Init(BOARD_GATE_DRIVER_PORT, &GPIOinitstruct);

  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull  = GPIO_PULLUP;
  GPIOinitstruct.Mode  = GPIO_MODE_INPUT;
  GPIOinitstruct.Pin   = BOARD_BUTTON_PIN;

  HAL_GPIO_Init(BOARD_BUTTON_PORT, &GPIOinitstruct);

  board_gate_driver_disable();

  return 0;
}

void board_step(const uint32_t period_ms)
{
  (void) period_ms;

  m_board_bat_v         = board_bat_voltage();
  m_board_temp_degC     = board_temp_degC();
  m_board_mcu_temp_degC = board_mcu_temp_degC();
}

void board_gate_driver_enable(void)
{
  HAL_GPIO_WritePin(BOARD_GATE_DRIVER_PORT, BOARD_GATE_DRIVER_PIN, GPIO_PIN_SET);
}

void board_gate_driver_disable(void)
{
  HAL_GPIO_WritePin(BOARD_GATE_DRIVER_PORT, BOARD_GATE_DRIVER_PIN, GPIO_PIN_RESET);
}

uint8_t board_button_pressed(void)
{
  return (uint8_t) (GPIO_PIN_RESET == HAL_GPIO_ReadPin(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN));
}

float board_bat_voltage(void)
{
  return adc_get_measurement(ADC_BAT_SENSE) * BOARD_BAT_SENSE_DIVIDER;
}

float board_temp_degC(void)
{
  const float v  = adc_get_measurement(ADC_BOARD_TEMP);
  const float r  = BOARD_R3 * (ADC_VREF/v - 1.0f);
  const float T0 = DEGC_TO_KELVIN(25.0f);
  const float T  = 1.0f / ( 1.0f/T0 + logf(r/BOARD_NTC)/BOARD_NTC_BETA );

  return KELVIN_TO_DEGC(T);
}

float board_mcu_temp_degC(void)
{
  const float v_sense = adc_get_measurement(ADC_MCU_TEMP);

  return 25.0f + (v_sense - BOARD_V25)/BOARD_AVG_SLOPE;
}

inline float board_ix_volt_to_ampere(float adc_volt) {
  return (adc_volt-BOARD_INA240_OFFSET_V)/(BOARD_R_SHUNT*BOARD_INA240_GAIN);
}

inline float board_ix_ampere_to_volt(float ix_ampere) {
  return BOARD_R_SHUNT*BOARD_INA240_GAIN*ix_ampere + BOARD_INA240_OFFSET_V;
}
