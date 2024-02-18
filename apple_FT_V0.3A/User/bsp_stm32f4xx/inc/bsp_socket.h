#ifndef __BSP_SOCKET_H
#define __BSP_SOCKET_H

enum prst_state_e
{
    prst_state_float = 0,
    prst_state_hi,
    prst_state_lo,
};

void bsp_kiwi_ft_init(void);
void bsp_socket_init(void);
void socket_touch_on(void);
void socket_touch_off(void);
void socket_i_self(void);
void socket_i_chip(void);
void socket_ivref_iref(void);
void socket_ivref_vref(void);

void bsp_kiwi_onoff_init(void);
void bsp_kiwi_onoff_deinit(void);
void bsp_kiwi_ft_relay_on(void);
void bsp_kiwi_ft_relay_off(void);
void bsp_kiwi_onoff_hi(void);
void bsp_kiwi_onoff_lo(void);
void bsp_socket_ldo3v3_init(void);
void bsp_socket_ldo3v3_enable(void);
void bsp_socket_ldo3v3_disable(void);
void bsp_prst_ctl_deinit(void);

void socket_prst_ctl(enum  prst_state_e state);
void socket_asu_pu_on(void);
void socket_asu_pu_off(void);
#endif
