#ifndef __BSP_POWER_CTRL_H
#define __BSP_POWER_CTRL_H	 
#include "stm32f4xx.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//蜂鸣器驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

#define	POWER_OFF	  0
#define	POWER_ON		1

void volt_out_init(void);//初始化	
void volt_out_enable(void);
void volt_out_disable(void);
void short_100R_init(void);

 
#endif
