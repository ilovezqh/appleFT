/* 1.配置芯片
   2.让芯片进入Standby进行功耗测量
   3.唤醒芯片(通过ONOFF信号触发)，看芯片是否成功唤醒（查看AVDD_CMU）
*/
#include "__test_common.h"

#define CURRENT_OFFSET  ee_data.current_offset

#define SIOVCC_EN  0

void iovcc_in_float(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 开启GPIOA的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //IOVCC
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void iovcc_in_pull_down(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 开启GPIOA的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //IOVCC
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_DOWN ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}


fsm_rt_t test_current(void)
{
    
    static fsm_rt_t ret=fsm_rt_cpl;
    static enum
    {
        START = 0,
        ENTER_HIBERNATE,
        IS_CALI_MODE,
        MEASURE_CURRENT,
 
    } s_tState = START;
 
    static uint32_t adc_cnt;
    static uint32_t adc_sum;
    static uint32_t siovcc_adc_sum;
    //fsm_rt_t ret;
    timer_creat();
    switch( s_tState )
    {
    case START:
        {
            apple_prst();
            test_power.consumption_on();                                        /* 接入功耗测量电阻100欧姆 */
            adc_cnt = 0;
            adc_sum = 0;
            uint8_t temp=0;//复位寄存器设置：使能debounce,低电平触发，使能上拉，使能复位
            apple_write_addr16(APPLE_DEV_ADDR,&temp,PRST_CTL_REG,1);  
            set_apple_debug_out(ITEM_PMU_DBG_OFF);
            set_apple_debug_out(ITEM_CMU_DBG_OFF);
            socket_touch_off();
            s_tState++;
        }
       
        //break;
    
    case ENTER_HIBERNATE:
 
        ret = set_apple_work_state(STANDBY);
        socket_asu_pu_off();
        socket_prst_ctl(prst_state_float);
//        if(ret != fsm_rt_cpl) 
//        {
//            ret = fsm_rt_err_st
//            goto fail;
//        }
        #if SIOVCC_EN
        iovcc_sample_pin_deinit();
        timer_start(50);
        #else
        
        timer_start(150);
        #endif
        
        s_tState = IS_CALI_MODE;
        break;
        
    case IS_CALI_MODE:
        if(is_time_up())
        {
            #if SIOVCC_EN
            iovcc_sample_pin_init();
            #endif
 
            
           // socket_i_self();
            timer_start(150);
            s_tState = MEASURE_CURRENT;
        }
        break;
    
    case MEASURE_CURRENT: //hibernate下测量siovcc电压 和 电流
        if( is_time_up() )
        {
            
            adc_sum += g_adc_value[ADC_RANK_AMP];
            siovcc_adc_sum += g_adc_value[ADC_RANK_BGR];
            adc_cnt++;
            timer_start(2);
            if(adc_cnt == 256)
            {
                uint32_t current_adc_mean = adc_sum/256;
                uint32_t siovcc_adc_mean = siovcc_adc_sum/256;
                trim_result.standby_current = current_adc_mean*3300/4095;
                trim_result.hibernate_siovcc_volt = siovcc_adc_mean*3300/4095;
                #if SIOVCC_EN
                if(trim_result.hibernate_siovcc_volt < SIOVCC_VOLT_MIN || trim_result.hibernate_siovcc_volt > SIOVCC_VOLT_MAX)
                {
                    logging("fail siovcc volt:%4dmv\r\n",trim_result.hibernate_siovcc_volt);
                    ret = fsm_rt_err_siovcc;
                    goto fail;
                }
                else
                {
                    logging("SUCCESS siovcc volt:%4dmv\r\n",trim_result.hibernate_siovcc_volt);
                }
                #endif
                
                if(trim_result.standby_current >  CURRENT_OFFSET) trim_result.standby_current -= CURRENT_OFFSET;
                if((trim_result.standby_current > STANDBY_CURRENT_LO) && (trim_result.standby_current < STANDBY_CURRENT_HI))
                {
                    //test_power.consumption_off();  //电流取样电阻去除
                    logging("SUCCESS current(%d):%d.%duA\r\n",ee_data.current_offset,trim_result.standby_current/10, trim_result.standby_current%10);
//                    adc_cnt = 0;
//                    adc_sum = 0;
//                    siovcc_adc_sum = 0;
                    s_tState = START;
                    return fsm_rt_cpl;
                }
                else
                {
                    
                    logging("fail current:%d.%duA\r\n",trim_result.standby_current/10, trim_result.standby_current%10);
                    ret = fsm_rt_err_standby_current;
                    goto fail;
                }
            }
        }
        break; 
    }
    
    return fsm_rt_on_going;
    
    fail:
    s_tState = START;
    test_power.consumption_off();  //电流取样电阻去除
    return ret;
}


 
#define CURRENT_OFFSET_MAX   15  //1.5uA
 
uint32_t g_no_load_current;
//空载电流校准
fsm_rt_t no_load_current_calibration(void)
{
    static enum
    {
        START = 0,
        CALI_POWER_ON,
        NO_LOAD_CALI,
        CALI_OVER,
        CPL,
    } s_tState = START;
 
    static uint32_t adc_cnt;
    static uint32_t adc_sum;
    static uint32_t cali_sum;
    static uint32_t cali_cnt;
    static uint32_t temp;
    
    timer_creat();
    switch( s_tState )
    {
    case START:
        temp = 0;
        cali_cnt = 0;
        cali_sum = 0;
        s_tState++;
        //break;
    
    case CALI_POWER_ON:
        adc_sum = 0;
        adc_cnt = 0;
        test_power.on();
        test_power.consumption_on();                                        //接入功耗测量电阻100欧姆
        timer_start(500);
        s_tState = NO_LOAD_CALI; 
        //break;
    
    case NO_LOAD_CALI:  //检查是否进入 standby ，如进入后则测功耗 之后退出低功耗
        if( is_time_up() )
        {
            IWDG_Feed();
            adc_sum += g_adc_value[ADC_RANK_AMP];
            adc_cnt++;
            //timer_start(1);
            if(adc_cnt == 1024)
            {
                uint32_t current_adc_mean = adc_sum/1024;
                temp = current_adc_mean*3300/4095;
                test_power.consumption_off();  //电流取样电阻去除
                test_power.off();
                s_tState = CALI_OVER;
                timer_start(50);
            }
        }
        break;

    case CALI_OVER:
        if(is_time_up())
        {
            cali_sum += temp;
            cali_cnt++;
            if(cali_cnt == 8)
            {
                temp = cali_sum/8;
                s_tState = CPL;
            }
            else
            {
                s_tState = CALI_POWER_ON;
            }
        }
        break;
    case CPL:   //结束
        
        s_tState = START;
        
        if( (temp < CURRENT_OFFSET_MAX))
        {
            test_power.consumption_off();  //电流取样电阻去除
            ee_data.current_offset = temp;
            ee_data_save();
            LCD_SHOW_WARN("SUCCESS CALI OFFSET:%d.%duA\r\n",temp/10, temp%10);
            return fsm_rt_cpl;
        }
        else
        {
            LCD_SHOW_WARN("FAIL CALI OFFSET:%d.%duA\r\n",temp/10, temp%10);
            return fsm_rt_err_no_load_current;
        }
        //break;
           
    }
    return fsm_rt_on_going;
}
