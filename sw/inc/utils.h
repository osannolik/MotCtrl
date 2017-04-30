/*
 * utils.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef UTILS_H_
#define UTILS_H_

typedef struct {
  float x_prev;
} rate_limit_t;

float rate_limit(rate_limit_t * const r, const float x, const float dx_min, const float dx_max);
void rate_limit_reset(rate_limit_t * const r, const float x0);
inline float saturatef(const float x, const float min, const float max);
int gcd(int a, int b);
int lcm(int a, int b);

#define MIN(x, y)             ((x)>(y) ? (y) : (x))
#define MAX(x, y)             ((x)>(y) ? (x) : (y))
#define ABS(x)                ((x)<(0) ? -(x) : (x))
#define SIGN(x)               ((x)<(0) ? -1 : 1)
#define DEGC_TO_KELVIN(degC)  ((degC)+273.15f)
#define KELVIN_TO_DEGC(K)     ((K)-273.15f)

#endif /* UTILS_H_ */
