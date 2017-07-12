/*
 * pwm.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "pwm.h"
#include "utils.h"
#include "math.h"
#include "calmeas.h"


/* Measurements */
CALMEAS_SYMBOL(uint32_t, m_pwm_period, 0, "");

/* Parameters */

static TIM_HandleTypeDef TIMhandle;

static inline void set_ccxe_ch1(TIM_TypeDef* TIMx, uint32_t ch_p_state, uint32_t ch_n_state) {
  uint32_t tmp = (TIM_CCER_CC1NE | TIM_CCER_CC1E) << TIM_CHANNEL_1;
  TIMx->CCER &= ~tmp;
  TIMx->CCER |= ((ch_n_state | ch_p_state) << TIM_CHANNEL_1);
}

static inline void set_ccxe_ch2(TIM_TypeDef* TIMx, uint32_t ch_p_state, uint32_t ch_n_state) {
  uint32_t tmp = (TIM_CCER_CC1NE | TIM_CCER_CC1E) << TIM_CHANNEL_2;
  TIMx->CCER &= ~tmp;
  TIMx->CCER |= ((ch_n_state | ch_p_state) << TIM_CHANNEL_2);
}

static inline void set_ccxe_ch3(TIM_TypeDef* TIMx, uint32_t ch_p_state, uint32_t ch_n_state) {
  uint32_t tmp = (TIM_CCER_CC1NE | TIM_CCER_CC1E) << TIM_CHANNEL_3;
  TIMx->CCER &= ~tmp;
  TIMx->CCER |= ((ch_n_state | ch_p_state) << TIM_CHANNEL_3);
}

static inline void set_oc_mode_ch1(TIM_TypeDef* TIMx, uint32_t OC_mode) {
  TIMx->CCMR1 &= ~TIM_CCMR1_OC1M;
  TIMx->CCMR1 |= OC_mode;
}

static inline void set_oc_mode_ch2(TIM_TypeDef* TIMx, uint32_t OC_mode) {
  TIMx->CCMR1 &= ~TIM_CCMR1_OC2M;
  TIMx->CCMR1 |= (OC_mode << 8);
}

static inline void set_oc_mode_ch3(TIM_TypeDef* TIMx, uint32_t OC_mode) {
  TIMx->CCMR2 &= ~TIM_CCMR2_OC3M;
  TIMx->CCMR2 |= OC_mode;
}

static inline void set_oc_mode_ch4(TIM_TypeDef* TIMx, uint32_t OC_mode) {
  TIMx->CCMR2 &= ~TIM_CCMR2_OC4M;
  TIMx->CCMR2 |= (OC_mode << 8);
}

int pwm_init(void)
{
  /* Setup IO pins for PWM output */
  GPIO_InitTypeDef GPIOinitstruct;

  GPIOinitstruct.Speed     = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull      = GPIO_NOPULL;
  GPIOinitstruct.Mode      = GPIO_MODE_AF_PP;
  GPIOinitstruct.Alternate = GPIO_AF1_TIM1;

  PWM_GATE_H_A_CLK_EN;
  PWM_GATE_L_A_CLK_EN;
  PWM_GATE_H_B_CLK_EN;
  PWM_GATE_L_B_CLK_EN;
  PWM_GATE_H_C_CLK_EN;
  PWM_GATE_L_C_CLK_EN;

  GPIOinitstruct.Pin = PWM_GATE_H_A_PIN;
  HAL_GPIO_Init(PWM_GATE_H_A_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = PWM_GATE_L_A_PIN;
  HAL_GPIO_Init(PWM_GATE_L_A_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = PWM_GATE_H_B_PIN;
  HAL_GPIO_Init(PWM_GATE_H_B_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = PWM_GATE_L_B_PIN;
  HAL_GPIO_Init(PWM_GATE_L_B_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = PWM_GATE_H_C_PIN;
  HAL_GPIO_Init(PWM_GATE_H_C_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = PWM_GATE_L_C_PIN;
  HAL_GPIO_Init(PWM_GATE_L_C_PORT, &GPIOinitstruct);


  /* Setup base timer for complementary PWM */
  __HAL_RCC_TIM1_CLK_ENABLE();

#if (PWM_EDGE_ALIGNMENT == PWM_CENTER)
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED3;
  m_pwm_period = HAL_RCC_GetPCLK2Freq()/PWM_FREQUENCY_HZ - 1; /* TIM1 clock source 2*APB2 (2*90 MHz), since APB2 presc != 1 */
#else
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  m_pwm_period = 2*HAL_RCC_GetPCLK2Freq()/PWM_FREQUENCY_HZ - 1; /* TIM1 clock source 2*APB2 (2*90 MHz), since APB2 presc != 1 */
#endif

  TIMhandle.Instance               = TIM1;
  TIMhandle.Init.Prescaler         = 0;
  TIMhandle.Init.Period            = m_pwm_period;
  TIMhandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  TIMhandle.Init.RepetitionCounter = 0;

  HAL_TIM_PWM_Init(&TIMhandle);

  /* Configure the timer to update the PWM output states ONLY at a COM-event */
  HAL_TIMEx_ConfigCommutationEvent(&TIMhandle, TIM_TS_NONE, TIM_COMMUTATION_SOFTWARE);

  /* Configure PWM output levels for each channel */
  TIM_OC_InitTypeDef OCinit;
  OCinit.OCFastMode   = TIM_OCFAST_DISABLE;
  OCinit.OCIdleState  = TIM_OCIDLESTATE_RESET;
  OCinit.OCMode       = TIM_OCMODE_PWM1;
  OCinit.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  OCinit.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  OCinit.OCPolarity   = TIM_OCPOLARITY_HIGH;
  OCinit.Pulse        = 0;

  HAL_TIM_PWM_ConfigChannel(&TIMhandle, &OCinit, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&TIMhandle, &OCinit, TIM_CHANNEL_2);
  HAL_TIM_PWM_ConfigChannel(&TIMhandle, &OCinit, TIM_CHANNEL_3);
  HAL_TIM_PWM_ConfigChannel(&TIMhandle, &OCinit, TIM_CHANNEL_4); // Ch4 is used to trigger adc

  pwm_set_sample_trigger_perc(0.0f);

#if 0
  TIM_MasterConfigTypeDef MasterConfigHandle;
  MasterConfigHandle.MasterOutputTrigger = TIM_TRGO_UPDATE;
  MasterConfigHandle.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  HAL_TIMEx_MasterConfigSynchronization(&TIMhandle, &MasterConfigHandle);
#endif

  /* Configure deadtime and lock all settings */
  TIM_BreakDeadTimeConfigTypeDef TIMBreakDThandle;
  TIMBreakDThandle.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
  TIMBreakDThandle.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
  TIMBreakDThandle.BreakState       = TIM_BREAK_ENABLE;
  TIMBreakDThandle.DeadTime         = 30u; // 30 * 1/180000000 = ~166 ns
  TIMBreakDThandle.LockLevel        = TIM_LOCKLEVEL_OFF;
  TIMBreakDThandle.OffStateIDLEMode = TIM_OSSI_DISABLE;
  TIMBreakDThandle.OffStateRunMode  = TIM_OSSR_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&TIMhandle, &TIMBreakDThandle);

  TIMBreakDThandle.LockLevel        = TIM_LOCKLEVEL_1;
  HAL_TIMEx_ConfigBreakDeadTime(&TIMhandle, &TIMBreakDThandle);

  /* Enable interrupt at a break event */
  __HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_BREAK);
  NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, PWM_BREAK_IRQ_PRIO);
  NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

#if 0
  /* Generate events for synchronizing with other peripherals, e.g. ADC */
  __HAL_TIM_ENABLE_IT(&TIMhandle, TIM_IT_CC4);
  NVIC_SetPriority(TIM1_CC_IRQn, PWM_BREAK_IRQ_PRIO);
  NVIC_EnableIRQ(TIM1_CC_IRQn);
#endif

  /* Start timer and set initial states OFF */
  HAL_TIM_PWM_Start(&TIMhandle, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&TIMhandle, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&TIMhandle, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&TIMhandle, TIM_CHANNEL_4);

  HAL_TIMEx_OCN_Start(&TIMhandle, TIM_CHANNEL_1);
  HAL_TIMEx_OCN_Start(&TIMhandle, TIM_CHANNEL_2);
  HAL_TIMEx_OCN_Start(&TIMhandle, TIM_CHANNEL_3);

  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);

  HAL_TIM_GenerateEvent(&TIMhandle, TIM_EVENTSOURCE_COM);

  return 0;
}

#if 0
void TIM1_CC_IRQHandler(void)
{
  /* Capture compare 4 event */
  if(__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_CC4) != RESET) {
    if(__HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_CC4) !=RESET) {
      __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_CC4);
      //DBG_PAD1_TOGGLE;
    }
  }
}

void TIM1_UP_TIM10_IRQHandler(void)
{
  /* TIM Update event */
  if(__HAL_TIM_GET_FLAG(&TIMhandle, TIM_FLAG_UPDATE) != RESET) {
    if(__HAL_TIM_GET_IT_SOURCE(&TIMhandle, TIM_IT_UPDATE) !=RESET) {
      __HAL_TIM_CLEAR_IT(&TIMhandle, TIM_IT_UPDATE);
      //DBG_PAD1_TOGGLE;
    }
  }
}
#endif

void pwm_disable_sample_trigger(void)
{
  set_oc_mode_ch4(TIMhandle.Instance, TIM_OCMODE_PWM1);
  TIMhandle.Instance->CCR4 = 0u;
}

/* Sets the time when the sample trigger is triggered.
 * Edge-aligned:      0 - 100, where 0 is positive edge.
 * Center-aligned: -100 - 100, where 0 is center of positive
 *                             waveform, and +/-100 is center of low. */
void pwm_set_sample_trigger_perc(const float duty)
{
  const uint32_t full_period = m_pwm_period - 1;
  const uint32_t ccr = (uint32_t) (0.01f * fabsf(duty) * (float)full_period);

  TIMhandle.Instance->CCR4 = MIN(MAX(ccr, 1u), full_period);

#if (PWM_EDGE_ALIGNMENT == PWM_CENTER)
  uint32_t polarity;
  if (duty > 0.0f) {
    polarity = TIM_OCMODE_PWM2;
  } else {
    polarity = TIM_OCMODE_PWM1;
  }
  set_oc_mode_ch4(TIMhandle.Instance, polarity);
#else
  set_oc_mode_ch4(TIMhandle.Instance, TIM_OCMODE_PWM2);
#endif
}

void pwm_set_duty_gate_abc_perc(const float duty)
{
  const uint32_t duty_period = (uint32_t) (0.01f * duty * ((float) m_pwm_period));

  TIMhandle.Instance->CCR1 = duty_period;
  TIMhandle.Instance->CCR2 = duty_period;
  TIMhandle.Instance->CCR3 = duty_period;
}

void pwm_set_duty_gate_perc(const float duty_a, const float duty_b, const float duty_c)
{
  const float max_period = (float) m_pwm_period * 0.01f;

  TIMhandle.Instance->CCR1 = (uint32_t) (duty_a * max_period);
  TIMhandle.Instance->CCR2 = (uint32_t) (duty_b * max_period);
  TIMhandle.Instance->CCR3 = (uint32_t) (duty_c * max_period);
}

void pwm_set_duty_gate_a_perc(const float duty)
{
  const uint32_t duty_period = (uint32_t) (0.01f * duty * ((float) m_pwm_period));
  TIMhandle.Instance->CCR1 = duty_period;
}

void pwm_set_duty_gate_b_perc(const float duty)
{
  const uint32_t duty_period = (uint32_t) (0.01f * duty * ((float) m_pwm_period));
  TIMhandle.Instance->CCR2 = duty_period;
}

void pwm_set_duty_gate_c_perc(const float duty)
{
  const uint32_t duty_period = (uint32_t) (0.01f * duty * ((float) m_pwm_period));
  TIMhandle.Instance->CCR3 = duty_period;
}

void pwm_update_event(void)
{
  HAL_TIM_GenerateEvent(&TIMhandle, TIM_EVENTSOURCE_COM);
}

void pwm_break_event(void)
{
  HAL_TIM_GenerateEvent(&TIMhandle, TIM_EVENTSOURCE_BREAK);
}

void pwm_ch1_complementary_pos(void) {
  set_oc_mode_ch1(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch1_complementary_neg(void) {
  set_oc_mode_ch1(TIMhandle.Instance, TIM_OCMODE_PWM2);
  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch2_complementary_pos(void) {
  set_oc_mode_ch2(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch2_complementary_neg(void) {
  set_oc_mode_ch2(TIMhandle.Instance, TIM_OCMODE_PWM2);
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch3_complementary_pos(void) {
  set_oc_mode_ch3(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch3_complementary_neg(void) {
  set_oc_mode_ch3(TIMhandle.Instance, TIM_OCMODE_PWM2);
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch3_off(void) {
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);
}

void pwm_ch3_pwm_afw(void) {
  set_oc_mode_ch3(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch3_sink(void) {
  set_oc_mode_ch3(TIMhandle.Instance, TIM_OCMODE_FORCED_ACTIVE);
  set_ccxe_ch3(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_ENABLE);
}

void pwm_ch2_off(void) {
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);
}

void pwm_ch2_pwm_afw(void) {
  set_oc_mode_ch2(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch2_sink(void) {
  set_oc_mode_ch2(TIMhandle.Instance, TIM_OCMODE_FORCED_ACTIVE);
  set_ccxe_ch2(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_ENABLE);
}

void pwm_ch1_off(void) {
  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_DISABLE);
}

void pwm_ch1_pwm_afw(void) {
  set_oc_mode_ch1(TIMhandle.Instance, TIM_OCMODE_PWM1);
  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_ENABLE, TIM_CCxN_ENABLE);
}

void pwm_ch1_sink(void) {
  set_oc_mode_ch1(TIMhandle.Instance, TIM_OCMODE_FORCED_ACTIVE);
  set_ccxe_ch1(TIMhandle.Instance, TIM_CCx_DISABLE, TIM_CCxN_ENABLE);
}
