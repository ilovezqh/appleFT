#ifndef __XOR_DEFAULT_H
#define __XOR_DEFAULT_H

#include "__otp_type.h"

const otp_t reg_default =
{
    .i2c_irq_pu_en_f       =    0,
    .lpvdd_f                =   (8>>1), 
    .vref_f                 =   8,
    .bgr_f                  =   2,
    .rcl_f                  =   (0x2f>>1), //47
    .prst_ctl_pe_f    =   0,
    .iref_f                 =   (8>>1),
    .prst_ctl_pol_f             =   0,
    .prst_ctl_en_f              =   0,
    .prst_ctl_vs_f    =   0,
    .asu_dev_addr_f         =   0, 
    .rch_f                  =   (0x58>>1), //88
};

#endif
