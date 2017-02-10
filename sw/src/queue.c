/*
 * queue.c
 *
 *  Created on: 14 nov 2015
 *      Author: osannolik
 */


#include "queue.h"


int queue_Init(volatile queue_t *q) 
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

int queue_Push(volatile queue_t *q, const queue_data_type_t data) 
{
  uint32_t idxNext = (q->idxNew + 1) % QUEUE_PBUF_LEN;

  if (idxNext == q->idxOld)
    return -1;

  q->pBuf[ q->idxNew ] = (queue_data_type_t) data;
  q->idxNew = idxNext;

  return 0;
}

int queue_Pull(volatile queue_t *q, queue_data_type_t *data) 
{
  if (q->idxNew == q->idxOld)
    return -1;

  *data = q->pBuf[ q->idxOld ];
  q->idxOld = (q->idxOld + 1) % QUEUE_PBUF_LEN;

  return 0;
}

int queue_Peek(volatile queue_t *q, uint32_t n, queue_data_type_t *data) 
{
  n = (q->idxOld + n) % QUEUE_PBUF_LEN;
  if (q->idxNew == n)
    return -1;

  *data = q->pBuf[n];

  return 0;
}

int queue_Flush(volatile queue_t *q, uint32_t n) 
{
  void *tmp;

  if (q == NULL)
    return -1;

  while (n--)
    queue_Pull(q, (queue_data_type_t*) &tmp);

  return 0;
}

int queue_Available(volatile queue_t *q)
{
  if (q->idxNew >= q->idxOld) {
    return (QUEUE_PBUF_LEN - q->idxNew + q->idxOld - 1);
  } else {
    return (q->idxOld - q->idxNew - 1);
  }
}

int queue_Occupied_address_range(volatile queue_t *q, uint8_t **start_address)
{
  *start_address = (uint8_t *) &(q->pBuf[q->idxOld]);
  if (q->idxNew >= q->idxOld) {
    return (q->idxNew - q->idxOld);
  } else {
    return (QUEUE_PBUF_LEN - q->idxOld);
  }
}
