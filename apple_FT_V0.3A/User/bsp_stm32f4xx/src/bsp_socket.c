/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 驱动LED指示灯
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2023-07-18 plxc  正式发布
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/
#include "bsp.h"

/*
    普林芯驰FT_F4 继电器口线分配：
        RELAY     : PC13          (高电平切到 比较器模式，低电平切到 频率测量模式)
*/

/*  socket各控制口对应的RCC时钟 */
#define RCC_ALL_SOCKET 	    (RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB)

#define GPIO_PORT_RELAY         GPIOE
#define GPIO_PIN_RELAY	        GPIO_Pin_2

#define GPIO_PORT_ONOFF         GPIOA
#define GPIO_PIN_ONOFF	        GPIO_Pin_0

#define GPIO_PORT_LDO3V3_EN         GPIOA
#define GPIO_PIN_LDO3V3_EN	        GPIO_Pin_15


#define GPIO_PORT_ASU_PU_EN         GPIOA
#define GPIO_PIN_ASU_PU_EN	        GPIO_Pin_8

/* apple 复位控制，在空片状态下默认是PRST开启 使能上拉，低电平复位 */
#define GPIO_PORT_APPLE_PRST        GPIOB
#define GPIO_PIN_APPLE_PRST	        GPIO_Pin_8

/* 电容接入，模拟触摸，测试apple是否能正确识别 */
#define GPIO_PORT_TOUCH_TEST       GPIOE
#define GPIO_PIN_TOUCH_TEST        GPIO_Pin_4

/* socket功耗自测校准，切换到 200K的负载上进行功耗校准 高电平：切到芯片功耗回路，低电平：切到校准回路 */
#define GPIO_PORT_I_SELF           GPIOE
#define GPIO_PIN_I_SELF            GPIO_Pin_3

/* IREF/VREF校准模式切换， 高电平：IREF（通过ADC123_IN3_PA3），低电平：VREF（通过ADC123_IN2_PA2） */
#define GPIO_PORT_IVREF_CALI        GPIOE
#define GPIO_PIN_IVREF_CALI         GPIO_Pin_5


/*
*********************************************************************************************************
*	功能说明: 配置控制socket板继电器的GPIO；输出低电平则
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_socket_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_ALL_SOCKET, ENABLE);
 
	bsp_kiwi_ft_relay_off();  
    socket_touch_off();
    socket_i_chip();//socket_i_self();
    socket_asu_pu_on();
    socket_prst_ctl(prst_state_float);
    socket_ivref_vref();
    GPIO_SetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
    
    
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;	        /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
 
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RELAY;
	GPIO_Init(GPIO_PORT_RELAY, &GPIO_InitStructure);
    
    
    
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_TOUCH_TEST;
	GPIO_Init(GPIO_PORT_TOUCH_TEST, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_I_SELF;
	GPIO_Init(GPIO_PORT_I_SELF, &GPIO_InitStructure);
 
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IVREF_CALI;
	GPIO_Init(GPIO_PORT_IVREF_CALI, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;	        /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
    
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ASU_PU_EN;
	GPIO_Init(GPIO_PORT_ASU_PU_EN, &GPIO_InitStructure);
}

//void apple_prst(void)
//{
//    GPIO_SetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
//    bsp_DelayMS(10);
//    GPIO_ResetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
//    bsp_DelayMS(10);
//    GPIO_SetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
//    bsp_DelayMS(10);
//}



//void bsp_prst_ctl_deinit(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    
//    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		/* 设为输出口 */
//	GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;		/* 设为推挽模式 */
//	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;	        /* 上下拉电阻不使能 */
//	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
// 
//    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_APPLE_PRST;
//	GPIO_Init(GPIO_PORT_APPLE_PRST, &GPIO_InitStructure);
//}

void socket_asu_pu_on(void)
{
    GPIO_SetBits(GPIO_PORT_ASU_PU_EN, GPIO_PIN_ASU_PU_EN);
}

void socket_asu_pu_off(void)
{
    GPIO_ResetBits(GPIO_PORT_ASU_PU_EN, GPIO_PIN_ASU_PU_EN);
}

void socket_prst_ctl(enum  prst_state_e state)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    
    switch(state)
    {
        case prst_state_float:
        {
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		/* 设为输出口 */
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;		/* 设为推挽模式 */
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;	        /* 上下拉电阻不使能 */
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
         
            GPIO_InitStructure.GPIO_Pin = GPIO_PIN_APPLE_PRST;
            GPIO_Init(GPIO_PORT_APPLE_PRST, &GPIO_InitStructure);
        }break;
        
        case prst_state_hi:
        {
            GPIO_SetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;		/* 设为输出口 */
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		/* 设为推挽模式 */
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;	        /* 上下拉电阻不使能 */
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
            
            GPIO_InitStructure.GPIO_Pin = GPIO_PIN_APPLE_PRST;
            GPIO_Init(GPIO_PORT_APPLE_PRST, &GPIO_InitStructure);
        }break;
        
        case prst_state_lo:
        {
            GPIO_ResetBits(GPIO_PORT_APPLE_PRST, GPIO_PIN_APPLE_PRST);
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;		/* 设为输出口 */
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		/* 设为推挽模式 */
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;	        /* 上下拉电阻不使能 */
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;	/* IO口最大速度 */
            
            GPIO_InitStructure.GPIO_Pin = GPIO_PIN_APPLE_PRST;
            GPIO_Init(GPIO_PORT_APPLE_PRST, &GPIO_InitStructure);
        }break;
        
        default:break;
    }
}

void bsp_kiwi_onoff_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_ALL_SOCKET, ENABLE);
 
 
    bsp_kiwi_onoff_hi();
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	    /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
 
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ONOFF;
	GPIO_Init(GPIO_PORT_ONOFF, &GPIO_InitStructure);
}


void bsp_kiwi_onoff_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_ALL_SOCKET, ENABLE);
    bsp_kiwi_onoff_hi();
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
    
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ONOFF;
	GPIO_Init(GPIO_PORT_ONOFF, &GPIO_InitStructure);
}

void bsp_kiwi_onoff_hi(void)
{
    GPIO_SetBits(GPIO_PORT_ONOFF, GPIO_PIN_ONOFF);
}

void bsp_kiwi_onoff_lo(void)
{
    GPIO_ResetBits(GPIO_PORT_ONOFF, GPIO_PIN_ONOFF);
}

void socket_touch_on(void)
{
    GPIO_SetBits(GPIO_PORT_TOUCH_TEST, GPIO_PIN_TOUCH_TEST);
}

void socket_touch_off(void)
{
    GPIO_ResetBits(GPIO_PORT_TOUCH_TEST, GPIO_PIN_TOUCH_TEST);
}

/* socket功耗自测校准，切换到 200K的负载上进行功耗校准 高电平：切到芯片功耗回路，低电平：切到校准回路 */
void socket_i_self(void)
{
    GPIO_SetBits(GPIO_PORT_I_SELF, GPIO_PIN_I_SELF);
}

void socket_i_chip(void)
{
    GPIO_ResetBits(GPIO_PORT_I_SELF, GPIO_PIN_I_SELF);
}

/* IREF/VREF校准模式切换， 高电平：IREF（通过ADC123_IN3_PA3），低电平：VREF（通过ADC123_IN2_PA2） */
void socket_ivref_iref(void)
{
    GPIO_SetBits(GPIO_PORT_IVREF_CALI, GPIO_PIN_IVREF_CALI);
}

void socket_ivref_vref(void)
{
    GPIO_ResetBits(GPIO_PORT_IVREF_CALI, GPIO_PIN_IVREF_CALI);
}


void bsp_socket_ldo3v3_init(void)
{
 
    GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_ALL_SOCKET, ENABLE);
    GPIO_ResetBits(GPIO_PORT_LDO3V3_EN, GPIO_PIN_LDO3V3_EN);
 
  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	    /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
 
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LDO3V3_EN;
	GPIO_Init(GPIO_PORT_LDO3V3_EN, &GPIO_InitStructure);
    
 
}

void bsp_socket_ldo3v3_enable(void)
{
    GPIO_SetBits(GPIO_PORT_LDO3V3_EN, GPIO_PIN_LDO3V3_EN);
}

void bsp_socket_ldo3v3_disable(void)
{
    GPIO_ResetBits(GPIO_PORT_LDO3V3_EN, GPIO_PIN_LDO3V3_EN);
}

// 开继电器 kiwi GPIO4|GPIO5 接socket板载比较器用于hp offset校准
void bsp_kiwi_ft_relay_on(void)
{
    GPIO_SetBits(GPIO_PORT_RELAY, GPIO_PIN_RELAY);
}

//关掉继电器 kiwi GPIO4|GPIO5 断开一切连接
void bsp_kiwi_ft_relay_off(void)
{
    GPIO_ResetBits(GPIO_PORT_RELAY, GPIO_PIN_RELAY);
}

void bsp_relay_toggle(void)
{
    GPIO_ToggleBits(GPIO_PORT_RELAY, GPIO_PIN_RELAY);
}

/****************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************/
