#ifndef __BTLDR_CONFIG_H
#define __BTLDR_CONFIG_H
/*
bootloader 配置
*/

#define B2A_MAGIC_NUM      0xAA553344  //boot to app
#define A2B_MAGIC_NUM      0x5AA51234  //app to boot

/* 设置未初始化变量 */
#if defined (__ICCARM__)  /* IAR */

#pragma location = ".NoInit"  
uint32_t g_JumpInit;

#elif defined(__CC_ARM)   /* MDK AC5 */

uint32_t g_JumpInit __attribute__((at(0x20000000), zero_init));  
char*    p_boot_ver __attribute__((at(0x20000004), zero_init));  //bootloader版本信息
 
#elif  (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) /* MDK AC6 */

uint32_t g_JumpInit __attribute__( ( section( ".bss.NoInit"))); 

#elif  defined(__GNUC__)      /* GCC */

__attribute__((section( ".NoInit"))) uint32_t g_JumpInit;

#endif


#endif


