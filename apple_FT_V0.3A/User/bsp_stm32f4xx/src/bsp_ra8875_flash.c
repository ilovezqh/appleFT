﻿/*
*********************************************************************************************************
*
*	模块名称 : RA8875芯片外挂的串行Flash驱动模块
*	文件名称 : bsp_ra8875_flash.c
*	版    本 : V1.3
*	说    明 : 访问RA8875外挂的串行Flash （字库芯片和图库芯片），支持 SST25VF016B、MX25L1606E 和
*			   W25Q64BVSSIG。 通过TFT显示接口中SPI总线和PWM口线控制7寸新屏上的串行Flash。
*				【备注： RA8875本身不支持外挂串行Flash的写操作，必须增加额外的电子开关电路才能实现】
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2012-06-25 plxc  发布首版
*
*	Copyright (C), 2011-2012, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	STM32F4XX 时钟计算.
		HCLK = 168M
		PCLK1 = HCLK / 4 = 42M
		PCLK2 = HCLK / 2 = 84M

		SPI2、SPI3 在 PCLK1, 时钟42M
		SPI1       在 PCLK2, 时钟84M

		STM32F4 支持的最大SPI时钟为 37.5 Mbits/S, 因此需要分频。
*/

#ifdef STM32_X3 	/* 普林芯驰 STM32-X3 开发板 */
	/*
		普林芯驰STM32-X4 口线分配： 串行Flash型号为 W25Q64BVSSIG (80MHz)
		PB12 = CS
		PB13 = SCK
		PB14 = MISO
		PB15 = MOSI

		STM32硬件SPI接口 = SPI2
	*/
	#define SPI_FLASH			SPI2

	#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI2, ENABLE)

	#define SPI_BAUD			SPI_BaudRatePrescaler_1		/* 选择2分频时, SCK时钟 = 21M */

	/* 片选口线置低选中  */
	#define W25_CS_LOW()		GPIOB->BSRRH = GPIO_Pin_12

	/* 片选口线置高不选中 */
	#define W25_CS_HIGH()		GPIOB->BSRRL = GPIO_Pin_12
#else
	/*
		普林芯驰FT_F4TFT接口中的SPI口线分配： 串行Flash型号为 W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK/SPI1_SCK
		PB4/SPI3_MISO/SPI1_MISO
		PB5/SPI3_MOSI/SPI1_MOSI
		PI10/TP_NCS

		STM32硬件SPI接口 = SPI3 或者 SPI1

		由于SPI1的时钟源是84M, SPI3的时钟源是42M。为了获得更快的速度，软件上选择SPI1。
	*/
	//#define SPI_FLASH			SPI3
	#define SPI_FLASH			SPI1

	//#define ENABLE_SPI_RCC() 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE)
	#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)

	/*
		【SPI时钟最快是2分频，不支持不分频】
		如果是SPI1，2分频时SCK时钟 = 42M，4分频时SCK时钟 = 21M
		如果是SPI3, 2分频时SCK时钟 = 21M
	*/
	#define SPI_BAUD			SPI_BaudRatePrescaler_8

	/* 片选GPIO端口  */
	#define W25_CS_GPIO			GPIOI
	#define W25_CS_PIN			GPIO_Pin_10

	#define W25_PWM_GPIO		GPIOF
	#define W25_PWM_PIN			GPIO_Pin_6
#endif

/* 片选口线置低选中  */
#define W25_CS_LOW()       W25_CS_GPIO->BSRRH = W25_CS_PIN
/* 片选口线置高不选中 */
#define W25_CS_HIGH()      W25_CS_GPIO->BSRRL = W25_CS_PIN


/*
	PWM口线置低选中
	PWM = 1  这个模式支持STM32读写RA8875外挂的串行Flash
	PWM = 0 这是正常工作模式，由RA8875 DMA读取外挂的串行Flash
*/
#define W25_PWM_LOW()       W25_PWM_GPIO->BSRRH = W25_PWM_PIN
#define W25_PWM_HIGH()      W25_PWM_GPIO->BSRRL = W25_PWM_PIN


#define CMD_AAI       0xAD  	/* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_DISWR	  0x04		/* 禁止写, 退出AAI状态 */
#define CMD_EWRSR	  0x50		/* 允许写状态寄存器的命令 */
#define CMD_WRSR      0x01  	/* 写状态寄存器命令 */
#define CMD_WREN      0x06		/* 写使能命令 */
#define CMD_READ      0x03  	/* 读数据区命令 */
#define CMD_RDSR      0x05		/* 读状态寄存器命令 */
#define CMD_RDID      0x9F		/* 读器件ID命令 */
#define CMD_SE        0x20		/* 擦除扇区命令 */
#define CMD_BE        0xC7		/* 批量擦除命令 */
#define DUMMY_BYTE    0xA5		/* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG      0x01		/* 状态寄存器中的正在编程标志（WIP) */


W25_T g_tW25;

void w25_ReadInfo(void);
static uint8_t w25_SendByte(uint8_t _ucValue);
static void w25_WriteEnable(void);
static void w25_WriteStatus(uint8_t _ucValue);
static void w25_WaitForWriteEnd(void);
static void bsp_CfgSPIForW25(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitRA8875Flash
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitRA8875Flash(void)
{

#ifdef STM32_X3		/* 普林芯驰 STM32-X3 开发板 */
	/*
		普林芯驰STM32-X3 口线分配： 串行Flash型号为 W25Q64BVSSIG (80MHz)
		PB12 = CS
		PB13 = SCK
		PB14 = MISO
		PB15 = MOSI

		STM32硬件SPI接口 = SPI2
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 使能GPIO 时钟 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* 配置片选口线为推挽输出模式 */
		W25_CS_HIGH();		/* 片选置高，不选中 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
#else
	/*
		普林芯驰FT_F4口线分配：  串行Flash型号为 W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK
		PB4/SPI3_MISO
		PB5/SPI3_MOSI
		PF8/W25_CS

		STM32硬件SPI接口 = SPI3
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 使能GPIO 时钟 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOI| RCC_AHB1Periph_GPIOF, ENABLE);

		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* 配置片选口线为推挽输出模式 */
		W25_CS_HIGH();		/* 片选置高，不选中 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = W25_CS_PIN;
		GPIO_Init(W25_CS_GPIO, &GPIO_InitStructure);

		/* 配置TFT接口中的PWM脚为为推挽输出模式，PWM = 1时 用于写RA8875外挂的串行Flash */
		/* PF6/LCD_PWM  不用于调节RA8875屏的背光 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = W25_PWM_PIN;
		GPIO_Init(W25_PWM_GPIO, &GPIO_InitStructure);
	}
#endif

	/* 配置SPI硬件参数用于访问串行Flash */
	bsp_CfgSPIForW25();
}

/*
*********************************************************************************************************
*	函 数 名: w25_CtrlByMCU
*	功能说明: 串行Flash控制权交给MCU （STM32）
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_CtrlByMCU(void)
{
	/*
		PWM口线置低选中
		PWM = 1  这个模式支持STM32读写RA8875外挂的串行Flash
		PWM = 0 这是正常工作模式，由RA8875 DMA读取外挂的串行Flash
	*/
	W25_PWM_HIGH();
}

/*
*********************************************************************************************************
*	函 数 名: w25_CtrlByRA8875
*	功能说明: 串行Flash控制权交给RA8875
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_CtrlByRA8875(void)
{
	/*
		PWM口线置低选中
		PWM = 1  这个模式支持STM32读写RA8875外挂的串行Flash
		PWM = 0 这是正常工作模式，由RA8875 DMA读取外挂的串行Flash
	*/
	W25_PWM_LOW();
}

/*
*********************************************************************************************************
*	函 数 名: w25_SelectChip
*	功能说明: 选择即将操作的芯片
*	形    参: _idex = FONT_CHIP 表示字库芯片;  idex = BMP_CHIP 表示图库芯片
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_SelectChip(uint8_t _idex)
{
	/*
		PWM = 1, KOUT3 = 0 写字库芯片
		PWM = 1, KOUT3 = 1 写图库芯片
	*/

	if (_idex == FONT_CHIP)
	{
		RA8875_CtrlGPO(3, 0);	/* RA8875 的 KOUT3 = 0 */
	}
	else	/* BMP图片芯片 */
	{
		RA8875_CtrlGPO(3, 1);	/* RA8875 的 KOUT3 = 1 */
	}

	w25_ReadInfo();				/* 自动识别芯片型号 */

	W25_CS_LOW();				/* 软件方式，使能串行Flash片选 */
	w25_SendByte(CMD_DISWR);		/* 发送禁止写入的命令,即使能软件写保护 */
	W25_CS_HIGH();				/* 软件方式，禁能串行Flash片选 */

	w25_WaitForWriteEnd();		/* 等待串行Flash内部操作完成 */

	w25_WriteStatus(0);			/* 解除所有BLOCK的写保护 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CfgSPIForW25
*	功能说明: 配置STM32内部SPI硬件的工作模式、速度等参数，用于访问SPI接口的串行Flash。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_CfgSPIForW25(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	ENABLE_SPI_RCC();

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI_FLASH, ENABLE);				/* 使能SPI  */
}

/*
*********************************************************************************************************
*	函 数 名: w25_EraseSector
*	功能说明: 擦除指定的扇区
*	形    参:  _uiSectorAddr : 扇区地址
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_EraseSector(uint32_t _uiSectorAddr)
{
	w25_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_SE);								/* 发送擦除命令 */
	w25_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	w25_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	w25_SendByte(_uiSectorAddr & 0xFF);				/* 发送扇区地址低8bit */
	W25_CS_HIGH();									/* 禁能片选 */

	w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: w25_EraseChip
*	功能说明: 擦除整个芯片
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_EraseChip(void)
{
	w25_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_BE);							/* 发送整片擦除命令 */
	W25_CS_HIGH();									/* 禁能片选 */

	w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: w25_WritePage
*	功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (g_tW25.ChipID == SST25VF016B_ID)
	{
		/* AAI指令要求传入的数据个数是偶数 */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		w25_WriteEnable();								/* 发送写使能命令 */

		W25_CS_LOW();									/* 使能片选 */
		w25_SendByte(CMD_AAI);							/* 发送AAI命令(地址自动增加编程) */
		w25_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
		w25_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
		w25_SendByte(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */
		w25_SendByte(*_pBuf++);							/* 发送第1个数据 */
		w25_SendByte(*_pBuf++);							/* 发送第2个数据 */
		W25_CS_HIGH();									/* 禁能片选 */

		w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */

		_usSize -= 2;									/* 计算剩余字节数 */

		for (i = 0; i < _usSize / 2; i++)
		{
			W25_CS_LOW();								/* 使能片选 */
			w25_SendByte(CMD_AAI);						/* 发送AAI命令(地址自动增加编程) */
			w25_SendByte(*_pBuf++);						/* 发送数据 */
			w25_SendByte(*_pBuf++);						/* 发送数据 */
			W25_CS_HIGH();								/* 禁能片选 */
			w25_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */
		}

		/* 进入写保护状态 */
		W25_CS_LOW();
		w25_SendByte(CMD_DISWR);
		W25_CS_HIGH();

		w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
	}
	else	/* for MX25L1606E 、 W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			w25_WriteEnable();								/* 发送写使能命令 */

			W25_CS_LOW();									/* 使能片选 */
			w25_SendByte(0x02);								/* 发送AAI命令(地址自动增加编程) */
			w25_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
			w25_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
			w25_SendByte(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */

			for (i = 0; i < 256; i++)
			{
				w25_SendByte(*_pBuf++);					/* 发送数据 */
			}

			W25_CS_HIGH();								/* 禁止片选 */

			w25_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */

			_uiWriteAddr += 256;
		}

		/* 进入写保护状态 */
		W25_CS_LOW();
		w25_SendByte(CMD_DISWR);
		W25_CS_HIGH();

		w25_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: w25_ReadBuffer
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiReadAddr ：首地址
*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tW25.TotalSize)
	{
		return;
	}

	/* 擦除扇区操作 */
	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_READ);							/* 发送读命令 */
	w25_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	w25_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	w25_SendByte(_uiReadAddr & 0xFF);				/* 发送扇区地址低8bit */
	while (_uiSize--)
	{
		*_pBuf++ = w25_SendByte(DUMMY_BYTE);			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	W25_CS_HIGH();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: w25_ReadID
*	功能说明: 读取器件ID
*	形    参:  无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/
uint32_t w25_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_RDID);								/* 发送读ID命令 */
	id1 = w25_SendByte(DUMMY_BYTE);					/* 读ID的第1个字节 */
	id2 = w25_SendByte(DUMMY_BYTE);					/* 读ID的第2个字节 */
	id3 = w25_SendByte(DUMMY_BYTE);					/* 读ID的第3个字节 */
	W25_CS_HIGH();									/* 禁能片选 */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	函 数 名: w25_ReadInfo
*	功能说明: 读取器件ID,并填充器件参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void w25_ReadInfo(void)
{
	/* 自动识别串行Flash型号 */
	{
		g_tW25.ChipID = w25_ReadID();	/* 芯片ID */

		switch (g_tW25.ChipID)
		{
			case SST25VF016B:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* 总容量 = 2M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			case MX25L1606E:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* 总容量 = 2M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			case W25Q64BV:
				g_tW25.TotalSize = 8 * 1024 * 1024;	/* 总容量 = 8M */
				g_tW25.PageSize = 4 * 1024;			/* 页面大小 = 4K */
				break;

			default:		/* 集通字库不支持ID读取 */
				g_tW25.TotalSize = 2 * 1024 * 1024;
				g_tW25.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: w25_SendByte
*	功能说明: 向器件发送一个字节，同时从MISO口线采样器件返回的数据
*	形    参:  _ucByte : 发送的字节值
*	返 回 值: 从MISO口线采样器件返回的数据
*********************************************************************************************************
*/
static uint8_t w25_SendByte(uint8_t _ucValue)
{
	/* 等待上个数据未发送完毕 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);

	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}

/*
*********************************************************************************************************
*	函 数 名: w25_WriteEnable
*	功能说明: 向器件发送写使能命令
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void w25_WriteEnable(void)
{
	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_WREN);								/* 发送命令 */
	W25_CS_HIGH();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: w25_WriteStatus
*	功能说明: 写状态寄存器
*	形    参:  _ucValue : 状态寄存器的值
*	返 回 值: 无
*********************************************************************************************************
*/
static void w25_WriteStatus(uint8_t _ucValue)
{

	if (g_tW25.ChipID == SST25VF016B_ID)
	{
		/* 第1步：先使能写状态寄存器 */
		W25_CS_LOW();									/* 使能片选 */
		w25_SendByte(CMD_EWRSR);							/* 发送命令， 允许写状态寄存器 */
		W25_CS_HIGH();									/* 禁能片选 */

		/* 第2步：再写状态寄存器 */
		W25_CS_LOW();									/* 使能片选 */
		w25_SendByte(CMD_WRSR);							/* 发送命令， 写状态寄存器 */
		w25_SendByte(_ucValue);							/* 发送数据：状态寄存器的值 */
		W25_CS_HIGH();									/* 禁能片选 */
	}
	else
	{
		W25_CS_LOW();									/* 使能片选 */
		w25_SendByte(CMD_WRSR);							/* 发送命令， 写状态寄存器 */
		w25_SendByte(_ucValue);							/* 发送数据：状态寄存器的值 */
		W25_CS_HIGH();									/* 禁能片选 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: w25_WaitForWriteEnd
*	功能说明: 采用循环查询的方式等待器件内部写操作完成
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void w25_WaitForWriteEnd(void)
{
	W25_CS_LOW();									/* 使能片选 */
	w25_SendByte(CMD_RDSR);							/* 发送命令， 读状态寄存器 */
	while((w25_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);	/* 判断状态寄存器的忙标志位 */
	W25_CS_HIGH();									/* 禁能片选 */
}

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
