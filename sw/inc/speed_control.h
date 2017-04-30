/*
 * speed_control.h
 *
 *  Created on: 30 apr 2017
 *      Author: osannolik
 */

#ifndef SPEED_CONTROL_H_
#define SPEED_CONTROL_H_

float spdctrl_controller(const float speed, const float speed_setpoint, const float dt);
void spdctrl_reset(void);
int spdctrl_init(void);

#endif /* SPEED_CONTROL_H_ */
