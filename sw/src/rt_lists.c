/*
 * rt_lists.c
 *
 *  Created on: 2 oct 2016
 *      Author: osannolik
 */

#include "rt_lists.h"


static volatile list_sorted_t delayed[RT_PRIO_LEVELS];
volatile list_sorted_t ready[RT_PRIO_LEVELS];

static void update_next_wakeup(void);
static list_item_t *list_sorted_next_item(list_item_t *item);
static list_item_t *list_sorted_get_iter_item(list_sorted_t *list);

uint8_t list_is_init = 0;

static void update_next_wakeup(void)
{
  uint32_t prio, wake_up_tick, minimum_wake_up_tick = RT_FOREVER_TICK;
  list_sorted_t *delayed_list;
  rt_task_t wake_up_task = NULL;

  for (prio=0; prio<RT_PRIO_LEVELS; prio++) {
    delayed_list = (list_sorted_t *) &(delayed[prio]);
    if (LIST_LENGTH(delayed_list)>0) {
      wake_up_tick = LIST_MIN_VALUE(delayed_list);
      if (wake_up_tick <= minimum_wake_up_tick) {
        minimum_wake_up_tick = wake_up_tick;
        wake_up_task = (rt_task_t) LIST_MIN_VALUE_REF(delayed_list);
      }
    }
  }

  if (wake_up_task != NULL) {
    next_wakeup_task = wake_up_task;
    next_wakeup_tick = minimum_wake_up_tick;
  } else {
    next_wakeup_tick = RT_FOREVER_TICK;
  }
}

static list_item_t *list_sorted_next_item(list_item_t *item)
{
  list_item_t *next_item = item->next;
  // Will get the next item but exclude the end item
  if (next_item == &(((list_sorted_t *) item->list)->end)) {
    return next_item->next;
  } else {
    return next_item;
  }
}

static list_item_t *list_sorted_get_iter_item(list_sorted_t *list)
{
  list_item_t *iter_item = NULL;

  if (list->len > 0) {
    iter_item = list->iterator;
    list->iterator = list_sorted_next_item(iter_item);
  }

  return iter_item;
}

void list_sorted_init(list_sorted_t *list)
{
  list->len = 0;
  list->end.value = LIST_END_VALUE;
  list->end.list = list;
  list->end.prev = &(list->end);
  list->end.next = &(list->end);
  list->end.reference = NULL;
  list->iterator = NULL;
}

void rt_lists_init(void)
{
  uint8_t prio;

  for (prio=0; prio<RT_PRIO_LEVELS; prio++) {
    list_sorted_init((list_sorted_t *) &(ready[prio]));
    list_sorted_init((list_sorted_t *) &(delayed[prio]));
  }

  list_is_init = 1;
}

uint8_t rt_lists_is_init(void) {
  return list_is_init;
}

void rt_list_task_ready(rt_task_t const task)
{
  uint32_t task_prio = task->priority;

  task->list_item.value = task_prio;
  list_sorted_insert((list_sorted_t *) &(ready[task_prio]), &(task->list_item));
}

void rt_list_task_ready_next(rt_task_t const task)
{
  uint32_t task_prio = task->priority;
  list_item_t *list_item = &(task->list_item);

  list_item->value = task_prio;
  list_sorted_iter_insert((list_sorted_t *) &(ready[task_prio]), list_item);
}

void rt_list_task_delayed(rt_task_t const task, const uint32_t wake_up_tick)
{
  list_sorted_t *delayed_list = (list_sorted_t *) &(delayed[task->priority]);
  list_item_t *list_item = &(task->list_item);

  if (wake_up_tick > rt_get_tick()) {
    // Remove from ready list and add to delayed list
    list_sorted_remove(list_item);

    list_item->value = wake_up_tick;
    list_sorted_insert(delayed_list, list_item);

    update_next_wakeup();

    // Trig a task switch
    rt_pend_yield();
  } else {
    
  }
}

void rt_list_task_undelayed(rt_task_t const task)
{
  // NOTE: Does not set task as Ready!
  list_sorted_remove(&(task->list_item));
  update_next_wakeup();
}

void *list_sorted_get_iter_ref(list_sorted_t *list)
{
  list_item_t *next_item = list_sorted_get_iter_item(list);
  
  if (next_item == NULL) {
    return NULL;
  } else {
    return next_item->reference;
  }
}

uint32_t list_sorted_insert(list_sorted_t *list, list_item_t *item)
{
  list_item_t *insert_at = &(list->end);
  uint32_t input_value = item->value;

  item->list = list;

  if (input_value == LIST_END_VALUE) {
    insert_at = list->end.prev;
  } else {
    // Find last maximum value and insert after that
    while (insert_at->next->value <= input_value)
      insert_at = insert_at->next;
  }

  insert_at->next->prev = item;
  item->next = insert_at->next;
  item->prev = insert_at;
  insert_at->next = item;

  if (list->len == 0) {
    list->iterator = item;
  }
  
  return ++(list->len);
}

uint32_t list_sorted_iter_insert(list_sorted_t *list, list_item_t *item)
{
  // Insert the item to where the iterator points
  // NOTE: It is the caller's responsibility to ensure that the position is correct (i.e. sorted)
  list_item_t *insert_at = list->iterator;

  if (list->len == 0) {
    return list_sorted_insert(list, item);
  } else {
    item->list = list;

    insert_at->next->prev = item;
    item->next = insert_at->next;
    item->prev = insert_at;
    insert_at->next = item;

    list->iterator = item;

    return ++(list->len);
  }
}

uint32_t list_sorted_remove(list_item_t *item)
{
  list_sorted_t *from_list = item->list;

  if (from_list != NULL) {
    if (from_list->len == 1) {
      // Removing the last item
      from_list->iterator = NULL;
    } else {
      from_list->iterator = list_sorted_next_item(item);
    }

    item->next->prev = item->prev;
    item->prev->next = item->next;

    (from_list->len)--;
    item->list = NULL;

  } else {
    return 0;
  }

  return from_list->len;
}
