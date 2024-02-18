/*
*********************************************************************************************************
*
*	模块名称 : ADC驱动模块
*	文件名称 : bsp_adc.c
*	版    本 : V1.0
*	说    明 : ADC多通道扫描
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2014-01-08  armfly   正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.spacetouch.co
*
*********************************************************************************************************
*/
#include "bsp.h"


/* define ---------------------------------------------------------------------*/
#define ADC1_DR_ADDRESS          ((uint32_t)0x4001204C)

/* 变量 ----------------------------------------------------------------------*/
#define ADC_DMA_CH_SUM   8
__IO uint16_t g_adc_value[ADC_DMA_CH_SUM];
 
 
    
/*
*********************************************************************************************************
*	函 数 名: bsp_InitADC
*	功能说明: ADC初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitADC(void)
{
    ADC_InitTypeDef       ADC_InitStructure={0};
	ADC_CommonInitTypeDef ADC_CommonInitStructure={0};
	DMA_InitTypeDef       DMA_InitStructure={0};
	GPIO_InitTypeDef      GPIO_InitStructure={0};
	
	/* 使能外设时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                                   //ADC123_IN2  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                                   //ADC123_IN3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                                   ////ADC123_IN13 IOVCC
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
 
    
    

	/* DMA2 Stream0 channel0 配置-------------------------------------------------- */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&g_adc_value;;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = ADC_DMA_CH_SUM;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	/* DMA2_Stream0 enable */
	DMA_Cmd(DMA2_Stream0, ENABLE);
    bsp_DelayMS(100);
    /****************************************************************************   
	  PCLK2 = HCLK / 2 
	  下面选择的是2分频
	  ADCCLK = PCLK2 /8 = HCLK / 8 = 168 / 8 = 21M
      ADC采样频率： Sampling Time + Conversion Time = 480 + 12 cycles = 492cyc
                    Conversion Time = 21MHz / 492cyc = 42.6ksps. 
	*****************************************************************************/
    
	/* ADC Common 配置 ----------------------------------------------------------*/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 regular channel 12 configuration ************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = ADC_DMA_CH_SUM;
    ADC_Init(ADC1, &ADC_InitStructure);
    
     /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
 
    /* ADC1 regular channel18 (VBAT) configuration ******************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_480Cycles);    //PA_OUT_N
    /* ADC1 regular channel18 (VBAT) configuration ******************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_480Cycles);    //PA_OUT_P
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 3,  ADC_SampleTime_480Cycles );//电流测量
    /* ADC1 regular channels 10, 11 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 4, ADC_SampleTime_480Cycles);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 5, ADC_SampleTime_480Cycles);   
    
    //ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 6, ADC_SampleTime_480Cycles); 
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 6, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vbat,    7, ADC_SampleTime_480Cycles);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10,  8, ADC_SampleTime_480Cycles);
    
    /* Enable VBAT channel */
    ADC_VBATCmd(ENABLE); 
    
    ADC_TempSensorVrefintCmd(ENABLE); 

    /* Enable DMA request after last transfer (Single-ADC mode) */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
 
    /* Enable ADC1 **************************************************************/
    ADC_Cmd(ADC1, ENABLE);
     
        /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConv(ADC1);
//    bsp_DelayMS(3000);
}


/*
*************************************************************************************************
  VSENSE：温度传感器的当前输出电压，与ADC_DR 寄存器中的结果ADC_ConvertedValue之间的转换关系为： 
            ADC_ConvertedValue * Vdd
  VSENSE = --------------------------
            Vdd_convert_value(0xFFF)  
    ADC转换结束以后，读取ADC_DR寄存器中的结果，转换温度值计算公式如下：
          V25 - VSENSE
  T(℃) = ------------  + 25
           Avg_Slope
  V25：  温度传感器在25℃时 的输出电压，典型值0.76 V。
  Avg_Slope：温度传感器输出电压和温度的关联参数，典型值2.5 mV/℃。
************************************************************************************************
*/
float GetTemp(uint16_t advalue)
{
    float Vtemp_sensor;
    float  Current_Temp;
    
    Vtemp_sensor = advalue * 3.3f/ 4095;  				           
    Current_Temp = (Vtemp_sensor - 0.76f)/0.0025f + 25;  
    
    return Current_Temp;
}

/***************************** 安富莱电子 www.spacetouch.co (END OF FILE) *********************************/
