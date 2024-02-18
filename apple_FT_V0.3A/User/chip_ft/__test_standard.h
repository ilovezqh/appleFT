#ifndef __TEST_STANDARD_H
#define __TEST_STANDARD_H
 
/************************* @Standby  电流(0.1uA) ******************************/
#define STANDBY_CURRENT_HI      50//60  //0.1uA
#define STANDBY_CURRENT_LO      20  //0.1uA


//电压测量地单位转成mv
#define IREF_LO        (45 )  //4.5uA
#define IREF_HI        (55 )
#define IREF_TAR       (50 )

#define IREF_TRIM_LO   4  //0   为防止FT出现异常，根据1k pcs的数据分析统计，测试IREF校准值锁定4或者5
#define IREF_TRIM_HI   5  //7 

/****************************** @PMU 电源(mv) **********************************/
#define VREF_TRIM_LO    0
#define VREF_TRIM_HI    15

#define VREF_LO         1282  
#define VREF_HI         1417   
#define VREF_TAR        1350

#define LPVDD_TRIM_LO   0
#define LPVDD_TRIM_HI   7

#define LPVDD_LO        1425
#define LPVDD_HI        1575  
#define LPVDD_TAR       1500 

/****************************** @CMU  时钟(hz)*************************************/

#define RCL_TRIM_P1_LO     0
#define RCL_TRIM_P1_HI     31
#define RCL_TRIM_P2_LO     32
#define RCL_TRIM_P2_HI     63

#define RCL_LO          30400
#define RCL_HI          33600
#define RCL_TAR         32000


#define RCH_TRIM_LO     0
#define RCH_TRIM_HI     128

#define RCH_LO          79200000//72000000
#define RCH_HI          80800000//88000000
#define RCH_TAR         80000000


#endif
