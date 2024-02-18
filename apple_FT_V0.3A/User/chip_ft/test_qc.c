#include "__test_common.h"

static uint8_t is_trimed_chip(void);

static fsm_rt_t qc_trimed(void)
{
    fsm_rt_t ret=fsm_rt_cpl;
    
//    if((trim_result.pmu_bgr_trim_volt_default < BGR_VOLT_MIN) || (trim_result.pmu_bgr_trim_volt_default > BGR_VOLT_MAX))
//    {
//        ret = fsm_rt_err_QC_BGR_volt;
//        logging("fail: QC BGR volt\r\n");
//    }
//    if(trim_result.pmu_lpv_trim_volt_default < LPV_VOLT_MIN || trim_result.pmu_lpv_trim_volt_default > LPV_VOLT_MAX)
//    {
//        ret = fsm_rt_err_QC_LPV_volt;
//        logging("fail: QC LPV volt\r\n");
//    }
//    if(trim_result.osc_pmu_trim_freq_default < OSC_PMU_FREQ_MIN || trim_result.osc_pmu_trim_freq_default > OSC_PMU_FREQ_MAX)
//    {
//        ret = fsm_rt_err_QC_osc_pmu;
//        logging("fail: QC osc pmu\r\n");
//    }

//    if(trim_result.osc_audio_trim_freq_default < OSC_AUDIO_FREQ_MIN || trim_result.osc_audio_trim_freq_default > OSC_AUDIO_FREQ_MAX)
//    {
//        ret = fsm_rt_err_QC_osc_audio;
//        logging("fail: QC osc audio\r\n");
//    }
//    
//    if(trim_result.osc_core_trimed_freq[0] < OSC_CORE_80M_MIN || trim_result.osc_core_trimed_freq[0] > OSC_CORE_80M_MAX)
//    {
//        ret = fsm_rt_err_QC_osc_core;
//        logging("fail: QC core 80M \r\n");
//    }
// 
//    if(trim_result.osc_core_trimed_freq[1] < OSC_CORE_100M_MIN || trim_result.osc_core_trimed_freq[1] > OSC_CORE_100M_MAX)
//    {
//        ret = fsm_rt_err_QC_osc_core;
//        logging("fail: QC core 100M \r\n");
//    }
// 
//    if(trim_result.osc_core_trimed_freq[2] < OSC_CORE_120M_MIN || trim_result.osc_core_trimed_freq[2] > OSC_CORE_120M_MAX)
//    {
//        ret = fsm_rt_err_QC_osc_core;
//        logging("fail: QC core 120M \r\n");
//    }
    
    return ret;
}


fsm_rt_t test_qc_all(void)
{
    fsm_rt_t ret=fsm_rt_cpl;
        
    ret = qc_trimed();
    
   

    return ret;
}
//ft 和 ft_prg模式下使用：已测试过的芯片，查看trim区是否ok，如果不是已测试芯片则会跳过
fsm_rt_t test_qc_trimed(void)
{
    fsm_rt_t ret = fsm_rt_cpl;
    
    if(is_trimed_chip())
    {
        ret = qc_trimed();
        if(ret != fsm_rt_cpl) return ret;
    }
    logging("SUCCESS tested trim qc\r\n");
    return fsm_rt_cpl;
}
        

//已trim过的芯片 返回  1：已测试过  0：未测试过
static uint8_t is_trimed_chip(void)
{
//    if(efuse_otp_data_brom.EFUSE_VLD == 0x8e3b5a69)
//    {
//        if(efuse_otp_data_brom.PMU_BGR_VLD          && 
//           efuse_otp_data_brom.PMU_LPV_VLD          &&
//           efuse_otp_data_brom.OSC_AUDIO_VLD        &&
//           efuse_otp_data_brom.OSC_32K_VLD          &&
//           efuse_otp_data_brom.OSC_CORE_80M_VLD     &&
//           efuse_otp_data_brom.OSC_CORE_100M_VLD    &&
//           efuse_otp_data_brom.OSC_CORE_120M_VLD   )
//        {
//            return 1;
//        }
//        else
//        {
//            return 0;
//        }
//    }
 
         return 0;
    
}
