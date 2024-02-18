﻿/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : bsp_tft_lcd.c
*	版    本 : V3.4
*	说    明 : 普林芯驰开发板标配的TFT显示器分辨率为240x400，3.0寸宽屏，带PWM背光调节功能。
*				支持的LCD内部驱动芯片型号有: SPFD5420A、OTM4001A、R61509V
*				驱动芯片的访问地址为:  0x60000000
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 plxc  ST固件库版本 V3.5.0版本。
*					a) 取消访问寄存器的结构体，直接定义
*		V2.0    2011-10-16 plxc  增加R61509V驱动，实现图标显示函数
*		V2.1    2012-07-06 plxc  增加RA8875驱动，支持4.3寸屏
*		V2.2    2012-07-13 plxc  改进LCD_DispStr函数，支持12点阵字符;修改LCD_DrawRect,解决差一个像素问题
*		V2.3    2012-08-08 plxc  将底层芯片寄存器操作相关的函数放到单独的文件，支持RA8875
*   	V3.0    2013-05-20 增加图标结构; 修改	LCD_DrawIconActive  修改DispStr函数支持文本透明
*		V3.1    2013-06-12 解决LCD_DispStr()函数BUG，如果内嵌字库中汉字个数多于256，则出现死循环。
*		V3.2    2013-06-28 完善Label控件, 当显示字符串比之前短时，自动清除多余的文字
*		V3.3    2013-06-29 FSMC初始化时，配置时序，写时序和读时序分开设置。 LCD_FSMCConfig 函数。
*		V3.4    2013-07-06 增加显示32位带Alpha图标的函数 LCD_DrawIcon32
*		V3.5    2013-07-24 增加显示32位带Alpha图片的函数 LCD_DrawBmp32
*		V3.6    2013-07-30 修改 DispEdit() 支持12点阵汉字对齐
*
*	Copyright (C), 2010-2011, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

/*
	友情提示:
	TFT控制器和一般的12864点阵显示器的控制器最大的区别在于引入了窗口绘图的机制，这个机制对于绘制局部图形
	是非常有效的。TFT可以先指定一个绘图窗口，然后所有的读写显存的操作均在这个窗口之内，因此它不需要CPU在
	内存中保存整个屏幕的像素数据。
*/

#include "bsp.h"
#include "fonts.h"

/* 下面3个变量，主要用于使程序同时支持不同的屏 */
uint16_t g_ChipID = IC_4001;		/* 驱动芯片ID */
uint16_t g_LcdHeight = 240;			/* 显示屏分辨率-高度 */
uint16_t g_LcdWidth = 400;			/* 显示屏分辨率-宽度 */
uint8_t s_ucBright;					/* 背光亮度参数 */

static void LCD_CtrlLinesConfig(void);
static void LCD_FSMCConfig(void);

/*
*********************************************************************************************************
*	函 数 名: LCD_InitHard
*	功能说明: 初始化LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	uint16_t id;

	/* 配置LCD控制口线GPIO */
	LCD_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMCConfig();

	/* FSMC重置后必须加延迟才能访问总线设备  */
	bsp_DelayMS(20);

	id = SPFD5420_ReadID();  	/* 读取LCD驱动芯片ID */
	if ((id == 0x5420) || (id ==  0xB509) || (id == 0x5520))
	{
		SPFD5420_InitHard();	/* 初始化5420和4001屏硬件 */
		/* g_ChipID 在函数内部设置了 */
	}
	else
	{
		if (RA8875_ReadID() == 0x75)
		{
			g_ChipID = IC_8875;
			RA8875_InitHard();	/* 初始化RA8875芯片 */
		}
		else
		{
			/* 如果未能成功识别，则缺省按4001屏处理 */
			g_ChipID = IC_4001;
			g_LcdHeight = LCD_30_HEIGHT;
			g_LcdWidth = LCD_30_WIDTH;
		}
	}

	LCD_ClrScr(CL_BLACK);	/* 清屏，显示全黑 */

	LCD_SetBackLight(BRIGHT_DEFAULT);	 /* 打开背光，设置为缺省亮度 */
}


/*
*********************************************************************************************************
*	函 数 名: LCD_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参: char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_GetChipDescribe(char *_str)
{
	switch (g_ChipID)
	{
		case IC_5420:
			strcpy(_str, CHIP_STR_5420);
			break;

		case IC_4001:
			strcpy(_str, CHIP_STR_4001);
			break;

		case IC_61509:
			strcpy(_str, CHIP_STR_61509);
			break;

		case IC_8875:
			strcpy(_str, CHIP_STR_8875);
			break;

		default:
			strcpy(_str, "Unknow");
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetHeight
*	功能说明: 读取LCD分辨率之高度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return g_LcdHeight;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetWidth
*	功能说明: 读取LCD分辨率之宽度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return g_LcdWidth;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispOn
*	功能说明: 打开显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOn(void)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DispOn();
	}
	else	/* 61509, 5420, 4001 */
	{
		SPFD5420_DispOn();
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispOff
*	功能说明: 关闭显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOff(void)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DispOff();
	}
	else	/* 61509, 5420, 4001 */
	{
		SPFD5420_DispOff();
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参: _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
	if (g_ChipID == IC_8875)	/* RA8875 分支 */
	{
		RA8875_ClrScr(_usColor);
	}
	else	/* 5420，4001，61509 分支 */
	{
		SPFD5420_ClrScr(_usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispStr
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串
*	形    参:
*		_usX : X坐标，对于3.0寸宽屏，范围为【0 - 399】
*		_usY : Y坐标，对于3.0寸宽屏，范围为 【0 - 239】
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address;
	uint8_t buf[24 * 24 / 8];	/* 最大支持24点阵汉字 */
	uint8_t width;
	uint16_t m;
	uint8_t font_width,font_height, font_bytes;
	uint16_t x, y;
	const uint8_t *pAscDot;

#ifdef USE_SMALL_FONT
	const uint8_t *pHzDot;
#else
	uint32_t AddrHZK;
#endif

	/* 如果字体结构为空指针，则缺省按16点阵 */
	if (_tFont->FontCode == FC_ST_12)
	{
		font_height = 12;
		font_width = 12;
		font_bytes = 24;
		pAscDot = g_Ascii12;

	#ifdef USE_SMALL_FONT
		pHzDot = g_Hz12;
	#else
		AddrHZK = HZK12_ADDR;
	#endif
	}
	else
	{
		/* 缺省是16点阵 */
		font_height = 16;
		font_width = 16;
		font_bytes = 32;
		pAscDot = g_Ascii16;

	#ifdef USE_SMALL_FONT
		pHzDot = g_Hz16;
	#else
		AddrHZK = HZK16_ADDR;
	#endif
	}

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		code1 = *_ptr;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
		if (code1 < 0x80)
		{
			/* 将ascii字符点阵复制到buf */
			memcpy(buf, &pAscDot[code1 * (font_bytes / 2)], (font_bytes / 2));
			width = font_width / 2;
		}
		else
		{
			code2 = *++_ptr;
			if (code2 == 0)
			{
				break;
			}

			/* 计算16点阵汉字点阵地址
				ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
				;
			*/
			#ifdef USE_SMALL_FONT
				m = 0;
				while(1)
				{
					address = m * (font_bytes + 2);
					m++;
					if ((code1 == pHzDot[address + 0]) && (code2 == pHzDot[address + 1]))
					{
						address += 2;
						memcpy(buf, &pHzDot[address], font_bytes);
						break;
					}
					else if ((pHzDot[address + 0] == 0xFF) && (pHzDot[address + 1] == 0xFF))
					{
						/* 字库搜索完毕，未找到，则填充全FF */
						memset(buf, 0xFF, font_bytes);
						break;
					}
				}
			#else	/* 用全字库 */
				/* 此处需要根据字库文件存放位置进行修改 */
				if (code1 >=0xA1 && code1 <= 0xA9 && code2 >=0xA1)
				{
					address = ((code1 - 0xA1) * 94 + (code2 - 0xA1)) * font_bytes + AddrHZK;
				}
				else if (code1 >=0xB0 && code1 <= 0xF7 && code2 >=0xA1)
				{
					address = ((code1 - 0xB0) * 94 + (code2 - 0xA1) + 846) * font_bytes + AddrHZK;
				}
				memcpy(buf, (const uint8_t *)address, font_bytes);
			#endif

				width = font_width;
		}

		y = _usY;
		/* 开始刷LCD */
		for (m = 0; m < font_height; m++)	/* 字符高度 */
		{
			x = _usX;
			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((buf[m * ((2 * width) / font_width) + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
				{
					LCD_PutPixel(x, y, _tFont->FrontColor);	/* 设置像素颜色为文字色 */
				}
				else
				{
					if (_tFont->BackColor != CL_MASK)	/* 透明色 */
					{
						LCD_PutPixel(x, y, _tFont->BackColor);	/* 设置像素颜色为文字背景色 */
					}
				}

				x++;
			}
			y++;
		}

		if (_tFont->Space > 0)
		{
			/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
		}
		_usX += width + _tFont->Space;	/* 列地址递增 */
		_ptr++;			/* 指向下一个字符 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_PutPixel(_usX, _usY, _usColor);
	}
	else
	{
		SPFD5420_PutPixel(_usX, _usY, _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	if (g_ChipID == IC_8875)
	{
		usRGB = RA8875_GetPixel(_usX, _usY);
	}
	else
	{
		usRGB = SPFD5420_GetPixel(_usX, _usY);
	}

	return usRGB;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usX2, _usY2 : 终止点Y坐标
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DrawLine(_usX1 , _usY1 , _usX2, _usY2 , _usColor);
	}
	else
	{
		SPFD5420_DrawLine(_usX1 , _usY1 , _usX2, _usY2 , _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     : 坐标数组
*			_usColor : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DrawRect(_usX, _usY, _usHeight, _usWidth, _usColor);
	}
	else
	{
		SPFD5420_DrawRect(_usX, _usY, _usHeight, _usWidth, _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_Fill_Rect
*	功能说明: 用一个颜色值填充一个矩形。【emWin 中有同名函数 LCD_FillRect，因此加了下划线区分】
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_Fill_Rect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	uint16_t i;

	if (g_ChipID == IC_8875)
	{
		RA8875_FillRect(_usX, _usY, _usHeight, _usWidth, _usColor);
	}
	else
	{
		for (i = 0; i < _usHeight; i++)
		{
			SPFD5420_DrawHLine(_usX, _usY, _usX + _usWidth - 1, _usColor);
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DrawCircle(_usX, _usY, _usRadius, _usColor);
	}
	else
	{
		SPFD5420_DrawCircle(_usX, _usY, _usRadius, _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序: 从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  : 图片高度
*			_usWidth   : 图片宽度
*			_ptr       : 图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	if (g_ChipID == IC_8875)
	{
		RA8875_DrawBMP(_usX, _usY, _usHeight, _usWidth, _ptr);
	}
	else
	{
		SPFD5420_DrawBMP(_usX, _usY, _usHeight, _usWidth, _ptr);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawWin
*	功能说明: 在LCD上绘制一个窗口
*	形    参: 结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawWin(WIN_T *_pWin)
{
#if 1
	uint16_t TitleHegiht;

	TitleHegiht = 20;

	/* 绘制窗口外框 */
	LCD_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
	LCD_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

	/* 窗口标题栏 */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

	/* 窗体填充 */
	LCD_Fill_Rect(_pWin->Left + 2, _pWin->Top + TitleHegiht + 2, _pWin->Height - 4 - TitleHegiht,
		_pWin->Width - 4, WIN_BODY_COLOR);

	LCD_DispStr(_pWin->Left + 3, _pWin->Top + 2, _pWin->pCaption, _pWin->Font);
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t TitleHegiht;

		TitleHegiht = 28;

		/* 绘制窗口外框 */
		RA8875_DrawRect(_pWin->Left, _pWin->Top, _pWin->Height, _pWin->Width, WIN_BORDER_COLOR);
		RA8875_DrawRect(_pWin->Left + 1, _pWin->Top + 1, _pWin->Height - 2, _pWin->Width - 2, WIN_BORDER_COLOR);

		/* 窗口标题栏 */
		RA8875_FillRect(_pWin->Left + 2, _pWin->Top + 2, TitleHegiht, _pWin->Width - 4, WIN_TITLE_COLOR);

		/* 窗体填充 */
		RA8875_FillRect(_pWin->Left + 2, _pWin->Top + TitleHegiht + 2, _pWin->Height - 4 - TitleHegiht, _pWin->Width - 4, WIN_BODY_COLOR);

		//RA8875_SetFont(_pWin->Font.FontCode, 0, 0);
		RA8875_SetFont(RA_FONT_24, 0, 0);

		RA8875_SetBackColor(WIN_TITLE_COLOR);
		RA8875_SetFrontColor(WIN_CAPTION_COLOR);
		RA8875_DispStr(_pWin->Left + 3, _pWin->Top + 2, _pWin->Caption);
	}
	else
	{
		;
	}
#endif
}


/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIcon
*	功能说明: 在LCD上绘制一个图标，四角自动切为弧脚
*	形    参: _pIcon : 图标结构
*			  _tFont : 字体属性
*			  _ucFocusMode : 焦点模式。0 表示正常图标  1表示选中的图标
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	p = _tIcon->pBmp;
	for (y = 0; y < _tIcon->Height; y++)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			/* 将图标的4个直角切割为弧角，弧角外是背景图标 */
			if ((y == 0 && (x < 6 || x > _tIcon->Width - 7)) ||
				(y == 1 && (x < 4 || x > _tIcon->Width - 5)) ||
				(y == 2 && (x < 3 || x > _tIcon->Width - 4)) ||
				(y == 3 && (x < 2 || x > _tIcon->Width - 3)) ||
				(y == 4 && (x < 1 || x > _tIcon->Width - 2)) ||
				(y == 5 && (x < 1 || x > _tIcon->Width - 2))	||

				(y == _tIcon->Height - 1 && (x < 6 || x > _tIcon->Width - 7)) ||
				(y == _tIcon->Height - 2 && (x < 4 || x > _tIcon->Width - 5)) ||
				(y == _tIcon->Height - 3 && (x < 3 || x > _tIcon->Width - 4)) ||
				(y == _tIcon->Height - 4 && (x < 2 || x > _tIcon->Width - 3)) ||
				(y == _tIcon->Height - 5 && (x < 1 || x > _tIcon->Width - 2)) ||
				(y == _tIcon->Height - 6 && (x < 1 || x > _tIcon->Width - 2))
				)
			{
				;
			}
			else
			{
				if (_ucFocusMode != 0)	/* 1表示选中的图标 */
				{
					/* 降低原始像素的亮度，实现图标被激活选中的效果 */
					uint16_t R,G,B;
					uint16_t bright = 15;

					/* rrrr rggg gggb bbbb */
					R = (usNewRGB & 0xF800) >> 11;
					G = (usNewRGB & 0x07E0) >> 5;
					B =  usNewRGB & 0x001F;
					if (R > bright)
					{
						R -= bright;
					}
					else
					{
						R = 0;
					}
					if (G > 2 * bright)
					{
						G -= 2 * bright;
					}
					else
					{
						G = 0;
					}
					if (B > bright)
					{
						B -= bright;
					}
					else
					{
						B = 0;
					}
					usNewRGB = (R << 11) + (G << 5) + B;
				}

				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
		}
	}

	/* 绘制图标下的文字 */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* 如果图标文本长度为0，则不显示 */
		}

		/* 计算文本的总宽度 */
		if (_tFont->FontCode == FC_ST_12)		/* 12点阵 */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* 水平居中 */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
	alpha范围0~32；
	src是从屏中读回的数据，dst是要混合的颜色，dst用白的就是普通透明效果，用其他颜色就像滤镜一样。
*/
uint16_t Blend565(uint16_t src, uint16_t dst, uint8_t alpha)
{
	uint32_t src2;
	uint32_t dst2;

	src2 = ((src << 16) |src) & 0x07E0F81F;
	dst2 = ((dst << 16) | dst) & 0x07E0F81F;
	dst2 = ((((dst2 - src2) * alpha) >> 5) + src2) & 0x07E0F81F;
	return (dst2 >> 16) | dst2;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIcon32
*	功能说明: 在LCD上绘制一个图标, 带有透明信息的位图(32位， RGBA). 图标下带文字
*	形    参: _pIcon : 图标结构
*			  _tFont : 字体属性
*			  _ucFocusMode : 焦点模式。0 表示正常图标  1表示选中的图标
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon32(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* 用于记录窗口内的相对坐标 */
	uint8_t R1,G1,B1,A;	/* 新像素色彩分量 */
	uint8_t R0,G0,B0;	/* 旧像素色彩分量 */

	p = (const uint8_t *)_tIcon->pBmp;
	p += 54;		/* 直接指向图像数据区 */

	/* 按照BMP位图次序，从左至右，从上至下扫描 */
	for (y = _tIcon->Height - 1; y >= 0; y--)
	{
		for (x = 0; x < _tIcon->Width; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha 值(透明度)，0-255, 0表示透明，1表示不透明, 中间值表示透明度 */

			if (A == 0x00)	/* 需要透明,显示背景 */
			{
				;	/* 不用刷新背景 */
			}
			else if (A == 0xFF)	/* 完全不透明， 显示新像素 */
			{
				usNewRGB = RGB(R1, G1, B1);
				if (_ucFocusMode == 1)
				{
					usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				}
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
			else 	/* 半透明 */
			{
				/* 计算公式： 实际显示颜色 = 前景颜色 * Alpha / 255 + 背景颜色 * (255-Alpha) / 255 */
				usOldRGB = LCD_GetPixel(x + _tIcon->Left, y + _tIcon->Top);
				R0 = RGB565_R(usOldRGB);
				G0 = RGB565_G(usOldRGB);
				B0 = RGB565_B(usOldRGB);

				R1 = (R1 * A) / 255 + R0 * (255 - A) / 255;
				G1 = (G1 * A) / 255 + G0 * (255 - A) / 255;
				B1 = (B1 * A) / 255 + B0 * (255 - A) / 255;
				usNewRGB = RGB(R1, G1, B1);
				if (_ucFocusMode == 1)
				{
					usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				}
				LCD_PutPixel(x + _tIcon->Left, y + _tIcon->Top, usNewRGB);
			}
		}
	}

	/* 绘制图标下的文字 */
	{
		uint16_t len;
		uint16_t width;

		len = strlen(_tIcon->Text);

		if  (len == 0)
		{
			return;	/* 如果图标文本长度为0，则不显示 */
		}

		/* 计算文本的总宽度 */
		if (_tFont->FontCode == FC_ST_12)		/* 12点阵 */
		{
			width = 6 * (len + _tFont->Space);
		}
		else	/* FC_ST_16 */
		{
			width = 8 * (len + _tFont->Space);
		}


		/* 水平居中 */
		x = (_tIcon->Left + _tIcon->Width / 2) - width / 2;
		y = _tIcon->Top + _tIcon->Height + 2;
		LCD_DispStr(x, y, (char *)_tIcon->Text, _tFont);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawBmp32
*	功能说明: 在LCD上绘制一个32位的BMP图, 带有透明信息的位图(32位， RGBA)
*	形    参: _usX, _usY : 显示坐标
*			  _usHeight, _usWidth : 图片高度和宽度
*			  _pBmp : 图片数据（带BMP文件头）
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawBmp32(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_pBmp)
{
	const uint8_t *p;
	uint16_t usOldRGB, usNewRGB;
	int16_t x, y;		/* 用于记录窗口内的相对坐标 */
	uint8_t R1,G1,B1,A;	/* 新像素色彩分量 */
	uint8_t R0,G0,B0;	/* 旧像素色彩分量 */

	p = (const uint8_t *)_pBmp;
	p += 54;		/* 直接指向图像数据区 */

	/* 按照BMP位图次序，从左至右，从上至下扫描 */
	for (y = _usHeight - 1; y >= 0; y--)
	{
		for (x = 0; x < _usWidth; x++)
		{
			B1 = *p++;
			G1 = *p++;
			R1 = *p++;
			A = *p++;	/* Alpha 值(透明度)，0-255, 0表示透明，1表示不透明, 中间值表示透明度 */

			if (A == 0x00)	/* 需要透明,显示背景 */
			{
				;	/* 不用刷新背景 */
			}
			else if (A == 0xFF)	/* 完全不透明， 显示新像素 */
			{
				usNewRGB = RGB(R1, G1, B1);
				//if (_ucFocusMode == 1)
				//{
				//	usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				//}
				LCD_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
			else 	/* 半透明 */
			{
				/* 计算公式： 实际显示颜色 = 前景颜色 * Alpha / 255 + 背景颜色 * (255-Alpha) / 255 */
				usOldRGB = LCD_GetPixel(x + _usX, y + _usY);
				R0 = RGB565_R(usOldRGB);
				G0 = RGB565_G(usOldRGB);
				B0 = RGB565_B(usOldRGB);

				R1 = (R1 * A) / 255 + R0 * (255 - A) / 255;
				G1 = (G1 * A) / 255 + G0 * (255 - A) / 255;
				B1 = (B1 * A) / 255 + B0 * (255 - A) / 255;
				usNewRGB = RGB(R1, G1, B1);
				//if (_ucFocusMode == 1)
				//{
				//	usNewRGB = Blend565(usNewRGB, CL_YELLOW, 10);
				//}
				LCD_PutPixel(x + _usX, y + _usY, usNewRGB);
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawLabel
*	功能说明: 绘制一个文本标签
*	形    参: 结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawLabel(LABEL_T *_pLabel)
{
#if 1
	char dispbuf[256];
	uint16_t i;
	uint16_t NewLen;

	NewLen = strlen(_pLabel->pCaption);

	if (NewLen > _pLabel->MaxLen)
	{
		LCD_DispStr(_pLabel->Left, _pLabel->Top, _pLabel->pCaption, _pLabel->Font);
		_pLabel->MaxLen = NewLen;
	}
	else
	{
		for (i = 0; i < NewLen; i++)
		{
			dispbuf[i] = _pLabel->pCaption[i];
		}
		for (; i < _pLabel->MaxLen; i++)
		{
			dispbuf[i] = ' ';		/* 末尾填充空格 */
		}
		dispbuf[i] = 0;
		LCD_DispStr(_pLabel->Left, _pLabel->Top, dispbuf, _pLabel->Font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		RA8875_SetFont(_pLabel->Font->FontCode, 0, 0);	/* 设置32点阵字体，行间距=0，字间距=0 */

		RA8875_SetBackColor(_pLabel->Font->BackColor);
		RA8875_SetFrontColor(_pLabel->Font->FrontColor);

		RA8875_DispStr(_pLabel->Left, _pLabel->Top, _pLabel->Caption);
	}
	else
	{

	}
#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawCheckBox
*	功能说明: 绘制一个检查框
*	形    参: 结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawCheckBox(CHECK_T *_pCheckBox)
{
#if 1
	uint16_t x, y;

	/* 目前只做了16点阵汉字的大小 */

	/* 绘制外框 */
	x = _pCheckBox->Left;
	LCD_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
	LCD_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
	LCD_Fill_Rect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

	/* 绘制文本标签 */
	x = _pCheckBox->Left + CHECK_BOX_W + 2;
	y = _pCheckBox->Top + CHECK_BOX_H / 2 - 8;
	LCD_DispStr(x, y, _pCheckBox->pCaption, _pCheckBox->Font);

	if (_pCheckBox->Checked)
	{
		FONT_T font;

	    font.FontCode = FC_ST_16;
		font.BackColor = CL_MASK;
		font.FrontColor = CHECK_BOX_CHECKED_COLOR;	/* 钩的颜色 */
		font.Space = 0;
		x = _pCheckBox->Left;
		LCD_DispStr(x + 3, _pCheckBox->Top + 3, "√", &font);
	}
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t x;

		RA8875_SetFont(_pCheckBox->Font.FontCode, 0, 0);	/* 设置32点阵字体，行间距=0，字间距=0 */

		/* 绘制标签 */
		//RA8875_SetBackColor(_pCheckBox->Font.BackColor);
		RA8875_SetBackColor(WIN_BODY_COLOR);
		RA8875_SetFrontColor(_pCheckBox->Font.FrontColor);
		RA8875_DispStr(_pCheckBox->Left, _pCheckBox->Top, _pCheckBox->Caption);

		/* 绘制外框 */
		x = _pCheckBox->Left + _pCheckBox->Width - CHECK_BOX_W;
		RA8875_DrawRect(x, _pCheckBox->Top, CHECK_BOX_H, CHECK_BOX_W, CHECK_BOX_BORDER_COLOR);
		RA8875_DrawRect(x + 1, _pCheckBox->Top + 1, CHECK_BOX_H - 2, CHECK_BOX_W - 2, CHECK_BOX_BORDER_COLOR);
		RA8875_FillRect(x + 2, _pCheckBox->Top + 2, CHECK_BOX_H - 4, CHECK_BOX_W - 4, CHECK_BOX_BACK_COLOR);

		if (_pCheckBox->Checked)
		{
			RA8875_SetBackColor(CHECK_BOX_BACK_COLOR);
			RA8875_SetFrontColor(CL_RED);
			RA8875_DispStr(x + 3, _pCheckBox->Top + 3, "√");
		}
	}
	else
	{

	}
#endif

}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawEdit
*	功能说明: 在LCD上绘制一个编辑框
*	形    参: _pEdit 编辑框结构体指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawEdit(EDIT_T *_pEdit)
{
#if 1
	uint16_t len, x, y;
	uint8_t width;

	/* 仿XP风格，平面编辑框 */
	LCD_DrawRect(_pEdit->Left, _pEdit->Top, _pEdit->Height, _pEdit->Width, EDIT_BORDER_COLOR);
	LCD_Fill_Rect(_pEdit->Left + 1, _pEdit->Top + 1, _pEdit->Height - 2, _pEdit->Width - 2, EDIT_BACK_COLOR);

	/* 文字居中 */
	if (_pEdit->Font->FontCode == FC_ST_12)
	{
		width = 6;
	}
	else
	{
		width = 8;
	}
	len = strlen(_pEdit->pCaption);
	x = _pEdit->Left +  (_pEdit->Width - len * width) / 2;
	y = _pEdit->Top + _pEdit->Height / 2 - width;

	LCD_DispStr(x, y, _pEdit->pCaption, _pEdit->Font);
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t len, x;

		/* 仿XP风格，平面编辑框 */
		RA8875_DrawRect(_pEdit->Left, _pEdit->Top, _pEdit->Height, _pEdit->Width, EDIT_BORDER_COLOR);
		RA8875_FillRect(_pEdit->Left + 1, _pEdit->Top + 1, _pEdit->Height - 2, _pEdit->Width - 2, EDIT_BACK_COLOR);

		RA8875_SetFont(_pEdit->Font.FontCode, 0, 0);	/* 设置32点阵字体，行间距=0，字间距=0 */
		RA8875_SetFrontColor(_pEdit->Font.FrontColor);
		RA8875_SetBackColor(EDIT_BACK_COLOR);

		/* 文字居中 */
		len = strlen(_pEdit->Caption);
		x = (_pEdit->Width - len * 16) / 2;

		RA8875_DispStr(_pEdit->Left + x, _pEdit->Top + 3, _pEdit->Caption);
	}
	else
	{
		//SPFD5420_DrawCircle(_usX, _usY, _usRadius, _usColor);
	}
#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawEdit
*	功能说明: 在LCD上绘制一个编辑框
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  : 图片高度
*			_usWidth   : 图片宽度
*			_ptr       : 图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawButton(BUTTON_T *_pBtn)
{
#if 1
		uint16_t len, fwidth, x, y;

		if (_pBtn->Focus == 1)
		{
			/* 仿XP风格，平面编辑框 */
			LCD_DrawRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, BUTTON_BORDER_COLOR);
			LCD_DrawRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, BUTTON_BORDER1_COLOR);
			LCD_DrawRect(_pBtn->Left + 2, _pBtn->Top + 2, _pBtn->Height - 4, _pBtn->Width - 4, BUTTON_BORDER2_COLOR);

			LCD_Fill_Rect(_pBtn->Left + 3, _pBtn->Top + 3, _pBtn->Height - 6, _pBtn->Width - 6, BUTTON_ACTIVE_COLOR);	/* 选中后的底色 */
		}
		else
		{
			/* 仿XP风格，平面编辑框 */
			LCD_DrawRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, BUTTON_BORDER_COLOR);
			LCD_DrawRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, BUTTON_BORDER1_COLOR);
			LCD_DrawRect(_pBtn->Left + 2, _pBtn->Top + 2, _pBtn->Height - 4, _pBtn->Width - 4, BUTTON_BORDER2_COLOR);

			LCD_Fill_Rect(_pBtn->Left + 3, _pBtn->Top + 3, _pBtn->Height - 6, _pBtn->Width - 6, BUTTON_BACK_COLOR);		/* 未选中的底色 */
		}

		/* 文字居中 */
		len = strlen(_pBtn->pCaption);

		if (_pBtn->Font->FontCode == FC_ST_16)
		{
			fwidth = 8;		/* 字体宽度 */
		}
		else if (_pBtn->Font->FontCode == FC_ST_12)
		{
			fwidth = 6;		/* 字体宽度 */
		}
		else
		{
			fwidth = 8;		/* 字体宽度 */
		}
		x = _pBtn->Left + _pBtn->Width / 2 - (len * fwidth) / 2;
		y = _pBtn->Top + _pBtn->Height / 2 - fwidth;

		LCD_DispStr(x, y, _pBtn->pCaption, _pBtn->Font);
#else
	if (g_ChipID == IC_8875)
	{
		uint16_t len, x, y;

		if (_pBtn->Focus == 1)
		{
			/* 仿XP风格，平面编辑框 */
			RA8875_DrawRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, BUTTON_BORDER_COLOR);
			RA8875_DrawRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, BUTTON_BORDER1_COLOR);
			RA8875_DrawRect(_pBtn->Left + 2, _pBtn->Top + 2, _pBtn->Height - 4, _pBtn->Width - 4, BUTTON_BORDER2_COLOR);

			RA8875_FillRect(_pBtn->Left + 3, _pBtn->Top + 3, _pBtn->Height - 6, _pBtn->Width - 6, BUTTON_ACTIVE_COLOR);

			RA8875_SetBackColor(BUTTON_ACTIVE_COLOR);
		}
		else
		{
			/* 仿XP风格，平面编辑框 */
			RA8875_DrawRect(_pBtn->Left, _pBtn->Top, _pBtn->Height, _pBtn->Width, BUTTON_BORDER_COLOR);
			RA8875_DrawRect(_pBtn->Left + 1, _pBtn->Top + 1, _pBtn->Height - 2, _pBtn->Width - 2, BUTTON_BORDER1_COLOR);
			RA8875_DrawRect(_pBtn->Left + 2, _pBtn->Top + 2, _pBtn->Height - 4, _pBtn->Width - 4, BUTTON_BORDER2_COLOR);

			RA8875_FillRect(_pBtn->Left + 3, _pBtn->Top + 3, _pBtn->Height - 6, _pBtn->Width - 6, BUTTON_BACK_COLOR);

			RA8875_SetBackColor(BUTTON_BACK_COLOR);
		}

		#if 1	/* 按钮文字字体和颜色固定 */
			if (strcmp(_pBtn->Caption, "←") == 0)	/* 退格键特殊处理 */
			{
				/* 退格键符号是单像素笔画，太细了不协调，因此特殊处理 */
				RA8875_SetFont(RA_FONT_16, 0, 0);
				RA8875_SetFrontColor(CL_BLACK);
				RA8875_SetTextZoom(RA_SIZE_X2, RA_SIZE_X2);	/* 放大2倍 */
			}
			else
			{
				RA8875_SetFont(RA_FONT_16, 0, 0);
				RA8875_SetFrontColor(CL_BLACK);
				RA8875_SetTextZoom(RA_SIZE_X1, RA_SIZE_X1);	/* 放大1倍 */
			}
		#else	/* 按钮文字字体和颜色有应用程序指定 */
			RA8875_SetFont(_pBtn->Font.FontCode, 0, 0);
			RA8875_SetFrontColor(_pBtn->Font.FrontColor);
		#endif

		/* 文字居中 */
		len = strlen(_pBtn->Caption);

		/* 此处统计不等宽字符有问题。暂时特殊处理下 */
		if (len != 3)
		{
			x = _pBtn->Left + (_pBtn->Width - len * 16) / 2;
		}
		else
		{
			x = _pBtn->Left + (_pBtn->Width - len * 20) / 2;
		}

		/* 对特殊字符特殊处理 */
		if ((len == 1) && (_pBtn->Caption[0] == '.'))
		{
			y = _pBtn->Top + 3;
			x += 3;
		}
		else
		{
			y = _pBtn->Top + 3;
		}

		RA8875_DispStr(x, y, _pBtn->Caption);

		RA8875_SetTextZoom(RA_SIZE_X1, RA_SIZE_X1);	/* 还原放大1倍 */
	}
#endif
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawGroupBox
*	功能说明: 在LCD上绘制一个分组框
*	形    参: _pBox 分组框
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawGroupBox(GROUP_T *_pBox)
{
	uint16_t x, y;

	/* 画阴影线 */
	LCD_DrawRect(_pBox->Left + 1, _pBox->Top + 5, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER2);

	/* 画主框线 */
	LCD_DrawRect(_pBox->Left, _pBox->Top + 4, _pBox->Height, _pBox->Width - 1, CL_BOX_BORDER1);

	/* 显示分组框标题（文字在左上角） */
	x = _pBox->Left + 9;
	y = _pBox->Top;
	LCD_DispStr(x, y, _pBox->pCaption, _pBox->Font);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispControl
*	功能说明: 绘制控件
*	形    参: _pControl 控件指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispControl(void *_pControl)
{
	uint8_t id;

	id = *(uint8_t *)_pControl;	/* 读取ID */

	switch (id)
	{
		case ID_ICON:
			//void LCD_DrawIcon(const ICON_T *_tIcon, FONT_T *_tFont, uint8_t _ucFocusMode);
			break;

		case ID_WIN:
			LCD_DrawWin((WIN_T *)_pControl);
			break;

		case ID_LABEL:
			LCD_DrawLabel((LABEL_T *)_pControl);
			break;

		case ID_BUTTON:
			LCD_DrawButton((BUTTON_T *)_pControl);
			break;

		case ID_CHECK:
			LCD_DrawCheckBox((CHECK_T *)_pControl);
			break;

		case ID_EDIT:
			LCD_DrawEdit((EDIT_T *)_pControl);
			break;

		case ID_GROUP:
			LCD_DrawGroupBox((GROUP_T *)_pControl);
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	普林芯驰FT_F4开发板接线方法:

	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FSMC_NWE		--- 写控制信号，WE = Output Enable ， N 表示低有效
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15
	PD13/FSMC_A18		--- 地址 RS
	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		--- 和主片选一起译码
	PE5/FSMC_A21		--- 和主片选一起译码
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PG12/FSMC_NE4		--- 主片选（TFT, OLED 和 AD7606）

	PC3/TP_INT			--- 触摸芯片中断 （对于RA8875屏，是RA8875输出的中断)  本例程未使用硬件中断

	---- 下面是 TFT LCD接口其他信号 （FSMC模式不使用）----
	PD3/LCD_BUSY		--- 触摸芯片忙       （RA8875屏是RA8875芯片的忙信号)
	PF6/LCD_PWM			--- LCD背光PWM控制  （RA8875屏无需此脚，背光由RA8875控制)

	PI10/TP_NCS			--- 触摸芯片的片选		(RA8875屏无需SPI接口触摸芯片）
	PB3/SPI3_SCK		--- 触摸芯片SPI时钟		(RA8875屏无需SPI接口触摸芯片）
	PB4/SPI3_MISO		--- 触摸芯片SPI数据线MISO(RA8875屏无需SPI接口触摸芯片）
	PB5/SPI3_MOSI		--- 触摸芯片SPI数据线MOSI(RA8875屏无需SPI接口触摸芯片）
*/
static void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能FSMC时钟 */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/* 使能 GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

	/* 设置 PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) 为复用推挽输出 */

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 设置 PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) 为复用推挽输出 */

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5 , GPIO_AF_FSMC);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 设置 PD.13(A18 (RS))  为复用推挽输出 */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 设置 PG12 (LCD/CS)) 为复用推挽输出 */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  init;
	FSMC_NORSRAMTimingInitTypeDef  timingWrite;
	FSMC_NORSRAMTimingInitTypeDef  timingRead;

	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 4 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	/* 摄像头DMA麻点，需设置 4 0 5 2 0 0 */
	timingWrite.FSMC_AddressSetupTime = 4;
	timingWrite.FSMC_AddressHoldTime = 0;
	timingWrite.FSMC_DataSetupTime = 6;
	timingWrite.FSMC_BusTurnAroundDuration = 1;
	timingWrite.FSMC_CLKDivision = 0;
	timingWrite.FSMC_DataLatency = 0;
	timingWrite.FSMC_AccessMode = FSMC_AccessMode_A;

	timingRead.FSMC_AddressSetupTime = 4;
	timingRead.FSMC_AddressHoldTime = 0;
	timingRead.FSMC_DataSetupTime = 8;
	timingRead.FSMC_BusTurnAroundDuration = 1;
	timingRead.FSMC_CLKDivision = 0;
	timingRead.FSMC_DataLatency = 0;
	timingRead.FSMC_AccessMode = FSMC_AccessMode_A;


	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* 注意旧库无这个成员 */
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FSMC_ReadWriteTimingStruct = &timingRead;
	init.FSMC_WriteTimingStruct = &timingWrite;

	FSMC_NORSRAMInit(&init);

	/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetBackLight
*	功能说明: 初始化控制LCD背景光的GPIO,配置为PWM模式。
*			当关闭背光时，将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)；将TIM3关闭 省电
*	形    参: _bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	s_ucBright =  _bright;	/* 保存背光值 */

	if (g_ChipID == IC_8875)
	{
		RA8875_SetBackLight(_bright);
	}
	else
	{
		SPFD5420_SetBackLight(_bright);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetBackLight
*	功能说明: 获得背光亮度参数
*	形    参: 无
*	返 回 值: 背光亮度参数
*********************************************************************************************************
*/
uint8_t LCD_GetBackLight(void)
{
	return s_ucBright;
}

/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
