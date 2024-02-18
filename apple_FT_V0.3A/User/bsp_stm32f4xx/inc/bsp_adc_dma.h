/*
*********************************************************************************************************
*
*	模块名称 : ADC驱动模块
*	文件名称 : bsp_adc.c
*	版    本 : V1.0
*	说    明 : adc头文件
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2014-01-08   armfly   正式发布
*
*	Copyright (C), 2013-2014, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef __BSP_ADC_DMA_H
#define __BSP_ADC_DMA_H

#include "stm32f4xx.h"

void bsp_InitADC(void);
float GetTemp(uint16_t advalue);

extern __IO uint16_t g_adc_value[];

//ADC DMA方式， ADC采样结果 g_adc_value[] 的索引号
#define     ADC_RANK_TRIM_PMU       0   //
#define     ADC_RANK_IREF           1
#define     ADC_RANK_AMP            2
#define     ADC_RANK_VREF           3
#define     ADC_RANK_BGR            4    
//#define     ADC_RANK_VMIC0          5
#define     ADC_RANK_STM32_VREFINT  5
#define     ADC_RANK_STM32_VBAT     6
#define     ADC_RANK_VBUS           7
#endif

/*********************** 普林芯驰 www.spacetouch.co (END OF FILE) ***************************/
