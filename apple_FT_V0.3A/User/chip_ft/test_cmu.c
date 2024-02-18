#include "__test_common.h"

#define CORE_TRIM_DIFF_MAX      4  //core 中间频点100M与80M的差和100M与120M的trim值的差，这两个差值的差距应该小于等于3
#define CORE_DIFF_RETRY_MAX     3

typedef struct
{
    uint32_t    tar;                                                            //trim目标频率
    int32_t    (*trim_val_set)(uint8_t, uint32_t*);                                        //trim设置函数
    uint8_t     trim_val_low;                                                   //trim 寄存器最小值
    uint8_t     trim_val_hi;                                                    //trim 寄存器最大值
    uint32_t    best_trim_freq;                                                 //频率结果
    uint8_t     best_trim_val;                                                  //trim寄存器最佳值
    uint8_t     dir;                                                            //1表示trim值越大频率越大，0则反之
}freq_search_t;

 
//static uint32_t pulse_5ms_get(void);
static uint32_t pulse_10ms_get(void);
//static int32_t  clk_freq_get(uint32_t* freq);
static int32_t  clk_freq_10ms_get(uint32_t* freq);
static int32_t freq_binary_search(freq_search_t*  freq_search_struct);
static int32_t rcl_trim_set(uint8_t val, uint32_t* pFreq);
static int32_t rch_trim_set(uint8_t val, uint32_t* pFreq);
 
static fsm_rt_t lrc_trim(void);
static fsm_rt_t rch_trim(void);


fsm_rt_t test_cmu(void)
{
    static enum
    {
        START = 0,    
        LRC_TRIM,
        OSC_TRIM,
    } s_tState = START;
    fsm_rt_t ret = fsm_rt_cpl;

    switch(s_tState)
    {
    case START:
        {
            //uint8_t temp=0;
            //apple_prst();
            //apple_write_addr16(APPLE_DEV_ADDR,&temp,PRST_CTL_REG,1);  //不要PRST功能，PRST/GPIO2 用来放出时钟
            //bsp_prst_ctl_deinit();
            s_tState++;
        }
        //break;
    
     
    case LRC_TRIM:
           do
            {
                ret = lrc_trim();
            }while(ret == fsm_rt_on_going);
            
            if(ret != fsm_rt_cpl) goto fail;
 
            logging("SUCCESS rcl:%d,%dHz(%dHz)\r\n", trim_reg_data.rcl_f, 
                                                    trim_measure.rclFreq,default_measure.rclFreq);
            s_tState = OSC_TRIM;
        break;
        
    case OSC_TRIM:  // 80M 对应的trim值
        { 
            apple_prst();
            do
            {
                ret = rch_trim();
            }while(ret == fsm_rt_on_going);
            
            if(ret != fsm_rt_cpl) goto fail;

            logging("SUCCESS rch:%d,%dHz(%dHz)\r\n",
                    trim_reg_data.rch_f,
                    trim_measure.rchFreq,default_measure.rchFreq);
            s_tState = START;
            return fsm_rt_cpl;
        }
        //break;
    }
    return fsm_rt_on_going;
    
    fail:
    s_tState = START;
    return ret;
}

uint32_t difference(uint32_t a, uint32_t b) 
{
    return (a > b) ? (a - b) : (b - a);
}
//获取OSC_DEV trim值
//返回频率， 0 表示没有合适的trim值 
static fsm_rt_t lrc_trim(void)
{
    freq_search_t freq_search_p1;
    freq_search_t freq_search_p2;
    freq_search_t* pFreq_search_best = NULL;
    //set_apple_trim_value(ITEM_VBG, BGR_FIX_VAL); 
    //set_apple_trim_value(ITEM_VBG,BGR_FIX_VAL); 
    set_apple_debug_out(ITEM_RCL); //32K 注入GPIO3
    delay_ms(2);
 
    //debug("32K reg value: %d\r\n",read_apple_trim_value(ITEM_RCL));
    default_measure.rclFreq = pulse_10ms_get(); //rcl_trim_set(trim_result.osc_pmu_trim_default);
    
    freq_search_p1.tar          = RCL_TAR;
    freq_search_p1.trim_val_set = rcl_trim_set;
    freq_search_p1.dir          = 0;//trim值越大，频率越小
    freq_search_p1.trim_val_low = RCL_TRIM_P1_LO;
    freq_search_p1.trim_val_hi  = RCL_TRIM_P1_HI;
    freq_binary_search(&freq_search_p1);
    
    freq_search_p2.tar          = RCL_TAR;
    freq_search_p2.trim_val_set = rcl_trim_set;
    freq_search_p2.dir          = 0;//trim值越大，频率越小
    freq_search_p2.trim_val_low = RCL_TRIM_P2_LO;
    freq_search_p2.trim_val_hi  = RCL_TRIM_P2_HI;
    freq_binary_search(&freq_search_p2);
    
    /* 比较32K两段线性区最佳trim值 */
    if(  difference(freq_search_p1.best_trim_freq,RCL_TAR) <  difference(freq_search_p2.best_trim_freq,RCL_TAR) )
    {
        pFreq_search_best = &freq_search_p1;
    }
    else
    {
        pFreq_search_best = &freq_search_p2;
    }
    
    if((pFreq_search_best->best_trim_freq <= RCL_HI)  && (pFreq_search_best->best_trim_freq >= RCL_LO))
    {
        trim_reg_data.rcl_f     = pFreq_search_best->best_trim_val;
        trim_measure.rclFreq    = pFreq_search_best->best_trim_freq;
        return fsm_rt_cpl;
    }
    else
    {
        logging("fail lrc:%d,%d \r\n", pFreq_search_best->best_trim_val*2, 
                                       pFreq_search_best->best_trim_freq);
        
        return fsm_rt_err_lrc;
    }
}


//获取OSC_DEV trim值
//返回频率， 0 表示没有合适的trim值 
static fsm_rt_t rch_trim(void)
{
    freq_search_t freq_search;
    
    set_apple_debug_out(ITEM_DBGCLK); //79K 注入GPIO3
    default_measure.rchFreq = pulse_10ms_get()*1024; //rcl_trim_set(trim_result.osc_pmu_trim_default);
    
    freq_search.tar          = RCH_TAR;
    freq_search.trim_val_set = rch_trim_set;
    freq_search.dir          = 1;//trim值越大，频率越小
    freq_search.trim_val_low = RCH_TRIM_LO;
    freq_search.trim_val_hi  = RCH_TRIM_HI;
    
    freq_binary_search(&freq_search);
    
    
    if((freq_search.best_trim_freq <= RCH_HI)  && (freq_search.best_trim_freq >= RCH_LO))
    {
        trim_reg_data.rch_f     = freq_search.best_trim_val;
        trim_measure.rchFreq    = freq_search.best_trim_freq;
        return fsm_rt_cpl;
    }
    else
    {
        logging("fail lrc:%d,%d \r\n", freq_search.best_trim_val*2, 
                                       freq_search.best_trim_freq);
        return fsm_rt_err_lrc;
    }
}




 

 
//range:偏差范围，超过偏差范围返回-1，否则返回0
int check_deviation(uint8_t* numbers, uint8_t n,uint8_t  range) 
{
    uint32_t sum = 0;
    
    for (int i = 0; i < n; i++) 
    {
        sum += numbers[i];
    }
    
    int average = sum / n;
    
    for (int i = 0; i < n; i++) 
    {
        if (abs(numbers[i] - average) > range) 
        {
            return -1;
        }
    }
    return 0;
}
 

#if 1
//设置trim值，返回频率
static int32_t rcl_trim_set(uint8_t val, uint32_t* pFreq)
{
     
    set_apple_trim_value(ITEM_RCL,val);
 
    int32_t err = clk_freq_10ms_get(pFreq);
    if(err) return -1;
    return 0; 
}
#else
//设置trim值，返回频率
static uint32_t rcl_trim_set(uint8_t val)
{
    brom_write_word(PMU_TRIM_CTL, (pmu_trim_ctl & ~(0x3f<<8)) | (val<<8));//OSC_ADC_TRIM值设置
    return pulse_5ms_get(); 
}
#endif

 

#if 1
//设置trim值，返回频率
static int32_t rch_trim_set(uint8_t val, uint32_t* pFreq)
{
    set_apple_trim_value(ITEM_RCH,val);
    int32_t err = clk_freq_10ms_get(pFreq);
    *pFreq = 1024*(*pFreq);
    if(err) return -1;
     
    return 0; 
}
#else
//设置trim值，返回频率
static uint32_t osc_core_trim_set(uint8_t val)
{
    brom_write_word(OSC_CORE_CTL,  (1<<8)|(val<<0));//OSC_CORE_CTL值设置
    return pulse_5ms_get()*128; 
}
#endif

//输入参数
//tar目标值
//trim_set:设置trim值的函数
//val_lwo:校准值 最小值
//val_hi:校准值 最大值
//dir 考虑 trim增大与频率增大是否通向
static int32_t freq_binary_search(freq_search_t*  freq_search_struct)
{
    uint32_t tar                = (*freq_search_struct).tar;
    int32_t (*trim_set)(uint8_t,uint32_t*)  = (*freq_search_struct).trim_val_set;
    uint8_t high                = (*freq_search_struct).trim_val_hi;
    uint8_t low                 = (*freq_search_struct).trim_val_low;
    uint8_t dir                 = (*freq_search_struct).dir;
    uint8_t mid;
    uint32_t mid_freq;
    uint32_t min_diff           = tar;
  
    while (low <= high) 
    {
        mid = (low + high) / 2;
        int32_t err = trim_set(mid, &mid_freq);
        if(err) return -1;
        //logging("mid=%3d, low=%3d, high=%3d, mid_freq=%8d\r\n",mid,low,high,mid_freq);
        
        uint32_t diff = (mid_freq > tar)?(mid_freq-tar):(tar-mid_freq);         //当前频率与目标频率的差值
        if (diff < min_diff) 
        {
            min_diff = diff;
            (*freq_search_struct).best_trim_val  = mid;
            (*freq_search_struct).best_trim_freq = mid_freq;
        }
        
        if (mid_freq == tar ) 
        {
            break;//return mid;
        } 
        else if((mid == low) && (mid== high)) /* low mid high三个数为同一值，穷举完毕 */
        {
            break;
        }
        else 
        {
            if(dir)
            {
                if (mid_freq < tar) 
                {
                    low = mid + 1;
                } 
                else 
                {
                    high = mid - 1;
                }
            }
            else
            {
                if (mid_freq < tar) 
                {
                   
                    high = mid - 1;
                } 
                else 
                {
                    low = mid + 1;
                }
            }
        }
    }
    return 0;
}



#if 0

static uint32_t pulse_5ms_get(void)
{
    uint32_t osc_dev_freq;                                                      //频率
    
//    pwm_measure_reset();                                                      //复位脉冲计数
    pwm_measure_start();                                                        // 启动定时器捕获
    set_tick(20000);                                                            //定时20ms
    wait_tick();                                                                //等待20ms结束
    osc_dev_freq  = pwm_measure_stop();                                         // 读取计数脉冲值
    reset_tick();                                                               //复位定时
    osc_dev_freq *= 50;                                                         //计算脉冲频率（kiwi分频后）
    osc_dev_freq *= 1;                                                          //实际振荡频率
   
    return osc_dev_freq;
}
#else
//static uint32_t pulse_5ms_get(void)
//{
//    uint32_t osc_dev_freq;                                                      //频率
//    
////    pwm_measure_reset();                                                      //复位脉冲计数
//    pwm_measure_start();                                                        // 启动定时器捕获
//    set_tick(5000);                                                            //定时20ms
//    wait_tick();                                                                //等待20ms结束
//    osc_dev_freq  = pwm_measure_stop();                                         // 读取计数脉冲值
//    reset_tick();                                                               //复位定时
//    osc_dev_freq *= 200;                                                        //计算脉冲频率（kiwi分频后）
//    osc_dev_freq *= 1;                                                          //实际振荡频率
//   
//    return osc_dev_freq;
//}

static uint32_t pulse_10ms_get(void)
{
    uint32_t osc_dev_freq;                                                      //频率
    
    pwm_measure_start();                                                        // 启动定时器捕获
    set_tick(10000);                                                            //定时20ms
    wait_tick();                                                                //等待20ms结束
    osc_dev_freq  = pwm_measure_stop();                                         // 读取计数脉冲值
    reset_tick();                                                               //复位定时
    osc_dev_freq *= 100;                                                         //计算脉冲频率（kiwi分频后）
    osc_dev_freq *= 1;                                                          //实际振荡频率
   
    return osc_dev_freq;
}
#endif

//static int32_t  clk_freq_get(uint32_t* freq)
//{
//    #define TRY_TIME_MAX  3
//    uint32_t try_cnt = 0;
//    uint32_t freq1   = 0;
//    uint32_t freq2   = 0;
//    uint32_t avg     = 0;
//    uint32_t diff    = 0;
//    
//    do
//    {
//        freq1 = pulse_5ms_get();
//        freq2 = pulse_5ms_get();
//        diff  = (freq1>freq2)? (freq1-freq2):(freq2-freq1);
//        avg   = (freq1+freq2)/2;
//        *freq = avg;
//        try_cnt++;
//        if((diff*100) <= avg) return 0;    //两次差值小于1%，
//    }while(try_cnt < TRY_TIME_MAX);

//    return -1;
//}


static int32_t  clk_freq_10ms_get(uint32_t* freq)
{
    #define TRY_TIME_MAX  3
    uint32_t try_cnt = 0;
    uint32_t freq1   = 0;
    uint32_t freq2   = 0;
    uint32_t avg     = 0;
    uint32_t diff    = 0;
    
    do
    {
        freq1 = pulse_10ms_get();
        freq2 = pulse_10ms_get();
        diff  = (freq1>freq2)? (freq1-freq2):(freq2-freq1);
        avg   = (freq1+freq2)/2;
        *freq = avg;
        try_cnt++;
        if((diff*100) <= avg) return 0;    //两次差值小于1%，
    }while(try_cnt < TRY_TIME_MAX);

    return -1;
}
