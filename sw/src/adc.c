/*
 * adc.c
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#include "adc.h"
#include "ext.h"
#include "fault.h"
#include "pwm.h"
#include "position.h"

#include "debug.h"
#include "calmeas.h"

/* Measurements */
CALMEAS_SYMBOL(uint32_t, m_adc_i_a,   0, "");
CALMEAS_SYMBOL(uint32_t, m_adc_i_b,   0, "");
CALMEAS_SYMBOL(uint32_t, m_adc_i_c,   0, "");
CALMEAS_SYMBOL(uint32_t, m_adc_emf_a, 0, "");
CALMEAS_SYMBOL(uint32_t, m_adc_emf_b, 0, "");
CALMEAS_SYMBOL(uint32_t, m_adc_emf_c, 0, "");

/* Parameters */
CALMEAS_SYMBOL(uint8_t, p_adc_debug_output_sel, 0, "");

static uint32_t * const adc_debug_variables[6] = {
    &m_adc_i_a,
    &m_adc_i_b,
    &m_adc_i_c,
    &m_adc_emf_a,
    &m_adc_emf_b,
    &m_adc_emf_c
};
static ADC_HandleTypeDef AdcHandle_1;
static ADC_HandleTypeDef AdcHandle_2;
static ADC_HandleTypeDef AdcHandle_3;
static DMA_HandleTypeDef DmaHandle;

static uint32_t measurement_buffer[ADC_NUMBER_OF_MEAS];

int adc_init()
{
  GPIO_InitTypeDef GPIOinitstruct;

  GPIOinitstruct.Pull = GPIO_NOPULL;
  GPIOinitstruct.Mode = GPIO_MODE_ANALOG;

  ADC_I_A_GPIO_CLK_EN;
  ADC_I_B_GPIO_CLK_EN;
  ADC_I_C_GPIO_CLK_EN;
  ADC_EMF_A_GPIO_CLK_EN;
  ADC_EMF_B_GPIO_CLK_EN;
  ADC_EMF_C_GPIO_CLK_EN;
  ADC_BOARD_TEMP_GPIO_CLK_EN;
  ADC_BAT_SENSE_GPIO_CLK_EN;

  GPIOinitstruct.Pin = ADC_I_A_PIN;
  HAL_GPIO_Init(ADC_I_A_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_I_B_PIN;
  HAL_GPIO_Init(ADC_I_B_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_I_C_PIN;
  HAL_GPIO_Init(ADC_I_C_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_EMF_A_PIN;
  HAL_GPIO_Init(ADC_EMF_A_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_EMF_B_PIN;
  HAL_GPIO_Init(ADC_EMF_B_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_EMF_C_PIN;
  HAL_GPIO_Init(ADC_EMF_C_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_BOARD_TEMP_PIN;
  HAL_GPIO_Init(ADC_BOARD_TEMP_PORT, &GPIOinitstruct);

  GPIOinitstruct.Pin = ADC_BAT_SENSE_PIN;
  HAL_GPIO_Init(ADC_BAT_SENSE_PORT, &GPIOinitstruct);


  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_ADC2_CLK_ENABLE();
  __HAL_RCC_ADC3_CLK_ENABLE();

  __HAL_RCC_DMA2_CLK_ENABLE();


  AdcHandle_1.Instance                    = ADC1;
  AdcHandle_1.Init.ClockPrescaler         = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle_1.Init.Resolution             = ADC_RESOLUTION12b;
  AdcHandle_1.Init.ScanConvMode           = ENABLE;
  AdcHandle_1.Init.ContinuousConvMode     = ENABLE;
  AdcHandle_1.Init.DiscontinuousConvMode  = DISABLE;
  AdcHandle_1.Init.NbrOfDiscConversion    = 0;
  AdcHandle_1.Init.ExternalTrigConvEdge   = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle_1.Init.ExternalTrigConv       = ADC_EXTERNALTRIG3_T2_CC4;
  AdcHandle_1.Init.DataAlign              = ADC_DATAALIGN_RIGHT;
  AdcHandle_1.Init.NbrOfConversion        = ADC_NUMBER_OF_MEAS;
  AdcHandle_1.Init.DMAContinuousRequests  = ENABLE;
  AdcHandle_1.Init.EOCSelection           = DISABLE;

  AdcHandle_2 = AdcHandle_1;
  AdcHandle_2.Instance                    = ADC2;

  AdcHandle_3 = AdcHandle_1;
  AdcHandle_3.Instance                    = ADC3;

  HAL_ADC_Init(&AdcHandle_1);
  HAL_ADC_Init(&AdcHandle_2);
  HAL_ADC_Init(&AdcHandle_3);


  DmaHandle.Instance                 = DMA2_Stream0;
  DmaHandle.Init.Channel             = DMA_CHANNEL_0;
  DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
  DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
  DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  DmaHandle.Init.Mode                = DMA_CIRCULAR;
  DmaHandle.Init.Priority            = DMA_PRIORITY_LOW;
  DmaHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  DmaHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
  DmaHandle.Init.MemBurst            = DMA_MBURST_SINGLE;
  DmaHandle.Init.PeriphBurst         = DMA_PBURST_SINGLE;

  HAL_DMA_Init(&DmaHandle);

  __HAL_LINKDMA(&AdcHandle_1, DMA_Handle, DmaHandle);


  ADC_ChannelConfTypeDef RegularConf;
  RegularConf.Channel = ADC_BAT_SENSE_CH;
  RegularConf.Rank = 1 + ADC_BAT_SENSE;
  RegularConf.SamplingTime = ADC_SAMPLETIME_112CYCLES;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);

  RegularConf.Channel = ADC_BOARD_TEMP_CH;
  RegularConf.Rank = 1 + ADC_BOARD_TEMP;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);

  RegularConf.Channel = ADC_CHANNEL_TEMPSENSOR;
  RegularConf.Rank = 1 + ADC_MCU_TEMP;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);


  ADC_InjectionConfTypeDef InjectionConf;
  InjectionConf.AutoInjectedConv              = DISABLE;
  InjectionConf.InjectedDiscontinuousConvMode = DISABLE;
  InjectionConf.ExternalTrigInjecConv         = ADC_EXTERNALTRIGINJECCONV_T1_CC4;
  InjectionConf.ExternalTrigInjecConvEdge     = ADC_EXTERNALTRIGINJECCONVEDGE_FALLING;
  InjectionConf.InjectedOffset                = 0;
  InjectionConf.InjectedSamplingTime          = ADC_SAMPLETIME_3CYCLES;
  InjectionConf.InjectedNbrOfConversion       = 2;

  InjectionConf.InjectedChannel               = ADC_I_A_CH;
  InjectionConf.InjectedRank                  = 1;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_1, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_A_CH;
  InjectionConf.InjectedRank                  = 2;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_1, &InjectionConf);

  InjectionConf.ExternalTrigInjecConv         = ADC_EXTERNALTRIGINJECCONVEDGE_NONE;

  InjectionConf.InjectedChannel               = ADC_I_B_CH;
  InjectionConf.InjectedRank                  = 1;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_2, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_B_CH;
  InjectionConf.InjectedRank                  = 2;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_2, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_I_C_CH;
  InjectionConf.InjectedRank                  = 1;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_3, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_C_CH;
  InjectionConf.InjectedRank                  = 2;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_3, &InjectionConf);


  ADC_MultiModeTypeDef MultiMode;
  MultiMode.DMAAccessMode     = ADC_DMAACCESSMODE_DISABLED;
  MultiMode.Mode              = ADC_TRIPLEMODE_INJECSIMULT;
  MultiMode.TwoSamplingDelay  = ADC_TWOSAMPLINGDELAY_5CYCLES;
  HAL_ADCEx_MultiModeConfigChannel(&AdcHandle_1, &MultiMode);

  NVIC_SetPriority(ADC_IRQn, ADC_INJECCONV_IRQ_PRIO);
  NVIC_EnableIRQ(ADC_IRQn);

  HAL_ADCEx_InjectedStart_IT(&AdcHandle_1);
  HAL_ADCEx_InjectedStart_IT(&AdcHandle_2);
  HAL_ADCEx_InjectedStart_IT(&AdcHandle_3);

  HAL_ADC_Start_DMA(&AdcHandle_1, (uint32_t*) measurement_buffer, ADC_NUMBER_OF_MEAS);

  return 0;
}

void ADC_IRQHandler(void)
{
  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_JEOC)) {
    //DBG_PAD1_TOGGLE;

    m_adc_i_a   = HAL_ADCEx_InjectedGetValue(&AdcHandle_1, 1);
    m_adc_emf_a = HAL_ADCEx_InjectedGetValue(&AdcHandle_1, 2);
    m_adc_i_b   = HAL_ADCEx_InjectedGetValue(&AdcHandle_2, 1);
    m_adc_emf_b = HAL_ADCEx_InjectedGetValue(&AdcHandle_2, 2);
    m_adc_i_c   = HAL_ADCEx_InjectedGetValue(&AdcHandle_3, 1);
    m_adc_emf_c = HAL_ADCEx_InjectedGetValue(&AdcHandle_3, 2);

    (void) position_angle_est_update(1.0f/((float)PWM_FREQUENCY_HZ));

    ext_dac_set_value_raw(*adc_debug_variables[p_adc_debug_output_sel]);

    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_JEOC);
  }

  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_OVR)) {
    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_OVR);

    fault_general_failure();
  }
}

float adc_get_measurement(adc_measurement_t m)
{
  return ((float) measurement_buffer[m]) * ADC_VOLTAGE_PER_LSB;
}
