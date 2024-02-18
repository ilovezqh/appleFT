/*
********************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.2
*	说    明 : FatFS文件系统例程(SD卡)
*	修改记录 :
*		版本号   日期       作者     说明
*		v1.2    2014-06-26 plxc    首发
*                                   1. fatfs使用当前最新的0.10b版本。
*                                   2. bsp_sdio_sd.c使用V1.1.2版本。
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
********************************************************************************
*/

#include "bsp.h"				/* 底层硬件驱动 */
#include "lcd.h"
#include "demo_sdio_fatfs.h"
#include "demo_i2c_eeprom.h"
#include "demo_spi_flash.h"
#include "test.h"
#include "crc32.h"
#include "log.h"
#include "MultiTimer.h"
#include "udisk_task.h"
#include "board_type.h"
#include "board_data_format.h"
#include "btldr_config.h"
#include "cmb_app.h"

//extern uint8_t aShowTime[50]; /* 在文件bsp_rtc里面 */

MultiTimer led_timer;
MultiTimer key_timer;

void DispMenu(void);
void dut_signal_test(void);
static void sys_run_led(MultiTimer* timer, void *userData);
static void key_run(MultiTimer* timer, void *userData);
//extern void key_task(void);

static uint64_t PlatformTicksGetFunc(void)  {   return g64_run_time; }
/*
********************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
********************************************************************************
*/
int main(void)
{
	/*
		ST固件库中的启动文件已经执行了 SystemInit() 函数，该函数在 system_stm32f4xx.c 文件，主要功能是
	配置CPU系统的时钟，内部Flash访问时序，配置FSMC用于外部SRAM
	*/
    g_JumpInit = B2A_MAGIC_NUM;                                                 // 不加此句，按复位键boot会认为没有APP导致进入升级界面
	crc32_init();   //crc32表格初始化
    bsp_Init();		                                                            /* 硬件初始化 */
    
	PrintfLogo();	                                                            /* 打印例程信息到串口1 */
    IWDG_Feed();
    bsp_DelayMS(100);
    IWDG_Feed();
    //init_cycle_counter(false);
    if(board_self_test())
    {
        //BEEP_Start(520, 20, 0);
        while(1);
    }
    #if 0  //新的板子需要配置此项 格式化
    board_data_format();
    #endif
    //udisk_task();
 
    DispMenu();

    test_init();
     
    MultiTimerInstall(PlatformTicksGetFunc);
    MultiTimerStart(&led_timer, 200 , sys_run_led, NULL);
    MultiTimerStart(&key_timer, 10 ,  key_run,     NULL);
    IWDG_Feed();
    bsp_DelayMS(1000);
    IWDG_Feed();
    cmb_init();
    
    while(1)
    {
        bsp_Idle();		                                                        /* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
        test_task();
        //dut_signal_test();
        //key_task();
        udisk_task();
        MultiTimerYield();
        
    }
}

static void sys_run_led(MultiTimer* timer, void *userData)
{
    bsp_LedToggle(LED_GREEN);	/* 翻转LED4的状态 */
    IWDG_Feed();                /* 喂狗 */
    MultiTimerStart(&led_timer, 200 , sys_run_led, userData);
}

static void key_run(MultiTimer* timer, void *userData)
{
    button_ticks();
    MultiTimerStart(&key_timer, 10 ,  key_run, userData);
}

#if 0
#define RTC_TIMER_ID  0
static void rtc_task(void)
{
    static uint8_t flg=0;
    static uint8_t init_timer=0;
    
    if(init_timer == 0)
    {
        init_timer = 1;
        bsp_StartAutoTimer(RTC_TIMER_ID, 500);	/* 启动1个500ms的自动重装的定时器 */
    }
    
    if(bsp_CheckTimer(RTC_TIMER_ID))
    {
        bsp_LedToggle(1);

        if(flg!=1)
        {
            flg = 1;
            
            RTC_DateShow(); /* 显示日期 */
            printf("%s\r\n", aShowTime);

            RTC_TimeShow(); /* 显示时间 */
            printf("当前时间%s\r\n", aShowTime);
        }
    }
}
#endif

/*
********************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
********************************************************************************
*/
void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择操作命令:\r\n");
	printf("Enter - 进行FT测试\r\n");
//	printf("2 - 创建一个新文件plxc.txt\r\n");
//	printf("3 - 读plxc.txt文件的内容\r\n");
//	printf("4 - 创建目录\r\n");
//	printf("5 - 删除文件和目录\r\n");
//	printf("6 - 读写文件速度测试\r\n");
//	printf("7 - 移除U盘\r\n");
//	printf("8 - 使能U盘\r\n");	
}


/****************** 普林芯驰 www.spacetouch.co (END OF FILE) *******************/
