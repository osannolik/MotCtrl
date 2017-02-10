/*
 * rt_sem.h
 *
 *  Created on: 28 sep 2016
 *      Author: osannolik
 */

#ifndef RT_SEM_H_
#define RT_SEM_H_

#include <stddef.h>
#include <stdint.h>

#include "rt_lists.h"


typedef struct {
  volatile uint32_t counter;
  list_sorted_t blocked;
} rt_sem_t;

void rt_sem_init(rt_sem_t *sem, uint32_t count);
uint32_t rt_sem_take_from_isr(rt_sem_t *sem);
uint32_t rt_sem_give_from_isr(rt_sem_t *sem);
uint32_t rt_sem_take(rt_sem_t *sem, const uint32_t ticks_timeout);
uint32_t rt_sem_give(rt_sem_t *sem);

#endif /* RT_SEM_H_ */
