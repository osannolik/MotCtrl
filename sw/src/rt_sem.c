/*
 * rt_sem.c
 *
 *  Created on: 28 sep 2016
 *      Author: osannolik
 */

#include "rt_kernel.h"
#include "rt_sem.h"


void rt_sem_init(rt_sem_t *sem, uint32_t count)
{
  sem->counter = count;
  list_sorted_init((list_sorted_t *) &(sem->blocked));
}

uint32_t rt_sem_take_from_isr(rt_sem_t *sem)
{
  uint32_t sem_taken = RT_OK;

  rt_enter_critical();
  {
    if (sem->counter > 0) {
      (sem->counter)--;
    } else {
      sem_taken = RT_NOK;
    }
  }
  rt_exit_critical();

  return sem_taken;
}

uint32_t rt_sem_give_from_isr(rt_sem_t *sem)
{
  uint32_t task_unblocked = RT_NOK;

  rt_enter_critical();
  {
    (sem->counter)++;

    if (LIST_LENGTH(&(sem->blocked)) > 0) {
      // Unblock the highest prio blocked task
      rt_task_t unblocked_task = (rt_task_t) LIST_MAX_VALUE_REF(&(sem->blocked));
      list_sorted_remove((list_item_t *) &(unblocked_task->blocked_list_item));
      rt_list_task_undelayed(unblocked_task);
      rt_list_task_ready_next(unblocked_task);

      if (unblocked_task->priority >= current_task->priority) {
        task_unblocked = RT_OK;
      }
    }
  }
  rt_exit_critical();

  return task_unblocked;
}

uint32_t rt_sem_take(rt_sem_t *sem, const uint32_t ticks_timeout)
{
  uint32_t sem_taken = RT_OK;

  rt_enter_critical();
  {

    if (sem->counter == 0) {
      list_item_t *blocked_list_item = &(current_task->blocked_list_item);

      // Add currently running task to blocked list
      blocked_list_item->value = current_task->priority;
      list_sorted_insert((list_sorted_t *) &(sem->blocked), blocked_list_item);

      // Suspend task for ticks_timeout ticks
      rt_list_task_delayed(current_task, rt_get_tick()+ticks_timeout);

      rt_exit_critical();

      // yields here

      rt_enter_critical();

      list_sorted_remove(blocked_list_item);

      // Is it still fully taken?
      if (sem->counter == 0) {
        sem_taken = RT_NOK;
      } else {
        (sem->counter)--;
      }
    } else {
      (sem->counter)--;
    }

  }
  rt_exit_critical();

  return sem_taken;
}

uint32_t rt_sem_give(rt_sem_t *sem)
{
  uint32_t higher_prio_task_unblocked;

  rt_enter_critical();
  {
    higher_prio_task_unblocked = rt_sem_give_from_isr(sem);

    if (higher_prio_task_unblocked != RT_NOK) {
      rt_pend_yield();
    }
  }
  rt_exit_critical();

  return higher_prio_task_unblocked;
}
