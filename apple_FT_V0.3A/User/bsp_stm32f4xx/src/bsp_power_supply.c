#include "bsp.h"
#include "test_interface.h"
#if 0
/**
  * @brief  对socket供电，初始化，默认关闭供电
  * @param  None
  * @retval None
  */
void volt_out_init(void)
{
    IO_Config(VOUT_EN_PORT, VOUT_EN_PIN, IO_MODE_PUSH_PULL_OUTPUT);
}

void volt_out_enable(void)
{
    GPIO_SetBits(VOUT_EN_PORT, VOUT_EN_PIN);	
}

void volt_out_disable(void)
{
    GPIO_ResetBits(VOUT_EN_PORT, VOUT_EN_PIN);	
}


#define SAMP_JUMP_GPIO_RCC      RCC_AHB1Periph_GPIOD
#define SAMP_JUMP_GPIO_PORT     GPIOD
#define SAMP_JUMP_GPIO_PIN      GPIO_Pin_14

void short_100R_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;	
	
	RCC_AHB1PeriphClockCmd( SAMP_JUMP_GPIO_RCC, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin    = SAMP_JUMP_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed  = GPIO_Fast_Speed;
	GPIO_Init(SAMP_JUMP_GPIO_PORT,&GPIO_InitStruct);
    
	GPIO_SetBits(SAMP_JUMP_GPIO_PORT,SAMP_JUMP_GPIO_PIN);//初始状态跳过采样电阻
}


void flows_by_100ohm (on_off_t status)
{

	if(status == ON) //使能mos 电流流过100R电阻
	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_14);	
	}
	else 
	{
		GPIO_SetBits(GPIOD,GPIO_Pin_14);	
	}
	
}
#endif

