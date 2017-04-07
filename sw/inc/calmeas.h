/*
 * calmeas.h
 *
 *  Created on: 29 aug 2015
 *      Author: osannolik
 */

#ifndef CALMEAS_H_
#define CALMEAS_H_

#include "stm32f4xx.h"
#include "com.h"
#include "rt_kernel.h"

#define _CALMEAS_ENTER_CRITICAL() rt_enter_critical()
#define _CALMEAS_EXIT_CRITICAL()  rt_exit_critical()

// TODO: Linker variables, all not actually needed
extern uint32_t _smeas_hdr;
extern uint32_t _emeas_hdr;
extern uint32_t _smeas;
extern uint32_t _emeas;
extern uint32_t _smeas_meta_hdr;
extern uint32_t _emeas_meta_hdr;
extern uint32_t _smeas_meta;
extern uint32_t _emeas_meta;
extern uint32_t _sparam;
extern uint32_t _eparam;
extern uint32_t _sparamptr;
extern uint32_t _eparamptr;
extern uint32_t _nbrparam;

enum {
  RASTER_0 = 0,
  RASTER_1 = 1,
  RASTER_2 = 2
};

typedef enum {
  ACCESS_OFF = 0,
  ACCESS_RASTER, 
  ACCESS_STREAM
} calmeas_access_type_t;

typedef struct {
  uint8_t typecode;
  char *symbolname;
  void* address;
  char *description;
} __attribute__((__packed__)) calmeas_meta_t;

typedef struct {
  uint8_t is_init;
  uint8_t nbr_of_meas;
  uint8_t use_port;
  calmeas_access_type_t access_type;
} calmeas_data_t;

typedef enum {
  MEAS_META = 0,
  MEAS_ALL,
  MEAS_STREAM_ALL,
  MEAS_RASTER,
  MEAS_RASTER_SET,
  MEAS_SYMBOL_NAME,
  MEAS_SYMBOL_DESC,
  MEAS_RASTER_PERIODS
} calmeas_id_t;

#define CALMEAS_INTERFACE          (3)


#define CALMEAS_DATA_INIT          {0, 0, uart, ACCESS_OFF}

// TODO: Here you can define at what periods each raster should be sent
#define CALMEAS_RASTER_0_PERIOD    (1)  // Relative to calmeas_handler period
#define CALMEAS_RASTER_1_PERIOD    (10)
#define CALMEAS_RASTER_2_PERIOD    (200)

#define CALMEAS_RASTERS_NBR        (3)
#define CALMEAS_RASTERS_LEN_MAX    (256)
#define CALMEAS_RASTERS_BUF_LEN    (1024)

#define CALMEAS_TYPECODE_uint8_t   (0x01)
#define CALMEAS_TYPECODE_int8_t    (0x81)
#define CALMEAS_TYPECODE_uint16_t  (0x02)
#define CALMEAS_TYPECODE_int16_t   (0x82)
#define CALMEAS_TYPECODE_uint32_t  (0x04)
#define CALMEAS_TYPECODE_int32_t   (0x84)
#define CALMEAS_TYPECODE_float     (0x94)
#define CALMEAS_TYPECODE(type)     (CALMEAS_TYPECODE_ ## type)
#define CALMEAS_TYPECODE_LEN_MASK  (0x0F)  // Use LS nibble as length info
#define CALMEAS_TYPECODE_SIGN_MASK (0x80)  // Use MSb as sign info
#define CALMEAS_TYPECODE_LEN(X)    ((X) & CALMEAS_TYPECODE_LEN_MASK)
#define CALMEAS_TYPECODE_SIGN(X)   ((X) & CALMEAS_TYPECODE_SIGN_MASK)

// TODO: You need to define these sections in the linker script:
#define CALMEAS_MEMSEC_META        __attribute__ ((section(".meas_meta"), aligned(1)))
#define CALMEAS_MEMSEC_4BYTE       __attribute__ ((section(".meas_4B"), aligned(4)))
#define CALMEAS_MEMSEC_1BYTE       __attribute__ ((section(".meas"), aligned(1)))
#define CALMEAS_MEMSEC_2BYTE       __attribute__ ((section(".meas"), aligned(1)))
#define CALMEAS_MEMSEC_uint32_t    CALMEAS_MEMSEC_4BYTE
#define CALMEAS_MEMSEC_int32_t     CALMEAS_MEMSEC_4BYTE
#define CALMEAS_MEMSEC_float       CALMEAS_MEMSEC_4BYTE
#define CALMEAS_MEMSEC_uint8_t     CALMEAS_MEMSEC_1BYTE
#define CALMEAS_MEMSEC_int8_t      CALMEAS_MEMSEC_1BYTE
#define CALMEAS_MEMSEC_uint16_t    CALMEAS_MEMSEC_2BYTE
#define CALMEAS_MEMSEC_int16_t     CALMEAS_MEMSEC_2BYTE
#define CALMEAS_MEMSEC(type)       CALMEAS_MEMSEC_ ## type


/*
 * Declare and define a symbol that will be available for both tuning and measurement.
 * The variable will be placed at the memory segment specified by the corresponding CALMEAS_MEMSEC macro.
 */
#define CALMEAS_SYMBOL(type, name, init, desc) type name CALMEAS_MEMSEC(type) = init; \
    calmeas_meta_t calmeas_meta_ ## name CALMEAS_MEMSEC_META = {CALMEAS_TYPECODE(type), #name, &(name), desc}

/*
 * Make a specific address become both tunable and measurable.
 * No data will be declared except the meta-data for the created symbol.
 */
#define CALMEAS_SYMBOL_BY_ADDRESS(type, name, address, desc) calmeas_meta_t calmeas_meta_ ## name CALMEAS_MEMSEC_META = {CALMEAS_TYPECODE(type), #name, address, desc}

int calmeas_init(void);
int calmeas_handler(void);

#endif /* CALMEAS_H_ */
