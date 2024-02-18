#ifndef __EE_DATA_H
#define __EE_DATA_H

typedef struct
{
    uint32_t head;
    uint32_t logger_cnt;
    uint32_t current_offset;
    uint32_t unuse[3];
    uint32_t tail;
    uint32_t crc;  
}ee_data_t;

extern ee_data_t ee_data;

int32_t ee_data_init(void);
int32_t ee_data_save(void);

#endif
