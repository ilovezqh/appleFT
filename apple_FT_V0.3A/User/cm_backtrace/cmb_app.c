#include "bsp.h"
#include <stdio.h>
#include <stdarg.h>
#include "cm_backtrace.h"
#include "ff.h"
#include "diskio.h"

#define APPNAME                        "CmBacktrace"
#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.0.1"

#define ERR_LOG_SIZE 1024
static char err_log[ERR_LOG_SIZE];
static int log_write_index = 0;

 

void cmb_init(void)
{
    /* USER CODE BEGIN 2 */
    printf("CmBacktrace Test...\r\n");
    //cmb_println("Test:abcdef\r\n");
    cm_backtrace_init(APPNAME, HARDWARE_VERSION, SOFTWARE_VERSION);
    //fault_test_by_div0();
}

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
void fault_test_by_div0(void)
{
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}
/* USER CODE END PV */

#define PRINTF_BUF 512 // Set this value according to your needs

void print_UART1(const char *fmt, ...)
{
    char buf[PRINTF_BUF];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (int i = 0; buf[i] != '\0'; i++)
    {
        err_log[log_write_index] = buf[i];
        log_write_index++;
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Wait for Empty
        USART_SendData(USART1, buf[i]); // Send char
    }
}


void log_error(uint32_t fault_handler_lr, uint32_t fault_handler_sp)
{
    // Init the error log
    memset(err_log, 0, sizeof(err_log)); 
    log_write_index = 0;
    GPIOD->BSRRL = GPIO_Pin_11;			/* beep  */
    // Trigger cmbacktrace to output the error log
    cm_backtrace_fault(fault_handler_lr, fault_handler_sp); 

    // Create a FATFS file object
    FIL log_file;
    FRESULT fr;

    // Try to open the error log file
    fr = f_open(&log_file, "0:/cm_error_log.txt", FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK)
    {
        // If the file is opened successfully, write the error log
        unsigned int written;
        fr = f_write(&log_file, err_log, log_write_index, &written);
        // Always make sure to close the file
        f_close(&log_file);
    }
    // else: handle the error of opening the file
}

