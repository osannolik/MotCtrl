#include "com.h"
#include "uart.h"

static com_port_handler_t com_data[COM_NUMBER_OF_PORTS];
static com_interface_t    com_interface[COM_NUMBER_OF_INTERFACES];

static int com_do_callback(com_message_t *msg);
static void com_commands_callback_handler(com_message_t *msg);

int com_init()
{
  uint8_t i;
  for (i=0; i<COM_NUMBER_OF_PORTS; i++) {
    com_data[i].state = WAIT_FOR_START;
    com_data[i].send_hook = NULL;
    com_data[i].receive_hook = NULL;
    queue_Init(&com_data[i].buffer_tx);
  }

  for (i=0; i<COM_NUMBER_OF_INTERFACES; i++) {
    com_interface[i].is_init = 0;
    com_interface[i].callback = NULL;
  }

  // TODO: Add hooks to other ports here
  com_data[uart].receive_hook  = uart_receive_data;
  com_data[uart].send_hook     = uart_send_data;

  com_enable_interface(COM_INTERFACE, com_commands_callback_handler);

  return 0;
}

int com_enable_interface(uint8_t new_interface, void (*callback)(com_message_t *))
{
  if (new_interface >= COM_NUMBER_OF_INTERFACES || callback == NULL)
    return -1;

  com_interface[new_interface].callback = callback;
  com_interface[new_interface].is_init = 1;

  return 0;
}

int com_disable_interface(uint8_t di_interface)
{
  if (di_interface >= COM_NUMBER_OF_INTERFACES)
    return -1;

  com_interface[di_interface].callback = NULL;
  com_interface[di_interface].is_init = 0;

  return 0;
}

static int com_do_callback(com_message_t *msg)
{
  // Perform callback for registered interfaces
  uint8_t interface = msg->header.interface;
  if (com_interface[interface].is_init && com_interface[interface].callback!=NULL)
    com_interface[interface].callback(msg);

  return 0;
}

int com_receive_message(uint8_t port)
{
  // Called from com_handler
  uint32_t len;
  uint8_t *data;
  
  while ( (len = com_data[port].receive_hook(&data)) ) {
    com_parse_message(data, len, port); // Deconstruct message and do callbacks
  }
  
  return 0;
}

int com_parse_message(uint8_t *data, uint32_t len, uint8_t port)
{
  // Input is either partial or full packet, or a combination
  // Could be called directly from port's rx-handler
  uint32_t i;
  com_message_t *inbox_msg = (com_message_t *) &com_data[port].message;

  for (i=0; i<len; i++) {
    
    switch (com_data[port].state) {
      case WAIT_FOR_START:
        if (data[i] == COM_PACKET_START)
          com_data[port].state = GET_HEADER;
        break;

      case GET_HEADER:
        inbox_msg->header.raw[0] = data[i];
        com_data[port].state = GET_SIZE_1;
        break;

      case GET_SIZE_1:
        inbox_msg->header.size = (com_header_size_t) data[i];
        com_data[port].state = GET_SIZE_2;
        break;

      case GET_SIZE_2:
        inbox_msg->header.size |= (((com_header_size_t) data[i]) << 8);
        inbox_msg->len = 0;
        if (inbox_msg->header.size == 0) {
          com_do_callback(inbox_msg);
          com_data[port].state = WAIT_FOR_START;
        } else {
          com_data[port].state = GET_DATA;
        }
        break;

      case GET_DATA:
        com_data[port].buffer_rx[ inbox_msg->len++ ] = data[i];
        if (inbox_msg->len == inbox_msg->header.size) {
          inbox_msg->address = com_data[port].buffer_rx;
          inbox_msg->port = port;

          com_do_callback(inbox_msg);
          com_data[port].state = WAIT_FOR_START;
        }
        break;

      // Do CRC?

      default:
        break;
    }

  }

  return 0;
}

int com_put_message(com_message_t *msg)
{
  // Called from application.
  // Message will be sent by com_handler
  uint8_t *data = msg->address;
  queue_t *buffer = &com_data[msg->port].buffer_tx;

  if (msg->len + COM_PACKET_OVERHEAD_SIZE > queue_Available(buffer))
    return -1;

  queue_Push(buffer, COM_PACKET_START);
  queue_Push(buffer, msg->header.raw[0]);
  queue_Push(buffer, (queue_data_type_t) (0xFF & msg->len));
  queue_Push(buffer, (queue_data_type_t) (msg->len >> 8));

  uint32_t i;
  for (i=0; i<msg->len; i++)
    queue_Push(buffer, data[i]);

  return 0;
}

int com_send_messages(uint8_t port)
{
  // Tries to send all data in buffer on port
  queue_t *buffer = &com_data[port].buffer_tx;

  uint8_t *start_address;
  uint32_t len, sent_bytes;

  while ( (len = queue_Occupied_address_range(buffer, &start_address)) ) {
    sent_bytes = com_data[port].send_hook(start_address, len);

    if (sent_bytes == 0)
      break;

    queue_Flush(buffer, sent_bytes);
  }

  return 0;
}

int com_handler()
{
  // Call periodically to receive/send messages synchronously

  uint8_t port;

  for (port=0; port<COM_NUMBER_OF_PORTS; port++) {
    com_receive_message(port);
  }


  for (port=0; port<COM_NUMBER_OF_PORTS; port++) {
    com_send_messages(port);
  }

  return 0;
}

int com_send_message_by_address(com_message_t *msg)
{
  // Called from application if message should be sent asynchronously.
  // If com_handler is never called this function need to be used.
  // Note that the user has to (if needed) construct packet overhead manually.
  if (com_data[msg->port].send_hook == NULL)
    return -1;

  if (com_data[msg->port].send_hook(msg->address, msg->len) != msg->len)
    return -1;

  return 0;
}

static void com_commands_callback_handler(com_message_t *msg)
{
  switch (msg->header.id) {
    case COM_WRITE_TO:
      // A request to write data to an address
      if (com_commands_write(msg))
        com_commands_send_error(msg);
      break;

    case COM_READ_FROM:
      // A request to read data from an address
      if (com_commands_read(msg))
        com_commands_send_error(msg);
      break;

    default:
      break;
  }
}

int com_commands_read(com_message_t *msg_request)
{
  com_message_t msg;
  com_commander_memory_range_t *from = (com_commander_memory_range_t *) msg_request->address;

  if (msg_request->len != sizeof(com_commander_memory_range_t))
    return -1;

  msg.header.interface = COM_INTERFACE;
  msg.header.id = COM_READ_FROM;
  msg.port = msg_request->port;

  msg.address = from->address;
  msg.len = from->len;

  return com_put_message(&msg);
}

int com_commands_write(com_message_t *msg_request)
{
  com_commander_memory_range_t *to = (com_commander_memory_range_t *) msg_request->address;
  uint8_t *data = (uint8_t *) ( ((uint8_t *)msg_request->address) + sizeof(com_commander_memory_range_t) );
  uint16_t i;

  if (msg_request->len != sizeof(com_commander_memory_range_t) + to->len)
    return -1;

  for (i=0; i<to->len; i++)
    to->address[i] = data[i];

  return 0;
}

int com_commands_send_error(com_message_t *msg)
{  
  uint8_t data[2] = {msg->header.interface, msg->header.id};
  
  msg->header.interface = COM_INTERFACE;
  msg->header.id = COM_ERROR;
  msg->header.size = 2;
  msg->address = data;
  msg->len = 2;

  return com_put_message(msg);
}
