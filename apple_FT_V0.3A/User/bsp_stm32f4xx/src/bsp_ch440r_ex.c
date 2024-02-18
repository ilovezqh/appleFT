/*
CH440R: IN  EN# 脚控制；
IN（单刀双掷模拟开关选择输入：高电平选择 2#端（S2x）；低电平选择 1#端（S1x））
EN#（全局使能输入，低电平有效；高电平全部断开）


kiwi  i2c | uart :i2c时 kiwi接外部eeprom，可测kiwi i2c功能；uart时接stm32串口，可测uart功能
kiwi  spi | asu  ：spi时，kiwi 接外部flash，可跑测试pattern； asu时接stm32，可asu控制kiwi  
         S1    S2
 
*/
#include "bsp.h"

#define RCC_ALL_CH440 	RCC_AHB1Periph_GPIOE	/* 按键口对应的RCC时钟 */

#define GPIO_PORT_IOTEST_CH440_EN   GPIOE
#define GPIO_PIN_IOTEST_CH440_EN	GPIO_Pin_6

#define GPIO_PORT_IOTEST_CH440_IN   GPIOE
#define GPIO_PIN_IOTEST_CH440_IN	GPIO_Pin_5

#define GPIO_PORT_UC_CH440_EN       GPIOE
#define GPIO_PIN_UC_CH440_EN	    GPIO_Pin_4

#define GPIO_PORT_UC_CH440_IN       GPIOE
#define GPIO_PIN_UC_CH440_IN	    GPIO_Pin_3

#define CH440R_IOTEST_DETACH()      GPIO_SetBits(   GPIO_PORT_IOTEST_CH440_EN, GPIO_PIN_IOTEST_CH440_EN)
#define CH440R_IOTEST_ATTACH()      GPIO_ResetBits( GPIO_PORT_IOTEST_CH440_EN, GPIO_PIN_IOTEST_CH440_EN)
#define CH440R_IOTEST_S1()          GPIO_ResetBits( GPIO_PORT_IOTEST_CH440_IN, GPIO_PIN_IOTEST_CH440_IN)
#define CH440R_IOTEST_S2()          GPIO_SetBits(   GPIO_PORT_IOTEST_CH440_IN, GPIO_PIN_IOTEST_CH440_IN)

#define CH440R_UC_DETACH()          GPIO_SetBits(   GPIO_PORT_UC_CH440_EN, GPIO_PIN_UC_CH440_EN)
#define CH440R_UC_ATTACH()          GPIO_ResetBits( GPIO_PORT_UC_CH440_EN, GPIO_PIN_UC_CH440_EN)
#define CH440R_UC_I2C()             GPIO_ResetBits( GPIO_PORT_UC_CH440_IN, GPIO_PIN_UC_CH440_IN)
#define CH440R_UC_UART()            GPIO_SetBits( GPIO_PORT_UC_CH440_IN, GPIO_PIN_UC_CH440_IN)
 

//初始化 CH440R的控制口;
void bsp_ch440r_init_ex(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 
	RCC_AHB1PeriphClockCmd(RCC_ALL_CH440, ENABLE);

    CH440R_IOTEST_DETACH();
    CH440R_UC_DETACH();//kiwi断开所有的与ch440r的连接
 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		                        /* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		                        /* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	                        /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	                        /* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IOTEST_CH440_EN;
	GPIO_Init(GPIO_PORT_IOTEST_CH440_EN, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IOTEST_CH440_IN;
	GPIO_Init(GPIO_PORT_IOTEST_CH440_IN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_UC_CH440_EN;
	GPIO_Init(GPIO_PORT_UC_CH440_EN, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_UC_CH440_IN;
	GPIO_Init(GPIO_PORT_UC_CH440_IN, &GPIO_InitStructure);
}

/*
CH440R 控制旁路 让 kiwi 脱离外部连接，防止IO倒灌电让kiwi掉电不彻底
CH440R 控制接入;在socket上电后进行控制
kiwi  i2c | uart :i2c时 kiwi接外部eeprom，可测kiwi i2c功能；uart时接stm32串口，可测uart功能
kiwi  spi | asu  ：spi时，kiwi 接外部flash，可跑测试pattern； asu时接stm32，可asu控制kiwi  
         S1    S2
*/
void bsp_ch440r_ctrl_ex(uint8_t ctrl)
{
    if( ctrl & CH440R_EX_CTRL_ALL_DETACH)
    {
        CH440R_IOTEST_DETACH();
        CH440R_UC_DETACH();
        return;
    }
    
    if( ctrl & CH440R_EX_CTRL_UC_DETACH)
    {
        CH440R_UC_DETACH();
    }
    
    if( ctrl & CH440R_EX_CTRL_IOTEST_DETACH)
    {
        CH440R_IOTEST_DETACH();
    }
    

    if( ctrl & CH440R_EX_CTRL_I2C)
    {
        CH440R_UC_ATTACH();
        CH440R_UC_I2C();
    }
    if( ctrl & CH440R_EX_CTRL_UART)
    {
        CH440R_UC_ATTACH();
        CH440R_UC_UART();
    }
 
    if( ctrl & CH440R_EX_CTRL_IO_S1)
    {
        CH440R_IOTEST_ATTACH();
        CH440R_IOTEST_S1();
    }
 
    if( ctrl & CH440R_EX_CTRL_IO_S2)
    {
        CH440R_IOTEST_ATTACH();
        CH440R_IOTEST_S2();
    }
 
}




