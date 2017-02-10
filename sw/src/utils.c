/*
 * utils.c
 *
 *  Created on: 5 feb 2017
 *      Author: osannolik
 */

#include "utils.h"

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
