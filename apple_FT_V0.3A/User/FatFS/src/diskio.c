/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"			/* FatFs lower layer API */
#include "bsp.h"	      


#define SECTOR_SIZE		512
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = STA_NOINIT;
	
	switch (pdrv) {

		case FS_SPI_FLASH :
			bsp_InitSFlash();
			stat =  RES_OK;
			break;
		
		case FS_SD :		
			if (SD_Init() == SD_OK)
			{
				stat = RES_OK;
			}
			else
			{
				stat = STA_NODISK;
			}	
	}
	return stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv)
	{
		case FS_SPI_FLASH :
			stat = 0;
			break;
		
		case FS_SD :
			stat = 0;
			break;
	}
	return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res = RES_PARERR;
	
	switch (pdrv) {

		case FS_SPI_FLASH :
			sf_ReadBuffer(buff, sector << 12, count<<12);
			res =  RES_OK;
			break;
		
		case FS_SD :
		{
			SD_Error Status = SD_OK;

			if (count == 1)
			{
				Status = SD_ReadBlock(buff, sector << 9 , SECTOR_SIZE);
			}
			else
			{
				Status = SD_ReadMultiBlocks(buff, sector << 9 , SECTOR_SIZE, count);
			}
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}

		#ifdef SD_DMA_MODE
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}

			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif

			res = RES_OK;
			break;
		}

	}
	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	switch (pdrv) {
		
		case FS_SPI_FLASH :
			{
			#if 0
				uint8_t i;
				BYTE *p;
				
				p = (BYTE *)buff;
				for(i = 0; i < count; i++)
				{
					sf_WriteBuffer((uint8_t *)p, (sector+i) << 12, 4096);
					p += 4096;					
				}
			#else
				sf_WriteBuffer((uint8_t *)buff, sector << 12, count<<12);
			#endif
				return RES_OK;
			}
			
		case FS_SD :
		{
			SD_Error Status = SD_OK;

			if (count == 1)
			{
				Status = SD_WriteBlock((uint8_t *)buff, sector << 9 ,SECTOR_SIZE);
			}
			else
			{
				
				//Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
				Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);
			}
			
			if (Status != SD_OK)
			{
				return RES_ERROR;
			}

		#ifdef SD_DMA_MODE
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
			{
				return RES_ERROR;
			}
			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif
			
			return RES_OK;
		}
	
	}
	
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	
	switch (pdrv) {
		
	case FS_SPI_FLASH :
		switch(cmd)
		{
			/* SPI Flash不需要同步 */
			case CTRL_SYNC :  
				return RES_OK;
			
			/* 返回SPI Flash扇区大小 */
			case GET_SECTOR_SIZE:
				*((WORD *)buff) = 4096;  
				return RES_OK;
			
			/* 返回SPI Flash扇区数 */
			case GET_SECTOR_COUNT:
				*((DWORD *)buff) = g_tSF.TotalSize/4096;    
				return RES_OK;
			
			/* 下面这两项暂时未用 */
			case GET_BLOCK_SIZE:   
				return RES_OK;
			
			case CTRL_ERASE_SECTOR:
				return RES_OK;       
		}
		
	 case FS_SD :
		{	
		   switch(cmd)
		   {
			/* 返回SD扇区大小 */
			case GET_SECTOR_SIZE:
				*((WORD *)buff) = 512;  
				return RES_OK;
			
			default:
				return RES_OK;    
		   }  
	   }		   
	}
	
	return RES_PARERR;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: get_fattime
*	功能说明: 获得系统时间，用于改写文件的创建和修改时间。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
DWORD get_fattime (void)
{
	/* 如果有全局时钟，可按下面的格式进行时钟转换. 这个例子是2013-01-01 00:00:00 */

	return	  ((DWORD)(2013 - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)1 << 21)				/* Month = 1 */
			| ((DWORD)1 << 16)				/* Day_m = 1*/
			| ((DWORD)0 << 11)				/* Hour = 0 */
			| ((DWORD)0 << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */
}

