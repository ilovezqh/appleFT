#ifndef __DUT_H
#define __DUT_H

#include "chip_type.h"

#include "run_mode_info.h"

typedef struct 
{
    spt511xx_t*     device;
    uint32_t        asu_baud;    
}dut_t;  

extern dut_t dut;
extern run_mode_t* run_mode_handle;

int32_t dut_param_init(void);

#endif
