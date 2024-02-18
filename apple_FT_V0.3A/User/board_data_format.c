#include "bsp.h"
#include "flash_fatfs.h"
//板子重置，恢复出厂
//1.spi flash全部擦除
//2.eeprom 恢复初始化
//3.spi flash 文件系统初始化 f_mkfs
//4.sd卡文件系统初始化 f_mkfs
//5.




void board_data_format(void)
{
    uint8_t temp[256]={0};
 
    debug(" board factory init !\r\n");
    debug(" spi flash erase chip !\r\n");
    bsp_InitSFlash();
    sf_EraseChip();
    debug(" eeprom !\r\n");
    ee_WriteBytes(temp, 0, 256); //eeprom 全部写0
    flash_fatfs_format();
}


