/*
 * uart.h
 *
 *  Created on: 29 nov 2015
 *      Author: osannolik
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f4xx_hal.h"
#include "cobs.h"

#define UART_TXDATA_TIMEOUT_MS    (1)
#define UART_INSTANCE             USART6
#define UART_BAUDRATE             (2000000)
#define UART_DMA_RX_IRQ_PRIO      (4u)
#define UART_DMA_TX_IRQ_PRIO      (5u)

#define UART_TX_PIN               GPIO_PIN_6
#define UART_TX_PORT              GPIOC
#define UART_RX_PIN               GPIO_PIN_7
#define UART_RX_PORT              GPIOC

#define UART_FRAME_DELIMITER      (0x00)
#define UART_FRAME_DELIMITER_LEN  (1)
#define UART_FRAME_LEN_MAX        (COBS_ENCODED_LEN_MAX + UART_FRAME_DELIMITER_LEN)

#define TX_BUF_LEN                (256u)
#define RX_BUF_LEN                (256u)

typedef enum {
  UART_IDLE = 0,
  UART_TRANSMITTING,
  UART_WAITING
} uart_StateTypeDef;

typedef struct
{
  volatile uart_StateTypeDef TxState;
  uint8_t isInit;
} uart_DataTypeDef;

#define UART_DATA_INIT {UART_IDLE, 0}

uint32_t uart_receive_data(uint8_t **data);
uint32_t uart_send_data(uint8_t* pData, uint16_t len);
uint32_t uart_send_bytes(uint8_t* pData, uint16_t len);
int uart_init();
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);

#endif /* UART_H_ */
