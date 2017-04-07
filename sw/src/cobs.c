/*
 * cobs.c
 *
 *  Created on: 4 dec 2015
 *      Author: osannolik
 */


#include "cobs.h"


uint8_t * cobs_Encode(const uint8_t *pData, uint32_t len, uint8_t *dst)
{
  const uint8_t *dataEnd = pData + len;
  uint8_t *pCode = dst++;
  uint8_t code = 0x01;

  while (pData < dataEnd) {
    if (*pData == 0) {
      *pCode = code;
      pCode = dst++;
      code = 0x01;
    } else {
      *dst++ = *pData;
      if (++code == 0xFF) {
        *pCode = code;
        pCode = dst++;
        code = 0x01;
      }
    }
    pData++;
  }

  *pCode = code;

  return dst;
}

uint8_t cobs_Decode(const uint8_t *pCoded, uint32_t len, uint8_t *pData)
{
  const uint8_t *codeEnd = pCoded + len;
  uint8_t i, code;
  uint8_t *init_pData = pData;

  while (pCoded < codeEnd) {
    code = *pCoded++;
    for (i=1; pCoded<codeEnd && i<code; i++)
      *pData++ = *pCoded++;
    if (code < 0xFF)
      *pData++ = 0;
  }

  return (uint8_t) (pData-init_pData);
}
