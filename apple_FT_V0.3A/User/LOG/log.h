
#ifndef __LOG_H_
#define __LOG_H_
#include <stdarg.h>
#define OPEN_LOG 1
#define LOG_LEVEL LOG_DEBUG

typedef enum
{
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
}E_LOGLEVEL;
void EM_LOG(const int level, const char* fun, const int line, const char* fmt, ...);
#define EMLOG(level,fmt,...) EM_LOG(level,__FUNCTION__,__LINE__,fmt, ##__VA_ARGS__)

extern uint32_t chip_counter_pass;
extern uint32_t chip_counter_fail;
extern uint32_t chip_counter_total;

int32_t sd_file_sys_mount(void);
int32_t logger_txt_creat(void);


//void logger_start(void);
void logger(const char *fmt,...);

int32_t logger_buffer_write(const char *fmt,...);
int32_t logger_sd_card_write(void);

#if 1  /* 可以看各个测试项的具体时间 ms*/
#define logging(fmt,arg...)    logger_buffer_write("[%8d]" fmt,g_iRunTime, ##arg)
#elif  0
#define logging(fmt,arg...)  logger("[%8d]" fmt,g_iRunTime, ##arg)
#elif 0 /* 不看各个测试项具体时间 */
#define logging  logger 
#endif
void logger_over(void);

#endif

