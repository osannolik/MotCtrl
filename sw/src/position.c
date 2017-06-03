/*
 * position.c
 *
 *  Created on: 12 mar 2017
 *      Author: osannolik
 */

#include "position.h"
#include "encoder.h"
//#include "utils.h"
#include "filter.h"

#include "calmeas.h"

static filter_speed_pll_t encoder_speed_filter;
static hall_filter_t hall_filter;

/* Measurements */
CALMEAS_SYMBOL(float, m_pos_encoder_speed_filtered, 0.0f, "");
CALMEAS_SYMBOL(float, m_pos_hall_speed_filtered, 0.0f, "");
CALMEAS_SYMBOL(float, m_pos_hall_angle_filtered, 0.0f, "");

/* Parameters */
CALMEAS_SYMBOL(float, p_pos_encoder_filter_Kp, 1500.0f, "");
CALMEAS_SYMBOL(float, p_pos_encoder_filter_Ki, 1000.0f, "");


static uint8_t sensor;


static void hall_filter_init(hall_filter_t * const h);
static void set_hall_filter_speed(hall_filter_t * const h, const float speed);
static void set_hall_filter_angle(hall_filter_t * const h, const float angle);
static void hall_state_change(uint8_t state);
static float get_hall_speed_filtered(hall_filter_t * const h);
static float get_hall_angle_filtered(hall_filter_t * const h);
static float update_hall_speed_filter(hall_filter_t * const h, const float speed_raw);
static float update_hall_angle_filter(hall_filter_t * const h, const float dt);


static void hall_filter_init(hall_filter_t * const h)
{
  set_hall_filter_speed(h, 0.0f);
  h->commutation_counter = 0;
  h->angle_est = 0.0f;
}

static void set_hall_filter_speed(hall_filter_t * const h, const float speed)
{
  uint8_t i;
  for (i=0; i<HALL_NUMBER_OF_COMMUTATIONS; i++) {
    h->speed_raw_per_commutation[i] = speed;
  }

  h->speed_est = 0.0f;
}

static void set_hall_filter_angle(hall_filter_t * const h, const float angle)
{
  h->angle_est = angle;
}

static void hall_state_change(uint8_t state)
{
  (void) state;

  position_update_hall_speed_filter();

  set_hall_filter_angle(&hall_filter, hall_get_angle_raw_rad());
}

static float get_hall_speed_filtered(hall_filter_t * const h)
{
  if (hall_get_speed_raw_radps() == 0.0f) {
    /* When the motor stops (speed=0), there are no more changes in hall state,
     * therefore the filter does not update anymore. Do it manually! */
    set_hall_filter_speed(h, 0.0f);
  }

  return h->speed_est;
}

static float get_hall_angle_filtered(hall_filter_t * const h)
{
  return h->angle_est;
}

static float update_hall_speed_filter(hall_filter_t * const h, const float speed_raw)
{
  h->commutation_counter++;
  if (h->commutation_counter >= HALL_NUMBER_OF_COMMUTATIONS) {
    h->commutation_counter = 0;
  }
  h->speed_raw_per_commutation[h->commutation_counter] = speed_raw;

  float sum = 0.0f;
  uint8_t c;
  for (c=0; c<HALL_NUMBER_OF_COMMUTATIONS; c++) {
    sum += h->speed_raw_per_commutation[c];
  }

  h->speed_est = sum/((float) HALL_NUMBER_OF_COMMUTATIONS);

  return h->speed_est;
}

static float update_hall_angle_filter(hall_filter_t * const h, const float dt)
{
  h->angle_est += (h->speed_est) * dt;

  return h->angle_est;
}

int position_init(const uint8_t source_sensor)
{
  int rv = 0;

  sensor = source_sensor;

  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      hall_filter_init(&hall_filter);
      rv = hall_init();
      hall_set_state_change_indication_cb(hall_state_change);
      break;

    case ENCODER:
      filter_speed_pll_init(&encoder_speed_filter, p_pos_encoder_filter_Kp, p_pos_encoder_filter_Ki);
      rv = encoder_init();
      break;

    default:
      break;
  }

  return rv;
}

float position_get_angle(void)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return get_hall_angle_filtered(&hall_filter);

    case ENCODER:
      return encoder_get_angle_rad();

    default:
      break;
  }

  return 0.0f;
}

float position_get_angle_raw(void)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return hall_get_angle_raw_rad();

    case ENCODER:
      return encoder_get_angle_rad();

    default:
      break;
  }

  return 0.0f;
}

float position_get_speed(void)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return get_hall_speed_filtered(&hall_filter);

    case ENCODER:
      return filter_speed_pll_get_speed(&encoder_speed_filter);

    default:
      break;
  }

  return 0.0f;
}

float position_get_speed_raw(void)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return hall_get_speed_raw_radps();

    case ENCODER:
      return encoder_get_speed_raw_radps();

    default:
      break;
  }

  return 0.0f;
}

float position_update_speed_filter(const float dt)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return position_update_hall_speed_filter();

    case ENCODER:
      return position_update_encoder_speed_filter(dt);

    default:
      break;
  }

  return 0.0f;
}

float position_update_angle_filter(const float dt)
{
  switch (sensor)
  {
    case SENSORLESS:
      break;

    case HALL:
      return position_update_hall_angle_filter(dt);

    case ENCODER:
      break;

    default:
      break;
  }

  return 0.0f;
}

float position_update_encoder_speed_filter(const float dt)
{
  filter_speed_pll_set_gains(&encoder_speed_filter, p_pos_encoder_filter_Kp, p_pos_encoder_filter_Ki);

  m_pos_encoder_speed_filtered = filter_speed_pll_update(&encoder_speed_filter, dt, encoder_get_angle_rad());

  return m_pos_encoder_speed_filtered;
}

float position_update_hall_angle_filter(const float dt)
{
  m_pos_hall_angle_filtered = update_hall_angle_filter(&hall_filter, dt);

  return m_pos_hall_angle_filtered;
}

float position_update_hall_speed_filter(void)
{
  m_pos_hall_speed_filtered = update_hall_speed_filter(&hall_filter, hall_get_speed_raw_radps());

  return m_pos_hall_speed_filtered;
}
