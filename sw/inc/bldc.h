/*
 * bldc.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef BLDC_H_
#define BLDC_H_

#include <stdint.h>
#include <stddef.h>
#include "position.h"

void bldc_step(uint32_t period_ms);
void bldc_request_duty_cycle(float duty_req);
int bldc_init(void);
void bldc_idle_state(void);
void bldc_safe_state(void);

#endif /* BLDC_H_ */
