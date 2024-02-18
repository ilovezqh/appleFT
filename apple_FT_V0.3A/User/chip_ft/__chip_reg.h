#ifndef __TEST_CHIP_REG_H
#define __TEST_CHIP_REG_H

#define I2C_DEV8_ADDR       0x13 //£®8Œªµÿ÷∑0x80£
#define APPLE_DEV_ADDR      0x60 //ASU i2c addr
#define I2C_DEV16_ADDR      0x60 //£®8Œªµÿ÷∑0xC0£©
#define APPLE_CHIP_CODE     0x53//

//2byte addr access register
#define CHIP_CODE_REG       0xC000

#define CMU_CTL_REG         0xC101
#define BOOT_CTL_REG        0xC103

#define DBG_ANA_REG         0xC125

#define ROM_LENH_REG        0xC130
#define ROM_LENL_REG        0xC131
#define ROM_CRCH_REG        0xC132
#define ROM_CRCL_REG        0xC133
#define ROM_CALH_REG        0xC134
#define ROM_CALL_REG        0xC135
#define ROM_CTL_REG         0xC136

#define PRST_CTL_REG        0xC21E
#define DBG_CTL_REG         0xC120
#define DBG_OE_REG          0xC124
#define BGR_CTL_REG         0xC106
#define REF_CTL_REG         0xC108
#define RCL_CTL_REG         0xC109
#define OSC_CTL_REG         0xC10A

#define GLOBAL3_REG         0xc303

#define OTP_PSW_REG         0xC400
#define OTP_BIT_REG         0xC401
#define OTP_CMD_REG         0xC402
#define OTP_VALUE0_REG      0xC404

#endif
