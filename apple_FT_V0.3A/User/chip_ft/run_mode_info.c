#include "run_mode_info.h"
#include "__test_common.h"

 
//ft 各测试项 函数指针数组  
static fsm_rt_t (*ft_list[])() =
{
    test_handshark,
    test_pmu,
    test_cmu,
    test_efuse,
    test_load_into_ram,
    test_spl,
    //test_qc_trimed,
    
    //test_flash_ft,

    //test_reboot_to_brom,
    test_current,
};

//ft_prg 各测试项 函数指针数组  
static fsm_rt_t (*ft_prg_list[])() =
{
    test_handshark,
    test_pmu,
    test_cmu,
    test_load_into_ram,
    test_spl,
    test_qc_trimed,
    test_efuse,
//    test_flash_programmer,

    //test_reboot_to_brom,
    test_current,
};

//erase 擦除
static fsm_rt_t (*erase_list[])() =
{
    test_handshark,
    test_load_into_ram,
//    test_flash_erase,
};

//prg 烧录
static fsm_rt_t (*prg_list[])() =
{
    test_handshark,
    test_load_into_ram,
    test_efuse,
//    test_flash_programmer,
};

//QC
static fsm_rt_t (*qc_list[])() =
{
    test_handshark,
    test_pmu,
    test_cmu,
    test_load_into_ram,
    test_spl,
//    test_flash_id,
    //test_efuse,
    test_qc_all,
    
    //test_reboot_to_brom,
    test_current,
};

//QC+flash
static fsm_rt_t (*qc_flash_list[])() =
{
    test_handshark,
    test_pmu,
    test_cmu,
    test_load_into_ram,
    test_spl,
    //test_efuse,  //会引入配置烧录
    test_qc_all,
//    test_flash_qc,
    
    //test_reboot_to_brom,
    test_current,
};

//--------------------------- 各模式对象实例化 ---------------------------------
run_mode_t run_mode_ft =
{
    .name = "FT",
    .test_list                  = ft_list,
    .test_list_size             = ARRY_SIZE(ft_list),
};

run_mode_t run_mode_ft_prg =
{
    .name = "FT(Programmer)",
    .test_list                  = ft_prg_list,
    .test_list_size             = ARRY_SIZE(ft_prg_list),
};

run_mode_t run_mode_prg =
{
    .name = "Programmer",
    .test_list                  = prg_list,
    .test_list_size             = ARRY_SIZE(prg_list),
};


run_mode_t run_mode_qc =
{
    .name = "QC",
    .test_list                  = qc_list,
    .test_list_size             = ARRY_SIZE(qc_list),
};

run_mode_t run_mode_qc_flash =
{
    .name = "QC(Flash)",
    .test_list                  = qc_flash_list,
    .test_list_size             = ARRY_SIZE(qc_flash_list),
};

run_mode_t run_mode_erase =
{
    .name = "Erase",
    .test_list                  = erase_list,
    .test_list_size             = ARRY_SIZE(erase_list),
};
