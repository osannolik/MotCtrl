/*
 * rt_queue.c
 *
 *  Created on: 9 oct 2016
 *      Author: osannolik
 */

#include "rt_queue.h"


uint32_t rt_queue_init(rt_queue_t *queue, uint8_t *buffer, uint32_t item_size, uint32_t max_items)
{
  queue->buffer_start = buffer;
  queue->buffer_end = buffer + item_size * max_items - 1;
  queue->old = buffer;
  queue->next = buffer;
  queue->item_size = item_size;
  queue->max_items = max_items;
  queue->items = 0;

  list_sorted_init(&(queue->blocked_push));
  list_sorted_init(&(queue->blocked_pull));

  return RT_OK;
}

uint32_t rt_queue_push_from_isr(rt_queue_t *queue, const void * const item)
{
  // Assumption: buffer size MUST be item_size*max_items

  uint8_t *queue_buffer;
  uint8_t *item_data;
  uint32_t cnt, task_unblocked = RT_NOK;

  rt_enter_critical();
  {

    if (RT_QUEUE_FULL(queue) == 0) {

      // Do that funky copying!
      queue_buffer = queue->next;
      item_data = (uint8_t *) item;
      cnt = queue->item_size;

      while (cnt--) {
        *queue_buffer++ = *item_data++;
      }

      if (queue_buffer > queue->buffer_end) {
        queue->next = queue->buffer_start;
      } else {
        queue->next = queue_buffer;
      }

      (queue->items)++;

      if (LIST_LENGTH(&(queue->blocked_pull)) > 0) {
        // Unblock the highest prio blocked task
        rt_task_t unblocked_task = (rt_task_t) LIST_MAX_VALUE_REF(&(queue->blocked_pull));
        list_sorted_remove(&(unblocked_task->blocked_list_item));
        rt_list_task_undelayed(unblocked_task);
        rt_list_task_ready_next(unblocked_task);

        if (unblocked_task->priority >= current_task->priority) {
          task_unblocked = RT_OK;
        }
      }
    }

  }
  rt_exit_critical();

  return task_unblocked;
}

uint32_t rt_queue_push(rt_queue_t *queue, const void * const item, const uint32_t ticks_timeout)
{
  // Assumption: buffer size MUST be item_size*max_items

  uint32_t item_pushed = RT_OK;
  uint32_t higher_prio_task_unblocked = RT_NOK;

  rt_enter_critical();
  {

    if (RT_QUEUE_FULL(queue)) {

      list_item_t *blocked_list_item = &(current_task->blocked_list_item);

      // Add currently running task to blocked list
      blocked_list_item->value = current_task->priority;
      list_sorted_insert(&(queue->blocked_push), blocked_list_item);

      // Suspend task for ticks_timeout ticks
      rt_list_task_delayed(current_task, rt_get_tick()+ticks_timeout);

      rt_exit_critical();

      // yields here

      rt_enter_critical();

      // Not blocked anymore, either it was unblocked or it timed out
      list_sorted_remove(&(current_task->blocked_list_item));

      // Is it still full?
      if (RT_QUEUE_FULL(queue)) {
        item_pushed = RT_NOK;
      } else {
        higher_prio_task_unblocked = rt_queue_push_from_isr(queue, item);
      }

    } else {
      higher_prio_task_unblocked = rt_queue_push_from_isr(queue, item);
    }

    if (higher_prio_task_unblocked != RT_NOK) {
      rt_pend_yield();
    }

  }
  rt_exit_critical();

  return item_pushed;
}

uint32_t rt_queue_pull_from_isr(rt_queue_t *queue, void * const item)
{
  uint8_t *queue_buffer;
  uint8_t *item_data;
  uint32_t cnt, task_unblocked = RT_NOK;

  rt_enter_critical();
  {

    if (RT_QUEUE_EMPTY(queue) == 0) {

      // Do that funky copying!
      queue_buffer = queue->old;
      item_data = (uint8_t *) item;
      cnt = queue->item_size;

      while (cnt--) {
        *item_data++ = *queue_buffer++;
      }

      if (queue_buffer > queue->buffer_end) {
        queue->old = queue->buffer_start;
      } else {
        queue->old = queue_buffer;
      }

      (queue->items)--;

      if (LIST_LENGTH(&(queue->blocked_push)) > 0) {
        // Unblock the highest prio blocked task
        rt_task_t unblocked_task = (rt_task_t) LIST_MAX_VALUE_REF(&(queue->blocked_push));
        list_sorted_remove(&(unblocked_task->blocked_list_item));
        rt_list_task_undelayed(unblocked_task);
        rt_list_task_ready_next(unblocked_task);

        if (unblocked_task->priority >= current_task->priority) {
          task_unblocked = RT_OK;
        }
      }
    }

  }
  rt_exit_critical();

  return task_unblocked;
}

uint32_t rt_queue_pull(rt_queue_t *queue, void * const item, const uint32_t ticks_timeout)
{
  uint32_t item_pulled = RT_OK;
  uint32_t higher_prio_task_unblocked = RT_NOK;

  rt_enter_critical();
  {

    if (RT_QUEUE_EMPTY(queue)) {

      list_item_t *blocked_list_item = &(current_task->blocked_list_item);

      // Add currently running task to blocked list
      blocked_list_item->value = current_task->priority;
      list_sorted_insert(&(queue->blocked_pull), blocked_list_item);

      // Suspend task for ticks_timeout ticks
      rt_list_task_delayed(current_task, rt_get_tick()+ticks_timeout);

      rt_exit_critical();

      // yields here

      rt_enter_critical();

      // Not blocked anymore, either it was unblocked or it timed out
      list_sorted_remove(&(current_task->blocked_list_item));

      // Is it still empty?
      if (RT_QUEUE_EMPTY(queue)) {
        item_pulled = RT_NOK;
      } else {
        higher_prio_task_unblocked = rt_queue_pull_from_isr(queue, item);
      }

    } else {
      higher_prio_task_unblocked = rt_queue_pull_from_isr(queue, item);
    }

    if (higher_prio_task_unblocked != RT_NOK) {
      rt_pend_yield();
    }

  }
  rt_exit_critical();

  return item_pulled;
}