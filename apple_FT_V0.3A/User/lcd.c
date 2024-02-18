#include "bsp.h"
#include "lcd.h"
#include "string.h"
#include "stdarg.h"
#include "lcd_font.h"
#define DEBUG(fmt,...)			printf(fmt, ##__VA_ARGS__)


#define DISPLAY_QUEUE_ITEM_SIZE 64
static char log_buf[DISPLAY_QUEUE_ITEM_SIZE];

void display_set_addr(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
    uint8_t send_cmd[3] = { 0x2A, 0x2B, 0x2C };
    uint8_t send_data[4] = { 0x00 };

    send_data[0] = (start_x >> 8) & 0xFF;
    send_data[1] = start_x & 0xFF;
    send_data[2] = (end_x >> 8) & 0xFF;
    send_data[3] = end_x & 0xFF;
    bsp_lcd_send_cmd(send_cmd, 1);
    bsp_lcd_send_data(send_data, 4);

    send_data[0] = (start_y >> 8) & 0xFF;
    send_data[1] = start_y & 0xFF;
    send_data[2] = (end_y >> 8) & 0xFF;
    send_data[3] = end_y & 0xFF;
    bsp_lcd_send_cmd(send_cmd + 1, 1);
    bsp_lcd_send_data(send_data, 4);

    bsp_lcd_send_cmd(send_cmd + 2, 1);
}

void display_fill(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint16_t fill_color)
{
    uint8_t color[2] = { (fill_color >> 8) & 0xFF, fill_color & 0xFF };

    display_set_addr(start_x, start_y, end_x - 1, end_y - 1);

    for(uint32_t i = 0; i < end_y; i++)
    {
        for(uint32_t x = 0; x < end_x; x++)
        {
            bsp_lcd_send_data(color, 2);
        }
    }
}

void display_show_char(uint16_t x, uint16_t y, uint8_t num, uint16_t font_color, uint16_t bg_color)
{
    uint8_t f_color[2] = { (font_color >> 8) & 0xFF, font_color & 0xFF};
    uint8_t b_color[2] = { (bg_color >> 8) & 0xFF, bg_color & 0xFF};
	uint8_t i, t;

	num -= ' ';    //得到偏移后的值
	display_set_addr(x, y, x + 7, y + 15);  //设置光标位置 
	for(i = 0; i < 16; i++)
	{ 
		for(t = 0; t < 8; t++)
		{
    		if(lcd_ascii_1608[num][i] & (1 << t))
            {
                bsp_lcd_send_data(f_color, 2);
    		}
    		else
    		{
                bsp_lcd_send_data(b_color, 2);
    		}
		}
	}
}

#define LCD_SHOW_LOG(fmt, ...)          display_show_log(DISP_SHOW_LOG, fmt, ##__VA_ARGS__)
void display_show_logger(char *p, uint16_t bg_color)
{
    uint16_t font_color;
    uint32_t now_x = 0, now_y = 0;
    static uint8_t fill_buff[64];
    uint8_t flg_ok_line=0;
    //display_fill(0, now_y, 240, now_y + 32, WHITE);
     while( *p != '\0' )
     {
         if(!flg_ok_line)
         {
            if( memcmp(p,"SUCCESS", 7) == 0 )
            {
                font_color = GREEN;
                flg_ok_line = 1;
                p=p+8;
            }
            else if( memcmp(p,"FINISH", 6) == 0 )
            {
                font_color = YELLOW;
                flg_ok_line = 1;
                p=p+7;
            }
            else if( memcmp(p,"fail", 4) == 0 )
            {
                font_color = RED;
                flg_ok_line = 1;
                p=p+5;
            }
            else if( memcmp(p,"ERROR", 5) == 0 )
            {
                font_color = RED;
                flg_ok_line = 1;
                p=p+6;
            }
            
         }
         
         
         if(*p =='\0') break;
         
         if(!flg_ok_line) 
         {
             p++;
             continue;
         }
         if(*p == '\r')p++;
         if(*p == '\n' )
         {
            flg_ok_line = 0;
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
            p++;
            continue;
        }
        if(now_x >= 240)
        {
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
        }
		display_show_char(now_x, now_y, *p, font_color, bg_color);
        now_x += 8;
		p++;
         
     } 
     for( ;now_y<=320; now_y+=16)
     {
        now_x = 0;
        memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
        display_set_addr(0, now_y, 240, now_y + 32);
        for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
        {
            bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
        }
     }
     
            
            
     
    #if 0
	while(*p != '\0')
	{
        do
        {
            p++;
        }while(memcmp(p,"SUCCESS", 7) != 0);
        
            
        if(*p == '\r')p++;
        if(*p == '\n')
        {
            
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
            p++;
            continue;
        }
        if(now_x >= 240)
        {
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
        }
		display_show_char(now_x, now_y, *p, font_color, bg_color);
        now_x += 8;
		p++;
	}
    #endif
}

void display_show_string(char *p, uint16_t font_color, uint16_t bg_color)
{
    static uint32_t now_x = 0, now_y = 0;
    static uint8_t fill_buff[64];

    //display_fill(0, now_y, 240, now_y + 32, WHITE);
	while(*p != '\0')
	{
        if(*p == '\r')p++;
        if(*p == '\n')
        {
          
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
            p++;
            continue;
        }
        if(now_x >= 240)
        {
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
        }
		display_show_char(now_x, now_y, *p, font_color, bg_color);
        now_x += 8;
		p++;
	}
}



void lcd_disp(char *p, uint16_t font_color, uint16_t bg_color,line_mode_e mode)
{
    static uint32_t now_x = 0, now_y = 0;
    static uint8_t fill_buff[64];

    switch(mode)
    {
    case page_start_line:
        now_x = 0; now_y = 0;
        break;
    case line_by_line:
        break;
    case page_tail_line:
        now_x = 0;now_y=320-16;
        break;
    }
    //display_fill(0, now_y, 240, now_y + 32, WHITE);
	while(*p != '\0')
	{
        if(*p == '\r')p++;
        if(*p == '\n')
        {
          
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
            p++;
            continue;
        }
        if(now_x >= 240)
        {
            now_x = 0;
            now_y += 16;
            if(now_y >= 320)
            {
                now_y = 0;
            }
            memset((uint16_t *)fill_buff, bg_color, sizeof(fill_buff) / sizeof(bg_color));
            display_set_addr(0, now_y, 240, now_y + 32);
            for(uint32_t i = 0; i < 7680 / sizeof(fill_buff); i++)
            {
                bsp_lcd_send_data(fill_buff, sizeof(fill_buff));
            }
        }
		display_show_char(now_x, now_y, *p, font_color, bg_color);
        now_x += 8;
		p++;
	}
}

/*
 * 函数名：display_show_log
 * 描述  ：LCD 日志模式显示
 * 输入  ：void
 * 输出  ：void
 */
void display_show_log(disp_show_type_enum show_type, const char *fmt, ...)
{
    int32_t output_len = 0;
    va_list args;

    memset(log_buf, 0x00, sizeof(log_buf));
    if(show_type == DISP_SHOW_LOG)
    {
        //log_buf[0] = 0xAA;
    }
    else if(show_type == DISP_SHOW_NG)
    {
        //log_buf[0] = 0xBB;
    }
    else if(show_type == DISP_SHOW_PASS)
    {
        //log_buf[0] = 0xCC;
    }
    else if(show_type == DISP_SHOW_WARN)
    {
        //log_buf[0] = 0xCC;
    }
    else
    {
        return ;
    }
    va_start(args, fmt);
    output_len = vsnprintf(log_buf + 1, sizeof(log_buf) - 1, fmt, args) + 1;
    va_end(args);
    log_buf[output_len] = '\n';
    output_len++;

    DEBUG("%s", log_buf + 1);
    
    if(show_type == DISP_SHOW_LOG)
    {
        display_show_string(log_buf+1, WHITE, BLACK);
    }
    else if(show_type == DISP_SHOW_NG)
    {
        display_show_string(log_buf+1, RED, BLACK);
    }
    else if(show_type == DISP_SHOW_PASS)
    {
        display_show_string(log_buf+1, GREEN, BLACK);
    }
    else if(show_type == DISP_SHOW_WARN)
    {
        display_show_string(log_buf+1, YELLOW, BLACK);
    }
            
}
