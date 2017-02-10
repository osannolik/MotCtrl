/*
 * rt_kernel_config.h
 *
 *  Created on: 22 jan 2017
 *      Author: osannolik
 */

#ifndef RT_KERNEL_CONFIG_H_
#define RT_KERNEL_CONFIG_H_

/* Configure debugging utilities */
#define RT_USE_DEBUG                  (0)

/* Configure performance utilities */
#define RT_USE_PERF                   (1)


/* The kernel interrupts (PendSV and SysTick) should be set to the lowest irq prio.
   In critical sections only irqs with higher prio than RT_MASK_IRQ_PRIO can cause interruption,
   therefore only irq with a prio equal or lower than RT_MASK_IRQ_PRIO may use kernel functions. */
#define RT_KERNEL_IRQ_PRIO            (0x0Fu)
#define RT_MASK_IRQ_PRIO              (0x06u << 4u)

/* The time in ms represented by one tick. */
#define RT_KERNEL_FUNDAMENTAL_TICK_MS (1u)

/* Keep the number of priority levels as few as possible since more levels yields higher overhead.
   A high number means a higher logical priority. The Idle task has a priority of 0. */
#define RT_PRIO_LEVELS                (4u)

/* How much stack should be reserved to the idle task? */
#define RT_IDLE_TASK_STACK_SIZE       (128u)

#if RT_USE_PERF
/* The minimum number of ticks between successive load calculations */
#define PERF_CALC_TICK_PERIOD_MIN     (20)
/* Get this value by running the system with only the idle task */
#define PERF_COUNTS_PER_TICK_NOLOAD   (3996)
#endif /* RT_USE_PERF */

#endif /* RT_KERNEL_CONFIG_H_ */
