/*
 *crc32.h
 *Author: liujian
 */

#ifndef _CRC32_H_
#define _CRC32_H_
#include "stdint.h"
/*
 * Exported functions
 */
void crc32_init(void);
void crc32_partial_start(uint32_t* pResult);
void crc32_partial_calcu(uint32_t* pResult,const uint8_t *pbuff, uint32_t size);
void crc32_partial_stop(uint32_t* pResult);
 

uint32_t crc32_entire_calcu(const uint8_t *pbuff, uint32_t size);

#endif		// __CRC32_H__
