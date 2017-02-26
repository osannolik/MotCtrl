/*
 * ext.c
 *
 *  Created on: 18 feb 2017
 *      Author: osannolik
 */

#include "ext.h"

#define BIT_MASK_8  (0xFFu)
#define BIT_MASK_12 (0xFFFu)

static DAC_HandleTypeDef DacHandle;

int ext_init(void)
{
#if (EXT_ADC_DAC_MODE == EXT_DAC)
  ext_dac_init();
#else
  //ext_adc_init();
#endif

  return 0;
}

int ext_dac_init(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  GPIOinitstruct.Pull = GPIO_NOPULL;
  GPIOinitstruct.Mode = GPIO_MODE_ANALOG;

  EXT_ADC_DAC_CLK_EN;

  GPIOinitstruct.Pin = EXT_ADC_DAC_PIN;
  HAL_GPIO_Init(EXT_ADC_DAC_PORT, &GPIOinitstruct);

  __HAL_RCC_DAC_CLK_ENABLE();


  DacHandle.Instance = DAC;
  HAL_DAC_Init(&DacHandle);

  DAC_ChannelConfTypeDef ChannelConf;
  ChannelConf.DAC_Trigger      = DAC_TRIGGER_NONE;
  ChannelConf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;

  HAL_DAC_ConfigChannel(&DacHandle, &ChannelConf, EXT_DAC_CHANNEL);

  ext_dac_set_value_raw(0u);

  HAL_DAC_Start(&DacHandle, EXT_DAC_CHANNEL);

  return 0;
}

void ext_dac_set_value_raw(uint32_t value)
{
#if (EXT_DAC_DATAWIDTH == EXT_DAC_12BIT)
  HAL_DAC_SetValue(&DacHandle, EXT_DAC_CHANNEL, DAC_ALIGN_12B_R, value & BIT_MASK_12);
#else
  HAL_DAC_SetValue(&DacHandle, EXT_DAC_CHANNEL, DAC_ALIGN_8B_R, value & BIT_MASK_8);
#endif
}

