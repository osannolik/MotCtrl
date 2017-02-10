/*
 * uart.h
 *
 *  Created on: 29 nov 2015
 *      Author: osannolik
 */

#ifndef UART_H_
#define UART_H_

#include "com.h"
#include "cobs.h"
#include "stm32f4xx_hal.h"

#define UART_FRAME_DELIMITER      0x00
#define UART_FRAME_DELIMITER_LEN  (1)
#define UART_FRAME_OVERHEAD       (COBS_OVERHEAD_MAX + UART_FRAME_DELIMITER_LEN)

#define UART_BUF_TX_LEN           (COBS_DATA_LEN_MAX + UART_FRAME_OVERHEAD) // Use maximum approx 0.8 * baudrate * usart_Handler period / 8, and not less than ...

#define UART_TXDATA_TIMEOUT_MS    (1)
#define UART_INSTANCE             USART6
#define UART_BAUDRATE             (2000000)//(921600)//(230400)
#define UART_IRQ_PRIO             (0x05u)

#define UART_TX_PIN               GPIO_PIN_6
#define UART_TX_PORT              GPIOC
#define UART_RX_PIN               GPIO_PIN_7
#define UART_RX_PORT              GPIOC

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
int uart_init_peripheral();
int uart_init_io();
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);

#endif /* UART_H_ */
