/*
 * rt_lists.h
 *
 *  Created on: 2 oct 2016
 *      Author: osannolik
 */

#ifndef RT_LISTS_H_
#define RT_LISTS_H_

#include <stddef.h>
#include <stdint.h>

#include "rt_kernel.h"



#define LIST_ITEM_INIT {0, NULL, NULL, NULL, NULL}
#define LIST_INIT {0, LIST_ITEM_INIT, NULL}

#define LIST_END_VALUE 0xFFFFFFFF

#define LIST_LENGTH(plist)        (((list_sorted_t *) plist)->len)
// These are only valid when there are >0 items in list:
#define LIST_MAX_ITEM(plist)      (((list_sorted_t *) plist)->end.prev)
#define LIST_MIN_ITEM(plist)      (((list_sorted_t *) plist)->end.next)
#define LIST_MAX_VALUE(plist)     (((list_sorted_t *) plist)->end.prev->value)
#define LIST_MIN_VALUE(plist)     (((list_sorted_t *) plist)->end.next->value)
#define LIST_MAX_VALUE_REF(plist) (((list_sorted_t *) plist)->end.prev->reference)
#define LIST_MIN_VALUE_REF(plist) (((list_sorted_t *) plist)->end.next->reference)

#define LIST_FIRST_ITEM LIST_MIN_ITEM
#define LIST_LAST_ITEM  LIST_MAX_ITEM
#define LIST_FIRST_REF  LIST_MIN_VALUE_REF
#define LIST_LAST_REF   LIST_MAX_VALUE_REF

#define LIST_SET_ITERATOR_TO(item) (((list_sorted_t *) ((list_item_t *) item)->list)->iterator = (list_item_t *) item)

void list_sorted_init(list_sorted_t *list);
void *list_sorted_get_iter_ref(list_sorted_t *list);
uint32_t list_sorted_insert(list_sorted_t *list, list_item_t *item);
uint32_t list_sorted_iter_insert(list_sorted_t *list, list_item_t *item);
uint32_t list_sorted_remove(list_item_t *item);

void rt_lists_init(void);
uint8_t rt_lists_is_init(void);

void rt_list_task_ready(rt_task_t const task);
void rt_list_task_ready_next(rt_task_t const task);

void rt_list_task_delayed(rt_task_t const task, const uint32_t wake_up_tick);
void rt_list_task_undelayed(rt_task_t const task);

extern volatile list_sorted_t ready[RT_PRIO_LEVELS];

#endif /* RT_LISTS_H_ */
