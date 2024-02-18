#include "__test_common.h"
#include "__test_return_type.h"
#include "__test_return_info.h"
 
struct 
{
    uint32_t head;
    uint32_t addr;
    uint32_t size;
}bin_cpu_map;

static uint16_t sys_halt_cnt=0;
static uint32_t txt_hex_read(char* s);
 
uint32_t manufacture_bin_crc;                                                   /*生产的bin，用于烧录器烧录,此CRC32计算方式与上位机spv1x_odt_v1.2.0一样,*/
uint32_t trim_pattern_crc;                                                      /* 此CRC32计算方式：http://www.ip33.com/crc.html ，参数模型：CRC-32/MPEG-2 */
uint32_t perf_pattern_crc;                                               /* 同上 */


//死机处理 ；死机狗触发看门狗复位；复位后直接报NG
void sys_halt_process(void)
{
    sys_halt_cnt++;
    led_fail(ON);
    BEEP_FAIL();
    msg_to_robot(PG_NG);
    logging("sys_halt_process\r\n");
}

void test_start_info_show(void)
{
    logging("#### %s START:chip %d test (%d)####\r\n",board_id_name,chip_counter_total+1,sys_halt_cnt);  /* 不从0开始显示，显示数值与结束后总数保持一致 */    
    display_fill(0, 0, BSP_LCD_WIDTH, BSP_LCD_HEIGHT, BLACK);     
}

//计算crc的时候，buf内的有效数据改成8的整数倍；目的：计算bin的校验码，与上位机计算结果保持一致
void crc_buf_size_8_mutiple(uint8_t* buf, uint32_t* pBW)
{
    if((*pBW)%8 !=0)
    {
        uint32_t __bw8 = (*pBW+7)/8 * 8;
        memset(&buf[*pBW],0,__bw8 - *pBW);
        *pBW = __bw8;
    }
}
 

FATFS   fs_flash;
FIL     trim_pattern_file;
FIL     perf_pattern_file;
FIL     customer_file;


static FRESULT     result;

/* 打开存储介质为spi flash的文件系统 */
int32_t spi_flash_file_sys_mount(void)
{
    /*-1- 挂载文件系统*/
    result = f_mount(&fs_flash, SPI_FLASH_VOLUME, 0);
    if(result)
    {
        LCD_SHOW_NG("FLASH mount error : %d \n",result);
        return -1; 
    }
    return 0;
}
 

 


 

/* 打开客户烧录文件 */
int32_t customer_file_open(void)
{
    FRESULT     result;
    FIL         config_file;

        /* 打开文件 */
    result = f_open(&config_file, CONFIG_TXT_PATH, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
        LCD_SHOW_NG("Don't Find File : %s\r",CONFIG_TXT_PATH);
		return -1;
	}
 
	/* 关闭文件*/
	result = f_close(&config_file);
    if (result !=  FR_OK)
    {
        return -1;
    }        
    return 0;
}

/* 打开客户烧录文件 */
int32_t board_id_file_open(char *str)
{
    FIL     board_id_file;
    FRESULT     result;
    char temp_buf[24];
    uint32_t bw;
    
    result = f_open(&board_id_file, BOARD_ID_PATH, FA_READ);
    if (result !=  FR_OK)
	{
        LCD_SHOW_WARN("WARN:%s\n",BOARD_ID_PATH);
        return -1;
	}
    
    /* 读取文件 */
	result = f_read(&board_id_file, &temp_buf, sizeof(temp_buf), &bw);
	if (bw > 0)
	{
		temp_buf[bw] = 0;
		printf("\r\n total_byte:%d 文件内容 : \n%s\n",bw ,temp_buf);
	}
    memcpy(str,temp_buf,strlen(temp_buf)+1);
	/* 关闭文件*/
	f_close(&board_id_file);
 
    return 0;
}


uint32_t config_code_get(void)
{
    uint32_t last_config = txt_hex_read(CONFIG_TXT_PATH);    //读两次
    uint32_t cur_config  = txt_hex_read(CONFIG_TXT_PATH);
    
    if(last_config != cur_config)
    {
        LCD_SHOW_NG("Error Config1:%08X  Config2:%08X\r",last_config,cur_config);
        return 0;
    }
    else
    {
        if(cur_config == 0)
        {
            LCD_SHOW_WARN("Config:default\r");
        }
        else
        {
            LCD_SHOW_WARN("Config:%08X\r",cur_config);
        }
    } 
    return cur_config;
}


void test_over_info_show(fsm_rt_t state)
{
    binning_send(state);
    
    if(state != fsm_rt_cpl) 
    {
        BEEP_FAIL();
        led_fail(ON);
        //msg_to_robot(PG_NG);

        uint8_t num_error = sizeof(err_info)/sizeof(err_info_t);
        for(uint8_t i=0; i<num_error; i++ )
        {
            if(state == err_info[i].errorCode) logging("ERROR Code=%d: %s\r\n",err_info[i].errorCode, err_info[i].errorMessage); 
        }
        chip_counter_fail++;
    }
    else
    {
        led_pass(ON);
 
        BEEP_PASS();
        //msg_to_robot(PG_OK);
        chip_counter_pass++;
    }
    chip_counter_total++;
    logging("FINISH sum:%d pass:%d fail:%d\r\n",chip_counter_total,chip_counter_pass,chip_counter_fail);
    logger_sd_card_write(); //日志写入sd卡
}

//读取文本内容（CRC32校验码），返回十六进制数字
static uint32_t txt_hex_read(char* s)
{
    uint32_t ret;
    uint8_t temp_buf[16];
    uint32_t bw;
    static FIL config_file;
    /* 打开文件 */
	FRESULT result = f_open(&config_file, s, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
        LCD_SHOW_NG("Don't Find File : %s\r",s);
		return 0;
	}
    
	/* 读取文件 */
	result = f_read(&config_file, &temp_buf, sizeof(temp_buf), &bw);
	if (bw > 0)
	{
		temp_buf[bw] = 0;
		printf("\r\n%s,byte:%d 文件内容 : \r\n%s\r\n",s,bw ,temp_buf);
	}
 
	/* 关闭文件*/
	f_close(&config_file);
    ret = strtoul((char*)temp_buf,NULL,16);
    
    return ret;
}



/* pass指示灯控制 */
void led_pass(on_off_t state)
{
    if(state == ON)
    {
        bsp_LedOn(LED_GREEN_KEY);
        bsp_LedOn(LED_GREEN);
    }
    else
    {
        bsp_LedOff(LED_GREEN_KEY);
        bsp_LedOff(LED_GREEN);
    }
}

/* fail指示灯控制 */
void led_fail(on_off_t state)
{
    if(state == ON)
    {
        bsp_LedOn(LED_RED_KEY);
        bsp_LedOn(LED_RED);
    }
    else
    {
        bsp_LedOff(LED_RED_KEY);
        bsp_LedOff(LED_RED);
    }
}


