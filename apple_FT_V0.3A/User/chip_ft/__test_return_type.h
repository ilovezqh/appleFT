#ifndef __TEST_RETURN_TYPE_H
#define __TEST_RETURN_TYPE_H

//! \name finit state machine return value
//! @{
typedef enum
{
    fsm_rt_err_custommer_conflict = -87,
    fsm_rt_err_key_NoAck = - 86,
    fsm_rt_err_asu_NoAck = -85,
    fsm_rt_err_vmic_enable = -84,
    fsm_rt_err_key_match = -83,
    fsm_rt_err_flash_ft_area_write_timeout = -82,
    fsm_rt_err_efuse_readback = -81,
    fsm_rt_err_core_diff = -80,
    fsm_rt_err_flash_id = -79,
    fsm_rt_err_gpadc = -78,
    fsm_rt_err_siovcc = -77,
    fsm_rt_err_handshark = -76,
    fsm_rt_err_wait_flash_timeout = -75,
    fsm_rt_err_ram_pattern_no_select = -74,
    fsm_rt_err_clk_xx = -73,
    fsm_rt_err_clk_shake = -72,
    fsm_rt_err_osc_pmu_twice_avg = -71,
    fsm_rt_err_tested_chip_qc = -70,
    fsm_rt_err_bin_only_4k = -69,
    fsm_rt_err_crc_flash_readback = -68,
    fsm_rt_err_kiwi_crc_err  = -67,
    fsm_rt_err_Read_bin_crc_err = -66,
    fsm_rt_err_QC_osc_core = -65,
    fsm_rt_err_QC_osc_audio = -64,
    fsm_rt_err_QC_osc_pmu  = -63,
    fsm_rt_err_QC_LPV_volt = -62,
    fsm_rt_err_QC_BGR_volt = -61,
    fsm_rt_err_efuse_1_to_0 = -60,
    fsm_rt_err_load_ram_ack_timeout = -59,
    fsm_rt_err_final_tested = -58,
    fsm_rt_err_otp_pure_data  = -57,
    fsm_rt_err_otp_dirty_data = -56,
    fsm_rt_err_efuse_otp_cmp = -55,
    fsm_rt_err_io_s2          = -54,
    fsm_rt_err_io_test          = -53,
    fsm_rt_err_bist_cache     = -52,
    fsm_rt_err_bist_asu       = -51,
    fsm_rt_err_flash_read_check       = -50,
    fsm_rt_err_flash_read_head_2k       = -49,
    fsm_rt_err_spl_loss_track = -48,
    fsm_rt_err_hibernate_ack  = -47,
    fsm_rt_err_no_load_current = -46,
    fsm_rt_err_lpv            = -45,
    fsm_rt_err_pmu_asu        = -44,
    fsm_rt_err_spl_flash_check = -43,
    fsm_rt_err_spl_flash_oversize = -42,
    fsm_rt_err_spl_head_crc32 = -41,
    fsm_rt_err_spl_body_crc32 = -40,
    fsm_rt_err_spl_ack_timeout  = -39,
    fsm_rt_err_chip_id  = -38,
    fsm_rt_err_sum_check = -37,
    fsm_rt_err_ack_timeout = -36,
    fsm_rt_err_brom_ack = -35,
    fsm_rt_err_uart = -34,
    fsm_rt_err_i2c = -33,
    fsm_rt_err_bist_usb_ram = -32,
    fsm_rt_err_lradc = -31,
    fsm_rt_err_pattern_startup_timeout = -30,
    fsm_rt_err_bist_cache_ram = -29,
    fsm_rt_err_global0_set = -28,
    fsm_rt_err_bist_udsp_ram = -27,
    fsm_rt_err_exit_standby = -26,
    fsm_rt_err_standby_current = -25,
    fsm_rt_err_enter_standby = -24,
    fsm_rt_err_bist_timeout = -23,
    fsm_rt_err_cali_cap_get = -22,
    fsm_rt_err_iref       = -21,
    fsm_rt_err_vref       = -20,
    fsm_rt_err_spu_timeout  = -19,
    fsm_rt_err_spu       = -18,
    fsm_rt_err_trim_var_load   = -17,
    fsm_rt_err_lrc   = -16,
    fsm_rt_err_osc_audio   = -15,
    fsm_rt_err_cali_cap = -14,
    fsm_rt_err_pattern_run_timeout = -13,
    fsm_rt_err_efuse_check    = -12,
    fsm_rt_err_efuse_dump_all = -11,
    fsm_rt_err_efuse_program_all = -10,
    fsm_rt_err_efuse_init   = -9,
    fsm_rt_err_touch_channel        = -8,                 //flash写完扇区校验失败或者超过写范围等因素
    fsm_rt_err_asu          = -7,
    fsm_rt_err_no_bin       = -6,
    fsm_rt_err_fatfs        = -5,
    fsm_rt_func_type_err    = -4,
    fsm_rt_efuse_err        = -3,
    fsm_rt_xor_err          = -2,    //! xor_check error
    fsm_rt_err              = -1,    //!< fsm error, error code can be get from other interface
    fsm_rt_cpl              = 0,     //!< fsm complete
    fsm_rt_on_going         = 1,     //!< fsm on-going
    fsm_rt_wait_for_obj     = 2,     //!< fsm wait for object
    fsm_rt_asyn             = 3,     //!< fsm asynchronose mode, you can check it later.
} fsm_rt_t;




#endif
