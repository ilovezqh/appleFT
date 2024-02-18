#include "bsp.h"
#include "key.h"
#include "debug.h"
#include "board_type.h"
#include "test.h"
#include "__test_return_type.h"
#include "lcd.h"

//按键驱动 使用multi_button 库;详见https://github.com/liu2guang/MultiButton
struct button  up_btn;
struct button  down_btn;
struct button  start_btn;
struct button  esc_btn;
struct button  robot_cmd_btn;
struct button  dut_cmd_btn;
struct button  dut_start_btn;

#define HARD_KEY_NUM    6				                                        /* 实体按键个数 */
#define KEY_COUNT       (HARD_KEY_NUM + 0)                                      /* 8个独立建 + 0个组合按键 */

/* 使能GPIO时钟 */
#define ALL_KEY_GPIO_CLK_ENABLE()                             \
	{                                                         \
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); \
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); \
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); \
	};

#define UP_BTN_PORT         GPIOE
#define UP_BTN_PIN          GPIO_Pin_7
    
#define DOWN_BTN_PORT       GPIOE
#define DOWN_BTN_PIN        GPIO_Pin_8
    
#define START_BTN_PORT      GPIOE
#define START_BTN_PIN       GPIO_Pin_9
    
#define ESC_BTN_PORT        GPIOE
#define ESC_BTN_PIN         GPIO_Pin_10
    
#define ROBOT_CMD_PORT      GPIOE
#define ROBOT_CMD_PIN       GPIO_Pin_12
    
#define DUT_CMD_PORT        GPIOD
#define DUT_CMD_PIN         GPIO_Pin_7
    
#define DUT_START_PORT      GPIOC
#define DUT_START_PIN       GPIO_Pin_13  


    
uint8_t  up_btn_read_pin(void)      { return GPIO_ReadInputDataBit(UP_BTN_PORT,     UP_BTN_PIN);}
uint8_t  down_btn_read_pin(void)    { return GPIO_ReadInputDataBit(DOWN_BTN_PORT,   DOWN_BTN_PIN);}
uint8_t  start_btn_read_pin(void)   { return GPIO_ReadInputDataBit(START_BTN_PORT,  START_BTN_PIN);}
uint8_t  esc_btn_read_pin(void)     { return GPIO_ReadInputDataBit(ESC_BTN_PORT,    ESC_BTN_PIN);}
uint8_t  robot_cmd_read_pin(void)   { return GPIO_ReadInputDataBit(ROBOT_CMD_PORT,  ROBOT_CMD_PIN);}
uint8_t  dut_cmd_read_pin(void)     { return GPIO_ReadInputDataBit(DUT_CMD_PORT,    DUT_CMD_PIN);}
uint8_t  dut_start_read_pin(void)   { return GPIO_ReadInputDataBit(DUT_START_PORT,    DUT_START_PIN);}

static void test_start_callback(void* btn);
static void clear_logger_callback(void* btn);
static void cali_current_callback(void* btn);

void key_init(void)
{
    GPIO_InitTypeDef gpio_init;

	/* 第1步：打开GPIO时钟 */
	ALL_KEY_GPIO_CLK_ENABLE();

	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	gpio_init.GPIO_Mode = GPIO_Mode_IN;	                                        // GPIO_Mode_IPU; /* 设置输入 */
    gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz; /* GPIO速度等级 */
    
    gpio_init.GPIO_Pin = UP_BTN_PIN;
    GPIO_Init(UP_BTN_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = DOWN_BTN_PIN;
    GPIO_Init(DOWN_BTN_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = START_BTN_PIN;
    GPIO_Init(START_BTN_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = ESC_BTN_PIN;
    GPIO_Init(ESC_BTN_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = ROBOT_CMD_PIN;
    GPIO_Init(ROBOT_CMD_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = DUT_CMD_PIN;
    GPIO_Init(DUT_CMD_PORT, &gpio_init);
    
    gpio_init.GPIO_Pin = DUT_START_PIN;
    GPIO_Init(DUT_START_PORT, &gpio_init);

    button_init(&up_btn,        up_btn_read_pin,    Bit_RESET);
    button_init(&down_btn,      down_btn_read_pin,  Bit_RESET);
    button_init(&start_btn,     start_btn_read_pin, Bit_RESET);
    button_init(&esc_btn,       esc_btn_read_pin,   Bit_RESET);
  
    
    if(g_eBoardType == board_v20)
    {
        button_init(&robot_cmd_btn, robot_cmd_read_pin, Bit_RESET);
    }
    else
    {
        button_init(&robot_cmd_btn, robot_cmd_read_pin, Bit_SET);
        button_init(&dut_cmd_btn,   dut_cmd_read_pin,   Bit_SET); //CMD高时（机台给低电平，MOS管关断，CMD变高）
        button_init(&dut_start_btn, dut_start_read_pin, Bit_SET); //CMD高时（机台给低电平，MOS管关断，CMD变高）
    }

    /* 启动测试的按键 绑定 */
    button_attach(&start_btn,       SINGLE_CLICK,  test_start_callback ); //启动
    button_attach(&robot_cmd_btn,   SINGLE_CLICK,  test_start_callback ); //启动
    button_attach(&dut_cmd_btn,     SINGLE_CLICK,  test_start_callback ); //启动
    button_attach(&dut_start_btn,   SINGLE_CLICK,  test_start_callback ); //启动
 
    button_attach(&down_btn,   DOUBLE_CLICK,      clear_logger_callback ); //双击清除日志
    button_attach(&down_btn,   LONG_PRESS_START,  cali_current_callback ); //长按校准电流
    
    button_start(&up_btn);
    button_start(&down_btn);
    button_start(&start_btn);
    button_start(&esc_btn);
    button_start(&robot_cmd_btn);
    button_start(&dut_cmd_btn);
    button_start(&dut_start_btn);
 
}

static void test_start_callback(void* btn)
{
    // do something here
    test_start_en();
}

static void clear_logger_callback(void* btn)
{
    //
    display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK); //相当于delay 450ms，执行时间450ms
    test_logger_clear(); //清除文件
}

static void cali_current_callback(void* btn)
{
    extern fsm_rt_t no_load_current_calibration(void);
    display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK); //相当于delay 450ms，执行时间450ms
    fsm_rt_t ret;
    do
    {
        ret = no_load_current_calibration();
    }while(ret == fsm_rt_on_going);
    BEEP_KeyTone();
}


