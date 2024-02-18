#ifndef __CHIP_BUS_H
#define __CHIP_BUS_H


uint8_t apple_CheckOk(void);
fsm_rt_t apple_read_addr16(uint8_t DevAddr,uint8_t *ReadData,uint16_t RegAddr,uint16_t length);
fsm_rt_t apple_read_addr8(uint8_t DevAddr,uint8_t *ReadData,uint8_t RegAddr,uint16_t length);
fsm_rt_t apple_write_addr8(uint8_t DevAddr,uint8_t *WriteData,uint8_t RegAddr,uint16_t length);
fsm_rt_t apple_write_addr16(uint8_t DevAddr,uint8_t *WriteData,uint16_t RegAddr,uint16_t length);

#endif
