/* 
  pattern 测试需求：
  1.根据不同型号不同的测试内容
  2.pattern中根据型号和运行硬件环境自行测试
  3.测试结果通过查询帧来获取
  
  数据结构：
  test_en:使能测试
  test_query:查询是否完毕，完毕则读取对应测试结果
  
  流程：主板设置TEST_EN,然后轮询pattern status（超时问题），有done bit置位则读取相关信息，判断是否NG，
  pattern运行完毕后错误再统计（显示所有错误测试项）
  
*/
#include "__test_common.h"

static fsm_rt_t spu_test(void);

fsm_rt_t test_spl(void)
{
    static enum
    {
        START = 0,
        TEST_QUERY,
    } s_tState = START;
    fsm_rt_t ret;
    
    switch(s_tState)
    {
    case START:
        //return fsm_rt_cpl;
        s_tState++;
        //break;

    case TEST_QUERY: //查询测试结果
        {
            ret = spu_test();
            if(ret != fsm_rt_on_going)
            {
                socket_touch_off();
                s_tState = START;
                return ret;
            }
        }
        break;
    }
    return fsm_rt_on_going;
 
}


/******************************************************************************
*功能：				
*注意：
*过程：
测试机使能芯片端自测程序运行；
在芯片端程序运行后，发出触摸控制信号；
在确定时间内监测UART_TX端口，如该时间段内收到UART数据，再启动DEV8I2C读取第二路触摸状态；
比较设计触摸状态及收到的两路触摸状态，匹配则测试通过。
//芯片端程序运行自测程序，通过UART_TX输出触摸通道数量，响应DEV8I2C读取命令发送触摸通道分布。
*******************************************************************************/

static fsm_rt_t spu_test(void)
{	
    static enum
    {
        START = 0,
        IS_SPU_RDY,
        TOUCH_ON,
        SPU_TEST,
        CALI_CAP_ANALYSE,
    } s_tState = START;
    
    #define ALL_CH_TOUCHED         0x1E0F // 0001 1110  0000 1111
    #define SPU_READY_MAGIC_NUM    0X5A   
    #define CALI_CAP_OFFSET_MAX       4
    #define CH_NUM_TOTAL              8
    #define SNS_NUM_TOTAL             13
    static const int8_t cali_cap_standard[SNS_NUM_TOTAL] =  //socket板测试环境下认为的校准电容的标准值
    { // 0    1   2   3   4     5    6    7   8    9   10   11  12   
        -91, -90,-90,-91,-108,-109,-108,-108,-108,-90,-88, -89, -92
    };
    static uint8_t recv_ch_num      =0;
    static uint8_t last_recv_num    =0;
    static uint8_t uart_pass        =0;
    static uint8_t spu_pass         =0;
    static uint8_t cali_cap_pass    =0;
    static uint8_t has_cali_cap_flg =0;
    static uint16_t pressing_ch_bit =0;
    static int8_t cali_cap[13];
    static int8_t cali_cap_offset[13];
    timer_creat();
    fsm_rt_t ret;
    
    switch(s_tState)
    {
    case START:
        recv_ch_num      = 0;
        last_recv_num    = 0;
        uart_pass        = 0;
        spu_pass         = 0;
        cali_cap_pass    = 0;
        pressing_ch_bit  = 0;
        has_cali_cap_flg = 0;
        memset(cali_cap,        0,sizeof(cali_cap));
        memset(cali_cap_offset, 0,sizeof(cali_cap_offset));
        set_apple_soft_work();
        comClearRxFifo(COM2);
        s_tState++;
        //break;
    
    case IS_SPU_RDY:
        {
            uint8_t num;
            i2c_read_register(I2C_DEV16_ADDR,&num,GLOBAL3_REG,1);
            if(num == SPU_READY_MAGIC_NUM)
            {
                timer_start(20); //SPU初始化完毕后再延时20ms 触发touch
                s_tState = TOUCH_ON;
            }
        }
        break;
        
    case TOUCH_ON:
        if(is_time_up())
        {
            socket_touch_on();
            timer_start(500);   //启动测试超时
            s_tState = SPU_TEST;
        }
        break;
         
    case SPU_TEST:
        if(comGetChar(COM2, &recv_ch_num))
        {
 
            if(recv_ch_num != last_recv_num)
            {
                uart_pass = 1;
                last_recv_num = recv_ch_num;
                uint8_t rd[2];
                i2c_read_register8(I2C_DEV8_ADDR, rd,0,2);                      //通过DEV8I2C读状态		
                pressing_ch_bit = ((uint16_t)rd[0]<<8) | rd[1];                 //获取各触发通道对应的bit置位
 
                if((recv_ch_num == CH_NUM_TOTAL) && (pressing_ch_bit == ALL_CH_TOUCHED))           
                {
                    spu_pass = 1;
                    s_tState = CALI_CAP_ANALYSE;
                }
                if(has_cali_cap_flg == 0)
                {
                    has_cali_cap_flg = 1;
                    uint8_t cali_cap_err=0;
                    for(uint8_t i=0; i<SNS_NUM_TOTAL; i++)
                    {
                        uint8_t temp;
                        i2c_read_register(I2C_DEV16_ADDR,&temp,0xC821+i*4,1);  //CHnCALL 电容校准值寄存器，只需要读取低BYTE，高BYTE作他用
                        cali_cap[i] = temp;	
                        
                        int8_t offset =  cali_cap[i] -  cali_cap_standard[i];  //与标准值的偏差
                        cali_cap_offset[i] = offset;
                        
                        if( abs((int)offset)  > CALI_CAP_OFFSET_MAX)
                        {
                            cali_cap_err = 1;
                        }
                    }
                    if(cali_cap_err == 0) cali_cap_pass = 1;  //无错误 cali_cap_pass
                }                 //-128 +127   (-110 +110)
            }
        }
 
        if(is_time_up())
        {
            logging("fail SPU(%d) uart(%d) cali_cap(%d):%04X,%d,[0]%d [1]%d [2]%d [3]%d [4]%d [5]%d [6]%d [7]%d [8]%d [9]%d [10]%d [11]%d [12]%d\r\n",
            spu_pass,uart_pass,cali_cap_pass,pressing_ch_bit,recv_ch_num,
            cali_cap_offset[0],cali_cap_offset[1],cali_cap_offset[2],cali_cap_offset[3],cali_cap_offset[4],cali_cap_offset[5],cali_cap_offset[6],
            cali_cap_offset[7],cali_cap_offset[8],cali_cap_offset[9],cali_cap_offset[10],cali_cap_offset[11],cali_cap_offset[12]);
            
            ret = fsm_rt_err_spu_timeout;
            goto fail;
        }
        break;
        
    case CALI_CAP_ANALYSE:
         
        if (cali_cap_pass && spu_pass && uart_pass) 
        {
            logging("SUCCESS SPU(%d) uart(%d) cali_cap(%d):%04X,%d,[0]%d [1]%d [2]%d [3]%d [4]%d [5]%d [6]%d [7]%d [8]%d [9]%d [10]%d [11]%d [12]%d\r\n",
            spu_pass,uart_pass,cali_cap_pass,pressing_ch_bit,recv_ch_num,
            cali_cap_offset[0],cali_cap_offset[1],cali_cap_offset[2],cali_cap_offset[3],cali_cap_offset[4],cali_cap_offset[5],cali_cap_offset[6],
            cali_cap_offset[7],cali_cap_offset[8],cali_cap_offset[9],cali_cap_offset[10],cali_cap_offset[11],cali_cap_offset[12]);
            s_tState = START;
            return fsm_rt_cpl;
        }
        else
        {
            logging("fail SPU(%d) uart(%d) cali_cap(%d):%04X,%d,[0]%d [1]%d [2]%d [3]%d [4]%d [5]%d [6]%d [7]%d [8]%d [9]%d [10]%d [11]%d [12]%d\r\n",
            spu_pass,uart_pass,cali_cap_pass,pressing_ch_bit,recv_ch_num,
            cali_cap_offset[0],cali_cap_offset[1],cali_cap_offset[2],cali_cap_offset[3],cali_cap_offset[4],cali_cap_offset[5],cali_cap_offset[6],
            cali_cap_offset[7],cali_cap_offset[8],cali_cap_offset[9],cali_cap_offset[10],cali_cap_offset[11],cali_cap_offset[12]);

            ret = fsm_rt_err_spu;
            goto fail;
        }
        //break;
    }
 
    return fsm_rt_on_going;
    
    fail:
    s_tState = START;
    return ret;
}

