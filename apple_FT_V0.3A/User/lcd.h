#ifndef __LCD_H
#define __LCD_H
#include "bsp_lcd_st7789.h"

typedef enum
{
    DISP_SHOW_LOG = 0,
    DISP_SHOW_NG,
    DISP_SHOW_PASS,
    DISP_SHOW_WARN,
} disp_show_type_enum;

typedef enum
{
    page_start_line = 0,  
    line_by_line,
    page_tail_line,
}line_mode_e;

#define BSP_LCD_WIDTH       240
#define BSP_LCD_HEIGHT      320

void display_set_addr(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);

void display_fill(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint16_t fill_color);
void display_show_string(char *p, uint16_t font_color, uint16_t bg_color);
void display_show_logger(char *p,  uint16_t bg_color);
/*
 * 函数名：display_show_log
 * 描述  ：LCD 日志模式显示
 * 输入  ：void
 * 输出  ：void
 */
void display_show_log(disp_show_type_enum show_type, const char *fmt, ...);
void lcd_disp(char *p, uint16_t font_color, uint16_t bg_color,line_mode_e mode);

#define LCD_SHOW_LOG(fmt, ...)          display_show_log(DISP_SHOW_LOG, fmt, ##__VA_ARGS__)
#define LCD_SHOW_NG(fmt, ...)           display_show_log(DISP_SHOW_NG, fmt, ##__VA_ARGS__)
#define LCD_SHOW_PASS(fmt, ...)         display_show_log(DISP_SHOW_PASS, fmt, ##__VA_ARGS__)
#define LCD_SHOW_WARN(fmt, ...)         display_show_log(DISP_SHOW_WARN, fmt, ##__VA_ARGS__)
#endif
