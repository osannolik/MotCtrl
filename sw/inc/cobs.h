/*
 * cobs.h
 *
 *  Created on: 4 dec 2015
 *      Author: osannolik
 */

#ifndef COBS_H_
#define COBS_H_

#include <stdint.h>
#include <stddef.h>

/*
 * COBS always adds 1 byte to the message length.
 * Additionally, for longer packets of length n,
 * it may add floor(n/254) additional bytes to the encoded packet size.
 * Hence, by setting data len to max 253, the stuffing size is always 1 byte. 
 */

#define COBS_DATA_LEN_MAX    (253)
#define COBS_OVERHEAD        (1) // Use data len 253 byte maximum
#define COBS_ENCODED_LEN_MAX (COBS_DATA_LEN_MAX+COBS_OVERHEAD)

uint8_t * cobs_Encode(const uint8_t *pData, uint32_t len, uint8_t *dst);
uint8_t cobs_Decode(const uint8_t *pCoded, uint32_t len, uint8_t *pData);


#endif /* COBS_H_ */
