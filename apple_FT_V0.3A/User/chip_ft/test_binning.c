

#include "__test_common.h"

#define NG_BIN_MAX    4

/* 各类bin，选取测试结果分bin，bin中没有填入的都放在NG5 */
static fsm_rt_t bin_ok[] =
{
   fsm_rt_cpl,  
};

static fsm_rt_t bin_ng2[] =
{
    fsm_rt_err_load_ram_ack_timeout,
    //fsm_rt_err_tested_chip_qc,
};

static fsm_rt_t bin_ng3[] =
{
    fsm_rt_err_handshark,
};

static fsm_rt_t bin_ng4[] =
{
    fsm_rt_err_flash_id,
};

//static fsm_rt_t bin_ng5[] = //默认其他的NG类都放在此bin
//{
//    //fsm_rt_err_bist_cache,
//};

//static fsm_rt_t* bin_type_box[NG_BIN_MAX ] = { bin_ng1,bin_ng2,bin_ng3,bin_ng4 };

//根据输入的ng类型，输出对应的bin_ng号
fsm_rt_t* ng_bin_get(fsm_rt_t state)
{
    for(uint8_t i=0; i<ARRY_SIZE(bin_ok); i++)
    {
        if(state == bin_ok[i])  return bin_ok;
    }
    
    for(uint8_t i=0; i<ARRY_SIZE(bin_ng2); i++)
    {
        if(state == bin_ng2[i])  return bin_ng2;
    }
    
    for(uint8_t i=0; i<ARRY_SIZE(bin_ng3); i++)
    {
        if(state == bin_ng3[i])  return bin_ng3;
    }
    
    for(uint8_t i=0; i<ARRY_SIZE(bin_ng4); i++)
    {
        if(state == bin_ng4[i])  return bin_ng4;
    }
    
//    for(uint8_t i=0; i<ARRY_SIZE(bin_ng5); i++)
//    {
//        if(state == bin_ng5[i])  return bin_ng5;
//    }
    return NULL;
}


void binning_send(fsm_rt_t ng)
{
    fsm_rt_t* pBinning = ng_bin_get(ng);
    
    if(pBinning == bin_ok)
    {
        msg_to_robot(DUT_OK);
    }
    
    else if(pBinning == bin_ng2) 
    {
        msg_to_robot(DUT_NG2);
    }
    else if(pBinning == bin_ng3) 
    {
        msg_to_robot(DUT_NG3);
    }
    else if(pBinning == bin_ng4) 
    {
        msg_to_robot(DUT_NG4);
    }
    else  
    {
        msg_to_robot(DUT_NG5);
    }
    
//    for(uint8_t i=0; i<ARRY_SIZE(bin_type_box); i++)
//    {
//        if(pBinning == bin_type_box[i])
//        {
//            
//        }
//    }
}

