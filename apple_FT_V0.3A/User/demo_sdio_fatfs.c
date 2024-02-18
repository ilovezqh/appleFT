/*
*********************************************************************************************************
*
*	模块名称 : SD卡Fat文件系统演示模块及其模拟U盘功能。
*	文件名称 : demo_sdio_fatfs.c
*	版    本 : V1.2
*	说    明 : 该例程移植FatFS文件系统（版本 R0.10b），演示如何创建文件、读取文件、创建目录和删除文件
*			并测试了文件读写速度以及SD卡模拟U盘功能
*
*	修改记录 :
*		版本号   日期        作者     说明
*		V1.0    2013-06-26   plxc  正式发布
*
*	Copyright (C), 2023-2024, 普林芯驰 www.spacetouch.co
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ff.h"			/* FatFS文件系统模块*/
#include "demo_sdio_fatfs.h"
#include "usb_bsp_msc.h"		/* usb底层驱动 */
#include "diskio.h"		/* Declarations of low level disk I/O functions */
#include "demo_spi_flash.h"
#include "test.h"
/* 用于测试读写速度 */
#define TEST_FILE_LEN			(2*1024*1024)	/* 用于测试的文件长度 */
#define BUF_SIZE				(4*1024)		/* 每次读写SD卡的最大数据长度 */
static uint8_t g_TestBuf[BUF_SIZE];

 

static void ViewRootDir(char* _DiskVolume);
static void CreateNewFile(char* _DiskVolume);
static void ReadFileData(char* _DiskVolume);
static void CreateDir(char* _DiskVolume);
static void DeleteDirFile(char* _DiskVolume);
static void WriteFileTest(void);

#define MOUNT_IMMEDIATELY 0

char *fs_path[2]=
{
    "0:",
    "1:"
};
#define FS_SD_STR  "0:"
#define FS_SPI_FLASH_STR "1:"
#define SD_TEST 0

char* ucVolume = FS_SD_STR;

extern void DispMenu(void);
#define enter  13
void fatfs_task(void)
{
 
    //extern fsm_rt_t no_load_current_calibration(void);
	uint8_t cmd;
    //uint8_t ucVolume = 0;
    
    if (comGetChar(COM1, &cmd))	/* 从串口读入一个字符(非阻塞方式) */
    {
        //printf("\r\n");
        //DemoSpiFlash(cmd);
      
        switch (cmd)
        {
        case enter:
            printf("\r\n[FT 测试]\r\n");
 
            #if SD_TEST
            ViewRootDir(FS_SD_STR);			/* 显示SD卡根目录下的文件名 */
            #else
            ViewRootDir(ucVolume);		/* 显示U盘根目录下的文件名 */
            #endif

            break;
        case '0':
            {
//                fsm_rt_t ret;
//                do
//                {
//                    ret = no_load_current_calibration();
//                }while(ret == fsm_rt_on_going);
            }       
            break;
        case '2':
            #if SD_TEST
            printf("【2 - CreateNewFile】\r\n");
            CreateNewFile(FS_SD_STR);	/* 创建一个新文件,写入一个字符串 */
            #else
            CreateNewFile(ucVolume);	/* 创建一个新文件,写入一个字符串 */
            #endif
            break;

        case '3':
            printf("【3 - ReadFileData】\r\n");
            #if SD_TEST
            ReadFileData(FS_SD_STR);		/* 读取根目录下plxc.txt的内容 */
            #else
            ReadFileData(ucVolume);		/* 读取根目录下plxc.txt的内容 */
            #endif
            break;

        case '4':
            printf("【4 - CreateDir】\r\n");
            #if SD_TEST
            CreateDir(FS_SD_STR);		/* 创建目录 */
            #else
            CreateDir(ucVolume);		/* 创建目录 */
            #endif
             
            break;

        case '5':
            printf("【5 - DeleteDirFile】\r\n");
            #if SD_TEST
            DeleteDirFile(FS_SD_STR);	/* 删除目录和文件 */
            #else
            DeleteDirFile(ucVolume);	/* 删除目录和文件 */
            #endif
             
            break;

        case '6':
            printf("【6 - TestSpeed】\r\n");
            
            WriteFileTest();	/* 速度测试 */
            break;
        
        case '7':	/* 移除U盘, 软件模拟U盘拔除 */
            printf("【7 - 移除U盘】\r\n");
            usbd_CloseMassStorage();
            break;

        case '8':	/* 使能U盘，软件模拟U盘插入 */
            printf("【8 - 使能U盘】\r\n");
            usbd_OpenMassStorage();
            break;
                
//        case '0':
//            printf("Start Format(Low Level) NAND Flash......\r\n");
//            //NAND_Format();
//           // printf("NAND Flash Format Ok\r\n");
//            break;
        
        case 'V':		/* 切换当前盘符 */
        case 'v':
            printf("【Vv - 切换盘符】\r\n");
            if (ucVolume == FS_SD_STR)
            {
                ucVolume = FS_SPI_FLASH_STR;
            }
            else
            {
                ucVolume = FS_SD_STR;
            }
             
            break;

        default:
            DispMenu();
            break;
        }
        
 
    }

}


#if 1
/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示根目录下的文件名
*	形    参：_ucVolume : 磁盘卷标 0 - 2
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewRootDir(char* _ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	static FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];
    char path[32];
    //FRESULT res_flash;
//    char strr[]="12345678";
 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, MOUNT_IMMEDIATELY);	/* Mount a logical drive */
	if (result != FR_OK)
	{
        //res_flash = f_mkfs(_ucVolume,0,0);
		printf("挂载文件系统失败 (%d)\r\n", result);
//        if (res_flash != FR_OK)
//        {
//            printf("》 FLASH 已成功格式化文件系统。 \r\n");
//            /* 格式化后，先取消挂载 */
//            res_flash = f_mount(NULL,_ucVolume,MOUNT_IMMEDIATELY);
//            /* 重新挂载 */
//            res_flash = f_mount(&fs,_ucVolume,MOUNT_IMMEDIATELY);
//        }
//        else
//        {
//        //LED_RED;
//            printf("《《格式化失败。》》 \r\n");
//            while (1);
//        }
        return;
	}
   // sprintf(path_str,"%s/",_ucVolume);
    //result = f_getcwd(strr, 10);
    //printf("file path=s%",strr);
	/* 打开根文件夹 */
    sprintf(path, "%s/", _ucVolume);	 /* 1: ±íÊ¾ÅÌ·û */
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		/* 打印文件大小, 最大4G */
		printf(" %10d", FileInf.fsize);

		printf("  %s |", FileInf.fname);	/* 短文件名 */

		printf("  %s\r\n", (char *)FileInf.lfname);	/* 长文件名 */
	}

	/* 卸载文件系统 */
	 f_mount(NULL, _ucVolume, MOUNT_IMMEDIATELY); 
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“www.spacetouch.co”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(char* _ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
    char path[32];
 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, MOUNT_IMMEDIATELY);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	result = f_open(&file, "plxc.txt", FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, "FatFS Write Demo \r\n www.spacetouch.co \r\n", 34, &bw);
	if (result == FR_OK)
	{
		printf("plxc.txt 文件写入成功\r\n");
	}
	else
	{
		printf("plxc.txt 文件写入失败\r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	 f_mount(NULL, _ucVolume, MOUNT_IMMEDIATELY);
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 读取文件plxc.txt前128个字符，并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadFileData(char* _ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];
    char path[32];
 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, MOUNT_IMMEDIATELY);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败(%d)\r\n", result);
	}

	/* 打开根文件夹 */
    sprintf(path, "%s/", _ucVolume);
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败(%d)\r\n", result);
		return;
	}

	/* 打开文件1 */
	result = f_open(&file, "plxc.txt", FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : plxc.txt\r\n");
		return;
	}

	/* 读取文件 */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		printf("\r\nplxc.txt 文件内容 : \r\n%s\r\n", buf);
	}
	else
	{
		printf("\r\nplxc.txt 文件内容 : \r\n");
	}

    /* 打开文件2 */
	result = f_open(&file, "CONFIG.txt", FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : CONFIG.txt\r\n");
		return;
	}

	/* 读取文件 */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		printf("\r\nCONFIG.txt 文件内容 : \r\n%s\r\n", buf);
	}
	else
	{
		printf("\r\nCONFIG.txt 文件内容 : \r\n");
	}
    
    /* 打开文件3 */
	result = f_open(&file, buf, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : %s\r\n",buf);
		return;
	}
    uint8_t bufT[128];
	/* 读取文件 */
	result = f_read(&file, &bufT, 8, &bw);
	if (bw > 0)
	{
		//bufT[bw] = 0;
        printf("\r\n%s  文件内容1 :\r\n", buf);
        
        for(uint8_t i=0;i<8;i++)
        {
            printf("\r\n 0x%02x \r\n", bufT[i]);
        }
		
	}
	else
	{
		printf("\r\n%s 文件内容2 : \r\n",buf);
	}
    
	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	 f_mount(NULL, _ucVolume, MOUNT_IMMEDIATELY);
}

/*
*********************************************************************************************************
*	函 数 名: CreateDir
*	功能说明: 在SD卡根目录创建Dir1和Dir2目录，在Dir1目录下创建子目录Dir1_1
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateDir(char* _ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;

 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, MOUNT_IMMEDIATELY);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 创建目录/Dir1 */
	result = f_mkdir("/Dir1");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1 失败 (%d)\r\n", result);
		return;
	}

	/* 创建目录/Dir2 */
	result = f_mkdir("/Dir2");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir2 目录已经存在(%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir2 失败 (%d)\r\n", result);
		return;
	}

	/* 创建子目录 /Dir1/Dir1_1	   注意：创建子目录Dir1_1时，必须先创建好Dir1 */
	result = f_mkdir("/Dir1/Dir1_1"); /* */
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1_1 成功\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1_1 目录已经存在 (%d)\r\n", result);
	}
	else
	{
		printf("f_mkdir Dir1_1 失败 (%d)\r\n", result);
		return;
	}

	/* 卸载文件系统 */
	f_mount(NULL, _ucVolume, MOUNT_IMMEDIATELY);
}

/*
*********************************************************************************************************
*	函 数 名: DeleteDirFile
*	功能说明: 删除SD卡根目录下的 plxc.txt 文件和 Dir1，Dir2 目录
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeleteDirFile(char* _ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint8_t i;
     
 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, MOUNT_IMMEDIATELY);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	#if 0
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败(%d)\r\n", result);
		return;
	}
	#endif

	/* 删除目录/Dir1 【因为还存在目录非空（存在子目录)，所以这次删除会失败】*/
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1/Dir1_1 */
	result = f_unlink("/Dir1/Dir1_1");
	if (result == FR_OK)
	{
		printf("删除子目录/Dir1/Dir1_1成功\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		printf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1 */
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除目录/Dir2 */
	result = f_unlink("/Dir2");
	if (result == FR_OK)
	{
		printf("删除目录 Dir2 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "/Dir2");
	}
	else
	{
		printf("删除Dir2 失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 plxc.txt */
	result = f_unlink("plxc.txt");
	if (result == FR_OK)
	{
		printf("删除文件 plxc.txt 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("没有发现文件或目录 :%s\r\n", "plxc.txt");
	}
	else
	{
		printf("删除plxc.txt失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "Speed%02d.txt", i);		/* 每写1次，序号递增 */
		result = f_unlink(FileName);
		if (result == FR_OK)
		{
			printf("删除文件%s成功\r\n", FileName);
		}
		else if (result == FR_NO_FILE)
		{
			printf("没有发现文件:%s\r\n", FileName);
		}
		else
		{
			printf("删除%s文件失败(错误代码 = %d) 文件只读或目录非空\r\n", FileName, result);
		}
	}

	/* 卸载文件系统 */
	f_mount(NULL, _ucVolume, MOUNT_IMMEDIATELY);
}

/*
*********************************************************************************************************
*	函 数 名: WriteFileTest
*	功能说明: 测试文件读写速度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}

  	/* 挂载文件系统 */
	result = f_mount(&fs, fs_path[0], MOUNT_IMMEDIATELY);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "0:/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	sprintf(TestFileName, "Speed%02d.txt", s_ucTestSn++);		/* 每写1次，序号递增 */
	result = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	printf("开始写文件%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_write(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}
		}
		else
		{
			err = 1;
			printf("%s文件写失败\r\n", TestFileName);
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  写耗时 : %dms   平均写速度 : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	f_close(&file);		/* 关闭文件*/


	/* 开始读文件测试 */
	result = f_open(&file, TestFileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("没有找到文件: %s\r\n", TestFileName);
		return;
	}

	printf("开始读文件 %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}

			/* 比较写入的数据是否正确，此语句会导致读卡速度结果降低到 3.5MBytes/S */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					printf("Speed1.txt 文件读成功，但是数据出错\r\n");
					break;
				}
			}
			if (err == 1)
			{
				break;
			}
		}
		else
		{
			err = 1;
			printf("Speed1.txt 文件读失败\r\n");
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  读耗时 : %dms   平均读速度 : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	 f_mount(NULL, fs_path[0], MOUNT_IMMEDIATELY);
}
#endif
/***************************** 普林芯驰 www.spacetouch.co (END OF FILE) *********************************/
