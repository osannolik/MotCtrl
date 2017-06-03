/*
 * encoder.c
 *
 *  Created on: 26 maj 2017
 *      Author: osannolik
 */

#include "encoder.h"
#include "math.h"

#define SPEED_COUNTER_MAX (0xFFFFu)

static volatile uint8_t encoder_capture_overflow = 0;

static const float counts_per_revolution = 4 * ENCODER_PPR; // x4 due to counting at all pulse edges

static float speed_timer_resolution_s;
static TIM_HandleTypeDef encoder_timer;
static TIM_HandleTypeDef speed_timer;

int encoder_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  __HAL_RCC_TIM4_CLK_ENABLE();

  GPIOinitstruct.Mode      = GPIO_MODE_AF_PP;
  GPIOinitstruct.Pull      = GPIO_NOPULL;
  GPIOinitstruct.Speed     = GPIO_SPEED_HIGH;
  GPIOinitstruct.Alternate = GPIO_AF2_TIM4;

  ENCODER_A_CLK_EN;
  GPIOinitstruct.Pin = ENCODER_A_PIN;
  HAL_GPIO_Init(ENCODER_A_PORT, &GPIOinitstruct);

  ENCODER_B_CLK_EN;
  GPIOinitstruct.Pin = ENCODER_B_PIN;
  HAL_GPIO_Init(ENCODER_B_PORT, &GPIOinitstruct);

#if 0
  ENCODER_INDEX_CLK_EN;
  GPIOinitstruct.Pin = ENCODER_INDEX_PIN;
  HAL_GPIO_Init(ENCODER_INDEX_PORT, &GPIOinitstruct);
#endif

  /* Initialize TIM4 and TIM3 peripheral as follows:
   *
   * TIM4 is configured in encoder mode.
   * The timer's counter will increment/decrement on all edges of both
   * encoder signal A and B, meaning the counter will reach 4*ppr after
   * one full revolution.
   *
   * Speed is measured by configuring TIM4 as Master and TIM3 as Slave:
   *   - TRGO of TIM4 is set to CC1 which pulses on the rising edge of
   *     encoder input A, giving ppr pulses per revolution, NOT 4*ppr.
   *   - Slave TIM3 is set to capture and reset based on TRC = TRGO
   *
   * Configuration:
   *   APB1 is the clock source = 2*APB1 (2*45 MHz)
   *   Using a prescaler of 1 and using all 16 bits yields:
   *   - Resolution = 1 / 90 MHz = 11.1 ns
   *   - Time until overflow = 2^16 / 90 MHz = 728 us
   *
   * T = 60/ppr/rpm => d/dT rpm = -ppr/60 * rpm^2
   *
   * If ppr = 2048:
   * this allows for a speed down to 40 rpm before an overflow occurs.
   * At 10000 rpm, the resolution will be approx 11.1 ns * 2048/60 * 10000^2 = 38 rpm
   *
   * An increased prescaler allows for lower speed operation, but gives worse resolution.
   */

  /* Encoder timer: Encoder Mode */
  encoder_timer.Instance               = TIM4;
  encoder_timer.Init.Period            = (uint32_t) counts_per_revolution - 1; 
  encoder_timer.Init.Prescaler         = 0;
  encoder_timer.Init.ClockDivision     = 0;
  encoder_timer.Init.CounterMode       = TIM_COUNTERMODE_UP;
  encoder_timer.Init.RepetitionCounter = 0;

  TIM_Encoder_InitTypeDef timer_config;
  timer_config.EncoderMode  = TIM_ENCODERMODE_TI12;
  timer_config.IC1Polarity  = TIM_ICPOLARITY_RISING;
  timer_config.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  timer_config.IC1Prescaler = TIM_ICPSC_DIV1;
  timer_config.IC1Filter    = 0;
  timer_config.IC2Polarity  = TIM_ICPOLARITY_RISING;
  timer_config.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  timer_config.IC2Prescaler = TIM_ICPSC_DIV1;
  timer_config.IC2Filter    = 0;

  HAL_TIM_Encoder_Init(&encoder_timer, &timer_config);

  /* Encoder timer: TRGO = Compare Pulse (CC1IF) */
  TIM_MasterConfigTypeDef master_config;
  master_config.MasterOutputTrigger = TIM_TRGO_OC1;
  master_config.MasterSlaveMode     = TIM_MASTERSLAVEMODE_ENABLE;
  HAL_TIMEx_MasterConfigSynchronization(&encoder_timer, &master_config);

  /* Speed timer: Trigger reset and capture at TRC */
  __HAL_RCC_TIM3_CLK_ENABLE();

  const uint32_t prescaler = 1u;
  speed_timer_resolution_s = ((float) prescaler) / ((float) 2.0f * HAL_RCC_GetPCLK1Freq());
  
  speed_timer.Instance               = TIM3;
  speed_timer.Init.Period            = SPEED_COUNTER_MAX;
  speed_timer.Init.Prescaler         = prescaler - 1;
  speed_timer.Init.ClockDivision     = 0;
  speed_timer.Init.CounterMode       = TIM_COUNTERMODE_UP;
  speed_timer.Init.RepetitionCounter = 0;

  HAL_TIM_Base_Init(&speed_timer);

  TIM_SlaveConfigTypeDef slave_config;
  slave_config.SlaveMode        = TIM_SLAVEMODE_RESET;
  slave_config.InputTrigger     = TIM_TS_ITR3; // TRC = TRGO of TIM4
  slave_config.TriggerPolarity  = TIM_TRIGGERPOLARITY_RISING;
  slave_config.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  slave_config.TriggerFilter    = 0;

  HAL_TIM_SlaveConfigSynchronization(&speed_timer, &slave_config);

  TIM_IC_InitTypeDef ic_config;
  ic_config.ICSelection = TIM_ICSELECTION_TRC;
  ic_config.ICPolarity  = TIM_ICPOLARITY_RISING;
  ic_config.ICPrescaler = TIM_ICPSC_DIV1;
  ic_config.ICFilter    = 0;
  HAL_TIM_IC_ConfigChannel(&speed_timer, &ic_config, TIM_CHANNEL_1);

  HAL_TIM_Encoder_Start(&encoder_timer, TIM_CHANNEL_ALL);

  speed_timer.Instance->CR1 |= 0x4u; // Only counter overflow/underflow generates an update interrupt
  __HAL_TIM_ENABLE_IT(&speed_timer, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&speed_timer, TIM_IT_CC1);

  NVIC_SetPriority(TIM3_IRQn, 0x03u);
  NVIC_EnableIRQ(TIM3_IRQn);

  HAL_TIM_IC_Start_IT(&speed_timer, TIM_CHANNEL_1);

  return 0;
}

void TIM3_IRQHandler(void)
{
  /* Capture event: ISR only needed to detect when overflow is over */
  if (__HAL_TIM_GET_FLAG(&speed_timer, TIM_FLAG_CC1) && \
      __HAL_TIM_GET_IT_SOURCE(&speed_timer, TIM_IT_CC1)) {
    __HAL_TIM_CLEAR_IT(&speed_timer, TIM_IT_CC1);

    encoder_capture_overflow = 0;

    __HAL_TIM_ENABLE_IT(&speed_timer, TIM_IT_UPDATE);
    __HAL_TIM_DISABLE_IT(&speed_timer, TIM_IT_CC1);
  }

  /* Speed timer overflow */
  if(__HAL_TIM_GET_FLAG(&speed_timer, TIM_FLAG_UPDATE) && \
     __HAL_TIM_GET_IT_SOURCE(&speed_timer, TIM_IT_UPDATE)) {
    __HAL_TIM_CLEAR_IT(&speed_timer, TIM_FLAG_UPDATE);

    encoder_capture_overflow = 1;

    __HAL_TIM_ENABLE_IT(&speed_timer, TIM_IT_CC1);
    __HAL_TIM_DISABLE_IT(&speed_timer, TIM_IT_UPDATE);
  }
}

float encoder_get_direction(void)
{
  if (encoder_capture_overflow) {
    return 0.0f;
  } else if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&encoder_timer)) {
    return -1.0f;
  } else {
    return 1.0f;
  }
}

float encoder_get_speed_raw_radps(void)
{
  const float t = speed_timer_resolution_s * (float) HAL_TIM_ReadCapturedValue(&speed_timer, TIM_CHANNEL_1);

  return encoder_get_direction() * 2.0f*PI / ((float) ENCODER_PPR * t);
}

float encoder_get_angle_rad(void)
{
  return ((float) encoder_timer.Instance->CNT)*2.0f*PI/counts_per_revolution;
}

float encoder_get_angle_deg(void)
{
  return ((float) encoder_timer.Instance->CNT)*360.0f/counts_per_revolution;
}

void encoder_set_angle_rad(float angle0)
{
  angle0 = wrap_to_range_f(0.0f, 2.0f*PI, angle0);
  const float set_counter = roundf(angle0 * counts_per_revolution/(2.0f*PI));
  encoder_set_counter(set_counter);
}

void encoder_set_angle_deg(const float angle0)
{
  encoder_set_angle_rad(DEG_TO_RAD(angle0));
}

uint32_t encoder_get_counter(void)
{
  return encoder_timer.Instance->CNT;
}

void encoder_set_counter(const uint32_t cnt0)
{
  encoder_timer.Instance->CNT = wrap_to_range_i(0, (int) counts_per_revolution, cnt0);
}
