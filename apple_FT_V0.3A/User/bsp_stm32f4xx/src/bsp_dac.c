#include "bsp.h"
#include "main.h"

void  dac_init(void)//PA4_dac_out
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    DAC_InitTypeDef DAC_InitType;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//使能DAC时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

    DAC_InitType.DAC_Trigger = DAC_Trigger_None;	//不使用触发功能 TEN1=0
    DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None; //不使用波形发生
    DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0; //屏蔽、幅值设置
    DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Disable ;	//DAC1输出缓存关闭 BOFF1=1
    DAC_Init(DAC_Channel_1, &DAC_InitType);	 //初始化DAC通道1

    DAC_Cmd(DAC_Channel_1, ENABLE);  //使能DAC通道1

    DAC_SetChannel1Data(DAC_Align_12b_R, 0);  //12位右对齐数据格式设置DAC值
}

void dac_deinit(void)
{
    DAC_DeInit();
}

//设置通道1输出电压
//vol:0~3300,代表0~3.3V
void dac_volt_2x_set(double volt)//29mV增量
{
    
    float dac_value = (volt/2/ g_st_user.verf)  * 4095 + 0.5;
    DAC_SetChannel1Data(DAC_Align_12b_R, dac_value); //12位右对齐数据格式设置DAC值
}







