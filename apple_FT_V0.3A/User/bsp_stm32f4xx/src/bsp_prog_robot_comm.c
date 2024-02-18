
/* 
robot user interface:与烧录机台的通讯 
      兼容华宇封测，DUT FT(分bin)接口 
    DUT接口     引脚
1    START     
2    NG5         
3    NG4         
4    NG3         
5    BUSY       
6    CMD        
7    NG2         
8    OK          

*/
#include "bsp.h"

#define RCC_ALL_ROBOT   (RCC_AHB1Periph_GPIOE )

#define PG_NG_PORT      GPIOE  
#define PG_NG_PIN		GPIO_Pin_13	

#define PG_BUSY_PORT    GPIOE
#define PG_BUSY_PIN		GPIO_Pin_14	

#define PG_OK_PORT      GPIOE
#define PG_OK_PIN		GPIO_Pin_15

/* 如果有接测试机台，则可以重定义下列端口；未使用可以沿用上面定义的端口 */
#define DUT_BUSY_PORT    GPIOE
#define DUT_BUSY_PIN     GPIO_Pin_14

#define DUT_OK_PORT      GPIOE
#define DUT_OK_PIN       GPIO_Pin_15

#define DUT_NG2_PORT     GPIOE
#define DUT_NG2_PIN      GPIO_Pin_13  

#define DUT_NG3_PORT     GPIOE
#define DUT_NG3_PIN      GPIO_Pin_13  

#define DUT_NG4_PORT     GPIOE
#define DUT_NG4_PIN      GPIO_Pin_13  

#define DUT_NG5_PORT     GPIOE
#define DUT_NG5_PIN      GPIO_Pin_13  



void robot_com_init(void)
{
	GPIO_InitTypeDef gpio_init={0};
    RCC_AHB1PeriphClockCmd(RCC_ALL_ROBOT , ENABLE); 
 
	GPIO_ResetBits(PG_NG_PORT,      PG_NG_PIN);
    GPIO_ResetBits(PG_OK_PORT,      PG_OK_PIN);
    GPIO_ResetBits(PG_BUSY_PORT,    PG_BUSY_PIN);
    
	GPIO_ResetBits(DUT_OK_PORT,     DUT_OK_PIN);
    GPIO_ResetBits(DUT_BUSY_PORT,   DUT_BUSY_PIN);
    GPIO_ResetBits(DUT_NG2_PORT,    DUT_NG2_PIN);
    GPIO_ResetBits(DUT_NG3_PORT,    DUT_NG3_PIN);
    GPIO_ResetBits(DUT_NG4_PORT,    DUT_NG4_PIN);
    GPIO_ResetBits(DUT_NG5_PORT,    DUT_NG5_PIN);
    
	gpio_init.GPIO_Mode     = GPIO_Mode_OUT;
	gpio_init.GPIO_OType    = GPIO_OType_PP;
    gpio_init.GPIO_PuPd     = GPIO_PuPd_UP;
	gpio_init.GPIO_Pin      = PG_BUSY_PIN ;
	GPIO_Init(PG_BUSY_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = PG_OK_PIN;
	GPIO_Init(PG_OK_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = PG_NG_PIN;
	GPIO_Init(PG_NG_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_BUSY_PIN;
	GPIO_Init(DUT_BUSY_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_OK_PIN;
	GPIO_Init(DUT_OK_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_NG2_PIN;
	GPIO_Init(DUT_NG2_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_NG3_PIN;
	GPIO_Init(DUT_NG3_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_NG4_PIN;
	GPIO_Init(DUT_NG4_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin  = DUT_NG5_PIN;
	GPIO_Init(DUT_NG5_PORT, &gpio_init);
}

#if 0
void msg_to_robot(robot_ctrl_e ack_status)
{
	u16 pg_pins = PG_BUSY_PIN |PG_NG_PIN | PG_OK_PIN;
	GPIO_ResetBits(PG_PORT,pg_pins);
	
	if(ack_status == PG_IDLE)
	{
		return;
	}
	else if(ack_status == PG_BUSY)
	{
		GPIO_SetBits(PG_PORT,PG_BUSY_PIN);
	}
	else if(ack_status == PG_OK)
	{
		GPIO_SetBits(PG_PORT,PG_OK_PIN);
	}
	else if(ack_status == PG_NG)
	{
		GPIO_SetBits(PG_PORT,PG_NG_PIN);
	}
}

#else

typedef struct
{
    robot_ctrl_e     ack_;
    GPIO_TypeDef*   GPIOx;
    uint16_t        GPIO_Pin;   
}binning_msg_t;

static binning_msg_t ng_bin_msg_table[]=
{
    DUT_OK,     DUT_OK_PORT,    DUT_OK_PIN,
    DUT_NG2,    DUT_NG2_PORT,   DUT_NG2_PIN,
    DUT_NG3,    DUT_NG3_PORT,   DUT_NG3_PIN,
    DUT_NG4,    DUT_NG4_PORT,   DUT_NG4_PIN,
    DUT_NG5,    DUT_NG5_PORT,   DUT_NG5_PIN,
};

void msg_to_robot(robot_ctrl_e ack_status)
{
	GPIO_ResetBits(PG_NG_PORT,      PG_NG_PIN);
    GPIO_ResetBits(PG_OK_PORT,      PG_OK_PIN);
    GPIO_ResetBits(PG_BUSY_PORT,    PG_BUSY_PIN);
    
	GPIO_ResetBits(DUT_OK_PORT,     DUT_OK_PIN);
    GPIO_ResetBits(DUT_BUSY_PORT,   DUT_BUSY_PIN);
    GPIO_ResetBits(DUT_NG2_PORT,    DUT_NG2_PIN);
    GPIO_ResetBits(DUT_NG3_PORT,    DUT_NG3_PIN);
    GPIO_ResetBits(DUT_NG4_PORT,    DUT_NG4_PIN);
    GPIO_ResetBits(DUT_NG5_PORT,    DUT_NG5_PIN);
 
    /* DUT测试和 烧录机台测试信号兼容；DUT分bin；多个NG bin统一为NG */
    if((ack_status == DUT_NG2) || (ack_status == DUT_NG3) || (ack_status == DUT_NG4) || (ack_status == DUT_NG5))
    {
        GPIO_SetBits(PG_NG_PORT,    PG_NG_PIN);
    }
    else if((ack_status == PG_OK) || (ack_status == DUT_OK))
    {
        GPIO_SetBits(PG_OK_PORT,    PG_OK_PIN);
        GPIO_SetBits(DUT_OK_PORT,   DUT_OK_PIN);
    }
    else if((ack_status == PG_BUSY) || (ack_status == DUT_BUSY))
    {
        GPIO_SetBits(PG_BUSY_PORT,  PG_BUSY_PIN);
        GPIO_SetBits(DUT_BUSY_PORT, DUT_BUSY_PIN);
    }

    if(ack_status == PG_IDLE)
	{
		return;
	}
    else
    {
        for(uint8_t i=0; i<ARRY_SIZE(ng_bin_msg_table); i++)
        {
            if( ack_status == ng_bin_msg_table[i].ack_ )
            {
                GPIO_SetBits(ng_bin_msg_table[i].GPIOx, ng_bin_msg_table[i].GPIO_Pin);
            }
        }
    }
}
    
#endif

