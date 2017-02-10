/*
 * rt_queue.h
 *
 *  Created on: 9 oct 2016
 *      Author: osannolik
 */

#ifndef RT_QUEUE_H_
#define RT_QUEUE_H_

#include <stddef.h>
#include <stdint.h>

#include "rt_lists.h"

typedef struct {
  uint8_t *buffer_start;
  uint8_t *buffer_end;
  uint8_t *next;
  uint8_t *old;
  uint32_t items;
  uint32_t max_items;
  uint32_t item_size;
  list_sorted_t blocked_pull;
  list_sorted_t blocked_push;
} rt_queue_t;

#define RT_QUEUE_FULL(pqueue) ( ((rt_queue_t *) (pqueue))->items == ((rt_queue_t *) (pqueue))->max_items )
#define RT_QUEUE_EMPTY(pqueue) ( ((rt_queue_t *) (pqueue))->items == 0 )

uint32_t rt_queue_init(rt_queue_t *queue, uint8_t *buffer, uint32_t item_size, uint32_t max_items);
uint32_t rt_queue_push_from_isr(rt_queue_t *queue, const void * const item);
uint32_t rt_queue_push(rt_queue_t *queue, const void * const item, const uint32_t ticks_timeout);
uint32_t rt_queue_pull_from_isr(rt_queue_t *queue, void * const item);
uint32_t rt_queue_pull(rt_queue_t *queue, void * const item, const uint32_t ticks_timeout);

#endif /* RT_QUEUE_H_ */
