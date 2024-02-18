/* 用于校准PMU，通过ASU写相关寄存器，PMU相关测试点输出的电压变化，得到最佳trim值  */

//TODO: 每个测试PMU用二分法处理, 每个PMU测试项抽出共性，封函数
#include "__test_common.h"
 
typedef struct
{
    uint32_t    tar;                                                            //trim目标电压
    uint32_t    (*trim_val_set)(uint8_t );                                      //trim设置函数
    uint8_t     trim_val_low;                                                   //trim 寄存器最小值
    uint8_t     trim_val_hi;                                                    //trim 寄存器最大值
    uint32_t    best_trim_volt;                                                 //电压结果
    uint8_t     best_trim_val;                                                  //trim寄存器最佳值
    uint8_t     dir;                                                            //1表示trim值越大频率越大，0则反之
}volt_search_t;

static fsm_rt_t vref_trim(void);
static fsm_rt_t lpvdd_trim(void);
static uint32_t pmu_volt_get(void);
static uint32_t iref_get(void);
static uint32_t vref_trim_set(uint8_t val);
static void volt_binary_search(volt_search_t*  volt_search_struct);
static fsm_rt_t iref_trim(void);

 

fsm_rt_t test_pmu(void)
{
    static enum
    {
        START = 0,
        IREF_TRIM,
        VREF_TRIM,  
        LPV_TRIM,
    } s_tState = START;
    
    fsm_rt_t ret;

    switch(s_tState)
    {
    case START:
        {
            #if ON_BOARD_FT //在板FT，无法校准IREF项
            s_tState = VREF_TRIM;
            break;
            #else
            socket_ivref_iref();
            //apple_prst();
            //uint8_t temp=0x85;//复位寄存器设置：使能debounce,低电平触发，使能上拉，使能复位
            //apple_write_addr16(APPLE_DEV_ADDR,&temp,PRST_CTL_REG,1);  
            
            s_tState++;
            #endif
            
            
        }
        //break;
        
    case IREF_TRIM:
        {
            ret = iref_trim();
            if(ret!= fsm_rt_cpl) goto fail;
 
            set_apple_trim_value(ITEM_IREF, trim_reg_data.iref_f);   //写入最佳trim值 
            logging("SUCCESS IREF:%d,%d.%duA(%d.%duA)\r\n",trim_reg_data.iref_f, trim_measure.irefCurrent/10,trim_measure.irefCurrent%10,
                                                                          default_measure.irefCurrent/10, default_measure.irefCurrent%10);
            socket_ivref_vref();
            s_tState = VREF_TRIM;    
        }
        break;
    case VREF_TRIM:
        {
            //apple_prst(); //不复位iref校准会带入脏寄存器值：待排查
            //set_apple_trim_value(ITEM_IREF, trim_reg_data.iref_f);   //写入最佳trim值 
            
            
            ret = vref_trim();
                
            if(ret!= fsm_rt_cpl) goto fail;
            set_apple_trim_value(ITEM_VREF, trim_reg_data.vref_f);   //写入最佳trim值 
            
            logging("SUCCESS VREF:[%d]%d,%dmv(%dmv)\r\n",trim_reg_data.bgr_f,
                                                         trim_reg_data.vref_f, 
                                                         trim_measure.vrefVolt, default_measure.vrefVolt);
            s_tState = LPV_TRIM;    
        }
        //break;
 
    case LPV_TRIM:
        {
            ret = lpvdd_trim();
            if(ret!= fsm_rt_cpl) goto fail;
            apple_prst();
            delay_ms(10);
            set_apple_trim_value(ITEM_LPVDD, trim_reg_data.lpvdd_f);    //写入最佳trim值 
            set_apple_trim_value(ITEM_VREF, trim_reg_data.vref_f);      //写入最佳trim值 
            logging("SUCCESS LPVDD:%d,%dmv(%dmv)\r\n",trim_reg_data.lpvdd_f, trim_measure.lpvddVolt,default_measure.lpvddVolt);
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

//设置trim值，返回电压
static uint32_t iref_trim_set(uint8_t val)
{
    //apple_prst();
    //set_apple_debug_out(ITEM_IREF);	            //VREF电压注入GPIO03
    set_apple_trim_value(ITEM_IREF, val);
    delay_ms(2);
    return iref_get();
}

static fsm_rt_t iref_trim(void)
{
    volt_search_t volt_search;
  
    set_apple_debug_out(ITEM_IREF);	            //IREF电流注入GPIO03
    delay_ms(50);
    default_measure.irefCurrent = iref_get();

    volt_search.tar          = IREF_TAR;
    volt_search.trim_val_set = iref_trim_set;
    volt_search.dir          = 1;//trim值越大，电压越大
    volt_search.trim_val_low = IREF_TRIM_LO;
    volt_search.trim_val_hi  = IREF_TRIM_HI;
    
    volt_binary_search(&volt_search); //换一个trim值区间 继续二分法查找
    
    if((volt_search.best_trim_volt <= IREF_HI)  && (volt_search.best_trim_volt >= IREF_LO))
    {
        trim_reg_data.iref_f         = volt_search.best_trim_val;
        trim_measure.irefCurrent     = volt_search.best_trim_volt ;
       
        return fsm_rt_cpl;
    }
    else
    {
      
        logging("fail iref:%d,%d.%duA\r\n",volt_search.best_trim_volt/10,volt_search.best_trim_volt%10);
     
        return fsm_rt_err_iref;
        
    }
}


//设置trim值，返回电压
static uint32_t vref_trim_set(uint8_t val)
{
    set_apple_trim_value(ITEM_VREF, val);
    delay_ms(1);
    return pmu_volt_get();
}

static fsm_rt_t vref_trim(void)
{
    volt_search_t volt_search;
    uint32_t search_result[4][2]={0};
    static uint8_t bgr_squence[4] = {2,3,1,0};
    //set_apple_trim_value(ITEM_VBG,BGR_FIX_VAL);   
    set_apple_debug_out(ITEM_VREF);	            //VREF电压注入GPIO03
    delay_ms(2);
    default_measure.vrefVolt = pmu_volt_get();
    volt_search.tar          = VREF_TAR;
    volt_search.trim_val_set = vref_trim_set;
    volt_search.dir          = 1;//trim值越大，电压越大
    volt_search.trim_val_low = VREF_TRIM_LO;
    volt_search.trim_val_hi  = VREF_TRIM_HI;
    uint8_t trim_val_get_flg = 0;
    for(uint8_t num=0; num < 4; num++)
    {
        set_apple_trim_value(ITEM_VBG, bgr_squence[num]);
        volt_binary_search(&volt_search);
        search_result[bgr_squence[num]][0] = volt_search.best_trim_val;
        search_result[bgr_squence[num]][1] = volt_search.best_trim_volt;
        
        if(trim_val_get_flg == 0)
        {
            if((volt_search.best_trim_volt <= VREF_HI)  && (volt_search.best_trim_volt >= VREF_LO))
            {
                trim_val_get_flg = 1;
                trim_reg_data.bgr_f     = bgr_squence[num]; //BGR设置固定的
                trim_reg_data.vref_f    = volt_search.best_trim_val;
                trim_measure.vrefVolt   = volt_search.best_trim_volt;
                break;
            }
        }
    }
    if(trim_val_get_flg == 0) //未获得可用的trim值
    {
        logging("fail vref:[0]%d,%dmv\r\n",search_result[0][0],search_result[0][1]);
        logging("fail vref:[1]%d,%dmv\r\n",search_result[1][0],search_result[1][1]);
        logging("fail vref:[2]%d,%dmv\r\n",search_result[2][0],search_result[2][1]);
        logging("fail vref:[3]%d,%dmv\r\n",search_result[3][0],search_result[3][1]);
        return fsm_rt_err_vref;
    }
    else
    {
        return fsm_rt_cpl;
    }
    
    
    
    
}


//设置trim值，返回电压
static uint32_t lpvdd_trim_set(uint8_t val)
{
    apple_prst();
    //delay_ms(10);
    //set_apple_trim_value(ITEM_VBG,BGR_FIX_VAL); 
    set_apple_trim_value(ITEM_VREF, trim_reg_data.vref_f);   //写入最佳trim值     
    set_apple_debug_out(ITEM_LPVDD);	                    //VREF电压注入GPIO03
    set_apple_trim_value(ITEM_LPVDD, val);
    set_apple_work_state(STANDBY);
    delay_ms(2);
    return pmu_volt_get();
}

static fsm_rt_t lpvdd_trim(void)
{
    volt_search_t volt_search;
 
    //apple_prst();
    //set_apple_trim_value(ITEM_VBG,BGR_FIX_VAL);       
    //set_apple_trim_value(ITEM_VREF, trim_reg_data.vref_f);   //写入最佳trim值     
    set_apple_debug_out(ITEM_LPVDD);	                                        //VREF电压注入GPIO03
    set_apple_work_state(STANDBY);
    delay_ms(2);
    default_measure.lpvddVolt = pmu_volt_get();
    volt_search.tar          = LPVDD_TAR;
    volt_search.trim_val_set = lpvdd_trim_set;
    volt_search.dir          = 0;//trim值越大，电压越小
    volt_search.trim_val_low = LPVDD_TRIM_LO;
    volt_search.trim_val_hi  = LPVDD_TRIM_HI;
    
    volt_binary_search(&volt_search); //换一个trim值区间 继续二分法查找
    
    if((volt_search.best_trim_volt <= LPVDD_HI)  && (volt_search.best_trim_volt >= LPVDD_LO))
    {
        trim_reg_data.lpvdd_f  = volt_search.best_trim_val;
        trim_measure.lpvddVolt = volt_search.best_trim_volt;
        
        return fsm_rt_cpl;
    }
    else
    {
        logging("fail vref:%d,%dmv\r\n",volt_search.best_trim_val*2, volt_search.best_trim_volt);
        return fsm_rt_err_lpv;
    }
}
 
//输入参数
//tar目标值
//trim_set:设置trim值的函数
//val_lwo:校准值 最小值
//val_hi:校准值 最大值
//dir 考虑 trim增大与频率增大是否通向
static void volt_binary_search(volt_search_t*  volt_search_struct)
{
    uint32_t tar                    = (*volt_search_struct).tar;
    uint32_t(*trim_set)(uint8_t )   = (*volt_search_struct).trim_val_set;
    uint8_t high                    = (*volt_search_struct).trim_val_hi;
    uint8_t low                     = (*volt_search_struct).trim_val_low;
    uint8_t dir                     = (*volt_search_struct).dir;
    uint8_t mid;
    uint32_t mid_volt;
    uint32_t min_diff               = tar;
  
    
    while (low <= high) 
    {
        mid = (low + high) / 2;
        mid_volt = trim_set(mid);
        //debug("mid=%3d, low=%3d, high=%3d, mid_volt=%8d\r\n",mid,low,high,mid_volt);
        
        uint32_t diff = (mid_volt > tar)?(mid_volt-tar):(tar-mid_volt);         //当前频率与目标频率的差值
        if (diff < min_diff) 
        {
            min_diff = diff;
            (*volt_search_struct).best_trim_val  = mid;
            (*volt_search_struct).best_trim_volt = mid_volt;
        }
        
        
        if (mid_volt == tar ) 
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
                if (mid_volt < tar) 
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
                if (mid_volt < tar) 
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
}

//返回电压 mv
static uint32_t pmu_volt_get(void)
{
    uint32_t adc_sum=0;
    for(uint8_t i=0; i<32; i++)
    {
        delay_us(50);
        adc_sum += g_adc_value[ADC_RANK_TRIM_PMU];
    }

    uint32_t pmu_volt = adc_sum/32 * 3300 / 4095;
   
    return pmu_volt;
}

static uint32_t iref_get(void)
{
    uint32_t adc_sum=0;
    for(uint8_t i=0; i<128; i++)
    {
        delay_ms(1);
        adc_sum += g_adc_value[ ADC_RANK_IREF];
    }

    uint32_t iref_200k_volt = adc_sum/128 * 3300 / 4095;
   
    #if 0
    return iref_200k_volt/20;  //200K 取样电压跟随
    #else
    return iref_200k_volt/36; //I-V 双电源供电
    #endif
    
}
