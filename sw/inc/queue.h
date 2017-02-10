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


#define QUEUE_PBUF_LEN (8191)

typedef uint8_t queue_data_type_t;

typedef struct {
  queue_data_type_t pBuf[QUEUE_PBUF_LEN];
  uint32_t idxNew;
  uint32_t idxOld;
} queue_t;

int queue_Init(volatile queue_t *q);
int queue_Push(volatile queue_t *q, const queue_data_type_t data);
int queue_Pull(volatile queue_t *q, queue_data_type_t *data);
int queue_Flush(volatile queue_t *q, uint32_t n);
int queue_Peek(volatile queue_t *q, uint32_t n, queue_data_type_t *data);
int queue_Available(volatile queue_t *q);
int queue_Occupied_address_range(volatile queue_t *q, uint8_t **start_address);

static inline uint8_t queue_IsEmpty(volatile queue_t *q) {
  return (q->idxNew == q->idxOld);
}

static inline uint8_t queue_IsFull(volatile queue_t *q) {
  return (((q->idxNew + 1) % QUEUE_PBUF_LEN) == q->idxOld);
}

#endif /* QUEUE_H_ */
