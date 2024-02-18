#include "bsp.h"

/*

主频:168MHz 

*/

/*
 * 初始化MCO引脚PA8;
 * 在F1系列中MCO引脚只有一个，即PA8，在F4系列中，MCO引脚会有两个
 */
void hp_osc_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 开启GPIOA的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 选择GPIO8引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

    //设置为复用功能推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

    //设置IO的翻转速率为100M
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    // 初始化GPIOA8
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_4); //主频/4=42Mhz
}


void hp_osc_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    // 开启GPIOA的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 选择GPIO8引脚
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_4);
}



