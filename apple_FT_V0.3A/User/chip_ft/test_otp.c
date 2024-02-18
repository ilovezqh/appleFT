#include "__test_common.h"
#include "__xor_default.h"
//otp1:  brom read
//pre:  spl read efuse
//post: post = pre; trim_result_rewrite
//otp2: brom repower; pre == otp2?

static otp_t  otp_final_write_data;             //otp 最终被写入的值
static otp_t  otp_wait_write_data;              //otp 待写入的值，经过trim值 XOR 后的值
 

static uint8_t  is_otp_err_1_to_0(otp_t* dest, otp_t* src);

#define is_iref_qc_ng()    ((default_measure.irefCurrent < IREF_LO)  || (default_measure.irefCurrent > IREF_HI))
#define is_vref_qc_ng()    ((default_measure.vrefVolt    < VREF_LO)  || (default_measure.vrefVolt    > VREF_HI))
#define is_lpvdd_qc_ng()   ((default_measure.lpvddVolt   < LPVDD_LO) || (default_measure.lpvddVolt   > LPVDD_HI))
#define is_rcl_qc_ng()     ((default_measure.rclFreq     < RCL_LO)   || (default_measure.rclFreq     > RCL_HI))
#define is_rch_qc_ng()     ((default_measure.rchFreq     < RCH_LO)   || (default_measure.rchFreq     > RCH_HI))

fsm_rt_t test_efuse(void)
{
    static enum
    {
        START = 0,    
        READ_EFUSE,
        FINAL_DATA_READY,        //最终写入的efuse值 准备就绪
        WRITE_EFUSE,
        READBACK_EFUSE,
    //READBACK_EFUSE, //立即读回efuse没有意义，spl程序中会进行写入再回读的操作，并返回状态给主机
    } s_tState = START;
 
    fsm_rt_t ret;
    
    switch(s_tState)
    {
    case START:
        s_tState++;
        
        //break;
    
    case READ_EFUSE:
        {
            s_tState = FINAL_DATA_READY;
//            if(otp_read_data.all == 0)    
//            {
//                s_tState = WRITE_EFUSE;
//            }
//            else 
//            {
//                s_tState = START;
//                return fsm_rt_cpl;
//            } 
        }
        break;
        
    case FINAL_DATA_READY:
        {
            /* 1.根据trim到寄存器的值，转成otp待写入的值  */
            
            otp_wait_write_data.all      = trim_reg_data.all ^ reg_default.all;
            
            /* 2.otp已被写入的区域,则忽视 trim值,延续读到的值  并进行QC*/
            uint8_t qc_err_cnt=0;
            if( otp_read_data.iref_f  || ON_BOARD_FT )  
            {
                otp_wait_write_data.iref_f  = otp_read_data.iref_f;
                if(is_iref_qc_ng() && (!ON_BOARD_FT))
                {
                    qc_err_cnt++;
                    logging("fail iref QC NG:%d.%duA\r\n", default_measure.irefCurrent/10,default_measure.irefCurrent%10);
                }
            }
            
            if( otp_read_data.bgr_f  )  otp_wait_write_data.bgr_f   = otp_read_data.bgr_f;
            
            if( otp_read_data.vref_f )
            {
                otp_wait_write_data.vref_f  = otp_read_data.vref_f;
                if(is_vref_qc_ng())
                {
                    qc_err_cnt++;
                    logging("fail vref QC NG:%dmv\r\n", default_measure.vrefVolt);
                }
            }                
            
            if(otp_read_data.lpvdd_f) 
            {
                otp_wait_write_data.lpvdd_f   = otp_read_data.lpvdd_f;
                if(is_vref_qc_ng())
                {
                    qc_err_cnt++;
                    logging("fail lpvdd QC NG:%dmv\r\n", default_measure.lpvddVolt);
                }
            }
            
            if(otp_read_data.rcl_f)   
            {
                otp_wait_write_data.rcl_f     = otp_read_data.rcl_f;
                if(is_rcl_qc_ng())
                {
                    qc_err_cnt++;
                    logging("fail rcl QC NG:%dhz\r\n", default_measure.rclFreq);
                }
            }
            
            if(otp_read_data.rch_f)
            {
                otp_wait_write_data.rch_f     = otp_read_data.rch_f;
                if(is_rch_qc_ng())
                {
                    qc_err_cnt++;
                    logging("fail rch QC NG:%dhz\r\n", default_measure.rchFreq);
                }
            }                
            if(qc_err_cnt)
            {
                ret =  fsm_rt_err_tested_chip_qc ;
                goto fail;
            }
            
            /* 3.其他的配置信息如果需要改写 直接报错 */
            //if(otp_read_data.i2c_irq_pu_en_f)   
            
            uint8_t err_cnt=0;
            if( otp_read_data.asu_dev_addr_f && !custommer_data.asu_dev_addr_f  ) 
            {
                err_cnt++;
                logging("fail: asu_dev_addr_f conflict\r\n");
            }
            if( otp_read_data.prst_ctl_vs_f && !custommer_data.prst_ctl_vs_f  ) 
            {
                err_cnt++;
                logging("fail: prst_ctl_vs_f conflict\r\n");
            }
            if( otp_read_data.prst_ctl_en_f && !custommer_data.prst_ctl_en_f   ) 
            {
                err_cnt++;
                logging("fail: prst_ctl_en_f  conflict\r\n");
            }
            if( otp_read_data.prst_ctl_pol_f && !custommer_data.prst_ctl_pol_f  ) 
            {
                err_cnt++;
                logging("fail: prst_ctl_pol_f conflict\r\n");
            }
            if( otp_read_data.prst_ctl_pe_f && !custommer_data.prst_ctl_pe_f  ) 
            {
                err_cnt++;
                logging("fail: prst_ctl_pe_f conflict\r\n");
            }
            if( otp_read_data.i2c_irq_pu_en_f && !custommer_data.i2c_irq_pu_en_f  ) 
            {
                err_cnt++;
                logging("fail: i2c_irq_pu_en_f conflict\r\n");
            }
           
            if(err_cnt)
            {
                ret =  fsm_rt_err_custommer_conflict;
                goto fail;
            }
 
            otp_wait_write_data.all |= custommer_data.all;//加入用户配置
            
            otp_final_write_data.all = otp_wait_write_data.all;
            
      
            if( is_otp_err_1_to_0(&otp_final_write_data,  &otp_read_data) ) //检测待写入OTP的数是否存在错误： 将OTP从1改成0的情况
            {
                ret = fsm_rt_err_efuse_1_to_0;   
                goto fail;
            }
            if(otp_final_write_data.all == otp_read_data.all) 
            {
                
                logging("SUCCESS otp no need update!\r\n");
                s_tState = START;
                return fsm_rt_cpl;
            }
            logging(" [ WRITE ]read=%08X  write:%08X\r\n",otp_read_data.all, otp_final_write_data.all);
            
            s_tState = WRITE_EFUSE;
        }
        break;
        
    case WRITE_EFUSE:
        {
//            for(uint8_t i=10;i!=0;i--)
            {
                otp_write(&otp_final_write_data);
                //delay_ms(5);
            }
            
            s_tState = READBACK_EFUSE;
        }
        break;
        
    case READBACK_EFUSE:
        {
            otp_read(&otp_read_data);
            set_apple_disable_otp();
            if(otp_read_data.all != otp_final_write_data.all)
            {
                logging("[ READBACK ] fail: read=%8X  write:%8X\r\n",otp_read_data.all, otp_final_write_data.all);
                ret = fsm_rt_err_efuse_readback;
                goto fail;
            }
            logging("SUCCESS otp update!\r\n");
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
 



//fsm_rt_err_efuse_1_to_0
static uint8_t is_otp_err_1_to_0(otp_t* dest, otp_t* src)
{
    uint32_t err_cnt=0;
 
    for(uint8_t k=0; k<32; k++)
    {
        uint8_t bit_dest = !!(dest->all &  ((uint32_t)0x01<<k));
        uint8_t bit_src  = !!(src->all  &  ((uint32_t)0x01<<k));
        
        if(!bit_dest && bit_src)// 从1改写成0
        {
            debug("efuse 1_to_0:bit:%d \r\n", k);
            err_cnt++;
        }
    }
 
    if(err_cnt)
    {
        return 1;
    }
    return 0;
}




void otp_struct_printf(const otp_t* efuse_data, char* str)
{
    debug("%s\r\n",str);
    debug("efuse all:%8X\r\n",  efuse_data->all);
    debug("iref:%d(%d)\r\n",    efuse_data->iref_f  ,   efuse_data->iref_f*2);
    debug("bgr:%d(%d)\r\n",     efuse_data->bgr_f   ,   efuse_data->bgr_f);
    debug("vref:%d(%d) \r\n",   efuse_data->vref_f  ,   efuse_data->vref_f*2);
    debug("rcl:%d(%d)\r\n",     efuse_data->rcl_f   ,   efuse_data->rcl_f*2);
    debug("rch:%d(%d)\r\n",     efuse_data->rch_f   ,   efuse_data->rch_f*2);
   
}


