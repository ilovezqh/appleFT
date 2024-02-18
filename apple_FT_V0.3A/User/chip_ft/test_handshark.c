#include "__test_common.h"
 
//static fsm_rt_t vmic_on(void);
//fsm_rt_t vmic_off(void);
    
fsm_rt_t test_handshark(void)
{
    
    uint8_t  chip_code;
    static enum
    {
        START = 0,
        GET_CHIP_CODE,
        READ_EFUSE,
    } s_tState = START;
    
    fsm_rt_t ret;
 
    switch(s_tState)
    {
    case START:
        s_tState++;
        //break;
    
    case GET_CHIP_CODE:
        {
            //apple_prst();
            //delay_ms(100);
            
            ret = apple_read_addr16(APPLE_DEV_ADDR, &chip_code, CHIP_CODE_REG,1); //读取chip  code：0x53
            if(ret != fsm_rt_cpl) 
            {
                ret = fsm_rt_err_handshark;
                goto fail;
            }
            if(chip_code != APPLE_CHIP_CODE)
            {
                ret = fsm_rt_err_chip_id;
                goto fail;
            }
            uint8_t prst_ctl_value;
            ret = apple_read_addr16(APPLE_DEV_ADDR, &prst_ctl_value, PRST_CTL_REG,1); //读取chip  code：0x53
            debug("prst_ctl_value:%X\r\n",prst_ctl_value);
            
            ret = trim_var_load();                                              //读出所有涉及trim的寄存器值的值
            if( ret != fsm_rt_cpl)
            {
                ret = fsm_rt_err_trim_var_load;
                goto fail;
            }
            
            s_tState = READ_EFUSE;
        }
        break;
        
    case READ_EFUSE:
        {
            otp_read(&otp_read_data);
            logging(" [ Read ]read=%08X\r\n",otp_read_data.all);
            otp_struct_printf(&otp_read_data, "[power on]otp read");
            uint8_t temp;
            temp = read_apple_trim_value(ITEM_IREF);
            debug("info [IREF ] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.iref_f, reg_default.iref_f, 
                                                                    otp_read_data.iref_f^reg_default.iref_f,  temp,temp );
            temp = read_apple_trim_value(ITEM_VBG);
            debug("info [BGR  ] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.bgr_f, reg_default.bgr_f, 
                                                                 otp_read_data.bgr_f^reg_default.bgr_f, temp,temp );
            temp = read_apple_trim_value(ITEM_VREF);
            debug("info [VREF ] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.vref_f, reg_default.vref_f, 
                                                                 otp_read_data.vref_f^reg_default.vref_f, temp,temp );
            
            temp = read_apple_trim_value(ITEM_LPVDD);
            debug("info [LPVDD] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.lpvdd_f, reg_default.lpvdd_f, 
                                                                 otp_read_data.lpvdd_f^reg_default.lpvdd_f, temp,temp );
            temp = read_apple_trim_value(ITEM_RCL);
            debug("info [32K  ] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.rcl_f, reg_default.rcl_f, 
                                                                 otp_read_data.rcl_f^reg_default.rcl_f, temp,temp );
            temp = read_apple_trim_value(ITEM_RCH);
            debug("info [80M  ] otp:%2X xor %2X = %2X; regActual:%2X(%2d)\r\n",otp_read_data.rch_f, reg_default.rch_f, 
                                                                 otp_read_data.rch_f^reg_default.rch_f, temp,temp );
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
