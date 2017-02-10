/*
 * rt_kernel.c
 *
 *  Created on: 27 aug 2016
 *      Author: osannolik
 */

#include "rt_kernel.h"
#if RT_USE_DEBUG
#include "debug.h"
#endif

static uint32_t rt_kernel_interrupts_init();
static uint32_t * rt_init_stack(void *code, void * const task_parameters, const uint32_t stack_size, volatile void * stack_data);
static void rt_error_handler(uint8_t err);
static uint32_t rt_increment_tick();
#if RT_USE_PERF
static void perf_calculate_cpu_load(uint8_t *load);
#endif /* RT_USE_PERF */

void rt_switch_task();

DEFINE_TASK(rt_idle, idle_task, "IDLE", 0, RT_IDLE_TASK_STACK_SIZE);

volatile rt_task_t volatile       current_task = NULL;

volatile uint32_t        tick = 0;
static volatile uint32_t ticks_in_suspend = 0;
static volatile uint32_t kernel_suspended = 0;
volatile uint32_t        next_wakeup_tick = RT_FOREVER_TICK;
rt_task_t volatile       next_wakeup_task = NULL;
static volatile uint32_t nest_critical = 0;

#if RT_USE_PERF
static uint8_t           perf_cpu_load = 0;
#endif /* RT_USE_PERF */

void rt_idle(void *p)
{
  while (1) {
#if RT_USE_PERF
    perf_calculate_cpu_load(&perf_cpu_load);
#endif /* RT_USE_PERF */
  }
}

#if RT_USE_PERF
static void perf_calculate_cpu_load(uint8_t *load)
{
  static uint32_t last_tick = 0;
  static uint32_t idle_cntr = 0;

  uint32_t tick_delta = tick - last_tick;
  uint32_t counts_per_tick;

  idle_cntr++;

  if (tick_delta >= PERF_CALC_TICK_PERIOD_MIN) {
    counts_per_tick = idle_cntr / tick_delta;
    *load = (uint8_t) 100 - (100 * counts_per_tick) / PERF_COUNTS_PER_TICK_NOLOAD;
    idle_cntr = 0;
    last_tick = tick;
  }
}

uint8_t rt_get_cpu_load(void)
{
  return perf_cpu_load;
}
#endif /* RT_USE_PERF */

void rt_enter_critical(void)
{
  rt_kernel_interrupts_disable();
  ++nest_critical;
}

void rt_exit_critical(void)
{
  if(--nest_critical == 0) {
    rt_kernel_interrupts_enable();
  }
}

void rt_suspend(void)
{
  ++kernel_suspended;
}

void rt_resume(void)
{
  // Note: No check against incorrect nesting of rt_suspend/rt_resume.
  rt_kernel_interrupts_disable();
  {
    --kernel_suspended;

    if (ticks_in_suspend > 0) {
      // Force a yield if the scheduler actually was blocked during the suspension.
      ticks_in_suspend = 0;
      rt_pend_yield();
    }
  }
  rt_kernel_interrupts_enable();
}

uint32_t rt_get_tick(void)
{
  return tick;
}

static uint32_t * rt_init_stack(void *code, void * const task_parameters, const uint32_t stack_size, volatile void * stack_data)
{
  uint32_t *stackptr = (uint32_t *) stack_data;
  stackptr = (uint32_t*) &stackptr[stack_size-1];

  // Needs to be 8-byte aligned
  if ((uint32_t) stackptr & 0x04) {
    stackptr--;
  }

  // PSR
  *stackptr-- = (uint32_t) 0x01000000;

  // PC, as PC is loaded on exit from ISR, bit0 must be zero
  *stackptr-- = (uint32_t) code & 0xFFFFFFFE;

  // LR: Use non-floating point popping and use psp when returning
  *stackptr = (uint32_t) 0xFFFFFFFD;

  // R12, R3, R2, R1
  stackptr -= 5;
  
  // R0
  *stackptr-- = (uint32_t) task_parameters;

  *stackptr = (uint32_t) 0xFFFFFFFD;

  // R11, R10, R9, R8, R7, R6, R5, R4
  stackptr -= 8;

  return stackptr;
}

uint32_t rt_create_task(rt_task_t const task, void * const task_parameters)
{
  uint32_t prio = task->priority;

  if (prio >= RT_PRIO_LEVELS) {
    task->priority = RT_PRIO_LEVELS - 1;
    task->base_prio = RT_PRIO_LEVELS - 1;
  }

  task->sp = rt_init_stack(task->code_start, task_parameters, task->stack_size, task->sp);

  if (task->sp == NULL) {
    return RT_NOK;
  }

  task->list_item.reference = (void *) task;
  task->blocked_list_item.reference = (void *) task;

  // Add to the ready list that corresponds to the task prio
  if (!rt_lists_is_init()) {
    rt_lists_init();
  }
  rt_list_task_ready(task);

  return RT_OK;
}

void rt_yield(void)
{
  rt_pend_yield();
  rt_kernel_interrupts_enable();
}

void rt_periodic_delay(const uint32_t period_ms)
{
  // This does of course not guarantee that the task will execute in the specified period
  // if there are other tasks of higher prio...
  rt_enter_critical();
  {
    uint32_t period_in_ticks = period_ms / RT_KERNEL_FUNDAMENTAL_TICK_MS;
    if (period_in_ticks == 0) {
      period_in_ticks = 1;
    }
    uint32_t task_nominal_wakeup_tick = current_task->delay_woken_tick + period_in_ticks;

    rt_list_task_delayed(current_task, task_nominal_wakeup_tick);
  }
  rt_exit_critical();

  // yields here

  current_task->delay_woken_tick = tick;
}

static uint32_t rt_increment_tick()
{
  // Figure out if context switch is needed and update ready list...

  if (kernel_suspended) {
    ++ticks_in_suspend;
    return RT_NOK;
  }

  ++tick;

  rt_task_t woken_task;
  uint8_t do_context_switch = 0;

  while (tick >= next_wakeup_tick) {
    // Wake up a delayed task
    woken_task = next_wakeup_task;
    
    //woken_task->delay_woken_tick = tick;

    rt_list_task_undelayed(woken_task); // Updates next_wakeup_tick and next_wakeup_task

    // Make it the next one up in its prio ready list
    rt_list_task_ready_next(woken_task);

    // Only trig a switch if the woken task has high enough prio
    if (woken_task->priority >= current_task->priority) {
      do_context_switch = 1;
    }
  }

  // If there are other tasks with the same prio as the current, let them get some cpu time
  if (do_context_switch || LIST_LENGTH(&(ready[current_task->priority])) > 1) {
    return RT_OK;
  } else {
    return RT_NOK;
  }
}

void rt_switch_task()
{
  uint8_t prio;

  rt_kernel_interrupts_disable();
  {
#if RT_USE_DEBUG
    DBG_PAD1_RESET;
    DBG_PAD2_RESET;
    DBG_PAD3_RESET;
#endif /* RT_USE_DEBUG */

    // Pick highest prio of the ready tasks
    for (prio = RT_PRIO_LEVELS-1; LIST_LENGTH(&(ready[prio])) == 0; prio--);

#if RT_USE_DEBUG
    switch (prio) {
      case 0:
        DBG_PAD1_SET;
        break;
      case 1:
        DBG_PAD2_SET;
        break;
      case 2:
        DBG_PAD3_SET;
        break;
      default:
        break;
    }
#endif /* RT_USE_DEBUG */

    // TODO: Check if there actually are any ready tasks?

    // Get the next reference in the ready list
    current_task = (rt_task_t) list_sorted_get_iter_ref((list_sorted_t *) &(ready[prio]));
  }
  rt_kernel_interrupts_enable();
}

static uint32_t rt_kernel_interrupts_init()
{
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); // No sub-priorities

  NVIC_SetPriority(PendSV_IRQn, RT_KERNEL_IRQ_PRIO);    // Set lowest prio
  NVIC_SetPriority(SysTick_IRQn, RT_KERNEL_IRQ_PRIO);

  SysTick->LOAD = (HAL_RCC_GetHCLKFreq()*RT_KERNEL_FUNDAMENTAL_TICK_MS)/1000 - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;

  return RT_OK;
}

void rt_start()
{
  uint8_t prio;

  // Create a kernel idle task with lowest priority
  rt_create_task(&idle_task, NULL);

  // Pick highest prio of the ready tasks as the first to execute
  for (prio=RT_PRIO_LEVELS-1; LIST_LENGTH(&(ready[prio]))==0; prio--) {
    if (prio==0) {
      rt_error_handler(RT_ERR_STARTFAILURE); // Found no ready tasks
    }
  }

  current_task = (rt_task_t) LIST_MIN_VALUE_REF(&(ready[prio]));

  if (rt_kernel_interrupts_init()) {
    // Brace yourselves, the kernel is starting!
    rt_systick_enable();
    rt_kernel_interrupts_enable();
    rt_cpu_interrupts_enable();

    __asm volatile (" svc 0 " );  // Need to be in handler mode to restore correctly => system call
  }

}

void rt_syscall()
{
  __asm volatile (
    " ldr r0, [%[SP]]           \n\t"
    " ldmia r0!, {r4-r11, lr}   \n\t"
    " msr psp, r0               \n\t" // Restore the process stack pointer
    " isb                       \n\t"
    " bx lr                     \n\t"
    : 
    : [SP] "r" (&(current_task->sp))
    : 
  );
}

void rt_systick()
{
  HAL_IncTick();

  rt_kernel_interrupts_disable();
  {
    if (RT_OK==rt_increment_tick()) {
      rt_pend_yield();
    }
  }
  rt_kernel_interrupts_enable();
}

void rt_switch_context()
{
  {
  __asm volatile (
    " mrs r0, psp               \n\t" // psp -> r0
    " isb                       \n\t" // Instruction synchronization barrier
    " tst lr, #0x10             \n\t" // Check LR:
    " it eq                     \n\t" // Push upper floating point regs if used by process
    " vstmdbeq r0!, {s16-s31}   \n\t"
    "                           \n\t"
    " stmdb r0!, {r4-r11, lr}   \n\t" // Push rest of core regs, and specifically lr so that 
    "                           \n\t" // it can be checked by switcher for fp use when restoring
    " str r0, [%[SP]]           \n\t" // Store current stack-pointer to tcb
    " dsb                       \n\t"
    " isb                       \n\t"
    " bl rt_switch_task         \n\t" // Possibly change context to next process
    "                           \n\t"
    :
    : [SP] "r" (&(current_task->sp))
    :
  );
  }

  // Split inline assembly to force re-evaluation of current_task (updated by context switcher)

  {
  __asm volatile (
    " ldr r0, [%[SP]]           \n\t" // Get stack-pointer from tcb
    " ldmia r0!, {r4-r11, lr}   \n\t" // Pop rest of core regs, and specifically lr
    "                           \n\t"
    " tst lr, #0x10             \n\t" // Check LR: 
    " it eq                     \n\t" // Pop upper floating point regs if used by process
    " vldmiaeq r0!, {s16-s31}   \n\t"
    "                           \n\t"
    " msr psp, r0               \n\t" // Restore the process stack pointer
    " isb                       \n\t"
    " bx lr                     \n\t" // Continue next task
    :
    : [SP] "r" (&(current_task->sp))
    :
  );
  }
}

void rt_error_handler(uint8_t err)
{

}
