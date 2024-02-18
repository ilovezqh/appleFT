/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H

#define LED_GREEN       1
#define LED_RED         2
#define LED_GREEN_KEY   3
#define LED_RED_KEY     4

/* 供外部调用的函数声明 */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
