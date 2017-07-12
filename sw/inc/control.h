/*
 * control.h
 *
 *  Created on: 30 apr 2017
 *      Author: osannolik
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <stdint.h>
#include <stddef.h>

enum CTRL_MODE {
  CTRL_MODE_DUTY = 0u,
  CTRL_MODE_SPEED = 1u,
  CTRL_MODE_CURRENT = 2u
};

void ctrl_step(const uint32_t period_ms);
int ctrl_init(void);

#endif /* CONTROL_H_ */
