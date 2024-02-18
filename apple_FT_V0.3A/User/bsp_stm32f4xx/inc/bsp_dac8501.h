﻿/*
*********************************************************************************************************
*
*	模块名称 : DAC8501 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_dac8501.c
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef _BSP_DAC8501_H
#define _BSP_DAC8501_H

void bsp_InitDAC8501(void);
void DAC8501_SendData(uint8_t _ch, uint16_t _dac);

#endif

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/