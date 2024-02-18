#ifndef __TEST_EFUSE_H
#define __TEST_EFUSE_H

fsm_rt_t test_efuse(void);
fsm_rt_t efuse_readback_repower(void);

void otp_struct_printf(const otp_t* efuse_data, char* str);

#endif
