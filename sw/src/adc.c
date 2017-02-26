/*
 * adc.c
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#include "adc.h"
#include "ext.h"
#include "fault.h"

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

  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_ADC2_CLK_ENABLE();
  __HAL_RCC_ADC3_CLK_ENABLE();


  AdcHandle_1.Instance                    = ADC1;
  AdcHandle_1.Init.ClockPrescaler         = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle_1.Init.Resolution             = ADC_RESOLUTION12b;
  AdcHandle_1.Init.ScanConvMode           = ENABLE;
  AdcHandle_1.Init.ContinuousConvMode     = DISABLE;
  AdcHandle_1.Init.DiscontinuousConvMode  = DISABLE;
  AdcHandle_1.Init.NbrOfDiscConversion    = 0;
  AdcHandle_1.Init.ExternalTrigConvEdge   = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle_1.Init.ExternalTrigConv       = ADC_EXTERNALTRIG3_T2_CC4;
  AdcHandle_1.Init.DataAlign              = ADC_DATAALIGN_RIGHT;
  AdcHandle_1.Init.NbrOfConversion        = 1;
  AdcHandle_1.Init.DMAContinuousRequests  = DISABLE;
  AdcHandle_1.Init.EOCSelection           = DISABLE;

  AdcHandle_2 = AdcHandle_1;
  AdcHandle_2.Instance                    = ADC2;

  AdcHandle_3 = AdcHandle_1;
  AdcHandle_3.Instance                    = ADC3;

  HAL_ADC_Init(&AdcHandle_1);
  HAL_ADC_Init(&AdcHandle_2);
  HAL_ADC_Init(&AdcHandle_3);


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

  return 0;
}

void ADC_IRQHandler(void)
{
  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_JEOC)) {
    DBG_PAD1_TOGGLE;

    m_adc_i_a   = HAL_ADCEx_InjectedGetValue(&AdcHandle_1, 1);
    m_adc_emf_a = HAL_ADCEx_InjectedGetValue(&AdcHandle_1, 2);
    m_adc_i_b   = HAL_ADCEx_InjectedGetValue(&AdcHandle_2, 1);
    m_adc_emf_b = HAL_ADCEx_InjectedGetValue(&AdcHandle_2, 2);
    m_adc_i_c   = HAL_ADCEx_InjectedGetValue(&AdcHandle_3, 1);
    m_adc_emf_c = HAL_ADCEx_InjectedGetValue(&AdcHandle_3, 2);

    ext_dac_set_value_raw(*adc_debug_variables[p_adc_debug_output_sel]);

    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_JEOC);
  }

  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_OVR)) {
    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_OVR);

    fault_general_failure();
  }
}


