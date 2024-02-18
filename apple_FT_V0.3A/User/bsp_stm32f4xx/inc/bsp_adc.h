#ifndef __ADC_H
#define	__ADC_H

#include "stm32f4xx.h"
// 注意：用作ADC采集的IO必须没有复用，否则采集电压会有影响
/********************ADC1输入通道（引脚）配置**************************/
#define    ADC_x                          ADC1
#define    ADC_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    ADC_CLK                       RCC_APB2Periph_ADC1

/********************led ADC CHECK输入通道（引脚）配置**************************/
#define    ADC_GPIO_APBxClock_FUN        RCC_AHB1PeriphClockCmd
#define    ADC_GPIO_CLK                  RCC_AHB1Periph_GPIOA  
#define    ADC_PORT                      GPIOA


#define    ADC_PIN                       GPIO_Pin_0
#define    ADC_CHANNEL                   ADC_Channel_0

/********************ADC CHECK输入通道（引脚）配置**************************/
#define    VREF_ADC_GPIO_APBxClock_FUN        RCC_AHB1PeriphClockCmd
#define    VREF_ADC_GPIO_CLK                  RCC_AHB1Periph_GPIOC  
#define    VREF_ADC_PORT                      GPIOC


#define    VREF_ADC_PIN                       GPIO_Pin_0
#define    VREF_ADC_CHANNEL                   ADC_Channel_10


/********************AT ADC CHECK输入通道（引脚）配置**************************/
#define    AT_ADC_GPIO_APBxClock_FUN        RCC_AHB1PeriphClockCmd
#define    AT_ADC_GPIO_CLK                  RCC_AHB1Periph_GPIOA  
#define    AT_ADC_PORT                      GPIOA


#define    AT_ADC_PIN                       GPIO_Pin_2
#define    AT_ADC_CHANNEL                   ADC_Channel_2

//extern __IO uint16_t ADC_ConvertedValue;
//void ADCx_Init(void);
//uint16_t  Get_ADCx_Value(void);

//void vr13_ADCx_Init(void);
//void vref_ADCx_Init(void);

void  adc_current_init(void);
void  scl_pu_adc(void);
void adc_scl_pu_init(void);
void VREF_PIN_Adc_Init(void);
void adc_at_init(void);
void  adc_at_deinit(void);
u16 Get_Adc(void);
u16 Get_Adc_Average(u8 ch,u8 times);
void  VREF_X_Adc_Init(void);
#endif /* __ADC_H */


