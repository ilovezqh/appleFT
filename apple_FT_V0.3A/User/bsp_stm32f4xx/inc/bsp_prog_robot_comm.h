#ifndef __BSP_ROBOT_UI_H
#define __BSP_ROBOT_UI_H

//回应机台
typedef enum
{
    PG_IDLE = 0,
    PG_BUSY,
    PG_OK,
    PG_NG,
    
    DUT_BUSY,
    DUT_OK,
    
    DUT_NG2,
    DUT_NG3,
    DUT_NG4,
    DUT_NG5,
}robot_ctrl_e;

void robot_com_init(void);
void msg_to_robot(robot_ctrl_e ack_status);

#endif
