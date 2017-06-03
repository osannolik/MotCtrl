/*
 * inverter.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include <stdint.h>
#include <stddef.h>

void ivtr_step(uint32_t period_ms);
void ivtr_request_duty_cycle(float duty_req);
int ivtr_init(void);
void ivtr_idle_state(void);
void ivtr_safe_state(void);

#endif /* INVERTER_H_ */
