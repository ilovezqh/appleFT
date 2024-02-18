#include "bsp_adc.h"
#include "bsp_timer.h"
#include "ft_common.h"
#include "main.h"



//初始化ADC
void  adc_current_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOc时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟


    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//Pc1 通道11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//不带上下拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化

//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);	//复位结束

    ADC_StructInit(&ADC_InitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC1, &ADC_InitStructure);//ADC初始化


    //设置指定ADC的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC1, 11, 1, ADC_SampleTime_56Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度

    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器
    ADC_SoftwareStartConv(ADC_x);

}


//初始化ADC
void  adc_scl_pu_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;

    ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);//使能GPIOA时钟
    ADC_APBxClock_FUN(ADC_CLK, ENABLE); //使能ADC1时钟

    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0 通道0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(ADC_PORT, &GPIO_InitStructure);//初始化


//    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;//PA0 通道0
//    GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
//    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT ;//模拟输入
//    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL ;//不带上下拉
//    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

//    RCC_APB2PeriphResetCmd(ADC_CLK, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(ADC_CLK, DISABLE);	//复位结束
    
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC_x, &ADC_InitStructure);//ADC初始化
    //设置指定ADC的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC1, 0, 1, ADC_SampleTime_480Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度

    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器
    ADC_SoftwareStartConv(ADC_x);
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    
}

void  scl_pu_adc(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0 通道0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    //GPIO_InitStructure.GPIO_OType =  GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(ADC_PORT, &GPIO_InitStructure);//初始化
}


void  VREF_X_Adc_Init(void)
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;

    ADC_APBxClock_FUN(ADC_CLK, ENABLE); //使能ADC1时钟

//    RCC_APB2PeriphResetCmd(ADC_CLK, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(ADC_CLK, DISABLE);	//复位结束

    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_StructInit(&ADC_InitStructure);
    
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC_x, &ADC_InitStructure);//ADC初始化

    
    //设置指定ADC的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC1, 17, 1, ADC_SampleTime_56Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度

    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器
    ADC_SoftwareStartConv(ADC_x);

}








void  VREF_PIN_Adc_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;


    VREF_ADC_GPIO_APBxClock_FUN(VREF_ADC_GPIO_CLK, ENABLE);//使能GPIOC时钟
    ADC_APBxClock_FUN(ADC_CLK, ENABLE); //使能ADC1时钟


    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin = VREF_ADC_PIN;//PA0 通道0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//不带上下拉
    GPIO_Init(VREF_ADC_PORT, &GPIO_InitStructure);//初始化

//    RCC_APB2PeriphResetCmd(ADC_CLK, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(ADC_CLK, DISABLE);	//复位结束
   ADC_StructInit(&ADC_InitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC_x, &ADC_InitStructure);//ADC初始化


    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器

}

//scp测试
void  adc_at_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;


    AT_ADC_GPIO_APBxClock_FUN(AT_ADC_GPIO_CLK, ENABLE);//使能GPIOA时钟
    ADC_APBxClock_FUN(ADC_CLK, ENABLE); //使能ADC1时钟


    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin = AT_ADC_PIN;//PA2 通道2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(AT_ADC_PORT, &GPIO_InitStructure);//初始化

//    RCC_APB2PeriphResetCmd(ADC_CLK, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(ADC_CLK, DISABLE);	//复位结束
    ADC_StructInit(&ADC_InitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC_x, &ADC_InitStructure);//ADC初始化
    
    ADC_RegularChannelConfig(ADC1, 2, 1, ADC_SampleTime_56Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度
    
    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器
    ADC_SoftwareStartConv(ADC_x);
}

//吸烟的时候需要用到 AT作为bsp_key 的引脚检测
void  adc_at_deinit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef       ADC_InitStructure;


    AT_ADC_GPIO_APBxClock_FUN(AT_ADC_GPIO_CLK, ENABLE);//使能GPIOA时钟
    ADC_APBxClock_FUN(ADC_CLK, ENABLE); //使能ADC1时钟


    //先初始化ADC1通道0 IO口
    GPIO_InitStructure.GPIO_Pin     = AT_ADC_PIN;//PA2 通道2
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;// 输入
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN ;//内部下拉 电阻40KΩ
    GPIO_Init(AT_ADC_PORT, &GPIO_InitStructure);//初始化

//    RCC_APB2PeriphResetCmd(ADC_CLK, ENABLE);	 //ADC1复位
//    RCC_APB2PeriphResetCmd(ADC_CLK, DISABLE);	//复位结束
    ADC_StructInit(&ADC_InitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC_x, &ADC_InitStructure);//ADC初始化
    
    ADC_RegularChannelConfig(ADC1, 2, 1, ADC_SampleTime_56Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度
    
    ADC_Cmd(ADC_x, ENABLE);//开启AD转换器
    ADC_SoftwareStartConv(ADC_x);
}



////返回值:转换结果
//u16 Get_AMP_Adc(void)
//{
//	  	//设置指定ADC的规则组通道，一个序列，采样时间
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_480Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度
//
//	ADC_SoftwareStartConv(ADC2);		//使能指定的ADC1的软件转换启动功能
//
//	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//等待转换结束

//	return ADC_GetConversionValue(ADC2);	//返回最近一次ADC1规则组的转换结果
//}






//获得ADC值
//ch: @ref ADC_channels
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
u16 Get_Adc(void)
{
   // 
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ) != SET);//等待转换结束
    ADC_SoftwareStartConv(ADC1);
    uint16_t _adc_value = ADC_GetConversionValue(ADC1);
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    return _adc_value;	//返回最近一次ADC1规则组的转换结果
}
//获取通道ch的转换值，取times次,然后平均
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
u16 Get_Adc_Average(u8 ch, u8 times)
{
    u32 temp_val = 0;
    u8 t;

    for(t = 0; t < times; t++)
    {
        temp_val += Get_Adc();
        //delay_ms(5);
        bsp_DelayMS(5);
    }

    return temp_val / times;
}





//__IO uint16_t ADC_ConvertedValue;

///**
//  * @brief  ADC GPIO 初始化
//  * @param  无
//  * @retval 无
//  */
//static void ADCx_GPIO_Config(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	// 打开 ADC IO端口时钟
//	ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);
//	// 配置 ADC IO 引脚模式
//	GPIO_InitStructure.GPIO_Pin  = ADC_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//SCL配置成模拟输入引脚
//	// 初始化 ADC IO
//	GPIO_Init(ADC_PORT, &GPIO_InitStructure);
//}

///**
//  * @brief  配置ADC工作模式
//  * @param  无
//  * @retval 无
//  */
//static void ADCx_Mode_Config(void)
//{
//	ADC_InitTypeDef ADC_InitStructure;
//	ADC_CommonInitTypeDef ADC_ConfigStructure;
//	// 打开ADC时钟
//	ADC_APBxClock_FUN ( ADC_CLK, ENABLE );
//	// ADC 模式配置
//
//
//	// 只使用一个ADC，属于独立模式
//	ADC_ConfigStructure.ADC_Mode = ADC_Mode_Independent;
//	ADC_ConfigStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
//	ADC_ConfigStructure.ADC_Prescaler = ADC_Prescaler_Div8;// 配置ADC时钟为PCLK2的8分频，
//	ADC_ConfigStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 配置
//	ADC_CommonInit(&ADC_ConfigStructure);
//
//
//	// 禁止扫描模式，多通道才要，单通道不需要
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE ;
//	// 连续转换模式
//	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//	// 不用外部触发转换，软件开启即可
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
//	// 转换结果右对齐
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	// 转换通道1个
//	ADC_InitStructure.ADC_NbrOfConversion = 1;
//	// 初始化ADC
//	ADC_Init(ADC_x, &ADC_InitStructure);

//	// 配置 ADC 通道转换顺序和采样时间
//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);
//	// ADC 转换结束产生中断，在中断服务程序中读取转换值
//// ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);
//	// 开启ADC ，并开始转换
//	ADC_Cmd(ADC_x, ENABLE);

//	ADC_SoftwareStartConv(ADC_x);
//}
///**
//  * @brief  ADC初始化
//  * @param  无
//  * @retval 无
//  */
//void ADCx_Init(void)
//{
//	ADCx_GPIO_Config();
//	ADCx_Mode_Config();
//}


//static void vr13_ADCx_GPIO_Config(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	// 打开 ADC IO端口时钟
//	ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);
//	// 配置 ADC IO 引脚模式
//	GPIO_InitStructure.GPIO_Pin  = ADC_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	// 初始化 ADC IO
//	GPIO_Init(ADC_PORT, &GPIO_InitStructure);
//}

///**
//  * @brief  配置ADC工作模式
//  * @param  无
//  * @retval 无
//  */
//static void vr13_ADCx_Mode_Config(void)
//{
//	ADC_InitTypeDef ADC_InitStructure;
//	// 打开ADC时钟
//	ADC_APBxClock_FUN ( ADC_CLK, ENABLE );
//	// ADC 模式配置
//
//	// 禁止扫描模式，多通道才要，单通道不需要
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE ;
//	// 连续转换模式
//	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//	// 不用外部触发转换，软件开启即可
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
//	// 转换结果右对齐
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	// 转换通道1个
//	ADC_InitStructure.ADC_NbrOfConversion = 1;
//	// 初始化ADC
//	ADC_Init(ADC_x, &ADC_InitStructure);
//
//	ADC_CommonInitTypeDef ADC_CommonStructure;
//
//	// 只使用一个ADC，属于独立模式
//	ADC_CommonStructure.ADC_Mode = ADC_Mode_Independent;
//	ADC_CommonStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
//	ADC_CommonStructure.ADC_Prescaler = ADC_Prescaler_Div8;	// 配置ADC时钟为PCLK2的8分频，即6MHz
//	ADC_CommonStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
//	ADC_CommonInit(&ADC_CommonStructure);
//

//	// 配置 ADC 通道转换顺序和采样时间
//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);
//	// ADC 转换结束产生中断，在中断服务程序中读取转换值
//// ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);
//	// 开启ADC ，并开始转换
//	ADC_Cmd(ADC_x, ENABLE);

//	ADC_SoftwareStartConv(ADC_x);
//}
///**
//  * @brief  ADC初始化
//  * @param  无
//  * @retval 无
//  */
//void vr13_ADCx_Init(void)
//{
//	vr13_ADCx_GPIO_Config();
//	vr13_ADCx_Mode_Config();
//}

//static void vref_ADCx_Mode_Config(void)
//{
//	ADC_InitTypeDef ADC_InitStructure;
//	// 打开ADC时钟
//	ADC_APBxClock_FUN ( ADC_CLK, ENABLE );
//	// ADC 模式配置

//	// 禁止扫描模式，多通道才要，单通道不需要
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE ;
//	// 连续转换模式
//	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//	// 不用外部触发转换，软件开启即可
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
//	// 转换结果右对齐
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	// 转换通道1个
//	ADC_InitStructure.ADC_NbrOfConversion = 1;
//	// 初始化ADC
//	ADC_Init(ADC_x, &ADC_InitStructure);

//	ADC_CommonInitTypeDef ADC_CommonStructure;
//	// 只使用一个ADC，属于独立模式
//	ADC_CommonStructure.ADC_Mode = ADC_Mode_Independent;
//	ADC_CommonStructure.ADC_Prescaler = ADC_Prescaler_Div8;	// 配置ADC时钟为PCLK2的8分频，即9MHz
//	ADC_CommonStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
//	ADC_CommonInit(&ADC_CommonStructure);
//
//	// 配置 ADC 通道转换顺序和采样时间
//	ADC_RegularChannelConfig(ADC_x, VREF_ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);
//	// ADC 转换结束产生中断，在中断服务程序中读取转换值
//// ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);
//	// 开启ADC ，并开始转换
//	ADC_Cmd(ADC_x, ENABLE);

//	ADC_SoftwareStartConv(ADC_x);
//}

//static void vref_ADCx_GPIO_Config(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	// 打开 ADC IO端口时钟
//	ADC_GPIO_APBxClock_FUN(VREF_ADC_GPIO_CLK, ENABLE);
//	// 配置 ADC IO 引脚模式
//	GPIO_InitStructure.GPIO_Pin  = VREF_ADC_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	// 初始化 ADC IO
//	GPIO_Init(VREF_ADC_PORT, &GPIO_InitStructure);
//}

///**
//  * @brief  ADC初始化
//  * @param  无
//  * @retval 无
//  */
//void vref_ADCx_Init(void)
//{
//	vref_ADCx_GPIO_Config();
//	vref_ADCx_Mode_Config();
//}


//uint16_t  Get_ADCx_Value(void)
//{
//	uint16_t now_ADCX_Value =0;
//    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) != SET);
//
//	now_ADCX_Value = ADC_x->DR;
//    return now_ADCX_Value;
//	//return now_ADCX_Value*33;
//    //return now_ADCX_Value;//*33/4096;
//}


/*********************************************END OF FILE**********************/
