#ifndef __CHIP_ACCESS_H
#define __CHIP_ACCESS_H

#define i2c_write_register          apple_write_addr16
#define i2c_read_register           apple_read_addr16
#define i2c_read_register8          apple_read_addr8

 enum
{
	ITEM_IREF 		    =1,
	ITEM_VREF 		    =2,
	ITEM_VBG			=3,
	ITEM_LPVDD 		    =4,
	ITEM_RCL 			=5,
	ITEM_RCH            =6,
	ITEM_DBGCLK		    =61,
	ITEM_OTP 			=7,
	ITEM_CURRENT 	    =8,
	ITEM_DOWNLOAD       =9,
	ITEM_AFE 			=10,
    ITEM_PMU_DBG_OFF    = 11,
    ITEM_CMU_DBG_OFF    = 12,
};

enum
{
	ACTIVE =0,
	SLOW =1,
	STANDBY =2,
	SLEEP =3
};

extern void trim_var_zero_init(void);
extern fsm_rt_t trim_var_load(void);

extern uint8_t  read_apple_trim_value(uint8_t item);
extern void set_apple_trim_value(uint8_t item,uint8_t value);
extern void set_apple_debug_out(uint8_t item);
extern void set_apple_asu_enable(void);
extern void set_apple_soft_work(void);
extern void set_apple_rework(void);
extern fsm_rt_t set_apple_work_state(uint8_t state);
extern void power_off_apple(void);
extern void power_on_apple(void);


extern fsm_rt_t otp_write(otp_t* efuse_data);
extern fsm_rt_t otp_read(otp_t* efuse_data);
extern void     set_apple_disable_otp(void);
#endif
