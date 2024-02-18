/*
*********************************************************************************************************
*
*	模块名称 : RTC
*	文件名称 : bsp_rtc.c
*	版    本 : V1.0
*	说    明 : RTC底层驱动
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef __BSP_RTC_H
#define __BSP_RTC_H
#include "stm32f4xx.h"

void RTC_TimeShow(void);
void RTC_AlarmShow(void);
void RTC_TimeRegulate(void);
uint8_t USART_Scanf(uint32_t value);
void RTC_DateShow(void);
void bsp_InitRTC(void);
void RTC_Config(void);

#endif

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/

