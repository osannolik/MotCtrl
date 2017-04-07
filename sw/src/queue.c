/*
 * queue.c
 *
 *  Created on: 14 nov 2015
 *      Author: osannolik
 */


#include "queue.h"


int queue_Init(queue_t * const q)
{
  if (q == NULL)
    return -1;

  q->idxNew = 0;
  q->idxOld = 0;

  uint32_t i;
  for (i=0; i<QUEUE_PBUF_LEN; i++)
    q->pBuf[i] = (queue_data_type_t) 0;

  return 0;
}

int queue_Push(queue_t * const q, const queue_data_type_t data)
{
  _QUEUE_ENTER_CRITICAL();
  const uint32_t idxNext = (q->idxNew + 1) % QUEUE_PBUF_LEN;

  if (idxNext == q->idxOld) {
    _QUEUE_EXIT_CRITICAL();
    return -1;
  }

  q->pBuf[ q->idxNew ] = (queue_data_type_t) data;
  q->idxNew = idxNext;

  _QUEUE_EXIT_CRITICAL();
  return 0;
}

int queue_Push_range(queue_t * const q, const queue_data_type_t * data, uint32_t n)
{
  _QUEUE_ENTER_CRITICAL();

  if (queue_Available(q) < n) {
    /* If not all fits, push none */
    _QUEUE_EXIT_CRITICAL();
    return -1;
  }

  while (n--) {
    q->pBuf[ q->idxNew ] = (queue_data_type_t) *data++;
    q->idxNew = (q->idxNew + 1) % QUEUE_PBUF_LEN;
  }

  _QUEUE_EXIT_CRITICAL();
  return 0;
}

int queue_Pull(queue_t * const q, queue_data_type_t * const data)
{
  _QUEUE_ENTER_CRITICAL();
  if (q->idxNew == q->idxOld) {
    _QUEUE_EXIT_CRITICAL();
    return -1;
  }

  *data = q->pBuf[ q->idxOld ];
  q->idxOld = (q->idxOld + 1) % QUEUE_PBUF_LEN;

  _QUEUE_EXIT_CRITICAL();
  return 0;
}

int queue_Peek(queue_t * const q, uint32_t n, queue_data_type_t * const data)
{
  _QUEUE_ENTER_CRITICAL();
  n = (q->idxOld + n) % QUEUE_PBUF_LEN;
  if (q->idxNew == n) {
    _QUEUE_EXIT_CRITICAL();
    return -1;
  }

  *data = q->pBuf[n];

  _QUEUE_EXIT_CRITICAL();
  return 0;
}

int queue_Flush(queue_t * const q, uint32_t n)
{
  void *tmp;

  if (q == NULL)
    return -1;

  while (n--)
    queue_Pull(q, (queue_data_type_t*) &tmp);

  return 0;
}

int queue_Available(queue_t * const q)
{
  int available;
  _QUEUE_ENTER_CRITICAL();
  if (q->idxNew >= q->idxOld) {
    available = (QUEUE_PBUF_LEN - q->idxNew + q->idxOld - 1);
  } else {
    available = (q->idxOld - q->idxNew - 1);
  }
  _QUEUE_EXIT_CRITICAL();

  return available;
}

int queue_Occupied_address_range(queue_t * const q, uint8_t **start_address)
{
  int occupied;
  _QUEUE_ENTER_CRITICAL();
  *start_address = (uint8_t *) &(q->pBuf[q->idxOld]);
  if (q->idxNew >= q->idxOld) {
    occupied = (q->idxNew - q->idxOld);
  } else {
    occupied = (QUEUE_PBUF_LEN - q->idxOld);
  }
  _QUEUE_EXIT_CRITICAL();

  return occupied;
}
