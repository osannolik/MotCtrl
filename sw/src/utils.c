/*
 * utils.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "utils.h"

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

inline float saturatef(const float x, const float min, const float max)
{
  return MAX(MIN(x,max),min);
}

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
