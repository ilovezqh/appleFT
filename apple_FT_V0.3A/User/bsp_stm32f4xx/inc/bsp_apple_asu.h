/*
*********************************************************************************************************
*
*	模块名称 : I2C总线驱动模块
*	文件名称 : bsp_asu_gpio.h
*	版    本 : V1.0
*	说    明 : 头文件。
*
*	Copyright (C), 2012-2013, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef _BSP_APPLE_ASU_H
#define _BSP_APPLE_ASU_H

#define ASU_WR	0		/* 写控制bit */
#define ASU_RD	1		/* 读控制bit */

void bsp_InitASU(void);
void bsp_deinitASU(void);
void asu_Start(void);
void asu_Stop(void);
void asu_SendByte(uint8_t _ucByte);
uint8_t asu_ReadByte(void);
uint8_t asu_WaitAck(void);
void asu_Ack(void);
void asu_NAck(void);
uint8_t asu_CheckDevice(uint8_t _Address);

#endif
