/*
 * adc.c
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#include "adc.h"
#include "fault.h"

#include "calmeas.h"


static volatile uint16_t measurement_buffer[ADC_NUMBER_OF_MEAS];
static void (* new_samples_indication_cb)(void) = NULL;

static ADC_HandleTypeDef AdcHandle_1;
static ADC_HandleTypeDef AdcHandle_2;
static ADC_HandleTypeDef AdcHandle_3;

/* Measurements */
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_i_a_raw,   &measurement_buffer[ADC_I_A],   "");
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_i_b_raw,   &measurement_buffer[ADC_I_B],   "");
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_i_c_raw,   &measurement_buffer[ADC_I_C],   "");
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_emf_a_raw, &measurement_buffer[ADC_EMF_A], "");
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_emf_b_raw, &measurement_buffer[ADC_EMF_B], "");
CALMEAS_SYMBOL_BY_ADDRESS(uint16_t, m_adc_emf_c_raw, &measurement_buffer[ADC_EMF_C], "");


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


  __HAL_RCC_TIM2_CLK_ENABLE();

  const uint32_t reg_conv_divider_tot = 2u * HAL_RCC_GetPCLK1Freq()/ADC_REGCONV_SAMPLE_FREQ_HZ;

  TIM_HandleTypeDef TIMhandle;
  TIMhandle.Instance               = TIM2;
  TIMhandle.Init.Period            = reg_conv_divider_tot / ADC_REGCONV_PRESCALER;
  TIMhandle.Init.Prescaler         = ADC_REGCONV_PRESCALER - 1;
  TIMhandle.Init.ClockDivision     = 0;
  TIMhandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TIMhandle.Init.RepetitionCounter = 0;

  HAL_TIM_Base_Init(&TIMhandle);

  TIM_MasterConfigTypeDef TIMMasterConfig;
  TIMMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  TIMMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  HAL_TIMEx_MasterConfigSynchronization(&TIMhandle, &TIMMasterConfig);


  AdcHandle_1.Instance                    = ADC1;
  AdcHandle_1.Init.ClockPrescaler         = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle_1.Init.Resolution             = ADC_RESOLUTION12b;
  AdcHandle_1.Init.ScanConvMode           = ENABLE;
  AdcHandle_1.Init.ContinuousConvMode     = DISABLE;
  AdcHandle_1.Init.DiscontinuousConvMode  = DISABLE;
  AdcHandle_1.Init.NbrOfDiscConversion    = 0;
  AdcHandle_1.Init.ExternalTrigConvEdge   = ADC_EXTERNALTRIGCONVEDGE_RISING;
  AdcHandle_1.Init.ExternalTrigConv       = ADC_EXTERNALTRIGCONV_T2_TRGO;
  AdcHandle_1.Init.DataAlign              = ADC_DATAALIGN_RIGHT;
  AdcHandle_1.Init.NbrOfConversion        = ADC_NUMBER_OF_REGULAR_MEAS;
  AdcHandle_1.Init.DMAContinuousRequests  = ENABLE;
  AdcHandle_1.Init.EOCSelection           = DISABLE;

  AdcHandle_2 = AdcHandle_1;
  AdcHandle_2.Instance                    = ADC2;

  AdcHandle_3 = AdcHandle_1;
  AdcHandle_3.Instance                    = ADC3;

  HAL_ADC_Init(&AdcHandle_1);
  HAL_ADC_Init(&AdcHandle_2);
  HAL_ADC_Init(&AdcHandle_3);

  DMA_HandleTypeDef DmaHandle;
  DmaHandle.Instance                 = DMA2_Stream0;
  DmaHandle.Init.Channel             = DMA_CHANNEL_0;
  DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
  DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
  DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  DmaHandle.Init.Mode                = DMA_CIRCULAR;
  DmaHandle.Init.Priority            = DMA_PRIORITY_LOW;
  DmaHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  DmaHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
  DmaHandle.Init.MemBurst            = DMA_MBURST_SINGLE;
  DmaHandle.Init.PeriphBurst         = DMA_PBURST_SINGLE;

  HAL_DMA_Init(&DmaHandle);

  __HAL_LINKDMA(&AdcHandle_1, DMA_Handle, DmaHandle);


  ADC_ChannelConfTypeDef RegularConf;
  RegularConf.SamplingTime = ADC_SAMPLETIME_480CYCLES;

  RegularConf.Channel = ADC_BAT_SENSE_CH;
  RegularConf.Rank = ADC_BAT_SENSE_RANK;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);

  RegularConf.Channel = ADC_BOARD_TEMP_CH;
  RegularConf.Rank = ADC_BOARD_TEMP_RANK;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);

  RegularConf.Channel = ADC_CHANNEL_TEMPSENSOR;
  RegularConf.Rank = ADC_MCU_TEMP_RANK;
  HAL_ADC_ConfigChannel(&AdcHandle_1, &RegularConf);


  ADC_InjectionConfTypeDef InjectionConf;
  InjectionConf.AutoInjectedConv              = DISABLE;
  InjectionConf.InjectedDiscontinuousConvMode = DISABLE;
  InjectionConf.ExternalTrigInjecConv         = ADC_EXTERNALTRIGINJECCONV_T1_CC4;
  InjectionConf.ExternalTrigInjecConvEdge     = ADC_EXTERNALTRIGINJECCONVEDGE_RISING;
  InjectionConf.InjectedOffset                = 0;
  InjectionConf.InjectedSamplingTime          = ADC_SAMPLETIME_3CYCLES;

  InjectionConf.InjectedNbrOfConversion       = ADC_NUMBER_OF_INJECTED_ADC1;

  InjectionConf.InjectedChannel               = ADC_I_A_CH;
  InjectionConf.InjectedRank                  = ADC_I_A_RANK;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_1, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_A_CH;
  InjectionConf.InjectedRank                  = ADC_EMF_A_RANK;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_1, &InjectionConf);

  InjectionConf.ExternalTrigInjecConv         = ADC_EXTERNALTRIGINJECCONVEDGE_NONE;

  InjectionConf.InjectedNbrOfConversion       = ADC_NUMBER_OF_INJECTED_ADC2;

  InjectionConf.InjectedChannel               = ADC_I_B_CH;
  InjectionConf.InjectedRank                  = ADC_I_B_RANK;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_2, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_B_CH;
  InjectionConf.InjectedRank                  = ADC_EMF_B_RANK;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_2, &InjectionConf);

  InjectionConf.InjectedNbrOfConversion       = ADC_NUMBER_OF_INJECTED_ADC3;

  InjectionConf.InjectedChannel               = ADC_I_C_CH;
  InjectionConf.InjectedRank                  = ADC_I_C_RANK;
  HAL_ADCEx_InjectedConfigChannel(&AdcHandle_3, &InjectionConf);

  InjectionConf.InjectedChannel               = ADC_EMF_C_CH;
  InjectionConf.InjectedRank                  = ADC_EMF_C_RANK;
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

  HAL_ADC_Start_DMA(&AdcHandle_1, (uint32_t*) measurement_buffer, ADC_NUMBER_OF_REGULAR_MEAS);

  HAL_TIM_Base_Start(&TIMhandle);

  return 0;
}
#include "debug.h"
void ADC_IRQHandler(void)
{
  //DBG_PAD3_SET;

  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_JEOC)) {

    measurement_buffer[ADC_I_A]   = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_1, ADC_I_A_RANK);
    measurement_buffer[ADC_EMF_A] = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_1, ADC_EMF_A_RANK);
    measurement_buffer[ADC_I_B]   = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_2, ADC_I_B_RANK);
    measurement_buffer[ADC_EMF_B] = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_2, ADC_EMF_B_RANK);
    measurement_buffer[ADC_I_C]   = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_3, ADC_I_C_RANK);
    measurement_buffer[ADC_EMF_C] = (uint16_t) HAL_ADCEx_InjectedGetValue(&AdcHandle_3, ADC_EMF_C_RANK);

    if (new_samples_indication_cb != NULL) {
      new_samples_indication_cb();
    }

    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_JEOC);
  }

  if(__HAL_ADC_GET_FLAG(&AdcHandle_1, ADC_FLAG_OVR)) {
    __HAL_ADC_CLEAR_FLAG(&AdcHandle_1, ADC_FLAG_OVR);

    fault_general_failure();
  }

  //DBG_PAD3_RESET;
}

float adc_get_measurement(adc_measurement_t m)
{
  return ((float) measurement_buffer[m]) * ADC_VOLTAGE_PER_LSB;
}

uint16_t adc_get_measurement_raw(adc_measurement_t m)
{
  return measurement_buffer[m];
}

void adc_set_new_samples_indication_cb(void (* callback)(void))
{
  new_samples_indication_cb = callback;
}
