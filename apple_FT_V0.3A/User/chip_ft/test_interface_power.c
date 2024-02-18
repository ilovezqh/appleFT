#include "test.h"
#include "stm32f4xx.h" 
#include "test_interface.h"


/* 取样电阻100Ω，用于 */
#define SAMPLE_100R_SWITCH_CTRL_PORT     GPIOD
#define SAMPLE_100R_SWITCH_CTRL_PIN      GPIO_Pin_14

typedef enum {
    IO_MODE_TIM = 0,
    IO_MODE_FLOAT_OUTPUT,
    IO_MODE_FLOAT_INPUT,
    IO_MODE_PUSH_PULL_OUTPUT,
    IO_MODE_ANALOG_INPUT
} IO_ModeTypeDef;

void IO_Config(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, IO_ModeTypeDef mode);
static void test_power_init(void);
static void test_power_enable(void);
static void test_power_disable(void);
static void test_power_consumption_enable(void);
static void test_power_consumption_disable(void);

//接口初始化，统一通过test_power调用，比较清晰 
struct test_power_ops test_power =
{
    .init             = test_power_init,
    .on               = test_power_enable,
    .off              = test_power_disable,
    .consumption_on   = test_power_consumption_enable,
    .consumption_off  = test_power_consumption_disable,
};

static void test_power_init(void)
{
    IO_Config(VOUT_EN_PORT, VOUT_EN_PIN, IO_MODE_PUSH_PULL_OUTPUT);
    test_power_disable();
    
    IO_Config(SAMPLE_100R_SWITCH_CTRL_PORT,SAMPLE_100R_SWITCH_CTRL_PIN, IO_MODE_PUSH_PULL_OUTPUT );
    test_power_consumption_disable();//初始状态跳过采样电阻
}
 
static void test_power_enable(void)
{
    GPIO_SetBits(VOUT_EN_PORT, VOUT_EN_PIN);	
}

static void test_power_disable(void)
{
    GPIO_ResetBits(VOUT_EN_PORT, VOUT_EN_PIN);	
}


static void test_power_consumption_enable(void)
{
    GPIO_ResetBits(SAMPLE_100R_SWITCH_CTRL_PORT,GPIO_Pin_14);		
}

static void test_power_consumption_disable(void)
{
    GPIO_SetBits(GPIOD,GPIO_Pin_14);
}
 


void IO_Config(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, IO_ModeTypeDef mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIO时钟
    if (GPIOx == GPIOA) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    } else if (GPIOx == GPIOB) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    } else if (GPIOx == GPIOC) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    } else if (GPIOx == GPIOD) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    } else if (GPIOx == GPIOE) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    } else if (GPIOx == GPIOF) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    } else if (GPIOx == GPIOG) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    } else if (GPIOx == GPIOH) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    } else if (GPIOx == GPIOI) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    }

    // 配置GPIO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    switch (mode) {
        case IO_MODE_TIM:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            GPIO_PinAFConfig(GPIOx, GPIO_PinSource0, GPIO_AF_TIM1);
            break;
        
        case IO_MODE_FLOAT_OUTPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            break;
        
        case IO_MODE_FLOAT_INPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            break;
        case IO_MODE_PUSH_PULL_OUTPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            break;
        case IO_MODE_ANALOG_INPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
            break;
        default:
            break;
    }

    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
