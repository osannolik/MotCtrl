/*
 * foc.h
 *
 *  Created on: 25 jun 2017
 *      Author: osannolik
 */

#ifndef FOC_H_
#define FOC_H_


typedef enum {
  ALIGNMENT_NOT_PERFORMED = 0,
  ALIGNMENT_ALPHA_VOLTAGE,
  ALIGNMENT_CURRENT_OFFSET,
  ALIGNMENT_CHECK,
  ALIGNMENT_OK,
  ALIGNMENT_FAILED
} foc_alignment_state_t;

void rec_pos_handler();

void foc_request_current(const float i_d, const float i_q);
foc_alignment_state_t foc_sensor_alignment_state(void);
void foc_sensor_alignment_step(const float period_ms);

void foc_period_by_period_handler(const float dt,
                                  float i_a, float i_b, float i_c,
                                  float emf_a, float emf_b, float emf_c);
int foc_init(void);

#endif /* FOC_H_ */
