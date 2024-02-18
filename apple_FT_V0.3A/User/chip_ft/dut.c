#include "__test_config.h"
#include "dut.h"
#include "chip_info.h"
#include "run_mode_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"


dut_t dut;
run_mode_t* run_mode_handle = NULL;


int32_t dut_param_init(void)
{
    /*  
        run_mode_ft 
        run_mode_ft_prg 
        run_mode_prg 
        run_mode_qc 
        run_mode_qc_flash 
        run_mode_erase 
    */
    run_mode_handle         = &run_mode_ft;                                     //工作模式 
    dut.device              = &SPT5113C;                                        //芯片型号
    dut.asu_baud            = 115200;
 
    return 0;
}

