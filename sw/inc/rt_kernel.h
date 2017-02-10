/*
 * rt_kernel.h
 *
 *  Created on: 27 aug 2016
 *      Author: osannolik
 */

#ifndef RT_KERNEL_H_
#define RT_KERNEL_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "rt_kernel_defs.h"
#include "rt_kernel_config.h"
#include "rt_lists.h"


#define rt_systick              SysTick_Handler
#define rt_switch_context       PendSV_Handler
#define rt_syscall              SVC_Handler

#define TCB_INIT(sp, fcn, name, prio, stack_size) {sp, fcn, name, prio, prio, stack_size, 0, LIST_ITEM_INIT, LIST_ITEM_INIT}

#define DEFINE_TASK(fcn, handle, name, prio, stack_size) \
  void fcn(void *p);\
  uint32_t handle ## _stack[stack_size];\
  rt_tcb_t handle = TCB_INIT(handle ## _stack, fcn, name, prio, stack_size);


void rt_systick();
void rt_switch_context() __attribute__((naked));
void rt_syscall()        __attribute__((naked));

void rt_yield(void);

void rt_enter_critical(void);
void rt_exit_critical(void);
void rt_suspend(void);
void rt_resume(void);
uint32_t rt_get_tick(void);
void rt_periodic_delay(const uint32_t period_ms);
uint32_t rt_create_task(rt_task_t const task, void * const task_parameters);
void rt_start();

uint8_t rt_get_cpu_load(void);

ALWAYS_INLINE static void rt_pend_yield()
{
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
  __asm volatile (
    " dsb    \n\t"
    " isb    \n\t"
  );
}

ALWAYS_INLINE static void rt_cpu_interrupts_enable(void)
{
  __asm volatile (
    " cpsie if    \n\t"
  );
}

ALWAYS_INLINE static void rt_cpu_interrupts_disable(void)
{
  __asm volatile (
    " cpsid if    \n\t"
  );
}

ALWAYS_INLINE static void rt_kernel_interrupts_disable(void)
{
  uint32_t tmp = RT_MASK_IRQ_PRIO;

  __asm volatile (

    " msr basepri, %0  \n\t"
    :
    : "r" (tmp)
  );
}

ALWAYS_INLINE static void rt_kernel_interrupts_enable(void)
{
  uint32_t tmp = 0;

  __asm volatile (

    " msr basepri, %0  \n\t"
    :
    : "r" (tmp)
  );
}

ALWAYS_INLINE static void rt_systick_enable()
{
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

ALWAYS_INLINE static void rt_systick_disable()
{
  SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

extern volatile rt_task_t volatile current_task;
extern volatile uint32_t next_wakeup_tick;
extern volatile rt_task_t volatile next_wakeup_task;

#endif /* RT_KERNEL_H_ */
