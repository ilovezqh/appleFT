#ifndef __TEST_RETURN_INFO_H
#define __TEST_RETURN_INFO_H

typedef struct {
    fsm_rt_t    errorCode;
    const char* errorMessage;
} err_info_t;

#define ERROR_INFO(x) {x,#x}  /* #x,会在预处理阶段，将x变量直接转成字符串，此种用法只能用在宏当中 */

err_info_t err_info[]=
{
    ERROR_INFO(fsm_rt_err_custommer_conflict),
    ERROR_INFO(fsm_rt_err_key_NoAck),
    ERROR_INFO(fsm_rt_err_asu_NoAck),
    ERROR_INFO(fsm_rt_err_vmic_enable),
    ERROR_INFO(fsm_rt_err_key_match),
    ERROR_INFO(fsm_rt_err_flash_ft_area_write_timeout),
    ERROR_INFO(fsm_rt_err_efuse_readback),
    ERROR_INFO(fsm_rt_err_core_diff),
    ERROR_INFO(fsm_rt_err_flash_id),
    ERROR_INFO(fsm_rt_err_gpadc),
    ERROR_INFO(fsm_rt_err_siovcc),
    ERROR_INFO(fsm_rt_err_handshark),
    ERROR_INFO(fsm_rt_err_wait_flash_timeout),
    ERROR_INFO(fsm_rt_err_ram_pattern_no_select),
    ERROR_INFO(fsm_rt_err_clk_shake),
    ERROR_INFO(fsm_rt_err_clk_shake),
    ERROR_INFO(fsm_rt_err_osc_pmu_twice_avg),
    ERROR_INFO(fsm_rt_err_tested_chip_qc),
    ERROR_INFO(fsm_rt_err_bin_only_4k),
    ERROR_INFO(fsm_rt_err_crc_flash_readback),
    ERROR_INFO(fsm_rt_err_kiwi_crc_err),
    ERROR_INFO(fsm_rt_err_Read_bin_crc_err),
    ERROR_INFO(fsm_rt_err_QC_osc_core),
    ERROR_INFO(fsm_rt_err_QC_osc_audio),
    ERROR_INFO(fsm_rt_err_QC_osc_pmu),
    ERROR_INFO(fsm_rt_err_QC_LPV_volt),
    ERROR_INFO(fsm_rt_err_QC_BGR_volt),
    ERROR_INFO(fsm_rt_err_efuse_1_to_0),
    ERROR_INFO(fsm_rt_err_load_ram_ack_timeout),
    ERROR_INFO(fsm_rt_err_final_tested),
    ERROR_INFO(fsm_rt_err_otp_dirty_data),
    ERROR_INFO(fsm_rt_err_otp_pure_data),
    ERROR_INFO(fsm_rt_err_efuse_otp_cmp),
    ERROR_INFO(fsm_rt_err_io_s2),
    ERROR_INFO(fsm_rt_err_io_test),
    ERROR_INFO(fsm_rt_err_bist_cache),
    ERROR_INFO(fsm_rt_err_bist_asu),
    ERROR_INFO(fsm_rt_err_flash_read_check),
    ERROR_INFO(fsm_rt_err_flash_read_head_2k),
    ERROR_INFO(fsm_rt_err_spl_loss_track),
    ERROR_INFO(fsm_rt_err_hibernate_ack),
    ERROR_INFO(fsm_rt_err_no_load_current),
    ERROR_INFO(fsm_rt_err_lpv),
    ERROR_INFO(fsm_rt_err_pmu_asu),
    ERROR_INFO(fsm_rt_err_spl_flash_check),
    ERROR_INFO(fsm_rt_err_spl_flash_oversize),
    ERROR_INFO(fsm_rt_err_spl_head_crc32),
    ERROR_INFO(fsm_rt_err_spl_body_crc32),
    ERROR_INFO(fsm_rt_err_spl_ack_timeout),
    ERROR_INFO(fsm_rt_err_chip_id),
    ERROR_INFO(fsm_rt_err_sum_check),
    ERROR_INFO(fsm_rt_err_ack_timeout),
    ERROR_INFO(fsm_rt_err_brom_ack),
    ERROR_INFO(fsm_rt_err_uart),
    ERROR_INFO(fsm_rt_err_i2c),
    ERROR_INFO(fsm_rt_err_bist_usb_ram),
    ERROR_INFO(fsm_rt_err_lradc),
    ERROR_INFO(fsm_rt_err_pattern_startup_timeout),
    ERROR_INFO(fsm_rt_err_bist_cache_ram),
    ERROR_INFO(fsm_rt_err_global0_set),
    ERROR_INFO(fsm_rt_err_bist_udsp_ram),
    ERROR_INFO(fsm_rt_err_exit_standby),
    ERROR_INFO(fsm_rt_err_standby_current),
    ERROR_INFO(fsm_rt_err_enter_standby),
    ERROR_INFO(fsm_rt_err_bist_timeout),
    ERROR_INFO(fsm_rt_err_cali_cap_get),
    ERROR_INFO(fsm_rt_err_iref),
    ERROR_INFO( fsm_rt_err_vref),
    ERROR_INFO( fsm_rt_err_spu_timeout ),
    ERROR_INFO( fsm_rt_err_spu ),
    ERROR_INFO( fsm_rt_err_trim_var_load ),
    ERROR_INFO( fsm_rt_err_lrc ),
    ERROR_INFO( fsm_rt_err_osc_audio ),
    ERROR_INFO( fsm_rt_err_cali_cap ),
    ERROR_INFO( fsm_rt_err_pattern_run_timeout ),
    ERROR_INFO( fsm_rt_err_efuse_check       ),
    ERROR_INFO( fsm_rt_err_efuse_dump_all    ),
    ERROR_INFO( fsm_rt_err_efuse_program_all ),
    ERROR_INFO( fsm_rt_err_efuse_init        ),
    ERROR_INFO( fsm_rt_err_touch_channel             ),                 //flash写完扇区校验失败或者超过写范围等因素
    ERROR_INFO( fsm_rt_err_asu               ),
    ERROR_INFO( fsm_rt_err_no_bin            ),
    ERROR_INFO( fsm_rt_err_fatfs             ),
    ERROR_INFO( fsm_rt_func_type_err         ),
    ERROR_INFO( fsm_rt_efuse_err             ),
    ERROR_INFO( fsm_rt_xor_err               ),    //! xor_check error
    ERROR_INFO( fsm_rt_err                   ),    //!< fsm error, error code can be get from other interface
};

#endif
