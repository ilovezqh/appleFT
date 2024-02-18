#include "bsp.h"


/*
**************************************************
*FunctionName:	void pwm_measure_start(void)
*
*Descriptor:	????TWI SCK????????????
*
*Arguments:		?
*
*Returns:		?
*
*Attentions:	?
**************************************************
*/
#if 0
void pwm_measure_start(void)
{
    // ?????? GPIO ???
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    IO_Config(GPIOA, GPIO_Pin_0, IO_MODE_TIM);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5); //PA0 TIM5

    NVIC_InitTypeDef NVIC_InitStructure;
    // ??????0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  //

    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //?????3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//????3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
    NVIC_Init(&NVIC_InitStructure);	//??????????VIC????

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1; //
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //??????
    TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; //??????
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    TIM_ICInitTypeDef  TIM5_ICInitStructure;
    //???TIM5??????
    TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	????? IC1???TI1?
    TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//?????
    TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //???TI1?
    TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //??????,???
    TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ??????? ???
    TIM_ICInit(TIM5, &TIM5_ICInitStructure);
    TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE);                     // 开启更新和捕获中断
    TIM_SetCounter(TIM5, 0);
    TIM_Cmd(TIM5, ENABLE ); 	                                                // 使能计数器

}
#else
void pwm_measure_start(void)
{
    // ?????? GPIO ???
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
 
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
    
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM8);
    
  
    TIM_TimeBaseInitTypeDef tim_base_init={0};
    tim_base_init.TIM_CounterMode = TIM_CounterMode_Up;
	tim_base_init.TIM_Prescaler = 0;
	tim_base_init.TIM_Period = UINT16_MAX;
    tim_base_init.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM8,&tim_base_init);

    //                            不分频                 
    TIM_ETRClockMode1Config(TIM8, TIM_ExtTRGPSC_DIV2, TIM_ExtTRGPolarity_NonInverted, 12); //12
//    TIM_TIxExternalClockConfig(TIM8, uint16_t TIM_TIxExternalCLKSource,
//                                uint16_t TIM_ICPolarity, uint16_t ICFilter)
//    
//    TIM_ETRClockMode1Config(TIM8, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
    
    
    TIM_Cmd(TIM8, ENABLE);
    
     
}
#endif


//// 定时器输入捕获用户自定义变量结构体声明
//typedef struct
//{

//    uint32_t Capture_RisingTime; // 捕获高脉冲寄存器的值
//    uint32_t Capture_Period;     // 自动重装载寄存器更新标志
//} TIM_ICUserValueTypeDef;

//TIM_ICUserValueTypeDef TIM_ICUserValueStructure;


//void pwm_measure_reset(void)
//{
//    TIM_ICUserValueStructure.Capture_RisingTime = 0;  //清空脉冲计数
//}

///* 在TIM5 中断中获取频率计数 */
//void TIM5_IRQHandler(void)
//{
//    // 当要被捕获的信号的周期大于定时器的最长定时时，定时器就会溢出，产生更新中断
//    // 这个时候我们需要把这个最长的定时周期加到捕获信号的时间里面去
//    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
//    {
//        TIM_ICUserValueStructure.Capture_Period++;
//        TIM_ClearITPendingBit(TIM5, TIM_FLAG_Update);
//    }

//    if (TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET)
//    {
//        TIM_SetCounter(TIM5, 0);
//        TIM_ICUserValueStructure.Capture_RisingTime++;
//        TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);
//    }
//}

/*
**************************************************
*FunctionName:	void pwm_measure_stop(void)
*
*Descriptor:	????TWI SCK?????????????
*
*Arguments:		?
*
*Returns:		????
*
*Attentions:	?
**************************************************
*/
u32 pwm_measure_stop(void)
{
    TIM_Cmd(TIM8,DISABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
    
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	return TIM_GetCounter(TIM8)*2;
}

/////////////////////////////////////////////////////////////////////////////////
static volatile uint32_t sem_tim3_tick = 0;

void tick_init(void)
{
    TIM_TimeBaseInitTypeDef tim_base_init;
    NVIC_InitTypeDef nvic_init;

    //TIM_TimeBaseStructInit(&tim_base_init);
    // TIM3????????
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    tim_base_init.TIM_CounterMode   = TIM_CounterMode_Up;
    tim_base_init.TIM_Prescaler     = 84 - 1;
    tim_base_init.TIM_Period        = 50 - 1;
    TIM_TimeBaseInit(TIM3, &tim_base_init);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    nvic_init.NVIC_IRQChannel       = TIM3_IRQn;
    nvic_init.NVIC_IRQChannelCmd    = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic_init);

//	 GPIO_InitTypeDef  GPIO_InitStructure;

//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//??GPIOF??

//  //GPIOD0,D1?????
//   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//??????
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//????
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//??
//   GPIO_Init(GPIOB, &GPIO_InitStructure);//???

}


void TIM3_IRQHandler(void)
{
    //static uint8_t rev;

    if (TIM_GetITStatus(TIM3, TIM_IT_Update))
    {
        sem_tim3_tick++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

/**
  * @brief  ?????tick
  * @param  us?????
  * @retval None
  */
void set_tick(u32 us)
{
    TIM_SetCounter(TIM3, 0);
    TIM_SetAutoreload(TIM3, us - 1);
    TIM_Cmd(TIM3, ENABLE);

    sem_tim3_tick = 0;
}

/**
  * @brief  ????tick??
  * @param  None
  * @retval None
  */
void wait_tick(void)
{
    // tos_sem_pend(&sem_tim3_tick,TOS_TIME_FOREVER);
    uint32_t tick_now = sem_tim3_tick;

    while (sem_tim3_tick == tick_now);
}

/**
  * @brief  ????????tick
  * @param  None
  * @retval None
  */
void reset_tick(void)
{
    TIM_Cmd(TIM3, DISABLE);
}

/**
  * @brief  ??????????????
  * @param  None
  * @retval None
  */
void tos_knl_sched_lock()
{
    DISABLE_INT();
    //TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
    //__disable_irq() ; //?????
}

/**
  * @brief  ??????????????
  * @param  None
  * @retval None
  */
void tos_knl_sched_unlock()
{
    ENABLE_INT();
    // TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    //__enable_irq() ; //?????
}
