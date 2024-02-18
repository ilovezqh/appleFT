#ifndef __BSP_CH440R_EX_H
#define __BSP_CH440R_EX_H


#define CH440R_EX_CTRL_ALL_DETACH    1<<0
#define CH440R_EX_CTRL_I2C           1<<1
#define CH440R_EX_CTRL_IO_S1         1<<2
#define CH440R_EX_CTRL_UART          1<<3
#define CH440R_EX_CTRL_IO_S2         1<<4
#define CH440R_EX_CTRL_UC_DETACH     1<<5
#define CH440R_EX_CTRL_IOTEST_DETACH 1<<6



void bsp_ch440r_init_ex(void);
void bsp_ch440r_ctrl_ex(uint8_t ctrl);

#endif
