/**
* @file         crc32.c
* @brief        CRC32计算函数。
*               多项式：x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1  \n
*               初始值：FFFFFFFF    \n
*               结果异或值：FFFFFFFF  \n
*               输入值反转：true  \n
*               输出值反转：true  \n
* @details      本文件包含CRC32的表格生成，CRC32校验码计算
* @author       liujian
* @date         2021-12-07
* @version      V1.0
* @copyright    Copyright (c) 2018-2021  SpaceTouch Inc.
*/
#include "crc32.h"

static uint32_t s_crc32_tab[256] = {}; //用于存储CRC32静态表



/**
* @brief        crc32表格初始化
* @details      初始化crc32的表格，加快后续的计算速度
* @param[in]    None
* @return       None
* @note         在调用crc32计算函数前，需要确保此函数调用过一次。
*/
void crc32_init(void)
{
    #if 0
    uint32_t poly = 0xEDB88320;     //采用的是从高位往低位计算，多项式的高低位需要反转。
    for(uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;
        for(uint32_t j = 0; j < 8; j++)
        {
            if(crc & 1UL)
            {
                crc >>= 1;
                crc ^= poly;
            }
            else
            {
                crc >>= 1;
            }
        }
        s_crc32_tab[i] = crc;
    }
    
    #else
    #define CRC32_POLYNOMIAL    0x04C11DB7
    #define CRC32_INITIAL_VALUE 0xFFFFFFFF     //采用的是从高位往低位计算，多项式的高低位需要反转。
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i << 24;
        for (uint32_t j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
        s_crc32_tab[i] = crc;
    }
    #endif
}

uint32_t g_nor_key;
 
/**
* @brief        crc32校验值分步计算开始
* @details      准备crc32初始值
* @param[in]    none
* @return       none
* @note         none
*/
void crc32_partial_start(uint32_t *pResult)
{
    *pResult = 0xffffffff;
}

uint32_t plxc_programmer_encrypt_calcu(void)
{
    uint32_t start=0x20170328; /* 起始数 */
    uint8_t key_tab[24];
    
    for(uint32_t i=0; i<24; i++)
    {
        key_tab[i] = (g_nor_key>>i) + i;
    }
    
    crc32_partial_calcu(&start, key_tab, sizeof(key_tab));
    
    start ^= 0xfaceb00c;
    
    return start;
}


/**
* @brief        crc32校验值分步计算
* @details
* @param[in]    pbuff   指向要计算的数据
* @param[in]    size    指定要计算的数据的长度
* @return       none
* @note         none
*/
void  crc32_partial_calcu(uint32_t* pResult,const uint8_t *pbuff, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        //*pResult = s_crc32_tab[(*pResult ^ pbuff[i]) & 0xff] ^ (*pResult >> 8);
        *pResult = (*pResult << 8) ^ s_crc32_tab[(*pResult >> 24) ^ pbuff[i]];
    }
}


/**
* @brief        crc32校验值分步计算结束
* @details      结束并返回最终的计算结果
* @param[in]    none
* @return       crc32计算结果
* @note         none
*/
void crc32_partial_stop(uint32_t* pResult)
{
    *pResult = *pResult; // ^ 0xFFFFFFFF;
}

/**
* @brief        crc32校验值计算
* @details      计算数据的crc32校验码
* @param[in]    pbuff   指向要计算的数据
* @param[in]    size    指定要计算的数据的长度
* @return       指定数据的crc32校验码
* @note         函数直接计算出整个缓冲区数据的crc32校验值
*/
uint32_t crc32_entire_calcu(const uint8_t *pbuff, uint32_t size)
{
    uint32_t temp;
    crc32_partial_start(&temp);
    crc32_partial_calcu(&temp ,pbuff,size);
    crc32_partial_stop(&temp);
    return temp;
}
