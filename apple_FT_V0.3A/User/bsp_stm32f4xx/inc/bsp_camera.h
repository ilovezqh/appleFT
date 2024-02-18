﻿/*
*********************************************************************************************************
*
*	模块名称 : 摄像头驱动BSP模块(For OV7670)
*	文件名称 : bsp_camera.h
*	版    本 : V1.0
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef __DCMI_H
#define __DCMI_H

#define OV7670_SLAVE_ADDRESS	0x42		/* I2C 总线地址 */

typedef struct
{
	uint8_t CaptureOk;		/* 捕获完成 */
}CAM_T;

void bsp_InitCamera(void);
uint16_t OV_ReadID(void);
void CAM_Start(uint32_t _uiDispMemAddr);
void CAM_Stop(void);

extern CAM_T g_tCam;
#endif

