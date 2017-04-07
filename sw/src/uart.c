/*
 * uart.c
 *
 *  Created on: 29 nov 2015
 *      Author: osannolik
 */

#include "stm32f4xx_hal.h"
#include "uart.h"
#include "com.h"


#define MAX(X,Y) ((X)>=(Y)?(X):(Y))

static int uart_init_peripheral(void);
static int uart_init_io(void);

static uart_DataTypeDef uart_data = UART_DATA_INIT;

/* Peripheral data */
static UART_HandleTypeDef UARThandle;
static DMA_HandleTypeDef  DMAhandle_RX;
static DMA_HandleTypeDef  DMAhandle_TX;

/* Buffers */
static uint8_t uart_decode_buffer[RX_BUF_LEN];
static volatile uint8_t dma_buffer_rx[RX_BUF_LEN];
static volatile uint8_t dma_buffer_tx[TX_BUF_LEN];


uint32_t uart_send_data(uint8_t* pData, uint16_t len)
{
  uint8_t *bufEntry;
  int32_t sent_data_len;

  /* Wait until previous tx is done (buffer might currently be accessed by DMA) */
  //uint32_t tickstart = HAL_GetTick();
  //while ((uart_data.TxState == UART_TRANSMITTING));// && ((HAL_GetTick() - tickstart) < UART_TXDATA_TIMEOUT_MS));

  if (uart_data.TxState == UART_TRANSMITTING) {
    return 0;
  }

  if (len > COBS_DATA_LEN_MAX) {
    len = COBS_DATA_LEN_MAX;
  }

  bufEntry = cobs_Encode((uint8_t *) pData, len, (uint8_t *) dma_buffer_tx);
  *bufEntry++ = UART_FRAME_DELIMITER;

  sent_data_len = uart_send_bytes((uint8_t *) dma_buffer_tx, (uint16_t) (bufEntry - dma_buffer_tx));
  sent_data_len -= (bufEntry - dma_buffer_tx - len); // subtract overhead

  return (uint32_t) MAX(0, sent_data_len);
}

uint32_t uart_send_bytes(uint8_t* pData, uint16_t len)
{
  /* Wait until previous tx is done */
  uint32_t tickstart = HAL_GetTick();
  while ((uart_data.TxState == UART_TRANSMITTING) && ((HAL_GetTick() - tickstart) < UART_TXDATA_TIMEOUT_MS));

  if (uart_data.TxState == UART_TRANSMITTING) {
    return 0;
  }

  __HAL_DMA_DISABLE(&DMAhandle_TX);

  /* Set source address and buffer length */
  DMAhandle_TX.Instance->M0AR = (uint32_t) pData;
  DMAhandle_TX.Instance->NDTR = (uint16_t) len;
  
  /* Enable transfer complete interrupt */
  __HAL_DMA_ENABLE_IT(&DMAhandle_TX, DMA_IT_TC);

  /* Enable UART as DMA enabled transmitter */
  UARThandle.Instance->CR3 |= USART_CR3_DMAT;
  
  uart_data.TxState = UART_TRANSMITTING;
  __HAL_DMA_ENABLE(&DMAhandle_TX);

  return (uint32_t) len;
}

static inline uint32_t bytes_in_dma_buffer_rx(const uint32_t i)
{
  const uint32_t end_idx = RX_BUF_LEN - DMAhandle_RX.Instance->NDTR;

  if (end_idx >= i) {
    return end_idx - i;
  } else {
    return RX_BUF_LEN - i + end_idx;
  }
}

uint32_t uart_receive_data(uint8_t **data)
{
  /* TODO: Detect buffer overrun.
   * TODO: Check that the frame length is reasonable. */

  static uint32_t i_prev = 0;
  uint32_t i = i_prev;
  uint32_t i_decode = 0;
  uint32_t decoded_len = 0;

  *data = (uint8_t *) uart_decode_buffer;

  while (bytes_in_dma_buffer_rx(i) > 0) {

    /* Find a delimiter and copy the bytes into the decode buffer. */
    while (dma_buffer_rx[i] != UART_FRAME_DELIMITER) {
      
      uart_decode_buffer[i_decode++] = dma_buffer_rx[i];

      i = (i + 1) % RX_BUF_LEN;

      if (bytes_in_dma_buffer_rx(i) == 0) {
        /* Oops, no more data: 
         * An incomplete frame was found. 
         * Just return whatever got decoded and try again next time. */
        return decoded_len;
      }

    }

    decoded_len += cobs_Decode((uint8_t *) &uart_decode_buffer[decoded_len], \
                               i_decode - decoded_len, \
                               (uint8_t *) &uart_decode_buffer[decoded_len]);
    decoded_len -= UART_FRAME_DELIMITER_LEN;

    /* Continue filling the decode buffer right after the decoded data. */
    i_decode = decoded_len;

    /* Keep track of index to the next byte to be processed,
     * but skip the frame delimiter it currently points to. */
    i = (i + 1) % RX_BUF_LEN;
    i_prev = i;
  }

  return decoded_len;
}

void DMA2_Stream1_IRQHandler(void)
{
  if(DMA2->LISR & DMA_FLAG_TCIF1_5) {
    /* The current buffer is completely filled */
    DMA2->LIFCR = DMA_FLAG_TCIF1_5;
  }
}

void DMA2_Stream7_IRQHandler(void)
{
  if(DMA2->HISR & DMA_FLAG_TCIF3_7) {
    /* When the DMA has moved all data to peripheral */
    DMA2->HIFCR = DMA_FLAG_TCIF3_7;
    uart_data.TxState = UART_IDLE;
  }
}

int uart_init()
{
  int err = 0;

  err |= uart_init_io();
  err |= uart_init_peripheral();

  if (err) {
    return -1;
  }

  uart_data.TxState = UART_IDLE;
  uart_data.isInit = 1;

  return err;
}

static int uart_init_io(void)
{
  GPIO_InitTypeDef GPIOinitstruct;

  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* TX */
  GPIOinitstruct.Pin = UART_TX_PIN;
  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_PULLUP;
  GPIOinitstruct.Mode = GPIO_MODE_AF_PP;
  GPIOinitstruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(UART_TX_PORT, &GPIOinitstruct);

  /* RX */
  GPIOinitstruct.Pin = UART_RX_PIN;
  GPIOinitstruct.Speed = GPIO_SPEED_HIGH;
  GPIOinitstruct.Pull = GPIO_PULLUP;
  GPIOinitstruct.Mode = GPIO_MODE_AF_PP;
  GPIOinitstruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(UART_RX_PORT, &GPIOinitstruct);

  return 0;
}

static int uart_init_peripheral(void)
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
  DMAhandle_RX.Init.Channel             = DMA_CHANNEL_5;
  DMAhandle_RX.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  DMAhandle_RX.Init.Mode                = DMA_CIRCULAR;

  UARThandle.hdmatx = &DMAhandle_TX;
  UARThandle.hdmarx = &DMAhandle_RX;
  HAL_UART_DeInit(&UARThandle);
  HAL_UART_Init(&UARThandle);

  UART_INSTANCE->BRR = (5u << 4u) | (5u); /* Baudrate = 2000000 with SYSCLK=180MHz,
                                           * HAL is not very good at figuring this number out... */

  HAL_DMA_DeInit(&DMAhandle_TX);
  HAL_DMA_Init(&DMAhandle_TX);
  HAL_DMA_DeInit(&DMAhandle_RX);
  HAL_DMA_Init(&DMAhandle_RX);

  NVIC_SetPriority(DMA2_Stream7_IRQn, UART_DMA_TX_IRQ_PRIO);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  NVIC_SetPriority(DMA2_Stream1_IRQn, UART_DMA_RX_IRQ_PRIO);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

  /* Use uart data register as peripheral destination for TX */
  DMAhandle_TX.Instance->PAR = (uint32_t) &(UARThandle.Instance->DR);

  /* Set source and destination address and buffer length */
  DMAhandle_RX.Instance->NDTR = RX_BUF_LEN;
  DMAhandle_RX.Instance->PAR  = (uint32_t) &(UARThandle.Instance->DR);
  DMAhandle_RX.Instance->M0AR = (uint32_t) dma_buffer_rx;

  /* Enable UART as DMA enabled receiver */
  UARThandle.Instance->CR3 |= USART_CR3_DMAR;

  /* Enable transfer complete interrupt */
  __HAL_DMA_ENABLE_IT(&DMAhandle_RX, DMA_IT_TC);

  __HAL_DMA_ENABLE(&DMAhandle_RX);

  return 0;
}
