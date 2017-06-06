/*
 * recorder.c
 *
 *  Created on: 5 jun 2017
 *      Author: osannolik
 */

#include "recorder.h"


int rec_init(recorder_t * const rec, float * const buffer, const uint32_t len_max)
{
  rec->buffer_start = buffer;
  rec->input_ptr = buffer;
  rec->output_ptr = buffer;

  if (buffer == NULL) {
    rec->buffer_end = NULL;
    return -1;
  }

  rec->buffer_end = buffer + len_max;

  rec->input_ptr = rec->buffer_end;
  rec->output_ptr = rec->buffer_end;

  return 0;
}

void rec_start(recorder_t * const rec)
{
  rec->input_ptr = rec->buffer_start;
}

void rec_stop(recorder_t * const rec)
{
  rec->input_ptr = rec->buffer_end;
}

void rec_input(recorder_t * const rec, const float input)
{
  if (rec->input_ptr < rec->buffer_end) {
    *(rec->input_ptr++) = input;
  }
}

uint8_t rec_output(recorder_t * const rec, float * output)
{
  if ((rec->output_ptr < rec->buffer_end) &&
      (rec->output_ptr < rec->input_ptr))
  {
    *output = *(rec->output_ptr++);
    return 1;
  } else {
    return 0;
  }
}

void rec_play(recorder_t * const rec)
{
  rec->output_ptr = rec->buffer_start;
}
