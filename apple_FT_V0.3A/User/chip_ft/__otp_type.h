#ifndef OTP_TYPE_H
#define OTP_TYPE_H

/* EFUSE区数据结构 256bit */
typedef struct 
{
    union
    {
        struct
        {
            uint32_t i2c_irq_pu_en_f        :   1;
            uint32_t lpvdd_f                :   3;
            uint32_t vref_f                 :   4;
            uint32_t bgr_f                  :   2;
            uint32_t rcl_f                  :   6;
            uint32_t prst_ctl_pe_f          :   1;
            uint32_t iref_f                 :   3;
            uint32_t prst_ctl_pol_f         :   2;
            uint32_t prst_ctl_en_f          :   1;
            uint32_t prst_ctl_vs_f          :   1;
            uint32_t asu_dev_addr_f         :   1;
            uint32_t rch_f                  :   7;
        };
        uint32_t all; //32 bit
    };
}otp_t;

#endif
