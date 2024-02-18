#ifndef __RUN_MODE_INFO_H
#define __RUN_MODE_INFO_H

#include "stdint.h"
#include "__test_return_type.h"

typedef struct
{
    char name[16];
    fsm_rt_t (**test_list)();       //指向函数指针数组的指针
    uint8_t   test_list_size;
}run_mode_t;

extern run_mode_t run_mode_ft;
extern run_mode_t run_mode_ft_prg;
extern run_mode_t run_mode_prg;
extern run_mode_t run_mode_qc;
extern run_mode_t run_mode_qc_flash;
extern run_mode_t run_mode_erase;
 
#endif
