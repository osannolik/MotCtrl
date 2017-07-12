/*
 * filter.c
 *
 *  Created on: 30 maj 2017
 *      Author: osannolik
 */

#include "filter.h"
#include "math.h"
#include "utils.h"

int filter_speed_pll_init(filter_speed_pll_t * const f, const float kp, const float ki)
{
  filter_speed_pll_reset(f, 0.0f, 0.0f);
  return filter_speed_pll_set_gains(f, kp, ki);
}

void filter_speed_pll_reset(filter_speed_pll_t * const f, const float angle_0, const float speed_0)
{
  f->angle_est = angle_0;
  f->speed_est = speed_0;
  f->integral_e_dt = 0.0f;
}

int filter_speed_pll_set_gains(filter_speed_pll_t * const f, const float kp, const float ki)
{
  if (kp<0.0f || ki<0.0f) {
    return -1;
  }

  f->kp = kp;
  f->ki = ki;

  return 0;
}

float filter_speed_pll_update(filter_speed_pll_t * const f, const float dt, float angle_meas)
{
  angle_meas = wrap_to_range_f(-PI, PI, angle_meas);

  const float e = wrap_to_range_f(-PI, PI, angle_meas - f->angle_est);

  f->integral_e_dt += e * dt;

  f->speed_est = f->kp * e + f->ki * f->integral_e_dt;

  f->angle_est = wrap_to_range_f(-PI, PI, f->angle_est + f->speed_est * dt);

  return f->speed_est;
}

float filter_speed_pll_get_speed(filter_speed_pll_t * const f)
{
  return f->speed_est;
}


inline float filter_iir_update(filter_iir_t * const f, const float x)
{
  float * const s = f->s;
  float * const a = f->a;
  float * const b = f->b;

  /* Transposed Direct-form 2 IIR */
  s[0] = f->inv_a0 * (s[1] + b[0]*x);

  int i;
  for (i = 1; i < f->n - 1; i++) {
    s[i] = s[i+1] + b[i]*x - a[i]*s[0];
  }

  s[i] = b[i]*x - a[i]*s[0];

  return s[0];
}

inline float filter_iir_get_output(filter_iir_t * const f)
{
  return f->s[0];
}

int filter_iir_init(filter_iir_t * const f)
{
  f->inv_a0 = 1.0f / f->a[0];

  filter_iir_reset(f, 0.0f);

  return 0;
}

void filter_iir_reset(filter_iir_t * const f, const float y0)
{
  /* Calculate internal states s assuming steady state output y=y0 */
  int i;
  float sum_a = 0.0f;
  float sum_b = 0.0f;

  for (i=0; i<f->n; i++) {
    sum_a += f->a[i];
    sum_b += f->b[i];
  }

  const float x0 = y0*(sum_a/sum_b); // y0 = dc_gain * x0

  i = f->n-1;
  f->s[i] = f->b[i] * x0 - f->a[i] * y0;

  for (i = f->n-2; i > 0; i--) {
    f->s[i] = f->s[i+1] + f->b[i]*x0 - f->a[i]*y0;
  }

  f->s[0] = y0;
}


int filter_lp_butter_n1_create(filter_lp_butter_n1_t * const f, const float f0, const float fs)
{
  f->iir.n = LP_BUTTER_N1_N;

  /* Pre-warp specified frequency */
  const float Fs = fs/(2.0f*fs*tanf(PI*f0/fs));

  /* Bilinear transformation of 1/(1+T*s) */
  f->a[0] = 1.0f + 2.0f*Fs;
  f->a[1] = 1.0f - 2.0f*Fs;
  f->b[0] = 1.0f;
  f->b[1] = 1.0f;

  f->iir.a = f->a;
  f->iir.b = f->b;
  f->iir.s = f->s;

  return filter_iir_init(&f->iir);
}

float filter_lp_butter_n1_update(filter_lp_butter_n1_t * const f, const float x)
{
  return filter_iir_update(&f->iir, x);
}

float filter_lp_butter_n1_get_output(filter_lp_butter_n1_t * const f)
{
  return filter_iir_get_output(&f->iir);
}

void filter_lp_butter_n1_reset(filter_lp_butter_n1_t * const f, const float y0)
{
  filter_iir_reset(&f->iir, y0);
}


int filter_lp_butter_n2_create(filter_lp_butter_n2_t * const f, const float f0, const float fs)
{
  f->iir.n = LP_BUTTER_N2_N;

  /* Pre-warp specified frequency */
  const float Fs = fs/(2.0f*fs*tanf(PI*f0/fs));

  /* Bilinear transformation of w^2/(s^2+sqrt(2)*w*s+w^2) */
  const float sqrt2 = 1.414213562f;
  f->a[0] = 1.0f + 4.0f*Fs*Fs + 2.0f*sqrt2*Fs;
  f->a[1] = 2.0f - 8.0f*Fs*Fs;
  f->a[2] = 1.0f + 4.0f*Fs*Fs - 2.0f*sqrt2*Fs;
  f->b[0] = 1.0f;
  f->b[1] = 2.0f;
  f->b[2] = 1.0f;

  f->iir.a = f->a;
  f->iir.b = f->b;
  f->iir.s = f->s;

  return filter_iir_init(&f->iir);
}

float filter_lp_butter_n2_update(filter_lp_butter_n2_t * const f, const float x)
{
  return filter_iir_update(&f->iir, x);
}

float filter_lp_butter_n2_get_output(filter_lp_butter_n2_t * const f)
{
  return filter_iir_get_output(&f->iir);
}

void filter_lp_butter_n2_reset(filter_lp_butter_n2_t * const f, const float y0)
{
  filter_iir_reset(&f->iir, y0);
}


int filter_lp_simple_n1_create(filter_lp_simple_n1_t * const f)
{
  filter_lp_simple_n1_reset(f, 0.0f);

  return 0;
}

float filter_lp_simple_n1_update(filter_lp_simple_n1_t * const f, const float dt, const float tau, const float x)
{
  /* First order low-pass filter */
  /* tau is specified time constant */
  /* f0 = 1/(2*pi*tau) */

  f->yn = f->yn + dt*(x-f->yn)/(dt+tau);

  return f->yn;
}

float filter_lp_simple_n1_get_output(filter_lp_simple_n1_t * const f)
{
  return f->yn;
}

void filter_lp_simple_n1_reset(filter_lp_simple_n1_t * const f, const float y0)
{
  f->yn = y0;
}
