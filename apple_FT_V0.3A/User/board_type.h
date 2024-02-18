#ifndef __BORAD_TYPE_H
#define __BORAD_TYPE_H

typedef enum
{
    board_v20 = 20,
    board_v21,
}board_type_e;

extern board_type_e  g_eBoardType;

void board_type_init(void);
int32_t board_self_test(void);
void PrintfLogo(void);

#endif
