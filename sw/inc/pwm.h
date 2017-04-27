/*
 * pwm.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef PWM_H_
#define PWM_H_

#include "stm32f4xx_hal.h"

#define PWM_GATE_H_A_PIN    GPIO_PIN_8
#define PWM_GATE_H_A_PORT   GPIOA
#define PWM_GATE_H_A_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()

#define PWM_GATE_L_A_PIN    GPIO_PIN_7
#define PWM_GATE_L_A_PORT   GPIOA
#define PWM_GATE_L_A_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()

#define PWM_GATE_H_B_PIN    GPIO_PIN_9
#define PWM_GATE_H_B_PORT   GPIOA
#define PWM_GATE_H_B_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()

#define PWM_GATE_L_B_PIN    GPIO_PIN_0
#define PWM_GATE_L_B_PORT   GPIOB
#define PWM_GATE_L_B_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define PWM_GATE_H_C_PIN    GPIO_PIN_10
#define PWM_GATE_H_C_PORT   GPIOA
#define PWM_GATE_H_C_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE()

#define PWM_GATE_L_C_PIN    GPIO_PIN_1
#define PWM_GATE_L_C_PORT   GPIOB
#define PWM_GATE_L_C_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE()

#define PWM_FREQUENCY_HZ    (20000u)

#define PWM_EDGE            (0u)
#define PWM_CENTER          (1u)
#define PWM_EDGE_ALIGNMENT  PWM_CENTER

#define PWM_BREAK_IRQ_PRIO  (0x00u)

int pwm_init(void);
void pwm_disable_sample_trigger(void);
void pwm_set_sample_trigger_perc(const float duty);
void pwm_set_duty_gate_abc_perc(const float duty);
void pwm_set_duty_gate_a_perc(const float duty);
void pwm_set_duty_gate_b_perc(const float duty);
void pwm_set_duty_gate_c_perc(const float duty);
void pwm_update_event(void);
void pwm_break_event(void);
void pwm_ch3_off(void);
void pwm_ch3_pwm_afw(void);
void pwm_ch3_sink(void);
void pwm_ch2_off(void);
void pwm_ch2_pwm_afw(void);
void pwm_ch2_sink(void);
void pwm_ch1_off(void);
void pwm_ch1_pwm_afw(void);
void pwm_ch1_sink(void);

#endif /* PWM_H_ */
