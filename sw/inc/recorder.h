/*
 * recorder.h
 *
 *  Created on: 5 jun 2017
 *      Author: osannolik
 */

#ifndef RECORDER_H_
#define RECORDER_H_


#include <stdint.h>
#include <stddef.h>


typedef struct {
  float * buffer_start;
  float * buffer_end;
  float * input_ptr;
  float * output_ptr;
} recorder_t;


int rec_init(recorder_t * const rec, float * const buffer, const uint32_t len_max);
void rec_start(recorder_t * const rec);
void rec_stop(recorder_t * const rec);
void rec_input(recorder_t * const rec, const float input);
uint8_t rec_output(recorder_t * const rec, float * output);
void rec_play(recorder_t * const rec);

#endif /* RECORDER_H_ */
