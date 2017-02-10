/*
 * rt_kernel_defs.h
 *
 *  Created on: 9 oct 2016
 *      Author: osannolik
 */

#ifndef RT_KERNEL_DEFS_H_
#define RT_KERNEL_DEFS_H_

#define ALWAYS_INLINE inline __attribute__((always_inline))

#define RT_FOREVER_TICK               (0xFFFFFFFFu)

enum {
  RT_NOK = 0,
  RT_OK
};

enum {
  RT_ERR_STARTFAILURE = 0
};

struct item {
  uint32_t value;
  void *reference;
  void *list;
  struct item *prev;
  struct item *next;
};

typedef struct {
  uint32_t len;
  struct item end;
  struct item *iterator;
} list_sorted_t;

typedef struct {
  volatile void *sp;
  void *code_start;
  const char *task_name;
  uint32_t priority;
  uint32_t base_prio;
  uint32_t stack_size;
  uint32_t delay_woken_tick;
  struct item list_item;
  struct item blocked_list_item;
} rt_tcb_t;

typedef rt_tcb_t* rt_task_t;
typedef struct item list_item_t;

#endif /* RT_KERNEL_DEFS_H_ */
