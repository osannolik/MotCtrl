/*
 * utils.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "utils.h"
#include "math.h"

/* TODO: Add initial behavior */
float rate_limit(rate_limit_t * const r, const float x, const float dx_min, const float dx_max)
{
  const float dx = x - r->x_prev;

  r->x_prev = r->x_prev + saturatef(dx, dx_min, dx_max);

  return r->x_prev;
}

void rate_limit_reset(rate_limit_t * const r, const float x0)
{
  r->x_prev = x0;
}

inline float maxf(const float x, const float y)
{
  return x > y ? x : y;
}

inline float minf(const float x, const float y)
{
  return x < y ? x : y;
}

inline float signf(const float x)
{
  return x<0.0f ? -1.0f : 1.0f;
}

inline float saturatef(const float x, const float min, const float max)
{
  return maxf(minf(x,max),min);
}

int saturate_2d_magnitude(float * const x, float * const y, const float max)
{
  const float magnitude = maxf(sqrtf(*x * *x + *y * *y), 1e-12);

  int is_saturated = (int) (magnitude > max);

  if (is_saturated != 0) {
    const float scaling = max/magnitude;
    *x = *x * scaling;
    *y = *y * scaling;
  }

  return is_saturated;
}

#if 0
inline float _vsqrtf(const float x)
{
  float result;

  __asm volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (x) );

  return result;
}
#endif

int gcd(int a, int b) {
  /* Euclidean algorithm */
  int tmp;
  while (b != 0) {
    tmp = b;
    b = a % b;
    a = tmp;
  }

  return a;
}

int lcm(int a, int b) {
  return (a * b / gcd(a, b));
}

float wrap_to_range_f(const float low, const float high, float x)
{
  /* Wrap x into interval [low, high) */
  /* Assumes high > low */

  const float range = high - low;

  if (range > 0.0f) {
    while (x >= high) {
      x -= range;
    }

    while (x < low) {
      x += range;
    }
  } else {
    x = low;
  }

  return x;
}

int wrap_to_range_i(const int low, const int high, int x)
{
  /* Wrap x into interval [low, high) */
  /* Assumes high > low */

  const int range = high - low;

  if (range > 0) {
    while (x >= high) {
      x -= range;
    }

    while (x < low) {
      x += range;
    }
  } else {
    x = low;
  }

  return x;
}

void recursive_mean_reset(recursive_mean_t * const r, const float x0)
{
  r->n = 0.0f;
  r->mean = x0;
}

float recursive_mean_get(recursive_mean_t * const r)
{
  return r->mean;
}

int recursive_mean_samples(recursive_mean_t * const r)
{
  return (int) r->n;
}

float recursive_mean_add(recursive_mean_t * const r, const float x_in)
{
  const float m = r->mean;
  const float n = r->n + 1.0f;

  r->mean = m + (x_in - m) / n;
  r->n = n;

  return r->mean;
}
