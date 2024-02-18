#include "__test_common.h"
#include "__test_return_type.h"
#include "test.h"
#include "log.h"
#include "lcd.h"
//#include "usb_bsp_msc.h"		/* usb底层驱动 */
//#include "key.h"
 
extern uint32_t manufacture_bin_crc;
extern uint32_t trim_pattern_crc;
extern uint32_t perf_pattern_crc;

trim_result_t trim_result = {0};  /*  全局变量，各ft项会将ft结果写入此变量        */



otp_t   otp_read_data;  //读出的otp的值 
otp_t   trim_reg_data;  //写入寄存器的efuse数据，otp_bit -> reg_bit
otp_t   custommer_data = {.all = 0}; //用户数据，用于配置复位，I2C地址等
const otp_t   custommer_trim_msk =  //用于检测用户配置数据合法性，禁止覆盖到trim区
{
    .lpvdd_f = 0x7,
    .vref_f  = 0xf,
    .bgr_f   = 0x3,
    .rcl_f   = 0x3f,
    .iref_f  = 0x7,
    .rch_f   = 0x7f,
    
};
trim_measure_t  trim_measure;
trim_measure_t  default_measure;

uint32_t      user_config;                 /* 全局变量，用户key， 0表示不加密，非0表示加密 */
static uint8_t test_start_flg = 0;      //测试开始置为1，结束后为0

 
static uint8_t  is_com_get_ft_cmd(void);
char board_id_name[24] = "undefine";  //板子的id名称，用于ft主板身份日志判定
static uint8_t file_env_err_flg=0;

static void test_start(void); 
static uint8_t  is_com_get_ft_cmd(void);

void test_init(void)
{
    bsp_InitASU();
    if(ee_data_init())  //获取eeprom数据:保存电流校准值，日志文本序列
    {
        LCD_SHOW_WARN("ee data init!\r"); //eeprom数据不对，被初始化的提示
    }
    if( dut_param_init() ) 
    {
        LCD_SHOW_NG("DUT Init Fail!\r");
        file_env_err_flg = 1;
    }
    
 
    LCD_SHOW_WARN("%s\r",CHIP_NAME );
    LCD_SHOW_LOG("%s:%s\r",run_mode_handle->name,TEST_FIRMWARE_VERSION);//
    LCD_SHOW_LOG("Boot:%s    cur_cali:%d\r", p_boot_ver,ee_data.current_offset);
    
 
    tick_init();
    bsp_socket_init();
    bsp_socket_ldo3v3_init();
    test_power.init();
    test_power.off();

    memset(&trim_result, 0 , sizeof(trim_result_t));
    memset(&trim_reg_data, 0 , sizeof(otp_t));
    
    //挂载sd卡，并打开 ram pattern文件，打开日志文件 在新建新的日志文件之前 关闭旧的日志文件
    
    if(spi_flash_file_sys_mount()) while(1);//挂载spi flash文件系统
    user_config = config_code_get();
    if(custommer_trim_msk.all & user_config) //用户配置数据不能涉及到trim区，否则报错
    {
        file_env_err_flg = 1;
        LCD_SHOW_NG("Config.txt Error:%X\r",custommer_trim_msk.all & user_config);
    }
    else
    {
        custommer_data.all = user_config ;
    }
    board_id_file_open(board_id_name); //获取板子名称，日志时候显示
    LCD_SHOW_WARN("%s\r",board_id_name);
    
 
    if(customer_file_open())    file_env_err_flg = 1;//{BEEP_Start(10, 20, 0);}//while(1); //打开客户烧录（FLASH）文件
    //debug("title:%s\r\n",board_id_name);
    
    LCD_SHOW_LOG("APP:%08X\r",manufacture_bin_crc);
    LCD_SHOW_LOG("PTN1:%08X   PTN2:%08X\r", trim_pattern_crc, perf_pattern_crc);

    sd_file_sys_mount();   //挂载文件系统
    logger_txt_creat();    //生成 日志.TXT文件
    
    if( file_env_err_flg ) 
    {
        BEEP_FAIL();
    }
}
 
void test_task(void)
{
    static enum 
    {
        START = 0,
        RUN,
        CPL,
    }s_tState = START;
    static uint8_t test_item_index=0;
    static fsm_rt_t ret;
    
    if(is_com_get_ft_cmd())
    {
        test_start_flg = 1;
    }
    if( test_start_flg && file_env_err_flg )  //文件环境不对 启动测试会报错
    {
        test_start_flg = 0;
        BEEP_FAIL();
        return;
    }
 
    switch(s_tState)
    {
    case START:
        if(test_start_flg)
        {
            test_item_index = 0;
            test_start();
            s_tState++;
        }
        else
        {
            comClearRxFifo(COM2);
        }
        break;
    
    case RUN:
        if(run_mode_handle != NULL && run_mode_handle->test_list_size)
        {
            ret = run_mode_handle->test_list[test_item_index]();
            if(ret != fsm_rt_on_going)
            {
                test_item_index ++;
                
                if(ret == fsm_rt_cpl)                                               //正常运行完毕
                {
                    if(test_item_index == run_mode_handle->test_list_size)
                    {
                       s_tState = CPL;
                    }
                }
                else //(ret err)                                                    //故障导致运行完毕
                {
                    s_tState = CPL;
                }
            }
        }
        else
        {
            LCD_SHOW_NG("run mode error:p=%d, size=%d\r\n", run_mode_handle, run_mode_handle->test_list_size);  
        }
        break;
        
    case CPL:
        test_start_flg  = 0;
        s_tState = START;
        bsp_ch440r_ctrl_ex(CH440R_EX_CTRL_ALL_DETACH);
        socket_touch_off();
        socket_prst_ctl(prst_state_float);
        bsp_kiwi_ft_relay_off();
        
        test_power.off();
        test_over_info_show(ret);  //根据上面返回的状态显示相应的信息 和 分binning操作
        break;
    }
}


static void test_start(void)
{
    led_fail(OFF);
    led_pass(OFF);
    msg_to_robot(PG_BUSY);
    memset(&trim_result, 0 , sizeof(trim_result_t));
    trim_var_zero_init();
    trim_reg_data.all = 0;
    socket_prst_ctl(prst_state_hi);
    socket_touch_off(); 
    test_power.consumption_off();  //电流取样电阻去除
    test_power.on();
    socket_i_chip();
    socket_asu_pu_on();
    test_start_info_show();
    //delay_ms(20);
    display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK); //相当于delay 450ms，执行时间450ms
    //delay_ms(50);
}

 


//其他模块通过调用此函数启动测试，如：key.c
void test_start_en(void)
{
    test_start_flg = 1;
    msg_to_robot(PG_BUSY);
}
static uint8_t  is_com_get_ft_cmd(void)
{
    #define enter  13 //enter键
    uint8_t cmd;
    
    if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
    {
        switch (cmd)
        {
        case enter:
            printf("\r\n[FT 测试]\r\n");
            return 1;
            //break;
        }
    }
    
    return 0;
}
