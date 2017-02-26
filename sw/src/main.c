
#include "main.h"
#include "utils.h"
#include "rt_kernel.h"
#include "uart.h"
#include "com.h"
#include "calmeas.h"

#include "board.h"
#include "hall.h"
#include "pwm.h"
#include "adc.h"
#include "ext.h"
#include "bldc.h"
#include "modes.h"

static void SystemClock_Config(void);
static void Error_Handler(void);

/* Tasks */
DEFINE_TASK(logger_task, logger_task_handle, "logger_task", 1, 512);
DEFINE_TASK(application_task, application_task_handle, "application_task", 2, 512);

/* Measurements */
CALMEAS_SYMBOL(uint8_t,  m_cpu_utilization_perc, 0, "");

/* Parameters */
// ...

int main(void)
{
  HAL_Init();

  /* Configure the system clock to 180 MHz */
  SystemClock_Config();

  /* Prevent systick from interrupting before kernel is started */
  rt_systick_disable();

  debug_init();

  board_init();
  hall_init();
  pwm_init();
  adc_init();

  ext_init();

  modes_init();
  bldc_init();

  uart_init();
  com_init();
  calmeas_init();

  /* Start the rtos */
  rt_create_task(&logger_task_handle, NULL);
  rt_create_task(&application_task_handle, NULL);
  rt_start();

  while(1);

  return 0;
}

void application_task(void *p)
{
  uint32_t lcm_ms;
  uint32_t task_period_ms;

  const uint32_t modes_period_ms = 10u;
  const uint32_t bldc_period_ms = 5u;

  task_period_ms = bldc_period_ms;
  lcm_ms = lcm(task_period_ms, bldc_period_ms);

  task_period_ms = MIN(task_period_ms, modes_period_ms);
  lcm_ms = lcm(lcm_ms, modes_period_ms);

  static uint32_t task_ticker = 0;

  while (1) {

    /* Periodically run step functions */

    if (0u == (task_ticker % modes_period_ms)) {
      modes_step(modes_period_ms);
    }

    if (0u == (task_ticker % bldc_period_ms)) {
      bldc_step(bldc_period_ms);
    }

    m_cpu_utilization_perc = rt_get_cpu_load();

    task_ticker += task_period_ms;
    if (task_ticker >= lcm_ms) {
      task_ticker = 0u;
    }

    rt_periodic_delay(task_period_ms);
  }
}

void logger_task(void *p)
{
  uint8_t com_period_counter = 0;

  while (1) {

    rt_periodic_delay(1);

    calmeas_handler();

    if (++com_period_counter > 2) {
      com_period_counter = 0;
      com_handler();
    }

  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 16000000
  *            PLL_M                          = 8 (gives 2 MHz input to PLL which minimizes jitter)
  *            PLL_N                          = 360
  *            PLL_P                          = 4
  *            PLL_Q                          = 15
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 15;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
