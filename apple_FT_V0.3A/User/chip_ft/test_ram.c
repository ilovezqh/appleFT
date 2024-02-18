/* 
用于将SD卡中的pattern，通过ASU灌入RAM进行运行
将eclipse 特殊工程生成在RAM上运行的bin，将bin放入SD卡，再边读取bin文件边通过ASU写入addr=0开始，
写完后 RMU_CPUVEC： BOOT_ADDR    = 0x00
                    RE_BOOT     |= 0x02;
*/
#include "__test_common.h"
#include "usb_bsp_msc.h"    /* usb???? */
#include "diskio.h"		    /* Declarations of low level disk I/O functions */
#include "spt5113c_firmware.h"

extern FIL     trim_pattern_file;
extern FIL     perf_pattern_file;

//#define temp_printf  //debug
#define temp_printf(...)

static fsm_rt_t ram_code_send(void);

//  本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用
fsm_rt_t  test_load_into_ram(void)
{
//	DIR         DirInf;
    static enum
    {
        START = 0,
        RAM_CODE_SEND,
        WAIT_SPL_RUN,
    }s_tState = START;
    
	static fsm_rt_t ret;

    timer_creat();
    
    switch(s_tState)
    {
    case START:
        apple_prst();
        set_apple_debug_out(ITEM_PMU_DBG_OFF);
        set_apple_debug_out(ITEM_CMU_DBG_OFF);
        s_tState++;
        //break;
        
    case RAM_CODE_SEND:
        {
            ret = ram_code_send();
            if(ret != fsm_rt_cpl)
            {
                goto fail;
            }
            logging("SUCCESS RAM load\r\n");
            s_tState = WAIT_SPL_RUN;
            timer_start(10);//
        }
        break;
    case WAIT_SPL_RUN:
        if(is_time_up())
        {
            s_tState = START;
            return fsm_rt_cpl;
        }
        break;
    }
    return fsm_rt_on_going;
    
    fail:
    s_tState = START;
    return ret;
}



////pattern 9
static void write_apple_code_zone(void)
{	
	i2c_write_register(I2C_DEV16_ADDR, (uint8_t *)spt5113c_firmware, 0, FIRMWARE_SIZE);
}

static void write_apple_crc_zone(void)
{
    uint8_t wr[4];
	//运行代码长度
	wr[0] = ((FIRMWARE_SIZE-1)>>8)&0xff;	
	wr[1] = (FIRMWARE_SIZE-1)&0xff;
	//运行代码CRC
    wr[2] = (CRC16_CHECK_CODE>>8)&0xff;
    wr[3] = CRC16_CHECK_CODE&0xff;
	//写入CRC区
	i2c_write_register(I2C_DEV16_ADDR,wr,ROM_LENH_REG,4);	
}

/******************************************************************************
*功能：				
*注意：
芯片上电连接，进入UPGRADE状态；
写入自测程序.h文件到芯片Code区；自测程序.h文件由附件提供；
设置寄存器计算CRC校验码；
比对.h文件自带CRC校验码与计算所得CRC校验值，正确则通过测试。
*******************************************************************************/
static fsm_rt_t ram_code_send(void)
{
 
    uint8_t temp;	
    uint8_t rd[2];
    uint16_t apple_cal_crc16;
    
    //运行代码写入code区
    write_apple_code_zone();
    //运行代码大小及CRC写入CRC对应寄存器
    write_apple_crc_zone();	
    //设置apple启动CRC计算,0xC136 ROM_CTL[0]=1 
    temp = 0x01; 
    i2c_write_register(I2C_DEV16_ADDR,&temp,ROM_CTL_REG,1);
    //等待CRC计算完成
    delay_ms(10); 	
    //读apple计算出来的CRC,0xC134/0xC135=high byte/low byte
    i2c_read_register(I2C_DEV16_ADDR,rd,ROM_CALH_REG,2); 
    apple_cal_crc16 = ((uint16_t)rd[0]<<8) + rd[1];	
    //比对确定CRC与计算CRC,确保写入内容正确
    if(apple_cal_crc16 != CRC16_CHECK_CODE)	return fsm_rt_err_Read_bin_crc_err;	
 
    return fsm_rt_cpl;
}
