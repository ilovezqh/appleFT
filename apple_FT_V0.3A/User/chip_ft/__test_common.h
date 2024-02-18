/* ft模块内部使用 */
#ifndef __TEST_COMMON_H
#define __TEST_COMMON_H

#include "bsp.h"
#include "debug.h"
#include "lwrb.h"
#include "lcd.h"
#include "__chip_reg.h"
//#include "platform.h"
#include "__test_return_type.h"
#include "test_interface.h"
#include "__otp_type.h"
#include "__test_config.h"
#include "ff.h"			    /* FatFS */
#include "crc32.h"
 
#include "log.h"
////////////////////////////////
#include "chip_bus.h"
#include "chip_access.h"
#include "test_handshark.h"
#include "test_cmu.h"
#include "test_pmu.h"
#include "test_ram.h"
#include "test_spl.h"
#include "test_otp.h"
//#include "test_flash.h"
#include "test_qc.h"
#include "test_current.h"
//#include "test_reboot_to_brom.h"
#include "test_binning.h"
#include "dut.h"
#include "ee_data.h"
 


#define delay_us                bsp_DelayUS
#define delay_ms                bsp_DelayMS
#define BEEP_PASS()             BEEP_Start(10, 10, 1);	/* 鸣叫100ms，停10ms，  1次 */
#define BEEP_FAIL()             BEEP_Start(5, 10, 3);	/* 鸣叫50ms， 停100ms， 3次 */	 

//#define APPLE_PRST()
#define apple_prst()                  \
{                                   \
    socket_prst_ctl(prst_state_lo); \
    delay_ms(10);                   \
    socket_prst_ctl(prst_state_hi); \
    delay_ms(10);                   \
}

 



typedef struct
{
    union  /* trim fail 标志 ,置位 为fail    */
    {
        struct
        {
            uint32_t    pass_efuse_read         :1;
            uint32_t    pass_pmu_bgr_trim       :1;
            uint32_t    pass_pmu_lpv_trim       :1;
            uint32_t    pass_osc_audio_trim     :1;
            uint32_t    pass_osc_pmu_trim       :1;
            uint32_t    pass_osc_core_trim      :1;
            uint32_t    pass_efuse_write        :1;
            uint32_t    pass_flash_program      :1;
            uint32_t    pass_reservd            :24;
        };
        uint32_t trim_pass;   
    };
    
    union  /* trim fail 标志 ,置位 为fail    */
    {
        struct
        {
            uint32_t    fail_efuse_read         :1;
            uint32_t    fail_pmu_lpv_trim       :1;
            uint32_t    fail_pmu_bgr_trim       :1;
            uint32_t    fail_osc_audio_trim     :1;
            uint32_t    fail_osc_pmu_trim       :1;
            uint32_t    fail_osc_core_trim      :1;
            uint32_t    fail_efuse_write        :1;
            uint32_t    fail_flash_program      :1;
            uint32_t    fail_reservd            :24;
        };
        uint32_t trim_fail;   
    };

//    union
//    {
//        uint32_t pmu_trim_reg_val;
//        struct
//        {
//            uint32_t reserved0       : 18;
//            uint32_t osc_pmu_trim    : 5;
//            uint32_t reserved1       : 3;
//            uint32_t lpv_trim        : 1;
//            uint32_t bgr_trim        : 4;
//        };
//    };
    
    uint8_t     pmu_bgr_trim_val;
    uint8_t     pmu_bgr_trim_power_on;
    uint8_t     pmu_bgr_trim_default;
    uint32_t    pmu_bgr_trim_volt;
    uint32_t    pmu_bgr_trim_volt_default;
    
    uint8_t     pmu_lpv_trim_val;
    uint8_t     pmu_lpv_trim_power_on;
    uint8_t     pmu_lpv_trim_default;
    uint32_t    pmu_lpv_trim_volt;
    uint32_t    pmu_lpv_trim_volt_default;
    
    uint8_t     osc_audio_trim_val;
    uint8_t     osc_audio_trim_power_on;
    uint8_t     osc_audio_trim_default;
    uint32_t    osc_audio_trim_freq;
    uint32_t    osc_audio_trim_freq_default;
    
    uint8_t     osc_pmu_trim_val;
    uint8_t     osc_pmu_trim_power_on;
    uint8_t     osc_pmu_trim_default;
    uint32_t    osc_pmu_trim_freq;
    uint32_t    osc_pmu_trim_freq_default;
    
    uint8_t     osc_core_trim_val[3];  //60 70 80 90 100 110 120
    uint32_t    osc_core_trim_freq[3];
    uint8_t     osc_core_trimed_val[3];
    uint32_t    osc_core_trimed_freq[3];
    uint8_t     osc_core_trim_default;
    //uint8_t     osc_core_trim_power_on; //不需要，osc core的trim值efuse保存区域是后定义的，不会上电自动加载
    uint32_t    osc_core_trim_freq_default;
    
    int32_t standby_current;
    uint32_t hibernate_siovcc_volt;

}trim_result_t;


typedef struct
{
    uint8_t irefVal;
    uint8_t bgrVal;
    uint8_t vrefVal;
    uint8_t lpvddVal;
    uint8_t rclVal;
    uint8_t rchVal;
}trim_val_t;


typedef struct
{
    uint32_t irefCurrent;
    uint32_t bgrVolt;
    uint32_t vrefVolt;
    uint32_t lpvddVolt;
    uint32_t rclFreq;
    uint32_t rchFreq;
}trim_measure_t;

 

typedef enum
{
    OFF = 0,
    ON  = !OFF,
}on_off_t;

extern trim_result_t    trim_result;
 
extern otp_t    otp_read_data;  
extern otp_t    trim_reg_data;  
extern const otp_t    reg_default;
extern otp_t   custommer_data;
 
extern trim_measure_t  trim_measure;
extern trim_measure_t  default_measure;

 
extern uint32_t user_config;
extern char board_id_name[];
 
 
int32_t customer_file_open(void);
int32_t spi_flash_file_sys_mount(void);
uint32_t config_code_get(void);
fsm_rt_t trim_result_into_efuse(void);
 
int32_t board_id_file_open(char *str);

 
 
void led_pass(on_off_t state);
void led_fail(on_off_t state);
void test_start_info_show(void);
void test_over_info_show(fsm_rt_t state);
 
 
 


    
#endif
