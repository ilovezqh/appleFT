#include "bsp.h"
#include "log.h"
#include "ff.h"			    /* FatFS */
#include "lcd.h"
#include "__test_config.h"
#include "ee_data.h"
/*
1.单个日志的file size 大于 1000,000（1M）则进行分割；新建一个logger_xxxx.txt
2.上电后默认新建一个新的logger_xxxx.txt; xxxx每上电一次 新建一个新的；
3.日志打印函数 logger();
*/
//#define logger(fmt,arg...)   printf(fmt,##arg)


#define FILE_OVER_SIZE       500000UL      /* 单一日志文件的最大尺寸（ 约 500 PCS芯片） ;一颗芯片的ft日志量大概是 1000*/



static FIL         logger_file;
static FRESULT     retSD;
static uint32_t    bw;
static FATFS       fs;
//static DIR         DirInf;
static char        log_file_name[30];                                           //logger文件名字不能超过这个长度
 
static uint8_t   is_logger_enable = 0;//日志使能标志
 
uint32_t chip_counter_pass;
uint32_t chip_counter_fail;
uint32_t chip_counter_total;



int32_t sd_file_sys_mount(void)
{
    /*-1- 挂载文件系统*/
    retSD = f_mount(&fs, SD_VOLUME, 0);
    if(retSD)
    {
        LCD_SHOW_WARN(" mount error : %d \r\n",retSD);
        return -1; 
    }
    return 0;
}

int32_t logger_txt_creat(void)
{
//    if(ee_data_load(&ee_data))
//    {
//        LCD_SHOW_NG("eeprom error!");
//        return -1;
//    }
    sprintf(log_file_name,LOGGER_TXT_FORMAT,ee_data.logger_cnt);      //根据eeprom保存的数据生成ft_logger_xxxx.txt文件名
    
    //retSD = f_open(&logger_file, log_file_name, FA_OPEN_ALWAYS);
    //if(retSD)
    //{
        /*-2-创建新的文件并写入数据*/
        retSD = f_open(&logger_file, log_file_name, FA_OPEN_ALWAYS | FA_WRITE);		//打开文件，权限包括创建、写（如果没有该文件，会创建该文件）
        if(retSD)															    //返回值不为0（出现问题）
        {
            //debug(" creat %s file error : %d\r\n",log_file_name,retSD);	    //打印问题代码
            LCD_SHOW_WARN("Disable SD card logger:%d\r\n",retSD);
            is_logger_enable = 0;
            return -1;
        }
        else
        {
            //printf(" file size: %d \r\n",(int)logger_file.fsize);
            is_logger_enable = 1;
            LCD_SHOW_PASS(" creat %s file success!\r\n",log_file_name);
        }
    //}
    return 0;
}

 


void test_logger_clear(void)
{
    FRESULT res; 
    uint32_t fail_cnt=0;
    
    chip_counter_fail   = 0;
    chip_counter_pass   = 0;
    chip_counter_total  = 0;
    ee_data.logger_cnt  = 0;
    ee_data_save();
    f_close(&logger_file); //关掉当前日志文件
    
    for(uint32_t i = 0; i<10000; i++)  //删除所有日志
    {
        sprintf(log_file_name,LOGGER_TXT_FORMAT,i);
        res = f_unlink(log_file_name);
        if (res == FR_OK) 
        {
            fail_cnt = 0;
            printf("%s 删除成功！\r\n",log_file_name);
        } 
        else 
        {
            fail_cnt++;
            if(fail_cnt == 100)
            {
                break;  //连续100次都删除失败，后续文件认为就没有了，直接退出，否则会耗费很多时间
            }
            //printf("文件删除失败，错误代码：%d\r\n", res); 
        }
    }
    uint8_t retry =0;
    do
    {
        if(logger_txt_creat()) 
        {
            retry++;
            LCD_SHOW_WARN("try %d logger txt creat fail!\n",retry); //eeprom数据不对，被初始化的提示
            bsp_DelayMS(3); 
        }    
        else
        {
            break;
        }
    }while(retry < 3);
}

#if 0
static void sd_file_sys_mount(void)
{
    sprintf(log_file_name,"ft_logger_%04d.txt",logger_num);  //
    
    printf("\r\n ****** 文件系统 ******\r\n\r\n");
 
    /*-1- 挂载文件系统*/
	retSD = f_mount(&fs, DISK_VOLUME, 0);
    if(retSD)
    {
        logger(" mount error : %d \r\n",retSD);
    }
    else
        printf(" mount sucess!!! \r\n");
    
    /*-2-创建新的文件并写入数据*/
    retSD = f_open(&logger_file, log_file_name, FA_OPEN_ALWAYS | FA_WRITE);		//打开文件，权限包括创建、写（如果没有该文件，会创建该文件）
    if(retSD)															//返回值不为0（出现问题）
        printf(" open file error : %d\r\n",retSD);						//打印问题代码
    else
        printf(" open file sucess!!! \r\n");
	
//      /*-3- 偏移指针到末尾处*/	   
//	printf(" file size: %d \r\n",(int)logger_file.fsize);
//	f_lseek(&logger_file,logger_file.fsize);
	
//    /*-4- 在txt文件尾续写数据*/	
// 
//	retSD = f_write(&logger_file, str_zt, sizeof(str_zt), (void *)&byteswritten);	//在文件内写入wtext内的内容	

//	if(retSD)															//返回值不为0（出现问题）
//        printf(" write file error : %d\r\n",retSD);						//打印问题代码
//    else
//    {
//        printf(" write file sucess!!! \r\n");
//        printf(" write Data : %s\r\n",str_zt);							//打印写入的内容
//    }
//     
//    /*-5- 关闭txt文件*/
//    retSD = f_close(&logger_file);												//关闭该文件
//    if(retSD)															//返回值不为0（出现问题）
//        printf(" close error : %d\r\n",retSD);							//打印问题代码
//    else
//        printf(" close sucess!!! \r\n");
}
#endif


void logger_start(void)
{
    
//    sprintf(log_file_name,"logger/ft_logger_%04d.txt",ee_data.logger_num);  //根据eeprom保存的数据生成ft_logger_xxxx.txt文件名
//    //debug("logger FILE:  %s\r\n",log_file_name);
//    
//    retSD = f_open(&logger_file, log_file_name, FA_OPEN_ALWAYS);
//    if(retSD)  //没有文件
//    {
//         /*-2-创建新的文件并写入数据*/
//        retSD = f_open(&logger_file, log_file_name, FA_OPEN_ALWAYS | FA_WRITE);		//打开文件，权限包括创建、写（如果没有该文件，会创建该文件）
//        if(retSD)															//返回值不为0（出现问题）
//        {
//            debug(" open file error : %d\r\n",retSD);						//打印问题代码
//        }
//        else
//        {
//            //printf(" file size: %d \r\n",(int)logger_file.fsize);
//        }
//    }
    
    
    
}

#if 0
void logging(char *str)
{
     /*-3- 偏移指针到末尾处*/	   
	//printf(" file size: %d \r\n",(int)logger_file.fsize);
	f_lseek(&logger_file,logger_file.fsize);
    /*-4- 在txt文件尾续写数据*/
 
	retSD = f_write(&logger_file, buf, strlen(str), (void *)&bw);	//在文件内写入wtext内的内容	

	if(retSD)															//返回值不为0（出现问题）
    {
        debug(" write file error : %d\r\n",retSD);						//打印问题代码
    }
    else
    {
        debug("%s",str);							//打印写入的内容
    }
}
#else

void logger(const char *fmt,...)
{
    char buf[256];
    va_list args;
     
    
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    //display_show_string(buf, WHITE, BLACK);
    logger_start();                /* 开始 */
     /*-3- 偏移指针到末尾处*/	   
	//printf(" file size: %d \r\n",(int)logger_file.fsize);
	f_lseek(&logger_file,logger_file.fsize);
    /*-4- 在txt文件尾续写数据*/
     
	retSD = f_write(&logger_file, buf, strlen(buf), (void *)&bw);	//在文件内写入wtext内的内容	

	if(retSD)															//返回值不为0（出现问题）
    {
        debug(" write file error : %d\r\n",retSD);						//打印问题代码
    }
    else
    {
        debug("%s",buf);							//打印写入的内容
    }
    
    logger_over();
}
#endif

//chip ft 日志结束；会判断是否大于文件是否超过了 限制
void logger_over(void)
{
    
    if(logger_file.fsize > FILE_OVER_SIZE)
    {
        ee_data.logger_cnt++;
        ee_data_save();
        /*-5- 关闭txt文件*/
        retSD = f_close(&logger_file);												//关闭该文件
        if(retSD)															//返回值不为0（出现问题）
        {
            debug(" close error : %d\r\n",retSD);							//打印问题代码
        }
        bsp_DelayMS(3); 
        uint8_t retry =0;
        do
        {
            if(logger_txt_creat()) 
            {
                retry++;
                LCD_SHOW_WARN("try %d logger txt creat fail!\n",retry); //eeprom数据不对，被初始化的提示
                bsp_DelayMS(3); 
            }    
            else
            {
                break;
            }
        }while(retry < 3);
        
    }
}


char* EM_LOGLevelGet(const int level)
{
	if (level == LOG_DEBUG) {
		return "DEBUG";
	}
	else if (level == LOG_INFO) {
		return "INFO";
	}
	else if (level == LOG_WARN) {
		return "WARN";
	}
	else if (level == LOG_ERROR) {
		return "ERROR";
	}
	return "UNLNOW";
}
void EM_LOG(const int level,const char* fun, const int line ,const char* fmt, ...)
{
#ifdef OPEN_LOG
	va_list arg;
	va_start(arg, fmt);
	char buf[50] = { 0 };
	vsnprintf(buf, sizeof(buf), fmt, arg);
	va_end(arg);
	if (level >= LOG_LEVEL)
		
		printf("[%-5s] [%-20s%4d] %s \r\n", EM_LOGLevelGet(level), fun, line, buf);
#endif
}

#define LOG_BUF_SIZE  4096
static char logger_buf[LOG_BUF_SIZE];
static uint32_t high_water_mark=0;

int32_t logger_buffer_write(const char *fmt,...)
{
    char buf[256];  /* 单次写入不要超过256 */
    va_list args;
 
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    debug("%s",buf);							//打印写入的内容
    uint32_t len = strlen(buf);
    if(high_water_mark + len > LOG_BUF_SIZE) // 超过logger buffer尺寸报错
    {
        debug("ERROR logger_buf oversize!");
        return -1;
    }
    memcpy( &logger_buf[high_water_mark], buf, len);  //将写入的内容转存到logger_buf
    high_water_mark += len;
    
    return 0;
}

int32_t logger_sd_card_write(void)
{
    int32_t ret=0;
    logger_start();                /* 开始 */
     /*-3- 偏移指针到末尾处*/	
    if(is_logger_enable)
    {
        printf(" file size: %d \r\n",(int)logger_file.fsize);
        f_lseek(&logger_file,logger_file.fsize);
        /*-4- 在txt文件尾续写数据*/
        //debug("filesize=%d\r\n",(uint32_t)logger_file.fsize);  
        retSD = f_write(&logger_file, logger_buf, high_water_mark, (void *)&bw);	//在文件内写入wtext内的内容	
        //debug("bw=%d\r\n",bw); //bw就是写入的数 
        if(retSD)															//返回值不为0（出现问题）
        {
            debug(" write file error : %d\r\n",retSD);						//打印问题代码
            ret = -1;
        }
        f_sync(&logger_file);
        logger_over();
    }        
	
    //debug("water_mark=%d\r\n",high_water_mark);
    display_show_logger(logger_buf, BLACK);
    memset(logger_buf,0,LOG_BUF_SIZE);
    high_water_mark = 0;
    return ret;
}

 

