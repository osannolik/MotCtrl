/*
 * queue.h
 *
 *  Created on: 14 nov 2015
 *      Author: osannolik
 */

#ifndef QUEUE_H_
#define QUEUE_H_


#include <stdint.h>
#include <stddef.h>

#include "rt_kernel.h"

#define QUEUE_PBUF_LEN (8192)

#define _QUEUE_ENTER_CRITICAL() rt_enter_critical()
#define _QUEUE_EXIT_CRITICAL()  rt_exit_critical()

typedef uint8_t queue_data_type_t;

typedef struct {
  queue_data_type_t pBuf[QUEUE_PBUF_LEN];
  uint32_t idxNew;
  uint32_t idxOld;
} queue_t;

int queue_Init(queue_t * const q);
int queue_Push(queue_t * const q, const queue_data_type_t data);
int queue_Push_range(queue_t * const q, const queue_data_type_t * data, uint32_t len);
int queue_Pull(queue_t * const q, queue_data_type_t * const data);
int queue_Flush(queue_t * const q, uint32_t n);
int queue_Peek(queue_t * const q, uint32_t n, queue_data_type_t * const data);
int queue_Available(queue_t * const q);
int queue_Occupied_address_range(queue_t * const q, uint8_t **start_address);

int queue_Push_2(queue_t * const q, const queue_data_type_t data);

static inline uint8_t queue_IsEmpty(queue_t * const q) {
  uint8_t empty;
  _QUEUE_ENTER_CRITICAL();
  empty = (q->idxNew == q->idxOld);
  _QUEUE_EXIT_CRITICAL();
  return empty;
}

static inline uint8_t queue_IsFull(queue_t * const q) {
  uint8_t full;
  _QUEUE_ENTER_CRITICAL();
  full = (((q->idxNew + 1u) % QUEUE_PBUF_LEN) == q->idxOld);
  _QUEUE_EXIT_CRITICAL();
  return full;
}

#endif /* QUEUE_H_ */
