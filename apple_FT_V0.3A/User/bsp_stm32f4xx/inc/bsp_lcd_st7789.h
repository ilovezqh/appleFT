#ifndef __BSP_LCD_ST7789_H__
#define __BSP_LCD_ST7789_H__

#include "stdbool.h"



//画笔颜色
#define WHITE               0xFFFF
#define BLACK         	    0x0000	  
#define BLUE           	    0x001F  
#define BRED                0xF81F
#define GRED                0xFFE0
#define GBLUE               0x07FF
#define RED           	    0xF800
#define MAGENTA       	    0xF81F
#define GREEN         	    0x07E0
#define CYAN          	    0x7FFF
#define YELLOW        	    0xFFE0
#define BROWN 			    0xBC40 //棕色
#define BRRED 			    0xFC07 //棕红色
#define GRAY  			    0x8430 //灰色
#define DARKBLUE      	    0x01CF	//深蓝色
#define LIGHTBLUE      	    0x7D7C	//浅蓝色  
#define GRAYBLUE       	    0x5458 //灰蓝色
#define LIGHTGREEN     	    0x841F //浅绿色
#define LGRAY 			    0xC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE           0xA651 //浅灰蓝色(中间层颜色)
#define LBBLUE              0x2B12 //浅棕蓝色(选择条目的反色)

/*
 * 函数名：bsp_lcd_init
 * 描述  ：LCD显示初始化
 * 输入  ：void
 * 输出  ：void
 */
void bsp_lcd_init(void);

/*
 * 函数名：bsp_lcd_send_data
 * 描述  ：LCD发送数据
 * 输入  ：uint8_t *send_data       需要发送的显示数据
 *         uint16_t data_len     显示数据长度
 * 输出  ：void
 */
bool bsp_lcd_send_data(uint8_t *send_data, uint16_t data_len);

/*
 * 函数名：bsp_lcd_send_cmd
 * 描述  ：LCD发送命令
 * 输入  ：uint8_t *send_cmd       需要发送的命令数据
 *         uint16_t cmd_len     命令数据长度
 * 输出  ：void
 */
bool bsp_lcd_send_cmd(uint8_t *send_cmd, uint16_t cmd_len);

#endif

