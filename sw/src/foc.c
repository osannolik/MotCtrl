/*
 * foc.c
 *
 *  Created on: 25 jun 2017
 *      Author: osannolik
 */

#include "foc.h"
#include "math.h"
#include "board.h"
#include "pwm.h"

#include "calmeas.h"


#define FOC_FULL_CLARKE (0)


CALMEAS_SYMBOL(float, p_foc_u_alpha_sp, 0.0f, "");

static foc_alignment_state_t alignment_state = ALIGNMENT_NOT_PERFORMED;

static float i_d_setpoint = 0.0f;
static float i_q_setpoint = 0.0f;

inline static void set_current_setpoint(const float i_d_sp, const float i_q_sp)
{
  i_d_setpoint = i_d_sp;
  i_q_setpoint = i_q_sp;
}

void foc_request_current(const float i_d, const float i_q)
{
  if (alignment_state == ALIGNMENT_OK) {
    set_current_setpoint(i_d, i_q);
  }
}

foc_alignment_state_t foc_sensor_alignment_state(void)
{
  return alignment_state;
}

static void clarke(const float a, const float b, const float c,
                   float * const alpha, float * const beta)
{
  /* k = 2/3 */

  (void) c;
  const float one_over_sqrt3 = 0.577350269f;

  *alpha = a;

#if FOC_FULL_CLARKE
  // TODO
#else
  *beta = one_over_sqrt3 * a + 2.0f * one_over_sqrt3 * b;
#endif
}

static void clarke_inv(const float alpha, const float beta,
                       float * const a, float * const b, float * const c)
{
  /* k = 2/3 */

  const float sqrt3_over_two = 0.866025404f;

  const float tmp1 = -0.5f * alpha;
  const float tmp2 = sqrt3_over_two * beta;

  *a = alpha;
  *b = tmp1 + tmp2;
  *c = tmp1 - tmp2;
}

static void park(const float alpha, const float beta, const float angle_rad_0to2pi,
                 float * const d_out, float * const q_out)
{
  const float s = sinf(angle_rad_0to2pi);
  const float c = cosf(angle_rad_0to2pi);

  *d_out =  c * alpha + s * beta;
  *q_out = -s * alpha + c * beta ;
}

static void park_inv(const float d, const float q, const float angle_rad_0to2pi,
                     float * const alpha, float * const beta)
{
  const float s = sinf(angle_rad_0to2pi);
  const float c = cosf(angle_rad_0to2pi);

  *alpha = c * d - s * q;
  *beta  = s * d + c * q;
}

static void phase_voltage_to_duty(const float u_a, const float u_b, const float u_c, const float u_bus,
                                  float * const d_a, float * const d_b, float * const d_c)
{
  /* u_x = u_xn if u_n = 0 */

  float u_to_d_factor;

  if (u_bus > 0.0f) {
    u_to_d_factor = 1.0f / u_bus;
  } else {
    u_to_d_factor = 0.0f;
  }

  *d_a = 0.5f + u_to_d_factor * u_a;
  *d_b = 0.5f + u_to_d_factor * u_b;
  *d_c = 0.5f + u_to_d_factor * u_c;
}



CALMEAS_SYMBOL(float, p_foc_alignment_i_d_sp, 12.0f, "");


#include "encoder.h"


static float i_a_offset = 0.0f;
static float i_b_offset = 0.0f;
static float i_c_offset = 0.0f;

CALMEAS_SYMBOL_BY_ADDRESS(float, m_foc_i_a_offset, &i_a_offset, "");
CALMEAS_SYMBOL_BY_ADDRESS(float, m_foc_i_b_offset, &i_b_offset, "");
CALMEAS_SYMBOL_BY_ADDRESS(float, m_foc_i_c_offset, &i_c_offset, "");

void foc_sensor_alignment_step(const float period_ms)
{
  static uint32_t delay_ms = 0u;
  static recursive_mean_t rmean_ia, rmean_ib, rmean_ic;

  const uint32_t alpha_voltage_delay_time_ms = 2000u;
  const uint32_t current_settle_delay_time_ms = 100u;
  const int nof_current_averaging_samples = 1000u;

  switch (alignment_state) {

    case ALIGNMENT_NOT_PERFORMED:
      alignment_state = ALIGNMENT_ALPHA_VOLTAGE;
      recursive_mean_reset(&rmean_ia, 0.0f);
      recursive_mean_reset(&rmean_ib, 0.0f);
      recursive_mean_reset(&rmean_ic, 0.0f);
      break;

    case ALIGNMENT_ALPHA_VOLTAGE:
      set_current_setpoint(p_foc_alignment_i_d_sp, 0.0f);

      delay_ms += period_ms;
      if (delay_ms >= alpha_voltage_delay_time_ms) {
        encoder_set_angle_rad(0.0f);
        alignment_state = ALIGNMENT_CURRENT_OFFSET;
        delay_ms = 0u;
        set_current_setpoint(0.0f, 0.0f);
      }
      break;

    case ALIGNMENT_CURRENT_OFFSET:
      delay_ms += period_ms;
      if (delay_ms >= current_settle_delay_time_ms) {
        i_a_offset = recursive_mean_add(&rmean_ia, board_ix_volt_to_ampere(adc_get_measurement(ADC_I_A)));
        i_b_offset = recursive_mean_add(&rmean_ib, board_ix_volt_to_ampere(adc_get_measurement(ADC_I_B)));
        i_c_offset = recursive_mean_add(&rmean_ic, board_ix_volt_to_ampere(adc_get_measurement(ADC_I_C)));

        if (recursive_mean_samples(&rmean_ia) >= nof_current_averaging_samples) {
          alignment_state = ALIGNMENT_CHECK;
        }
      }

      break;

    case ALIGNMENT_CHECK:
      alignment_state = ALIGNMENT_OK;
      break;

    case ALIGNMENT_OK:
      break;

    default:
      break;
  }
}



#include "recorder.h"

#define REC_POS_SAMPLES (5000)

float rec_pos_buffer[REC_POS_SAMPLES];
float rec_pos_buffer2[REC_POS_SAMPLES];
float rec_pos_buffer3[REC_POS_SAMPLES];
recorder_t rec_pos;
recorder_t rec_pos2;
recorder_t rec_pos3;

CALMEAS_SYMBOL(float, mdbg_rec_pos_a, 0.0, "");
CALMEAS_SYMBOL(float, mdbg_rec_pos_b, 0.0, "");
CALMEAS_SYMBOL(float, mdbg_rec_pos_c, 0.0, "");
CALMEAS_SYMBOL(uint8_t, pdbg_rec_start, 0, "");
CALMEAS_SYMBOL(uint8_t, pdbg_rec_play, 0, "");

void rec_pos_handler(void)
{
  static uint8_t pdbg_rec_start_prev = 0;

  if (pdbg_rec_start && (pdbg_rec_start != pdbg_rec_start_prev)) {
    rec_start(&rec_pos);
    rec_start(&rec_pos2);
    rec_start(&rec_pos3);
  }
  pdbg_rec_start_prev = pdbg_rec_start;

  static uint8_t pdbg_rec_play_prev = 0;
  if (pdbg_rec_play && (pdbg_rec_play != pdbg_rec_play_prev)) {
    rec_play(&rec_pos);
    rec_play(&rec_pos2);
    rec_play(&rec_pos3);
  }
  pdbg_rec_play_prev = pdbg_rec_play;

  rec_output(&rec_pos, &mdbg_rec_pos_a);
  rec_output(&rec_pos2, &mdbg_rec_pos_b);
  rec_output(&rec_pos3, &mdbg_rec_pos_c);
}


CALMEAS_SYMBOL(float, p_foc_kp, 0.3f, "");
CALMEAS_SYMBOL(float, p_foc_ki, 0.0f, "");

CALMEAS_SYMBOL(float, m_foc_i_d, 0.0f, "");
CALMEAS_SYMBOL(float, m_foc_i_q, 0.0f, "");

CALMEAS_SYMBOL(uint8_t, m_foc_ctrl_is_sat, 0, "");

CALMEAS_SYMBOL(float, p_foc_i_filter_k, 0.5f, "");

#include "position.h"

#include "adc.h"
#include "ext.h"
CALMEAS_SYMBOL(uint8_t,  p_ivtr_debug_output_sel, 0, "");

#include "filter.h"
CALMEAS_SYMBOL(float, p_foc_filter_f0, 100.0f, "");
CALMEAS_SYMBOL(uint8_t, p_foc_filter_sel, 0, "");
CALMEAS_SYMBOL(float, m_foc_ubus_filtered, 0.0f, "");

static filter_lp_butter_n1_t butter_n1;
static filter_lp_butter_n2_t butter_n2;
static filter_lp_simple_n1_t simple_n1;

inline uint32_t f_to_u12(float val, const float maxabs) {
  return (uint32_t) (2048.0f + minf(2047.0f, val * (2047.0f / maxabs)));
}

void foc_period_by_period_handler(const float dt,
                                  float i_a, float i_b, float i_c,
                                  float emf_a, float emf_b, float emf_c)
{
  float angle = position_get_angle();
  const float i_d_sp = i_d_setpoint;
  const float i_q_sp = i_q_setpoint;

  switch (alignment_state) {
    case ALIGNMENT_OK:
      break;

    case ALIGNMENT_ALPHA_VOLTAGE:
      angle = 0.0f;
      break;

    case ALIGNMENT_CURRENT_OFFSET:
      pwm_set_duty_gate_perc(50.0f, 50.0f, 50.0f);
      return;

    default:
      return;
  }

  i_a -= i_a_offset;
  i_b -= i_b_offset;
  i_c -= i_c_offset;

#if 0
  const float i_cm = (i_a + i_b + i_c)/3.0f;
  i_a -= i_cm;
  i_b -= i_cm;
  i_c -= i_cm;
#endif

  i_a = (1-p_foc_i_filter_k)*i_a - p_foc_i_filter_k*(i_b + i_c);
  i_b = (1-p_foc_i_filter_k)*i_b - p_foc_i_filter_k*(i_a + i_c);
  i_c = (1-p_foc_i_filter_k)*i_c - p_foc_i_filter_k*(i_a + i_b);


  float i_alpha, i_beta;

  clarke(i_a, i_b, i_c, &i_alpha, &i_beta);

  float i_d, i_q;

  park(i_alpha, i_beta, angle, &i_d, &i_q);

  m_foc_i_d = i_d;
  m_foc_i_q = i_q;



  float u_d, u_q;
  const float e_d = i_d_sp - i_d;
  const float e_q = i_q_sp - i_q;
  static float int_q = 0.0f;
  static float int_d = 0.0f;



  if (m_foc_ctrl_is_sat) {
    if (int_q < 0.0f) {
      int_q = int_q + maxf(e_q, 0.0f) * p_foc_ki * dt;
    } else {
      int_q = int_q + minf(e_q, 0.0f) * p_foc_ki * dt;
    }
    if (int_d < 0.0f) {
      int_d = int_d + maxf(e_d, 0.0f) * p_foc_ki * dt;
    } else {
      int_d = int_d + minf(e_d, 0.0f) * p_foc_ki * dt;
    }
  } else {
    int_q = int_q + e_q * p_foc_ki * dt;
    int_d = int_d + e_d * p_foc_ki * dt;
  }

  u_d = p_foc_kp * e_d + int_d;
  u_q = p_foc_kp * e_q + int_q;


  const float u_bus_raw = board_bat_voltage();

  switch (p_foc_filter_sel) {
    case 0:
      m_foc_ubus_filtered = filter_lp_simple_n1_update(&simple_n1, dt, 1.0f / (2.0f * PI * p_foc_filter_f0),
                                                       u_bus_raw);
      break;
    case 1:
      m_foc_ubus_filtered = filter_lp_butter_n1_update(&butter_n1, u_bus_raw);
      break;
    case 2:
      m_foc_ubus_filtered = filter_lp_butter_n2_update(&butter_n2, u_bus_raw);
      break;
    default:
      m_foc_ubus_filtered = u_bus_raw;
      break;
  }

  const float u_bus = m_foc_ubus_filtered;


  m_foc_ctrl_is_sat = (uint8_t) saturate_2d_magnitude(&u_d, &u_q, u_bus*0.5f);


  float u_alpha, u_beta;

  park_inv(u_d, u_q, angle, &u_alpha, &u_beta);

  float u_a, u_b, u_c;

  clarke_inv(u_alpha, u_beta, &u_a, &u_b, &u_c);

  float d_a, d_b, d_c;


  phase_voltage_to_duty(u_a, u_b, u_c, u_bus, &d_a, &d_b, &d_c);

  pwm_set_duty_gate_perc(100.0f*d_a, 100.0f*d_b, 100.0f*d_c);



  uint32_t dac_val = 0;

  if (p_ivtr_debug_output_sel <= ADC_EMF_C) {
    dac_val = adc_get_measurement_raw((adc_measurement_t) p_ivtr_debug_output_sel);
  } else {
    switch (p_ivtr_debug_output_sel) {
      case ADC_EMF_C+1: // 9
        dac_val = f_to_u12(i_q, 20.0f);
        break;
      case ADC_EMF_C+2: // 10
        dac_val = f_to_u12(i_d, 20.0f);
        break;
      case ADC_EMF_C+3: // 11
        dac_val = f_to_u12(u_a, 15.0f);
        break;
      case ADC_EMF_C+4: // 12
        dac_val = f_to_u12(u_b, 15.0f);
        break;
      case ADC_EMF_C+5: // 13
        dac_val = f_to_u12(u_c, 15.0f);
        break;
      case ADC_EMF_C+6: // 14
        dac_val = f_to_u12(u_bus_raw, 30.0f);
        break;
      default:
        break;
    }
  }

  ext_dac_set_value_raw(dac_val);
  rec_input(&rec_pos, u_bus_raw);
  rec_input(&rec_pos2, u_bus);
  rec_input(&rec_pos3, i_c);
}

int foc_init(void)
{
  pwm_set_duty_gate_perc(0.0f, 0.0f, 0.0f);

  pwm_set_sample_trigger_perc(0.0f);

  pwm_ch1_complementary_pos();
  pwm_ch2_complementary_pos();
  pwm_ch3_complementary_pos();

  pwm_update_event();

  rec_init(&rec_pos, rec_pos_buffer, REC_POS_SAMPLES);
  rec_init(&rec_pos2, rec_pos_buffer2, REC_POS_SAMPLES);
  rec_init(&rec_pos3, rec_pos_buffer3, REC_POS_SAMPLES);


  filter_lp_butter_n1_create(&butter_n1, p_foc_filter_f0, (float) PWM_FREQUENCY_HZ);
  filter_lp_butter_n2_create(&butter_n2, p_foc_filter_f0, (float) PWM_FREQUENCY_HZ);
  filter_lp_simple_n1_create(&simple_n1);


  return 0;
}
