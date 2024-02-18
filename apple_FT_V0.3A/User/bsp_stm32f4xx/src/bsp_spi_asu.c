/*
*********************************************************************************************************
*
*	模块名称 : SPI总线驱动
*	文件名称 : bsp_spi_bus.h
*	版    本 : V1.1
*	说    明 : SPI总线底层驱动。提供SPI配置、收发数据、多设备共享SPI支持。通过宏切换是硬件SPI还是软件模拟
*	修改记录 :
*		版本号  日期        作者    说明
*       v1.0    2015-05-18 plxc  首版。将串行FLASH、TSC2046、VS1053、AD7705、ADS1256等SPI设备的配置
*									和收发数据的函数进行汇总分类。并解决不同速度的设备间的共享问题。
*		V1.1	2015-05-21 plxc  硬件SPI时，没有开启GPIOB时钟，已解决。
*
*	Copyright (C), 2015-2016, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#include "bsp.h"
 
/*
	普林芯驰STM32-V4 开发板口线分配
	PA5/SPI1_SCK
	PA6/SPI1_MISO
	PA7/SPI1_MOSI

	SPI1的时钟源是 APB2Periph
*/

/* 定义SPI总线的 GPIO端口 */
#define ASU_RCC_SCK 	RCC_AHB1Periph_GPIOB
#define ASU_PORT_SCK	GPIOB
#define ASU_PIN_SCK		GPIO_Pin_3

#define ASU_RCC_MISO 	RCC_AHB1Periph_GPIOB
#define ASU_PORT_MISO	GPIOB
#define ASU_PIN_MISO	GPIO_Pin_4

#define ASU_RCC_MOSI 	RCC_AHB1Periph_GPIOB
#define ASU_PORT_MOSI	GPIOB
#define ASU_PIN_MOSI	GPIO_Pin_5

#ifdef SOFT_SPI_ASU		/* 软件SPI */
	#define ASU_SCK_0()		ASU_PORT_SCK->BSRRH = ASU_PIN_SCK
	#define ASU_SCK_1()		ASU_PORT_SCK->BSRRL = ASU_PIN_SCK

	#define ASU_MOSI_0()	ASU_PORT_MOSI->BSRRH = ASU_PIN_MOSI
	#define ASU_MOSI_1()	ASU_PORT_MOSI->BSRRL = ASU_PIN_MOSI

	#define ASU_MISO_IS_HIGH()	(GPIO_ReadInputDataBit(ASU_PORT_MISO, ASU_PIN_MISO) == Bit_SET)
#endif

#ifdef HARD_SPI_ASU
	#define SPI_HARD	SPI1
	#define RCC_SPI		RCC_APB2Periph_SPI1
	
	/* SPI or I2S mode selection masks */
	#define SPI_Mode_Select      ((uint16_t)0xF7FF)
	#define I2S_Mode_Select      ((uint16_t)0x0800) 
	
	/* SPI registers Masks */
	#define CR1_CLEAR_Mask       ((uint16_t)0x3040)
	#define I2SCFGR_CLEAR_Mask   ((uint16_t)0xF040)

	/* SPI SPE mask */
	#define CR1_SPE_Set          ((uint16_t)0x0040)
	#define CR1_SPE_Reset        ((uint16_t)0xFFBF)
#endif

uint8_t g_spi_busy_asu = 0;		/* SPI 总线共享标志 */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPIBus
*	功能说明: 配置SPI总线。 只包括 SCK、 MOSI、 MISO口线的配置。不包括片选CS，也不包括外设芯片特有的INT、BUSY等
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_asu_init(void)
{
#ifdef SOFT_SPI_ASU		/* 软件SPI */
	GPIO_InitTypeDef  GPIO_InitStructure={0};

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(ASU_RCC_SCK | ASU_RCC_MOSI | ASU_RCC_MISO, ENABLE);	
    
	/* 配置SPI引脚SCK、MISO 和 MOSI为复用推挽模式 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	/* 推挽输出模式 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_SCK;
	GPIO_Init(ASU_PORT_SCK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MOSI;
	GPIO_Init(ASU_PORT_MOSI, &GPIO_InitStructure);	

 
   // GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* MISO 设置为输入上拉 */
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MISO;
	GPIO_Init(ASU_PORT_MISO, &GPIO_InitStructure);
#endif

#ifdef HARD_SPI_ASU		/* 硬件SPI */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 开启 SPI 时钟 */
	//RCC_APB2PeriphClockCmd(RCC_SPI, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* 使能 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(ASU_RCC_SCK | ASU_RCC_MOSI | ASU_RCC_MISO, ENABLE);	

	/* 配置 SPI引脚SCK、MISO 和 MOSI为复用推挽模式 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_SCK;	
	GPIO_Init(ASU_PORT_SCK, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MISO;	
	GPIO_Init(ASU_PORT_MISO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MOSI;	
	GPIO_Init(ASU_PORT_MOSI, &GPIO_InitStructure);
		
	bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
		| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_64 | SPI_FirstBit_MSB);	
	
	/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
	SPI_HARD->I2SCFGR &= SPI_Mode_Select;		/* 选择SPI模式，不是I2S模式 */

	/*---------------------------- SPIx CRCPOLY Configuration --------------------*/
	/* Write to SPIx CRCPOLY */
	SPI_HARD->CRCPR = 7;		/* 一般不用 */


	SPI_Cmd(SPI_HARD, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI_HARD, ENABLE);			/* 使能SPI  */
#endif
}


void bsp_asu_deinit(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure={0};

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(ASU_RCC_SCK | ASU_RCC_MOSI | ASU_RCC_MISO, ENABLE);	
    
	/* 配置SPI引脚SCK、MISO 和 MOSI为复用推挽模式 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	/* 推挽输出模式 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_SCK;
	GPIO_Init(ASU_PORT_SCK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MOSI;
	GPIO_Init(ASU_PORT_MOSI, &GPIO_InitStructure);	

 
	GPIO_InitStructure.GPIO_Pin = ASU_PIN_MISO;
	GPIO_Init(ASU_PORT_MISO, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SPI_Init
*	功能说明: 配置STM32内部SPI硬件的工作模式。 简化库函数，提高执行效率。 仅用于SPI接口间切换。
*	形    参: _cr1 寄存器值
*	返 回 值: 无
*********************************************************************************************************
*/
#ifdef HARD_SPI_ASU		/* 硬件SPI */
void bsp_SPI_Init(uint16_t _cr1)
{
	SPI_HARD->CR1 = ((SPI_HARD->CR1 & CR1_CLEAR_Mask) | _cr1);
	  
	//SPI_Cmd(SPI_HARD, DISABLE);			/* 先禁止SPI  */	    
    SPI_HARD->CR1 &= CR1_SPE_Reset;	/* Disable the selected SPI peripheral */

	//SPI_Cmd(SPI_HARD, ENABLE);			/* 使能SPI  */		    
    SPI_HARD->CR1 |= CR1_SPE_Set;	  /* Enable the selected SPI peripheral */
}
#endif

#ifdef SOFT_SPI_ASU		/* 软件SPI */
/*
*********************************************************************************************************
*	函 数 名: bsp_SpiDelay
*	功能说明: 时序延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiDelay_ASU(void)
{
	uint32_t i;

	for (i = 0; i < 1; i++);
   
}
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_spiWrite0
*	功能说明: 向SPI总线发送一个字节。SCK上升沿采集数据, SCK空闲时为低电平。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t  bsp_spiWrite0_ASU(uint8_t _ucByte)
{
#ifdef SOFT_SPI_ASU		/* 软件SPI */
	uint8_t i;
    uint8_t read = 0;
    
	for(i = 0; i < 8; i++)
	{
        read = read<<1;
        if(ASU_MISO_IS_HIGH())
        {
            read++;
        }
        
		if (_ucByte & 0x80)  
		{
			ASU_MOSI_1();
		}
		else
		{
			ASU_MOSI_0();
		}
		bsp_spiDelay_ASU();
		ASU_SCK_1();
		_ucByte <<= 1;
		bsp_spiDelay_ASU();
		ASU_SCK_0();
       // bsp_spiDelay_ASU();
        
	}
	bsp_spiDelay_ASU();
    return read;
#endif

#ifdef HARD_SPI_ASU		/* 硬件SPI */
	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	SPI_I2S_ReceiveData(SPI1);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiRead0
*	功能说明: 从SPI总线接收8个bit数据。 SCK上升沿采集数据, SCK空闲时为低电平。
*	形    参: 无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t bsp_spiRead0_ASU(void)
{
#ifdef SOFT_SPI_ASU		/* 软件SPI */
	uint8_t i;
	uint8_t read = 0;

	for (i = 0; i < 8; i++)
	{
		read = read<<1;

		if (ASU_MISO_IS_HIGH())
		{
			read++;
		}
		ASU_SCK_1();
		bsp_spiDelay_ASU();
		ASU_SCK_0();
		bsp_spiDelay_ASU();
	}
	return read;
#endif

#ifdef HARD_SPI_ASU		/* 硬件SPI */
	uint8_t read;

	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, 0);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	read = SPI_I2S_ReceiveData(SPI1);

	/* 返回读到的数据 */
	return read;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiWrite1
*	功能说明: 向SPI总线发送一个字节。  SCK上升沿采集数据, SCK空闲时为高电平
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiWrite1_ASU(uint8_t _ucByte)
{
#ifdef SOFT_SPI_ASU		/* 软件SPI */
	uint8_t i;

	for(i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			ASU_MOSI_1();
		}
		else
		{
			ASU_MOSI_0();
		}
		ASU_SCK_0();
		_ucByte <<= 1;
		bsp_spiDelay_ASU();
		ASU_SCK_1();				/* SCK上升沿采集数据, SCK空闲时为高电平 */
		bsp_spiDelay_ASU();
	}
#endif

#ifdef HARD_SPI_ASU		/* 硬件SPI */
	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	SPI_I2S_ReceiveData(SPI1);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiRead1
*	功能说明: 从SPI总线接收8个bit数据。  SCK上升沿采集数据, SCK空闲时为高电平
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t bsp_spiRead1_ASU(void)
{
#ifdef SOFT_SPI_ASU		/* 软件SPI */
	uint8_t i;
	uint8_t read = 0;

	for (i = 0; i < 8; i++)
	{
		ASU_SCK_0();
		bsp_spiDelay_ASU();
		read = read << 1;
		if (ASU_MISO_IS_HIGH())
		{
			read++;
		}
		ASU_SCK_1();
		bsp_spiDelay_ASU();
	}
	return read;
#endif

#ifdef HARD_SPI_ASU		/* 硬件SPI */
	uint8_t read;

	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, 0);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	read = SPI_I2S_ReceiveData(SPI1);

	/* 返回读到的数据 */
	return read;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusEnter
*	功能说明: 占用SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_SpiBusEnter_ASU(void)
{
	g_spi_busy_asu = 1;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusExit
*	功能说明: 释放占用的SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_SpiBusExit_ASU(void)
{
	g_spi_busy_asu = 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusBusy
*	功能说明: 判断SPI总线忙。方法是检测其他SPI芯片的片选信号是否为1
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
uint8_t bsp_SpiBusBusy_ASU(void)
{
	return g_spi_busy_asu;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetSpiSck
*	功能说明: 用于软件模式。设置SCK GPIO的状态。在函数CS=0之前被调用，用于不同相序的SPI设备间切换。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#ifdef SOFT_SPI_ASU		/* 软件SPI */
void bsp_SetSpiSck_ASU(uint8_t _data)
{
	if (_data == 0)
	{
		ASU_SCK_0();
	}
	else
	{
		ASU_SCK_1();
	}
}
#endif


/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
