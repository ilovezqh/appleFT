#include "bsp.h"
#include "__otp_type.h"
#include "__test_return_type.h"
#include "__chip_reg.h"
#include "test_interface.h"
#include "chip_access.h"
#include "chip_bus.h"

 

uint8_t write_data[16]  = {0};
uint8_t read_data[16]   = {0};
 

//以下数据与spec描述的值会不一致：主要考虑的是被OTP改写的区域，不受OTP控制的区域不会去改写相关bit数据
struct 
{
    union
    {
        struct
        {
            uint8_t  vref_trim_f     : 4;  //实际4bit
            uint8_t  iref_loss_f     : 1;
            uint8_t  iref_trim_f     : 3;  //实际4bit
        };
        uint8_t ref_ctl_trim;
    };
    union
    {
        struct
        {
            uint8_t  lpvdd_loss_f    : 1;
            uint8_t  lpvdd_trim_f    : 3;
            uint8_t  vbg_trim_f      : 2;
            uint8_t  reserved1       : 2;
        };
        uint8_t bgr_ctl_trim;
    };
    union
    {
        struct
        {
            uint8_t  rcl_loss_f      : 1;
            uint8_t  rcl_trim_f      : 6;
            uint8_t  reserved2       : 1;
        };
        uint8_t rcl_ctl_trim;
    };
    union
    {
        struct
        {
            uint8_t osc_loss_f : 1;
            uint8_t osc_trim_f : 7;
        };
        uint8_t osc_ctl_trim;
    };
    
}trim_ctl;

static void     set_apple_enable_otp(void);
static uint32_t set_apple_read_otp(void);
static uint8_t  set_apple_check_otp_cmd(void);

void trim_var_zero_init(void)
{
    memset(&trim_ctl,0,sizeof(trim_ctl));
    //trim_ctl.vbg_trim_f = BGR_FIX_VAL;
}

fsm_rt_t trim_var_load(void)
{
    fsm_rt_t ret;
    
    ret = i2c_read_register(I2C_DEV16_ADDR,&trim_ctl.ref_ctl_trim,REF_CTL_REG,1); if(ret != fsm_rt_cpl) return ret;
    ret = i2c_read_register(I2C_DEV16_ADDR,&trim_ctl.bgr_ctl_trim,BGR_CTL_REG,1); if(ret != fsm_rt_cpl) return ret;
    ret = i2c_read_register(I2C_DEV16_ADDR,&trim_ctl.rcl_ctl_trim,RCL_CTL_REG,1); if(ret != fsm_rt_cpl) return ret;
    ret = i2c_read_register(I2C_DEV16_ADDR,&trim_ctl.osc_ctl_trim,OSC_CTL_REG,1); if(ret != fsm_rt_cpl) return ret;
    
    //trim_ctl.vbg_trim_f = BGR_FIX_VAL; //写入默认的BGR值
    
    return fsm_rt_cpl;
}

#if 1
void set_apple_trim_value(uint8_t item,uint8_t value)
{
	switch(item)
	{
    case ITEM_IREF://REF_CTL_REG REF_CTL[7:4] actually[7:5] for OTP;
        trim_ctl.iref_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.ref_ctl_trim,REF_CTL_REG,1);
        break;
    case ITEM_VREF://REF_CTL_REG REF_CTL[3:0];
        trim_ctl.vref_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.ref_ctl_trim,REF_CTL_REG,1);			
        break;
    case ITEM_VBG://BGR_CTL_REG BGR_CTL[5:4];
        trim_ctl.vbg_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.bgr_ctl_trim,BGR_CTL_REG,1);			
        break;
    case ITEM_LPVDD://BGR_CTL_REG BGR_CTL[3:0] actually[3:1] for OTP;
        trim_ctl.lpvdd_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.bgr_ctl_trim,BGR_CTL_REG,1);				
        break;
    case ITEM_RCL://RCL_CTL_REG RCL_CTL[6:0] actually[6:1] for OTP;
        trim_ctl.rcl_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.rcl_ctl_trim,RCL_CTL_REG,1);			
        break;
    case ITEM_RCH://OSC_CTL_REG RCH_CTL[7:0] actually[7:1] for OTP;
        trim_ctl.osc_trim_f = value;
        i2c_write_register(I2C_DEV16_ADDR,&trim_ctl.osc_ctl_trim,OSC_CTL_REG,1);			
        break;
    default:
        break;
	}
}

#else
void set_apple_trim_value(uint8_t item,uint8_t value)
{
	uint8_t temp;
	switch(item)
	{
		case ITEM_IREF://REF_CTL_REG REF_CTL[7:4] actually[7:5] for OTP;
			temp = (value<<4)&0xf0;
			i2c_write_register(I2C_DEV16_ADDR,&temp,REF_CTL_REG,1);
			break;
		case ITEM_VREF://REF_CTL_REG REF_CTL[3:0];
			temp = value&0x0f;
			i2c_write_register(I2C_DEV16_ADDR,&temp,REF_CTL_REG,1);			
			break;
		case ITEM_VBG://BGR_CTL_REG BGR_CTL[5:4];
			temp = (value<<4)&0x30;
			i2c_write_register(I2C_DEV16_ADDR,&temp,BGR_CTL_REG,1);			
			break;
		case ITEM_LPVDD://BGR_CTL_REG BGR_CTL[3:0] actually[3:1] for OTP;
			temp = (value&0x0f) | (3<<4);
			i2c_write_register(I2C_DEV16_ADDR,&temp,BGR_CTL_REG,1);				
			break;
		case ITEM_RCL://RCL_CTL_REG RCL_CTL[6:0] actually[6:1] for OTP;
			temp = value&0x7f;
			i2c_write_register(I2C_DEV16_ADDR,&temp,RCL_CTL_REG,1);			
			break;
		case ITEM_RCH://OSC_CTL_REG RCH_CTL[7:0] actually[7:1] for OTP;
			temp = value;
			i2c_write_register(I2C_DEV16_ADDR,&temp,OSC_CTL_REG,1);			
			break;
		default:
			break;
	}
}
#endif


uint8_t  read_apple_trim_value(uint8_t item)
{
	uint8_t temp=0xff;
	switch(item)
	{
    case ITEM_IREF://REF_CTL_REG REF_CTL[7:4] actually[7:5] for OTP;
        i2c_read_register(I2C_DEV16_ADDR,&temp,REF_CTL_REG,1);
        temp = temp>>5;
        break;
    
    case ITEM_VREF://REF_CTL_REG REF_CTL[3:0];
        i2c_read_register(I2C_DEV16_ADDR,&temp,REF_CTL_REG,1);	
        temp = temp&0x0f;
        break;
    
    case ITEM_VBG://BGR_CTL_REG BGR_CTL[5:4];
        i2c_read_register(I2C_DEV16_ADDR,&temp,BGR_CTL_REG,1);	
        temp = (temp&0x30)>>4;
        break;
    
    case ITEM_LPVDD://BGR_CTL_REG BGR_CTL[3:0] actually[3:1] for OTP;
        i2c_read_register(I2C_DEV16_ADDR,&temp,BGR_CTL_REG,1);		
        temp = (temp&0x0f) >> 1;
        break;
    case ITEM_RCL://RCL_CTL_REG RCL_CTL[6:0] actually[6:1] for OTP;
        
        i2c_read_register(I2C_DEV16_ADDR,&temp,RCL_CTL_REG,1);		
        temp = (temp&0x7f) >> 1;
        break;
    
    case ITEM_RCH://OSC_CTL_REG RCH_CTL[7:0] actually[7:1] for OTP;
        i2c_read_register(I2C_DEV16_ADDR,&temp,OSC_CTL_REG,1);	
        temp = temp>>1;
        break;
    default:
        break;
	}
    return temp;
}

void set_apple_asu_enable(void)
{
	uint8_t temp;
	i2c_read_register(I2C_DEV16_ADDR,&temp,0xC10D,1);
	temp |= (1<<0);//ASU_CTL[1]=1	
	i2c_write_register(I2C_DEV16_ADDR,&temp,0xC10D,1);
}

fsm_rt_t set_apple_work_state(uint8_t state)
{
	uint8_t temp;
	temp = state;
	return i2c_write_register(I2C_DEV16_ADDR,&temp,0xC102,1);
    
} 

void power_off_apple(void)
{
    test_power.off();
}

void power_on_apple(void)
{
    test_power.on();
}

void set_apple_soft_work(void)
{
	uint8_t temp;
	i2c_read_register(I2C_DEV16_ADDR,&temp,BOOT_CTL_REG,1);
	temp &= ~(1<<5);//release CPU hold
	temp |= (1<<0); //soft_boot=1
	i2c_write_register(I2C_DEV16_ADDR,&temp,BOOT_CTL_REG,1);
}

void set_apple_debug_out(uint8_t item)
{
	uint8_t temp;
	switch(item)
	{
		case ITEM_IREF://0xC125 DBG_ANA[3]=1
			temp = (1<<3);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_ANA_REG,1);
			break;
		case ITEM_VREF://0xC125 DBG_ANA[2]=1
			temp = (1<<2);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_ANA_REG,1);			
			break;
		case ITEM_LPVDD://0xC125 DBG_ANA[0]=1
			temp = (1<<0);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_ANA_REG,1);				
			break;
		case ITEM_RCL://0xC120 DBG_CTL[4:0]=5,[7:5]=1; 0xC124 DBG_OE[3]=1;
			temp = (1<<5)|(5<<0);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_CTL_REG,1);	
			temp = (1<<4);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_OE_REG,1);			
			break;
		case ITEM_RCH://0xC120 DBG_CTL[4:0]=5,[7:5]=2; 0xC124 DBG_OE[3]=1;
			temp = (2<<5)|(5<<0);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_CTL_REG,1);	
			temp = (1<<4);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_OE_REG,1);			
			break;
		case ITEM_DBGCLK://0xC120 DBG_CTL[4:0]=6,[7:5]=1; //0xC124 DBG_OE[7]=1,[3]=1;
			temp = (1<<5)|(6<<0);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_CTL_REG,1);	
			temp = (1<<7)|(1<<4);
			i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_OE_REG,1);				
			break;
        case ITEM_PMU_DBG_OFF:
            temp = 0;
            i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_ANA_REG,1);		
            break;
        case ITEM_CMU_DBG_OFF:
            temp = 0;
            i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_CTL_REG,1);
            i2c_write_register(I2C_DEV16_ADDR,&temp,DBG_OE_REG,1);		
            break;
		default:
			break;
	}
}



/******************************************************************************
*功能：芯片重新开始工作
*注意：
			有三种途径：断上电（需重连）/PRST复位/reboot复位
			不同途径对应不同硬件电路及寄存器操作
			本示例对应reboot复位
*******************************************************************************/
void set_apple_rework(void)
{
	//PRST??
//	prst_apple();
	//reboot??	
	uint8_t temp;
	i2c_read_register(I2C_DEV16_ADDR,&temp,0xC100,1);
	temp |= (1<<0);//RMU_CTL re_boot=1	
	i2c_write_register(I2C_DEV16_ADDR,&temp,0xC100,1);
}
 


fsm_rt_t otp_write(otp_t* efuse_data)
{
    fsm_rt_t ret=fsm_rt_cpl;
    uint8_t sector;
    set_apple_enable_otp();
	for(sector=0;sector<4;sector++)
	{
		//OTP_KEY unlock
		write_data[0] = 0x5A;
		i2c_write_register(I2C_DEV16_ADDR, write_data, OTP_PSW_REG, 1); 
		//OTP_DATA
		write_data[0] = (uint8_t)((*efuse_data).all>>(sector*8));		
		i2c_write_register(I2C_DEV16_ADDR, write_data, OTP_BIT_REG, 1);
		//OTP_CMD=0X40|SECTOR|PROGRAM|RUN
		write_data[0] = 0x40|(sector<<4)|(2<<1)|(1<<0);
		i2c_write_register(I2C_DEV16_ADDR, write_data, OTP_CMD_REG, 1);
		//判断操作是否结束，延时或查询RUN状态
//		Delayms(1);//试验1msOK
		while(set_apple_check_otp_cmd()==1){;}
	}
    return ret;
}
 
fsm_rt_t otp_read(otp_t* efuse_data)
{
    fsm_rt_t ret=fsm_rt_cpl;
    
    set_apple_enable_otp();
    (*efuse_data).all = set_apple_read_otp(); 
    
    return ret;
}

/******************************************************************************
*功能：开/关OTP时钟
*注意：0xC101,CMU_CTL[5]=OTP_EN=OTP clock enable
*******************************************************************************/
static void set_apple_enable_otp(void)
{
	uint8_t temp;
	i2c_read_register(I2C_DEV16_ADDR,&temp,CMU_CTL_REG,1);
	temp |= (1<<5);
	i2c_write_register(I2C_DEV16_ADDR,&temp,CMU_CTL_REG,1);
}

void set_apple_disable_otp(void)
{
	uint8_t temp;
	i2c_read_register(I2C_DEV16_ADDR,&temp,CMU_CTL_REG,1);	  //CMU_CTL.OTP_EN
	temp &= ~(1<<5);
	i2c_write_register(I2C_DEV16_ADDR,&temp,CMU_CTL_REG,1);
}

/******************************************************************************
*功能：查询OTP操作状态
*注意：0=结束，1=未结束
*******************************************************************************/
static uint8_t set_apple_check_otp_cmd(void)
{
	uint8_t temp;
	//读OTP_CMD寄存器，操作结束RUN清0
	i2c_read_register(I2C_DEV16_ADDR,&temp,OTP_CMD_REG,1);
	if((temp&0x01) == 0x01)
		return 1;
	else
		return 0;
}

 
/******************************************************************************
*功能：读取OTP数据
*注意：
*******************************************************************************/
static uint32_t set_apple_read_otp(void)
{
	uint8_t sector;
	for(sector=0;sector<4;sector++)
	{
		//OTP_KEY unlock
		write_data[0] = 0x5A;
		i2c_write_register(I2C_DEV16_ADDR,write_data,OTP_PSW_REG,1); 
		//OTP_CMD=0X40|SECTOR|read|RUN
		write_data[0] = 0x40|(sector<<4)|(0<<1)|(1<<0);
		i2c_write_register(I2C_DEV16_ADDR, write_data,OTP_CMD_REG,1);
		//判断操作结束，延时或查询RUN状态
		//Delayms(1);//试验1msOK
		while(set_apple_check_otp_cmd()==1){;}
	}
	//读OTP_VALUE寄存器获取OTP写入内容
	i2c_read_register(I2C_DEV16_ADDR, read_data, OTP_VALUE0_REG, 4);
	return (((uint32_t)read_data[3]<<24)|((uint32_t)read_data[2]<<16)|((uint32_t)read_data[1]<<8)|(read_data[0]));
}

