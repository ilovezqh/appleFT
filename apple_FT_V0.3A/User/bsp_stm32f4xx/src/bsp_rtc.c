/*
*********************************************************************************************************
*
*	模块名称 : RTC
*	文件名称 : bsp_rtc.c
*	版    本 : V1.0
*	说    明 : RTC底层驱动
*	修改记录 :
*		版本号   日期        作者     说明
*		V1.0    2013-12-11  Eric2013   正式发布
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#include "bsp.h"


#define RTC_Debug   /* 用于选择调试模式 */
/* 选择RTC的时钟源 */
#define RTC_CLOCK_SOURCE_LSE       /* LSE */
//#define RTC_CLOCK_SOURCE_LSI     /* LSI */ 

RTC_TimeTypeDef  RTC_TimeStructure;
RTC_InitTypeDef  RTC_InitStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;
RTC_DateTypeDef  RTC_DateStructure;

__IO uint32_t uwAsynchPrediv = 0;
__IO uint32_t uwSynchPrediv = 0;
uint8_t aShowTime[50] = {0};
#define SET_MARK  0x32F1//0x32F2
/* 
******************************************************************************** 
*  函 数 名: bsp_InitRTC 
*  功能说明: 初始化RTC 
*  形 参：无  *  返 回 值: 无   
******************************************************************************** 
*/ 
void bsp_InitRTC(void) 
{ 
	NVIC_InitTypeDef NVIC_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure; 
	/* 用于检测是否已经配置过RTC，如果配置过的话，会在配置结束时 
	设置RTC备份寄存器为0x32F2。如果检测RTC备份寄存器不是0x32F2   那么表示没有配置过，需要配置RTC.   */ 
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != SET_MARK) 
	{ 
		/* RTC 配置 */ 
		RTC_Config(); 
		/* 打印调试信息 */ 
		#ifdef RTC_Debug 
		printf("第一次使用RTC \n\r"); 
		#endif  
		/* 检测上电复位标志是否设置 */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 
		/* 发生上电复位 */ 
		#ifdef RTC_Debug 
		printf("发生上电复位 \n\r"); 
		#endif 
		} 
	} 
	else 
	{ 
		/* 打印调试信息 */ 
		#ifdef RTC_Debug 
			printf("第n次使用RTC \n\r"); 
		#endif 
		/* 检测上电复位标志是否设置 */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 
			/* 发生上电复位 */ 
			#ifdef RTC_Debug 
				printf("发生上电复位 \n\r"); 
			#endif 
		} 
		/* 检测引脚复位标志是否设置 */ 
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) 
		{ 
			/* 发生引脚复位 */ 
			#ifdef RTC_Debug 
				printf("发生引脚复位 \n\r");     
			#endif    
		} 
		
		/* 使能PWR时钟 */ 
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); 
		/* 允许访问RTC */ 
		PWR_BackupAccessCmd(ENABLE); 
		/* 等待 RTC APB 寄存器同步 */ 
		RTC_WaitForSynchro(); 
		/* 清除RTC闹钟标志 */ 
		RTC_ClearFlag(RTC_FLAG_ALRAF); 
		/* 清除RTC闹钟中断挂起标志 */ 
		EXTI_ClearITPendingBit(EXTI_Line17); 
	} 
	
	/* RTC闹钟中断配置 */ 
	/* EXTI 配置 */ 
	EXTI_ClearITPendingBit(EXTI_Line17); 
	EXTI_InitStructure.EXTI_Line = EXTI_Line17; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure); 
	/* 使能RTC闹钟中断 */ 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}	  
/*
*********************************************************************************************************
*	函 数 名: RTC_Config
*	功能说明: 1. 选择不同的RTC时钟源LSI或者LSE。
*             2. 配置RTC时钟。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_Config(void)
{
	/* 使能PWR时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* 允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* 选择LSI作为时钟源 */
#if defined (RTC_CLOCK_SOURCE_LSI)  
	
	/* Enable the LSI OSC */ 
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* 选择RTC时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;
	
	/* 选择LSE作为RTC时钟 */
#elif defined (RTC_CLOCK_SOURCE_LSE)
	/* 使能LSE振荡器  */
	RCC_LSEConfig(RCC_LSE_ON);

	/* 等待就绪 */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* 选择RTC时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;

#else
#error Please select the RTC Clock source inside the main.c file
#endif 

	/* 使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);

	/* 等待RTC APB寄存器同步 */
	RTC_WaitForSynchro();

	/* 配置RTC数据寄存器和分频器  */
	RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
	RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	/* 设置年月日和星期 */
	RTC_DateStructure.RTC_Year = 0x23;
	RTC_DateStructure.RTC_Month = RTC_Month_April;
	RTC_DateStructure.RTC_Date = 0x13;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

	/* 设置时分秒，以及显示格式 */
	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours   = 0x16;
	RTC_TimeStructure.RTC_Minutes = 0x24;
	RTC_TimeStructure.RTC_Seconds = 0x00; 
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);   

	/* 配置备份寄存器，表示已经设置过RTC */
	RTC_WriteBackupRegister(RTC_BKP_DR0, SET_MARK);

	/* 设置闹钟 BCD格式，05小时，20分钟，30秒 */
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x05;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x20;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x30;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

	/* 配置 RTC Alarm A 寄存器 */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	/* 使能 RTC Alarm A 中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* 使能闹钟 */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	/* 清除RTC闹钟标志 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
}

/*
*********************************************************************************************************
*	函 数 名: RTC_TimeShow
*	功能说明: 显示时间
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_TimeShow(void)
{
	/* 得到当前时分秒时间 */
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	/* 时间显示格式 : hh:mm:ss */
	sprintf((char*)aShowTime,"%0.2d:%0.2d:%0.2d",RTC_TimeStructure.RTC_Hours, 
	                                           RTC_TimeStructure.RTC_Minutes, 
	                                           RTC_TimeStructure.RTC_Seconds);
}

/*
*********************************************************************************************************
*	函 数 名: RTC_DateShow
*	功能说明: 显示时间
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_DateShow(void)
{
	/* 得到当前时分秒时间 */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	/* 时间显示格式 : hh:mm:ss */
	sprintf((char*)aShowTime,"%0.2d年%0.2d月%0.2d日_星期%0.1d",RTC_DateStructure.RTC_Year, 
	                                                         RTC_DateStructure.RTC_Month, 
	                                                          RTC_DateStructure.RTC_Date,
											               RTC_DateStructure.RTC_WeekDay);
}

/*
*********************************************************************************************************
*	函 数 名: RTC_AlarmShow
*	功能说明: 显示设置的闹钟时间
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_AlarmShow(void)
{	
	uint8_t showalarm[50] = {0};

	/* 设置当前闹钟 */
	RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
	sprintf((char*)showalarm,"%0.2d:%0.2d:%0.2d", RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours, 
												RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes, 
												RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds);
}

/*
*********************************************************************************************************
*	函 数 名: RTC_AlarmShow
*	功能说明: 显示设置的闹钟时间
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_TimeRegulate(void)
{
	uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;

	printf("\n\r===================时间设置=======================\n\r");
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	
	/*******************************************************/
	
	printf("  请设置小时:\n\r");
	while (tmp_hh == 0xFF)
	{
		tmp_hh = USART_Scanf(23);
		RTC_TimeStructure.RTC_Hours = tmp_hh;
	}
	printf("  %0.2d\n\r", tmp_hh);
	
    /*******************************************************/
	
	printf("  请设置分钟:\n\r");
	while (tmp_mm == 0xFF)
	{
	tmp_mm = USART_Scanf(59);
	RTC_TimeStructure.RTC_Minutes = tmp_mm;
	}
	printf("  %0.2d\n\r", tmp_mm);

	/*******************************************************/
	
	printf("  请设置秒:\n\r");
	while (tmp_ss == 0xFF)
	{
	tmp_ss = USART_Scanf(59);
	RTC_TimeStructure.RTC_Seconds = tmp_ss;
	}
	printf("  %0.2d\n\r", tmp_ss);
	
	/*******************************************************/

	/* 配置RTC时间寄存器 */
	if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR)
	{
		printf("\n\r>> !! RTC 时间设置失败 !! <<\n\r");
	} 
	else
	{
		printf("\n\r>> !! RTC 时间设置成功 !! <<\n\r");
		RTC_TimeShow();
		/* 设置RTC备份寄存器，以说明已经配置过 */
		RTC_WriteBackupRegister(RTC_BKP_DR0, SET_MARK);
	}
	
	/*******************************************************/

	tmp_hh = 0xFF;
	tmp_mm = 0xFF;
	tmp_ss = 0xFF;

	/* 禁能 Alarm A */
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	printf("\n\r==============Alarm A 设置=========================\n\r");
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	
	/*******************************************************/
	
	printf("  请设置闹钟小时:\n\r");
	while (tmp_hh == 0xFF)
	{
	tmp_hh = USART_Scanf(23);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = tmp_hh;
	}
	printf("  %0.2d\n\r", tmp_hh);

	/*******************************************************/
	
	printf("  请设置闹钟分钟:\n\r");
	while (tmp_mm == 0xFF)
	{
	tmp_mm = USART_Scanf(59);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = tmp_mm;
	}
	printf("  %0.2d\n\r", tmp_mm);
	
	/*******************************************************/

	printf("  请设置闹钟秒:\n\r");
	while (tmp_ss == 0xFF)
	{
	tmp_ss = USART_Scanf(59);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = tmp_ss;
	}
	printf("  %0.2d", tmp_ss);
	
	/*******************************************************/
	
	/* 设置 Alarm A */
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

	/* 配置 RTC Alarm A 寄存器 */
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
	printf("\n\r>> !! RTC 设置 Alarm 成功. !! <<\n\r");

	/* 使能 RTC Alarm A 中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* 使能 alarm  A */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	
}

/*
*********************************************************************************************************
*	函 数 名: USART_Scanf
*	功能说明: 接收串口数据
*	形    参：value 表示输入数据最大值
*	返 回 值: 返回串口接收到的数值
*********************************************************************************************************
*/
uint8_t USART_Scanf(uint32_t value)
{
	uint32_t index = 0;
	uint32_t tmp[2] = {0, 0};

	while (index < 2)
	{
		/* 等待直到 RXNE = 1 */
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		{}
		tmp[index++] = (USART_ReceiveData(USART1));
		if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39))
		{
			printf("\n\r 有效数据必须在 0 和 9 直接\n\r");
			index--;
		}
	}
	
	/* 计算两个字节合并后的数值 */
	index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);
	/* Checks */
	if (index > value)
	{
		printf("\n\r 有效数据必须在 0 和 %d 直接\n\r", value);
		return 0xFF;
	}
	
	return index;
}

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
