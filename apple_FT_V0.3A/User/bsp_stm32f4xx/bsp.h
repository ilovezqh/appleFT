/*
*********************************************************************************************************
*
*	模块名称 : BSP模块
*	文件名称 : bsp.h
*	说    明 : 这是底层驱动模块所有的h文件的汇总文件。 应用程序只需 #include bsp.h 即可，
*			  不需要#include 每个模块的 h 文件
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_

#define STM32_V5
//#define STM32_X4

/* 检查是否定义了开发板型号 */
#if !defined (STM32_V5) && !defined (STM32_X4)
	#error "Please define the board model : STM32_X4 or STM32_V5"
#endif

/* 定义 BSP 版本号 */
#define __STM32F1_BSP_VERSION		"1.1"

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
#define UNUSED(x) (void)(x)
 


/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf
#define debug(fmt,arg...)    printf(fmt,##arg)
//#define BSP_Printf(...)

#include "stm32f4xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif


#define ARRY_SIZE(arry)           sizeof(arry)/sizeof(arry[0]) //数组元素总数量

#define nano_timer_creat(name)        static int32_t name##timer=0
#define nano_timer_start(name,var)    name##timer=g_iRunTime+var
#define is_nano_time_up()             ((g_iRunTime - name##timer) < (UINT32_MAX/2))

#define timer_creat()        static int32_t timer=0
#define timer_start(var)     timer=g_iRunTime+var
#define is_time_up()        ((g_iRunTime - timer) < (UINT32_MAX/2))

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
#include "bsp_uart_fifo.h"
#include "bsp_led.h"
#include "bsp_timer.h"
#include "bsp_iwdg.h"
//#include "bsp_key.h"
#include "key.h"
#include "bsp_beep.h"
#include "bsp_rtc.h"
#include "bsp_hp_osc.h"
//#include "bsp_adc.h"
#include "bsp_adc_dma.h"
//#include "bsp_dac.h"
#include "bsp_power_supply.h"
//#include "bsp_spi_flash.h"
//#include "bsp_cpu_flash.h"
#include "bsp_sdio_sd.h"
#include "bsp_spi_bus.h"
//#include "bsp_spi_bus_asu.h"
#include "bsp_spi_flash.h"
#include "bsp_lcd_st7789.h"
#include "bsp_i2c_gpio.h"
#include "bsp_ch440r.h"
#include "bsp_ch440r_ex.h"


#include "bsp_eeprom_24xx.h"
#include "bsp_timer_capture.h"
#include "bsp_prog_robot_comm.h"
#include "bsp_socket.h"
#include "main.h"
#include "bsp_apple_asu.h"
//#include "bsp_si4730.h"
//#include "bsp_hmc5883l.h"
//#include "bsp_mpu6050.h"
//#include "bsp_bh1750.h"
//#include "bsp_bmp085.h"
//#include "bsp_wm8978.h"
//#include "bsp_fsmc_sram.h"
//#include "bsp_nand_flash.h"
//#include "bsp_nor_flash.h"
//#include "LCD_RA8875.h"
//#include "LCD_SPFD5420.h"
//#include "bsp_touch.h"
//#include "bsp_camera.h"
//#include "bsp_ad7606.h"
//#include "bsp_gps.h"
//#include "bsp_oled.h"
//#include "bsp_mg323.h"

/* 提供给其他C文件调用的函数 */
void bsp_Init(void);
void bsp_Idle(void);

#endif

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
