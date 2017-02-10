/*
 * utils.h
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#ifndef UTILS_H_
#define UTILS_H_

int gcd(int a, int b);
int lcm(int a, int b);

#define MIN(x, y) ((x)>(y) ? (y) : (x))
#define MAX(x, y) ((x)>(y) ? (x) : (y))
#define ABS(x)    ((x)<(0) ? -(x) : (x))
#define SIGN(x)   ((x)<(0) ? -1 : 1)

#endif /* UTILS_H_ */
