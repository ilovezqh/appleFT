/*
*********************************************************************************************************
*
*	模块名称 : ASU总线驱动模块
*	文件名称 : bsp_asu_gpio.c
*	版    本 : V1.0
*	说    明 : 用gpio模拟asu总线, 适用于STM32F4系列CPU。该模块不包括应用层命令帧，仅包括ASU总线基本操作函数。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 plxc  正式发布
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

/*
	应用说明：
	在访问ASU设备前，请先调用 asu_CheckDevice() 检测ASU设备是否正常，该函数会配置GPIO
*/

#include "bsp.h"

/*
	普林芯驰FT_F4开发板 asu总线GPIO:
 		PH4/ASU2_SCL
 		PH5/ASU2_SDA
*/

/* 定义ASU总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define GPIO_PORT_ASU	GPIOA			/* GPIO端口 */
#define RCC_ASU_PORT 	RCC_AHB1Periph_GPIOA		/* GPIO端口时钟 */
#define ASU_SCL_PIN		GPIO_Pin_7			/* 连接到SCL时钟线的GPIO */
#define ASU_SDA_PIN		GPIO_Pin_6         /* 连接到SDA数据线的GPIO */

/* 定义读写SCL和SDA的宏 */
#define ASU_SCL_1()  GPIO_PORT_ASU->BSRRL = ASU_SCL_PIN				/* SCL = 1 */
#define ASU_SCL_0()  GPIO_PORT_ASU->BSRRH = ASU_SCL_PIN				/* SCL = 0 */

#define ASU_SDA_1()  GPIO_PORT_ASU->BSRRL = ASU_SDA_PIN				/* SDA = 1 */
#define ASU_SDA_0()  GPIO_PORT_ASU->BSRRH = ASU_SDA_PIN				/* SDA = 0 */

#define ASU_SDA_READ()  ((GPIO_PORT_ASU->IDR & ASU_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define ASU_SCL_READ()  ((GPIO_PORT_ASU->IDR & ASU_SCL_PIN) != 0)	/* 读SCL口线状态 */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitASU
*	功能说明: 配置ASU总线的GPIO，采用模拟IO的方式实现
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitASU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_ASU_PORT, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为开漏模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = ASU_SCL_PIN | ASU_SDA_PIN;
	GPIO_Init(GPIO_PORT_ASU, &GPIO_InitStructure);

	/* 给一个停止信号, 复位ASU总线上的所有设备到待机模式 */
	asu_Stop();
}


void bsp_deinitASU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_ASU_PORT, ENABLE);	/* 打开GPIO时钟 */
    ASU_SDA_0();
    ASU_SCL_0();
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为开漏模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = ASU_SCL_PIN | ASU_SDA_PIN;
	GPIO_Init(GPIO_PORT_ASU, &GPIO_InitStructure);
 
}

/*
*********************************************************************************************************
*	函 数 名: asu_Delay
*	功能说明: ASU总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void asu_Delay(void)
{
	uint8_t i;

	/*　
		CPU主频168MHz时，在内部Flash运行, MDK工程不优化。用台式示波器观测波形。
		循环次数为5时，SCL频率 = 1.78MHz (读耗时: 92ms, 读写正常，但是用示波器探头碰上就读写失败。时序接近临界)
		循环次数为10时，SCL频率 = 1.1MHz (读耗时: 138ms, 读速度: 118724B/s)
		循环次数为30时，SCL频率 = 440KHz， SCL高电平时间1.0us，SCL低电平时间1.2us

		上拉电阻选择2.2K欧时，SCL上升沿时间约0.5us，如果选4.7K欧，则上升沿约1us

		实际应用选择400KHz左右的速率即可
	*/
	for (i = 0; i < 10; i++)
    {
        __nop();
    }
 
}

/*
*********************************************************************************************************
*	函 数 名: asu_Start
*	功能说明: CPU发起ASU总线启动信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void asu_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示ASU总线启动信号 */
	ASU_SDA_1();
	ASU_SCL_1();
	asu_Delay();
	ASU_SDA_0();
	asu_Delay();
	ASU_SCL_0();
	asu_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: asu_Start
*	功能说明: CPU发起ASU总线停止信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void asu_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示ASU总线停止信号 */
	ASU_SDA_0();
	ASU_SCL_1();
	asu_Delay();
	ASU_SDA_1();
}

/*
*********************************************************************************************************
*	函 数 名: asu_SendByte
*	功能说明: CPU向ASU总线设备发送8bit数据
*	形    参:  _ucByte ： 等待发送的字节
*	返 回 值: 无
*********************************************************************************************************
*/
void asu_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			ASU_SDA_1();
		}
		else
		{
			ASU_SDA_0();
		}
		asu_Delay();
		ASU_SCL_1();
		asu_Delay();
		ASU_SCL_0();
		if (i == 7)
		{
			 ASU_SDA_1(); // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		asu_Delay();
	}
}

/*
*********************************************************************************************************
*	函 数 名: asu_ReadByte
*	功能说明: CPU从ASU总线设备读取8bit数据
*	形    参:  无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t asu_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		ASU_SCL_1();
		asu_Delay();
		if (ASU_SDA_READ())
		{
			value++;
		}
		ASU_SCL_0();
		asu_Delay();
	}
	return value;
}

/*
*********************************************************************************************************
*	函 数 名: asu_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参:  无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t asu_WaitAck(void)
{
	uint8_t re;

	ASU_SDA_1();	/* CPU释放SDA总线 */
	asu_Delay();
	ASU_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	asu_Delay();
	if (ASU_SDA_READ())	/* CPU读取SDA口线状态 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	ASU_SCL_0();
	asu_Delay();
	return re;
}

/*
*********************************************************************************************************
*	函 数 名: asu_Ack
*	功能说明: CPU产生一个ACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void asu_Ack(void)
{
	ASU_SDA_0();	/* CPU驱动SDA = 0 */
	asu_Delay();
	ASU_SCL_1();	/* CPU产生1个时钟 */
	asu_Delay();
	ASU_SCL_0();
	asu_Delay();
	ASU_SDA_1();	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	函 数 名: asu_NAck
*	功能说明: CPU产生1个NACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void asu_NAck(void)
{
	ASU_SDA_1();	/* CPU驱动SDA = 1 */
	asu_Delay();
	ASU_SCL_1();	/* CPU产生1个时钟 */
	asu_Delay();
	ASU_SCL_0();
	asu_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: asu_CheckDevice
*	功能说明: 检测ASU总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参:  _Address：设备的ASU总线地址
*	返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t asu_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	if (ASU_SDA_READ() && ASU_SCL_READ())
	{
		asu_Start();		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
		asu_SendByte(_Address | ASU_WR);
		ucAck = asu_WaitAck();	/* 检测设备的ACK应答 */

		asu_Stop();			/* 发送停止信号 */

		return ucAck;
	}
	return 1;	/* ASU总线异常 */
}
