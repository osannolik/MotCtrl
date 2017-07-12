/*
 * filter.h
 *
 *  Created on: 30 maj 2017
 *      Author: osannolik
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>


typedef struct {
  float angle_est;
  float speed_est;
  float integral_e_dt;
  float kp;
  float ki;
} filter_speed_pll_t;

int filter_speed_pll_init(filter_speed_pll_t * const f, const float kp, const float ki);
void filter_speed_pll_reset(filter_speed_pll_t * const f, const float angle_0, const float speed_0);
int filter_speed_pll_set_gains(filter_speed_pll_t * const f, const float kp, const float ki);
float filter_speed_pll_update(filter_speed_pll_t * const f, const float dt, float angle_meas);
float filter_speed_pll_get_speed(filter_speed_pll_t * const f);


typedef struct {
  int n;
  float * a;
  float * b;
  float * s;
  float inv_a0;
} filter_iir_t;

inline float filter_iir_update(filter_iir_t * const f, const float x);
inline float filter_iir_get_output(filter_iir_t * const f);
int filter_iir_init(filter_iir_t * const f);
void filter_iir_reset(filter_iir_t * const f, const float y0);


#define LP_BUTTER_N1_N (2u)

typedef struct {
  float a[LP_BUTTER_N1_N];
  float b[LP_BUTTER_N1_N];
  float s[LP_BUTTER_N1_N];
  filter_iir_t iir;
} filter_lp_butter_n1_t;

int filter_lp_butter_n1_create(filter_lp_butter_n1_t * const f, const float f0, const float fs);
float filter_lp_butter_n1_update(filter_lp_butter_n1_t * const f, const float x);
float filter_lp_butter_n1_get_output(filter_lp_butter_n1_t * const f);
void filter_lp_butter_n1_reset(filter_lp_butter_n1_t * const f, const float y0);


#define LP_BUTTER_N2_N (3u)

typedef struct {
  float a[LP_BUTTER_N2_N];
  float b[LP_BUTTER_N2_N];
  float s[LP_BUTTER_N2_N];
  filter_iir_t iir;
} filter_lp_butter_n2_t;

int filter_lp_butter_n2_create(filter_lp_butter_n2_t * const f, const float f0, const float fs);
float filter_lp_butter_n2_update(filter_lp_butter_n2_t * const f, const float x);
float filter_lp_butter_n2_get_output(filter_lp_butter_n2_t * const f);
void filter_lp_butter_n2_reset(filter_lp_butter_n2_t * const f, const float y0);


typedef struct {
  float yn;
} filter_lp_simple_n1_t;

int filter_lp_simple_n1_create(filter_lp_simple_n1_t * const f);
float filter_lp_simple_n1_update(filter_lp_simple_n1_t * const f, const float dt, const float tau, const float x);
float filter_lp_simple_n1_get_output(filter_lp_simple_n1_t * const f);
void filter_lp_simple_n1_reset(filter_lp_simple_n1_t * const f, const float y0);

#endif /* FILTER_H_ */
