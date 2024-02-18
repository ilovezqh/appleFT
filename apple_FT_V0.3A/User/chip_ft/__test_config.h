#ifndef __TEST_CONFIG_H
#define __TEST_CONFIG_H
#include "stdint.h"

/***************************** @Pack 芯片型号和外接板选择 *********************************/
#define TEST_FIRMWARE_VERSION         "V0.3A"

/*************************** @Standard 测试标准 *******************************/
#include "__test_standard.h" 
/*************************** @TestItem 测试项 *********************************/
#define ON_BOARD_FT      (0) //1:开启在板FT（iref直接默认值，跳过）   0：关闭
/***************************** @Mode 测试模式 *********************************/
#define CHIP_NAME               dut.device->chip_name

/***************************** @fatfs 文件系统 ********************************/
#define SPI_FLASH_VOLUME                "0:"  //spi flashd的盘号
#define SD_VOLUME                       "1:"  //SD卡的盘号
#define CONFIG_TXT_PATH                 "0:/Config.txt"
#define BOARD_ID_PATH                   "0:/pattern/board_id.txt"   //不超过24个byte             
#define LOGGER_TXT_FORMAT               "1:/logger_%04d.txt"

#endif

