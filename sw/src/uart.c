/*
 * uart.c
 *
 *  Created on: 29 nov 2015
 *      Author: osannolik
 */

#include "stm32f4xx_hal.h"
#include "uart.h"
#include "queue.h"
#include "com.h"


#define MAX(X,Y) ((X)>=(Y)?(X):(Y))


static uart_DataTypeDef uart_data = UART_DATA_INIT;

// Peripheral data
static UART_HandleTypeDef UARThandle;
static DMA_HandleTypeDef DMAhandle_TX, DMAhandle_RX;

// Rx data
static volatile uint8_t uart_DecodeBufRx[COBS_FRAME_LEN_MAX];
static volatile queue_t uart_ByteBufRx;

// Tx Data
static volatile uint8_t uart_ByteBufTx[UART_BUF_TX_LEN];


static int uart_delimiter_found();

static int uart_delimiter_found()
{
  // Stub
  // Call com-parser from here for asynch rx
  return 0;
}

uint32_t uart_receive_data(uint8_t **data)
{
  uint32_t len;
  uint8_t d;  

  while (!queue_IsEmpty(&uart_ByteBufRx)) {

    len = 0;
    d = ~UART_FRAME_DELIMITER;  // Something != delimiter
    
    // Peek the received bytes and try to find packet delimiter
    do {
      if (queue_Peek(&uart_ByteBufRx, len, &d))
        return 0;
      uart_DecodeBufRx[len++] = d;
    } while ((d != UART_FRAME_DELIMITER) && (len < COBS_FRAME_LEN_MAX));

    if ((d == UART_FRAME_DELIMITER) && (len >= COM_HDR_SIZE + COBS_OVERHEAD_MAX + UART_FRAME_DELIMITER_LEN)) {
      // Delimiter found but we should not expect frame with 
      // size less than packet header + overhead byte
      queue_Flush(&uart_ByteBufRx, len);
      *data = (uint8_t *) uart_DecodeBufRx;
      return (cobs_Decode((uint8_t *) uart_DecodeBufRx, len-UART_FRAME_DELIMITER_LEN, (uint8_t *) uart_DecodeBufRx) - COBS_OVERHEAD_MAX);
      
    } else if ((len == COBS_FRAME_LEN_MAX) || (len<COM_HDR_SIZE + COBS_OVERHEAD_MAX + UART_FRAME_DELIMITER_LEN)) {
      // Too many delimiters sent, or a lot of crap?
      queue_Flush(&uart_ByteBufRx, len);
    } 

  }

  return 0;
}

uint32_t uart_send_data(uint8_t* pData, uint16_t len)
{
  uint8_t *bufEntry;
  int32_t sent_data_len;

  // Wait until previous tx is done (buffer might currently be accessed by DMA)
  //uint32_t tickstart = HAL_GetTick();
  //while ((uart_data.TxState == UART_TRANSMITTING));// && ((HAL_GetTick() - tickstart) < UART_TXDATA_TIMEOUT_MS));

  if (uart_data.TxState == UART_TRANSMITTING)
    return 0;

  if (len > COBS_DATA_LEN_MAX)
    len = COBS_DATA_LEN_MAX;

  bufEntry = cobs_Encode((uint8_t *) pData, len, (uint8_t *) uart_ByteBufTx);
  *bufEntry++ = UART_FRAME_DELIMITER;

  sent_data_len = uart_send_bytes((uint8_t *) uart_ByteBufTx, (uint16_t) (bufEntry - uart_ByteBufTx));
  sent_data_len -= (bufEntry - uart_ByteBufTx - len); // subtract overhead

  return (uint32_t) MAX(0, sent_data_len);
}

uint32_t uart_send_bytes(uint8_t* pData, uint16_t len)
{
  // Wait until previous tx is done
  uint32_t tickstart = HAL_GetTick();
  while ((uart_data.TxState == UART_TRANSMITTING) && ((HAL_GetTick() - tickstart) < UART_TXDATA_TIMEOUT_MS));

  if (uart_data.TxState == UART_TRANSMITTING)
    return 0;

  __HAL_DMA_DISABLE(&DMAhandle_TX);

  // Set source address and buffer length
  DMAhandle_TX.Instance->M0AR = (uint32_t) pData;
  DMAhandle_TX.Instance->NDTR = (uint16_t) len;
  
  // Enable transfer complete interrupt
  __HAL_DMA_ENABLE_IT(&DMAhandle_TX, DMA_IT_TC);

  // Enable UART as DMA enabled transmitter
  UARThandle.Instance->CR3 |= USART_CR3_DMAT;
  
  uart_data.TxState = UART_TRANSMITTING;
  __HAL_DMA_ENABLE(&DMAhandle_TX);

  return (uint32_t) len;
}

int uart_init()
{
  int err = 0;

  err |= uart_init_io();
  err |= uart_init_peripheral();
  err |= queue_Init(&uart_ByteBufRx);

  if (err)
    return -1;

  uart_data.TxState = UART_IDLE;
  uart_data.isInit = 1;

  return err;
}

int uart_init_peripheral()
{
  __HAL_RCC_USART6_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  UARThandle.Instance                   = UART_INSTANCE;
  UARThandle.Init.BaudRate              = UART_BAUDRATE;
  UARThandle.Init.WordLength            = UART_WORDLENGTH_8B;
  UARThandle.Init.StopBits              = UART_STOPBITS_1;
  UARThandle.Init.Parity                = UART_PARITY_NONE;
  UARThandle.Init.HwFlowCtl             = UART_HWCONTROL_NONE;
  UARThandle.Init.OverSampling          = UART_OVERSAMPLING_8;
  UARThandle.Init.Mode                  = UART_MODE_TX_RX;

  DMAhandle_TX.Instance                 = DMA2_Stream7;
  DMAhandle_TX.Init.Channel             = DMA_CHANNEL_5;
  DMAhandle_TX.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  DMAhandle_TX.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  DMAhandle_TX.Init.MemBurst            = DMA_MBURST_SINGLE;
  DMAhandle_TX.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  DMAhandle_TX.Init.MemInc              = DMA_MINC_ENABLE;
  DMAhandle_TX.Init.Mode                = DMA_NORMAL;
  DMAhandle_TX.Init.PeriphBurst         = DMA_PBURST_SINGLE;
  DMAhandle_TX.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  DMAhandle_TX.Init.PeriphInc           = DMA_PINC_DISABLE;
  DMAhandle_TX.Init.Priority            = DMA_PRIORITY_LOW;

  DMAhandle_RX                          = DMAhandle_TX;
  DMAhandle_RX.Instance                 = DMA2_Stream1;
  DMAhandle_RX.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  DMAhandle_RX.Init.Mode                = DMA_CIRCULAR;

  UARThandle.hdmatx = &DMAhandle_TX;
  //UARThandle.hdmarx = &DMAhandle_RX;
  HAL_UART_DeInit(&UARThandle);
  HAL_UART_Init(&UARThandle);

  UART_INSTANCE->BRR = (5u << 4u) | (5u); // Baudrate = 2000000 with SYSCLK=180MHz,
                                          // HAL is not very good at figuring this number out...

  HAL_DMA_DeInit(&DMAhandle_TX);
  HAL_DMA_Init(&DMAhandle_TX);
  //HAL_DMA_DeInit(&DMAhandle_RX);
  //HAL_DMA_Init(&DMAhandle_RX);

  //HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
  NVIC_SetPriority(DMA2_Stream7_IRQn, UART_IRQ_PRIO);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  //HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
  //HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);


  // Use uart data register as peripheral destination for TX
  DMAhandle_TX.Instance->PAR = (uint32_t) &(UARThandle.Instance->DR);

  // Setup uart module to irq when data was received
  NVIC_SetPriority(USART6_IRQn, UART_IRQ_PRIO);
  __HAL_UART_ENABLE_IT(&UARThandle, UART_IT_RXNE);
  HAL_NVIC_EnableIRQ(USART6_IRQn);

/*
  // Set source and destination address and buffer length
  DMAhandle_RX.Instance->NDTR = 16;
  DMAhandle_RX.Instance->PAR = (uint32_t) &(UARThandle.Instance->DR);
  DMAhandle_RX.Instance->M0AR = (uint32_t) uart_DecodeBufRx;

  // Enable transfer complete interrupt
  __HAL_DMA_ENABLE_IT(&DMAhandle_RX, DMA_IT_TC);

  // Enable UART as DMA enabled receiver
  UARThandle.Instance->CR3 |= USART_CR3_DMAR;

  __HAL_DMA_ENABLE(&DMAhandle_RX);
*/

  return 0;
}

void USART6_IRQHandler()
{
  uint8_t d;
  if (UART_INSTANCE->SR & UART_FLAG_RXNE) {
    queue_Push(&uart_ByteBufRx, d = (uint8_t) UART_INSTANCE->DR);
    if (d == UART_FRAME_DELIMITER)
      uart_delimiter_found();
  }
}

int uart_init_io()
{
  GPIO_InitTypeDef GPIOinitstruct;

  __HAL_RCC_GPIOC_CLK_ENABLE();

  // TX
  GPIOinitstruct.Pin = UART_TX_PIN;
  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_PULLUP;
  GPIOinitstruct.Mode = GPIO_MODE_AF_PP;
  GPIOinitstruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(UART_TX_PORT, &GPIOinitstruct);

  // RX
  GPIOinitstruct.Pin = UART_RX_PIN;
  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_PULLUP;
  GPIOinitstruct.Mode = GPIO_MODE_AF_PP;
  GPIOinitstruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(UART_RX_PORT, &GPIOinitstruct);

  return 0;
}

void DMA2_Stream7_IRQHandler(void)
{
  // When the DMA has moved all data to peripheral
  if(DMA2->HISR & DMA_FLAG_TCIF3_7) {
    DMA2->HIFCR = DMA_FLAG_TCIF3_7;
    uart_data.TxState = UART_IDLE;
  }
}

/*
void DMA1_Stream2_IRQHandler(void)
{
  // When the DMA wraps around 
  if(DMA1->LISR & DMA_FLAG_TCIF2_6)
    DMA1->LIFCR = DMA_FLAG_TCIF2_6;
}
*/
