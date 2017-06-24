/*
 * calmeas.c
 *
 *  Created on: 29 aug 2015
 *      Author: osannolik
 */

#include "calmeas.h"


static uint32_t calmeas_update_raster_buffer(const uint8_t raster);
static int calmeas_send_raster(const uint8_t raster);
static uint8_t calmeas_get_measurement(uint8_t * const to_address, const calmeas_meta_t * const meta);
static void calmeas_callback_handler(com_message_t * const msg);
static int calmeas_send_symbol_name(com_message_t * const msg_request);
static int calmeas_send_symbol_description(com_message_t * const msg_request);
static int calmeas_send_raster_periods(void);
static int calmeas_set_raster(com_message_t * const msg);
static int calmeas_send_meas_all(void);
static int calmeas_send_meas_meta(void);


#if 0 /* Not implemented */
// Packet header placed byte-aligned right in front of measurement symbols.
// This allows a full packet to be constructed in RAM, hence minimizing transfer overhead using e.g. DMA
//com_header_t calmeas_measurement_streaming_header __attribute__ ((section(".meas_hdr"), aligned(1))) = {{0,0}};
#endif

static calmeas_data_t calmeas_data = CALMEAS_DATA_INIT;
static calmeas_meta_t* calmeas_meta_ptr = (calmeas_meta_t *) &_smeas_meta;

static uint8_t calmeas_rasters_meta_idx[CALMEAS_RASTERS_NBR][CALMEAS_RASTERS_LEN_MAX];
static uint8_t calmeas_rasters_len[CALMEAS_RASTERS_NBR];
static uint8_t calmeas_rasters_buf[CALMEAS_RASTERS_NBR][CALMEAS_RASTERS_BUF_LEN];
static const uint32_t calmeas_rasters_periods[CALMEAS_RASTERS_NBR] = {CALMEAS_RASTER_0_PERIOD, CALMEAS_RASTER_1_PERIOD, CALMEAS_RASTER_2_PERIOD};


int calmeas_init(void)
{
  calmeas_data.nbr_of_meas = ((uint32_t)&_emeas_meta - (uint32_t)(&_smeas_meta))/sizeof(calmeas_meta_t);
  
#if 0 /* Not implemented */
  // Prepare headers
  //calmeas_measurement_streaming_header.interface = CALMEAS_INTERFACE;
  //calmeas_measurement_streaming_header.id = MEAS_STREAM_ALL;
  //calmeas_measurement_streaming_header.size = (uint8_t) ((uint32_t)&_emeas - (uint32_t)(&_smeas));
#endif

  // Create interfaces
  com_enable_interface(CALMEAS_INTERFACE, calmeas_callback_handler);

  calmeas_rasters_len[RASTER_0] = 0;
  calmeas_rasters_len[RASTER_1] = 0;
  calmeas_rasters_len[RASTER_2] = 0;

  calmeas_data.is_init = 1;

  return 0;
}

int calmeas_handler(void)
{
  static uint32_t raster_0_cntr = 0, raster_1_cntr = 0, raster_2_cntr = 0;

  if (++raster_0_cntr >= calmeas_rasters_periods[RASTER_0]) {
    raster_0_cntr = 0;
    calmeas_send_raster(RASTER_0);
  }

  if (++raster_1_cntr >= calmeas_rasters_periods[RASTER_1]) {
    raster_1_cntr = 0;
    calmeas_send_raster(RASTER_1);
  }

  if (++raster_2_cntr >= calmeas_rasters_periods[RASTER_2]) {
    raster_2_cntr = 0;
    calmeas_send_raster(RASTER_2);
  }

  return 0;
}

static uint32_t calmeas_update_raster_buffer(const uint8_t raster)
{
  if (raster >= CALMEAS_RASTERS_NBR) {
    return 0;
  }

  uint8_t i, meta_idx;
  uint8_t *buf_ptr = calmeas_rasters_buf[raster];
  uint8_t *buf_end = &(calmeas_rasters_buf[raster][CALMEAS_RASTERS_BUF_LEN-1]);
  calmeas_meta_t *meta;

  *buf_ptr++ = raster;

  for (i=0; (i < calmeas_rasters_len[raster]) && (buf_ptr <= buf_end); i++) {
    meta_idx = calmeas_rasters_meta_idx[raster][i];
    meta = &(calmeas_meta_ptr[meta_idx]);

    buf_ptr += calmeas_get_measurement(buf_ptr, meta);
  }

  return (uint32_t) (buf_ptr - calmeas_rasters_buf[raster]);
}

static int calmeas_send_raster(const uint8_t raster)
{
  if (raster >= CALMEAS_RASTERS_NBR) {
    return -1;
  }

  if (calmeas_rasters_len[raster] == 0) {
    return 0;
  }

  com_message_t msg;

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_RASTER;
  msg.port = calmeas_data.use_port;
  msg.address = calmeas_rasters_buf[raster];
  msg.len = calmeas_update_raster_buffer(raster);

  return com_put_message(&msg);
}

static uint8_t calmeas_get_measurement(uint8_t * const to_address, const calmeas_meta_t * const meta)
{
  uint8_t type_len = (uint8_t) CALMEAS_TYPECODE_LEN(meta->typecode);

  _CALMEAS_ENTER_CRITICAL();
  {

    switch (type_len) {
      case 1:
        *to_address = *((uint8_t *) meta->address);
        break;

      case 2:

        *((uint16_t *) to_address) = *((uint16_t *) meta->address);
        break;

      case 4:
        *((uint32_t *) to_address) = *((uint32_t *) meta->address);
        break;

      default:
        type_len = 0;
        break;
    }

  }
  _CALMEAS_EXIT_CRITICAL();

  return type_len;
}

static void calmeas_callback_handler(com_message_t * const msg)
{
  switch (msg->header.id) {
    case MEAS_ALL:
      // A request to send all measurements
      if (calmeas_send_meas_all()) {
        com_commands_send_error(msg);
      }
      break;

    case MEAS_META:
      // A request to send measurement meta data
      if (calmeas_send_meas_meta()) {
        com_commands_send_error(msg);
      }
      break;

    case MEAS_SYMBOL_NAME:
      // A request to send symbol name string
      if (calmeas_send_symbol_name(msg)) {
        com_commands_send_error(msg);
      }
      break;

    case MEAS_SYMBOL_DESC:
      // A request to send symbol description string
      if (calmeas_send_symbol_description(msg)) {
        com_commands_send_error(msg);
      }
      break;

    case MEAS_RASTER_SET:
      // Set raster content
      if (calmeas_set_raster(msg)) {
        com_commands_send_error(msg);
      }
      break;

    case MEAS_RASTER_PERIODS:
      // A request to send raster periods
      if (calmeas_send_raster_periods()) {
        com_commands_send_error(msg);
      }
      break;

    default:
      break;
  }
}

static int calmeas_send_symbol_name(com_message_t * const msg_request)
{
  com_message_t msg;

  if (msg_request->len != sizeof(uint8_t)) {
    return -1;
  }

  uint8_t meta_idx = *(msg_request->address);
  calmeas_meta_t *meta = &(calmeas_meta_ptr[meta_idx]);

  uint8_t name_length = 0;

  while (meta->symbolname[name_length] != '\0') {
    name_length++;
  }

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_SYMBOL_NAME;
  msg.port = msg_request->port;

  msg.address = (uint8_t *) meta->symbolname;
  msg.len = name_length;

  return com_put_message(&msg);
}

static int calmeas_send_symbol_description(com_message_t * const msg_request)
{
  com_message_t msg;

  if (msg_request->len != sizeof(uint8_t)) {
    return -1;
  }

  uint8_t meta_idx = *(msg_request->address);
  calmeas_meta_t *meta = &(calmeas_meta_ptr[meta_idx]);

  uint8_t desc_length = 0;

  while (meta->description[desc_length] != '\0') {
    desc_length++;
  }

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_SYMBOL_DESC;
  msg.port = msg_request->port;

  msg.address = (uint8_t *) meta->description;
  msg.len = desc_length;

  return com_put_message(&msg);
}

static int calmeas_send_raster_periods(void)
{
  com_message_t msg;

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_RASTER_PERIODS;
  msg.port = calmeas_data.use_port;
  msg.address = (uint8_t *) calmeas_rasters_periods;
  msg.len = ((uint32_t) CALMEAS_RASTERS_NBR*sizeof(*calmeas_rasters_periods));

  return com_put_message(&msg);
}

static int calmeas_set_raster(com_message_t * const msg)
{
  uint8_t *data = msg->address;
  uint8_t i, raster = *data++;

  if (raster>=CALMEAS_RASTERS_NBR || msg->len==0 || (msg->len-1) > CALMEAS_RASTERS_LEN_MAX) {
    return -1;
  }

  calmeas_rasters_len[raster] = (uint8_t) (msg->len - 1);

  for (i=0; i<calmeas_rasters_len[raster]; i++) {
    calmeas_rasters_meta_idx[raster][i] = data[i];
  }

  return 0;
}

static int calmeas_send_meas_all(void)
{
  com_message_t msg;

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_ALL;
  msg.port = calmeas_data.use_port;
  msg.address = (uint8_t *) (&_smeas);
  msg.len = ((uint32_t)&_emeas - (uint32_t)(&_smeas));

  return com_put_message(&msg);
}

static int calmeas_send_meas_meta(void)
{
  com_message_t msg;

  msg.header.interface = CALMEAS_INTERFACE;
  msg.header.id = MEAS_META;
  msg.port = calmeas_data.use_port;
  msg.address = (uint8_t *) (&_smeas_meta);
  msg.len = ((uint32_t)&_emeas_meta - (uint32_t)(&_smeas_meta));

  return com_put_message(&msg);
}

