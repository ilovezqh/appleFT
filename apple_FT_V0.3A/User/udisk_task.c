#include "bsp.h"				/* 底层硬件驱动 */
#include "usb_bsp_msc.h"
#include "lcd.h"
#include "demo_spi_flash_fatfs.h"
#include "MultiTimer.h"
//#include "ugui.h"

extern void fatfs_task(void);
extern void DemoSpiFlash(uint8_t cmd);

 
void udisk_task(void)
{
    static uint32_t btn_val;
    
    btn_val =  get_button_event(&esc_btn);
     
    
    if(btn_val == LONG_PRESS_HOLD)
    {
        BEEP_KeyTone();
        display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK);
        bsp_InitSFlash();
        //display_show_string("USB ... extern flash update!",GREEN , RED);
        lcd_disp("USB ... extern flash update!",YELLOW , BLACK,page_start_line);
        usbd_OpenMassStorage();
        
        while(1)
        {
             MultiTimerYield();
        }
 
    }
}


void gui_test(void)
{
    
}

