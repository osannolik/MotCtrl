/*
 * com.h
 *
 *  Created on: 9 feb 2015
 *      Author: osannolik
 */

#ifndef COM_H_
#define COM_H_

#include <stdint.h>
#include "queue.h"

#define COM_INTERFACE             (0)

#define COM_HDR_SIZE              (3)
#define COM_PACKET_OVERHEAD_SIZE  (1+COM_HDR_SIZE) // 1 is start byte

#define COM_INTERFACE_BITS        (4)
#define COM_ID_BITS               (8-COM_INTERFACE_BITS)
#define COM_NUMBER_OF_INTERFACES  (1<<COM_INTERFACE_BITS)
#define COM_NUMBER_OF_IDS         (1<<COM_ID_BITS)

#define COM_PACKET_START          's'

#define COM_BUFFER_RX_SIZE        (256)

typedef enum {
  COM_ERROR = 0,
  COM_WRITE_TO = 1,
  COM_READ_FROM = 2
} com_id_t;

// TODO: Here you can add additional ports (e.g. CAN, Ethernet...)
enum {
  // Must start at 0 and be sequential 0,1,2... etc
  uart = 0,
  //radio,
  // my_new_port,
  COM_NUMBER_OF_PORTS
};

typedef enum {
  WAIT_FOR_START = 0,
  GET_HEADER,
  GET_SIZE_1,
  GET_SIZE_2,
  GET_DATA
} com_parser_state_t;

typedef uint16_t com_header_size_t;

typedef union {
  uint8_t raw[COM_HDR_SIZE];
  struct {
    uint8_t interface : COM_INTERFACE_BITS;
    uint8_t id        : COM_ID_BITS;
    com_header_size_t size;
  };
} __attribute__((packed)) com_header_t;

typedef struct {
  com_header_t header;
  uint8_t* address;
  uint32_t len;
  uint8_t port;
} __attribute__((packed)) com_message_t;

typedef struct {
  com_parser_state_t state;
  com_message_t message;

  // TODO: If adding your own ports you need to implement these functions.
  // Send: Gets pointer to data and number of bytes to send. Return number of bytes sent. 
  // Receive: Return number of bytes received, and by parameter a pointer to data
  uint32_t (*send_hook)(uint8_t *data, uint16_t len);
  uint32_t (*receive_hook)(uint8_t **data);

  uint8_t buffer_rx[COM_BUFFER_RX_SIZE];
  queue_t buffer_tx;
} com_port_handler_t;

typedef struct {
  void (*callback)(com_message_t *);
  uint8_t is_init;
} com_interface_t;

typedef struct {
  uint8_t *address;
  uint16_t len;
} __attribute__((packed)) com_commander_memory_range_t;


int com_init();
int com_enable_interface(uint8_t new_interface, void (*callback)(com_message_t *));
int com_disable_interface(uint8_t di_interface);

int com_receive_message(uint8_t port);
int com_parse_message(uint8_t *data, uint32_t len, uint8_t port);
int com_put_message(com_message_t *msg);
int com_send_messages(uint8_t port);
int com_handler();
int com_send_message_by_address(com_message_t *msg);
int com_commands_read(com_message_t *msg_request);
int com_commands_write(com_message_t *msg_request);
int com_commands_send_error(com_message_t *msg);

#endif /* COM_H_ */
