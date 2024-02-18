/**
  ******************************************************************************
  * @file    usbd_storage_msd.c
  * @author  MCD application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the disk operations functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_msc_mem.h"
#include "usb_conf.h"
#include "bsp_spi_flash.h"
#include "bsp_sdio_sd.h"
#include "stdio.h"
#include "lcd.h"
#include "bsp_iwdg.h"

//#define mass_printf	printf
#define mass_printf(...)

#define LUN_SPI_FLASH	0

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup STORAGE
  * @brief media storage application module
  * @{
  */

/** @defgroup STORAGE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup STORAGE_Private_Defines
  * @{
  */

#define STORAGE_LUN_NBR                  1
/**
  * @}
  */


/** @defgroup STORAGE_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup STORAGE_Private_Variables
  * @{
  */
/* USB Mass storage Standard Inquiry Data */
/*  长度 = USBD_STD_INQUIRY_LENGTH */
const int8_t  STORAGE_Inquirydata[] = 
{
    
    /* LUN 0 SPI Flash */
	0x00,
	0x80,
	0x02,
	0x02,
	(USBD_STD_INQUIRY_LENGTH - 5),
	0x00,
	0x00,
	0x00,
	'P', 'L', 'X', 'C', ' ', 'Z', 'G', 'Q', /* Manufacturer : 8 bytes */
	'S', 'P', 'I', ' ', ' ', 'F', 'l', 'a', /* Product      : 16 Bytes */
	's', 'h', ' ', ' ', ' ', ' ', ' ', ' ',
	'1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
};

/**
  * @}
  */


/** @defgroup STORAGE_Private_FunctionPrototypes
  * @{
  */
int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun,
                           uint32_t *block_num,
                           uint32_t *block_size);

int8_t  STORAGE_IsReady (uint8_t lun);

int8_t  STORAGE_IsWriteProtected (uint8_t lun);

int8_t STORAGE_Read (uint8_t lun,
                        uint8_t *buf,
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_Write (uint8_t lun,
                        uint8_t *buf,
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_GetMaxLun (void);


USBD_STORAGE_cb_TypeDef USBD_user_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

/* USBD_STORAGE_fops 变量被固件库中的 usbd_msc_scsi.c 调用 */
USBD_STORAGE_cb_TypeDef  *USBD_STORAGE_fops = &USBD_user_fops;

//#ifndef USE_STM3210C_EVAL
//extern SD_CardInfo SDCardInfo;
//#endif

__IO uint32_t count = 0;
/**
  * @}
  */


/** @defgroup STORAGE_Private_Functions
  * @{
  */

/**
  * @brief  Initialize the storage medium
  * @param  lun : logical unit number
  * @retval Status
  */

int8_t STORAGE_Init (uint8_t lun)
{	
	switch (lun)
	{
		 
		case LUN_SPI_FLASH:
//			if (NAND_Init() != NAND_OK)
//			{
//				return (-1);
//			}
            bsp_InitSFlash();
			mass_printf("> SPI STORAGE_Init Ok\r\n");
			break;
	}

	return (0);
}

/**
  * @brief  return medium capacity and block size
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    switch (lun)
	{
		 
		
		case LUN_SPI_FLASH:
			mass_printf("> SPI FLASH STORAGE_GetCapacity\r\n");
			#if 1
				*block_size =  4096;
				*block_num =  g_tSF.TotalSize/(*block_size);//512*2;
			#else
				*block_size =  NAND_PAGE_SIZE;
				*block_num =  NAND_BLOCK_SIZE * NAND_ZONE_SIZE * NAND_MAX_ZONE;
			#endif
			break;
	}

	return (0);
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{
	//mass_printf("> STORAGE_IsReady\r\n");
    uint32_t id = sf_ReadID();
	switch (lun)
	{
    

    case LUN_SPI_FLASH:
        if (id == P25Q32H_ID) return 0;
        else if(id == BY25Q128_ID)  return 0;
        else                       
        {
            printf("flash No support:0x%x \r\n",id);
            display_show_string("flash No support:0x%x \n",BLACK , RED);
            return -1;
        }
     
	}

	return (0);
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
	mass_printf("> STORAGE_IsWriteProtected\r\n");
	return  0;
}

/**
  * @brief  Read data from the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to save data
  * @param  blk_addr :  address of 1st block to be read
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Read (uint8_t lun,
                 uint8_t *buf,
                 uint32_t blk_addr,
                 uint16_t blk_len)
{
	switch (lun)
	{
    
    case LUN_SPI_FLASH:

        sf_ReadBuffer((uint8_t *)buf, blk_addr<<12, blk_len<<12);
        mass_printf("read : BlockNo=%d, BlockCount=%d\r\n", blk_addr,blk_len);
        break;
	}
	return 0;
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Write (uint8_t lun,
                  uint8_t *buf,
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
	switch (lun)
	{
		 

		case LUN_SPI_FLASH:		
//			if (NAND_OK != NAND_WriteMultiSectors(buf, blk_addr, 512, blk_len))
//			{
//				return -1;
//			}
            IWDG_Feed();                /* 喂狗 */
            sf_EraseSector(blk_addr<<12);
            sf_WriteBuffer((uint8_t *)buf,blk_addr<< 12,blk_len<<12);	
			mass_printf("write : BlockNo=%d, BlockCount=%d\r\n", blk_addr,blk_len);
			break;
	}		
	return 0;
}

/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */

int8_t STORAGE_GetMaxLun (void)
{
	return (STORAGE_LUN_NBR - 1);
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

