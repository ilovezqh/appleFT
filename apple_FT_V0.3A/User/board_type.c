#include "bsp.h"
#include "lcd.h"
#include "board_type.h"

/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"F4_FT_apple"
#define FW_VER          "1.0" 
#define FW_INFO         "apple ft_prg V1.0 \n"

board_type_e  g_eBoardType;

#define VREFINT_HI   1240
#define  VREFINT_LO  1180

uint32_t VBUS_HI;      
uint32_t VBUS_LO;      

void board_type_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		    /* 设为输出口 */
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN;	    /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    for(uint32_t i=0;i<1000;i++)
    {
        __nop();
    }
    
    uint32_t cnt=0;
    while( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_10) == Bit_SET)
    {
        cnt++;
        if(cnt == 1000) 
        {
            break;
        }
    }
    
    if(cnt == 1000)
    {
        
        VBUS_HI     =   5000;
        VBUS_LO     =   4400;
        g_eBoardType = board_v21;
        printf("board_v21\r\n");
    }
    else
    {
        VBUS_HI     =   5300;
        VBUS_LO     =   4700;
        g_eBoardType = board_v20;
        printf("board_v20\r\n");
    }
}

int32_t board_self_test(void)
{
    display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK);
    //display_show_string(FW_INFO, WHITE,   BLACK);
    bsp_DelayMS(100);
    if (ee_CheckOk() == 0)
	{
		/* 没有检测到EEPROM */
        display_show_log(DISP_SHOW_NG,"fail:eeprom check\r\n");
	}
    uint32_t vrefint_sum=0;
    uint32_t vbus_sum   =0;
    for(uint32_t i=0; i<128; i++)
    {
        vrefint_sum += g_adc_value[ADC_RANK_STM32_VREFINT];
        vbus_sum    += g_adc_value[ADC_RANK_VBUS];
    }
//    uint32_t vbus    = (vbus_sum/128)*5*3300/2/4095;
    uint32_t vrefint = (vrefint_sum/128)*3300/4095;
    //display_show_log(DISP_SHOW_LOG,"board:V%d",g_eBoardType);
//    if((vbus > VBUS_HI) || (vbus < VBUS_LO))
//    {
//        display_show_log(DISP_SHOW_NG,"fail: VBUS = %d mv\r\n",vbus);
//        return -1;
//    }
//    else
//    {
//        display_show_log(DISP_SHOW_LOG,"VBUS     = %d mv\r",vbus);
//    }
    if((vrefint > VREFINT_HI) || (vrefint < VREFINT_LO))
    {
        display_show_log(DISP_SHOW_NG,"board:V%d   fail: VREFINT = %dmv\r\n",g_eBoardType,vrefint);
        return -1;
    }
    else
    {
        //display_show_log(DISP_SHOW_LOG,"board:V%d   VREFINT = %dmv\r",g_eBoardType,vrefint);
    }
 
    return 0;
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void PrintfLogo(void)
{
	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 项目名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 程序版本   : %s\r\n", FW_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", __DATE__);	    /* 打印例程日期 */

	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	printf("* 固件库版本 : V%d.%d.%d (STM32F4xx_StdPeriph_Driver)\r\n", __STM32F4XX_STDPERIPH_VERSION_MAIN,
			__STM32F4XX_STDPERIPH_VERSION_SUB1,__STM32F4XX_STDPERIPH_VERSION_SUB2);
	printf("* \n\r");	/* 打印一行空格 */
	printf("* Copyright www.spacetouch.co 普林芯驰\r\n");
	printf("*************************************************************\n\r");
}
