#ifndef __CRC_CODE_H__
#define	__CRC_CODE_H__

#include "stm32f4xx.h"

unsigned int _getCRC32(uint8_t *buffer, unsigned int bufferLen);
extern const unsigned char testdata[64];
#endif

