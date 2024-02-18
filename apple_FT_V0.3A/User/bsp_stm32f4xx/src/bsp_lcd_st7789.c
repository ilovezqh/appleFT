#include "bsp.h"

//#include "bsp_gpio.h"
//#include "bsp_timer.h"

/* 涓茶Flsh鐨勭墖閫塆PIO绔彛  */
#define LCD_RCC_CS 			RCC_AHB1Periph_GPIOB
#define LCD_PORT_CS			GPIOB
#define LCD_PIN_CS			GPIO_Pin_9
#define LCD_CS_0()			LCD_PORT_CS->BSRRH = LCD_PIN_CS
#define LCD_CS_1()			LCD_PORT_CS->BSRRL = LCD_PIN_CS

#define LCD_RCC_RST 	    RCC_AHB1Periph_GPIOE
#define LCD_PORT_RST		GPIOE
#define LCD_PIN_RST			GPIO_Pin_0
#define LCD_CLR_RST() 	    do{LCD_PORT_RST->BSRRH = LCD_PIN_RST;}while(0)
#define LCD_SET_RST()		do{LCD_PORT_RST->BSRRL = LCD_PIN_RST;}while(0)

#define LCD_RCC_DC 	        RCC_AHB1Periph_GPIOD
#define LCD_PORT_DC		    GPIOD
#define LCD_PIN_DC			GPIO_Pin_8
#define LCD_CLR_DC() 	    do{LCD_PORT_DC->BSRRH = LCD_PIN_DC;}while(0)
#define LCD_SET_DC()		do{LCD_PORT_DC->BSRRL = LCD_PIN_DC;}while(0)

#define LCD_RCC_BL 	        RCC_AHB1Periph_GPIOE
#define LCD_PORT_BL		    GPIOE
#define LCD_PIN_BL			GPIO_Pin_1
#define LCD_CLR_BL() 	    do{LCD_PORT_BL->BSRRH = LCD_PIN_BL;}while(0)
#define LCD_SET_BL()		do{LCD_PORT_BL->BSRRL = LCD_PIN_BL;}while(0)

#define os_delay_ms(var)    bsp_DelayMS(var)

const uint8_t lcd_config_init_cmd[] = 
{
    0x11, 0x36, 0x3A, 0xB2, 0xB7, 0xBB, 0xC2, 0xC3, 
    0xC4, 0xC6, 0xD0, 0xE0, 0xE1, 0x21, 0x29
};

const uint8_t lcd_config_init_data[] = 
{
    0x00, 0x05, 0x0C, 0x0C, 0x00, 0x33, 0x33, 0x35,
    0x32, 0x01, 0x15, 0x20, 0x0F, 0xA4, 0xA1, 0xD0,
    0x08, 0x0E, 0x09, 0x09, 0x05, 0x31, 0x33, 0x48,
    0x17, 0x14, 0x15, 0x31, 0x34, 0xD0, 0x08, 0x0E,
    0x09, 0x09, 0x15, 0x31, 0x33, 0x48, 0x17, 0x14,
    0x15, 0x31, 0x34
};

static void lcd_SetCS(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* 鍗犵敤SPI鎬荤嚎锛?鐢ㄤ簬鎬荤嚎鍏变韩 */

		#ifdef SOFT_SPI		/* 杞欢SPI */
			bsp_SetSpiSck(1);
			LCD_CS_0();
		#endif

		#ifdef HARD_SPI		/* 纭欢SPI */

			bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
				| SPI_CPOL_High | SPI_CPHA_2Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_2 | SPI_FirstBit_MSB);

			LCD_CS_0();
		#endif
	}
	else
	{
		LCD_CS_1();

		bsp_SpiBusExit();	/* 閲婃斁SPI鎬荤嚎锛?鐢ㄤ簬鎬荤嚎鍏变韩 */
	}
}
/*
*********************************************************************************************************
*	鍑?鏁?鍚? sf_ConfigGPIO
*	鍔熻兘璇存槑: 閰嶇疆涓茶Flash鐨勭墖閫塆PIO銆?璁剧疆涓烘帹鎸借緭鍑?
*	褰?   鍙? 鏃?
*	杩?鍥?鍊? 鏃?
*********************************************************************************************************
*/
static void lcd_ConfigGPIO(void)
{
	/*
		????瀵岃幈STM32-V4 寮€鍙戞澘鍙ｇ嚎鍒嗛厤锛? 涓茶Flash鍨嬪彿涓?W25Q64BVSSIG (80MHz)
		PF8/LCD_CS
	*/
	GPIO_InitTypeDef GPIO_InitStructure={0};

	/* 浣胯兘GPIO 鏃堕挓 */
	RCC_AHB1PeriphClockCmd(LCD_RCC_CS | LCD_RCC_RST | LCD_RCC_DC | LCD_RCC_BL, ENABLE);

	/* 閰嶇疆鐗囬€夊彛绾夸负鎺ㄦ尳杈撳嚭妯″紡 */
	lcd_SetCS(1);		/* 鐗囬€夌疆楂橈紝涓嶉€変腑 */
    LCD_SET_DC();
    LCD_SET_RST();
    LCD_SET_BL();
    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LCD_PIN_CS;
	GPIO_Init(LCD_PORT_CS, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_PIN_DC;
	GPIO_Init(LCD_PORT_DC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_PIN_RST;
	GPIO_Init(LCD_PORT_RST, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_PIN_BL;
	GPIO_Init(LCD_PORT_BL, &GPIO_InitStructure);
    
}

static void bsp_lcd_config_init(void)
{
    LCD_CLR_RST();
    os_delay_ms(10);
    LCD_SET_RST();
    os_delay_ms(10);

    bsp_lcd_send_cmd((uint8_t *)lcd_config_init_cmd, 1);
    os_delay_ms(10);
    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 1), 1);
    bsp_lcd_send_data((uint8_t *)lcd_config_init_data, 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 2), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 1), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 3), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 2), 5);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 4), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 7), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 5), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 8), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 6), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 9), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 7), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 10), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 8), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 11), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 9), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 12), 1);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 10), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 13), 2);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 11), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 15), 14);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 12), 1);
    bsp_lcd_send_data((uint8_t *)(lcd_config_init_data + 29), 14);

    bsp_lcd_send_cmd((uint8_t *)(lcd_config_init_cmd + 13), 2);
}

/*
 * 函数名：bsp_lcd_init
 * 描述  ：LCD显示初始化
 * 输入  ：void
 * 输出  ：void
 */
void bsp_lcd_init(void)
{
    lcd_ConfigGPIO();

    bsp_lcd_config_init();
    
}

/*
 * 函数名：bsp_lcd_send_data
 * 描述  ：LCD发送数据
 * 输入  ：uint8_t *send_data       需要发送的显示数据
 *         uint16_t data_len     显示数据长度
 * 输出  ：void
 */
bool bsp_lcd_send_data(uint8_t *send_data, uint16_t data_len)
{
    bool result = false;
    uint32_t timeout = 1000;
    uint8_t *send_data_ptr = send_data;
    
    #if 0
    while(data_len--)
    {
        hspi2.Instance->DR = *send_data_ptr;
        send_data_ptr++;
        timeout = 1000;
        while(timeout--)
        {
            if(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE))
            {
                break;
            }
        }
    }

    if(timeout > 0)
    {
        result = true;
    }
    #else
    lcd_SetCS(0);
    while(data_len--)
    {
        uint8_t data = *send_data_ptr;
        send_data_ptr++;
        timeout = 1000;
        while(timeout--)
        {
            //if(!bsp_SpiBusBusy()) 
            //{
                
                bsp_spiWrite1(data);
                break;
            //}   
        }
    }

    if(timeout > 0)
    {
        result = true;
    }
    #endif
    lcd_SetCS(1);
    return result;
}

 
bool bsp_lcd_send_cmd(uint8_t *send_cmd, uint16_t cmd_len)
{
    bool result = false;
    uint32_t timeout = 1000;
    uint8_t *send_cmd_ptr = send_cmd;

    if(send_cmd_ptr == NULL)
    {
        return false;
    }
    lcd_SetCS(0);
    LCD_CLR_DC();
    #if 0
    while(cmd_len--)
    {
        hspi2.Instance->DR = *send_cmd_ptr;
        send_cmd_ptr++;
        timeout = 1000;
        while(timeout--)
        {
            if(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE))
            {
                break;
            }
        }
    }
    #else
    while(cmd_len--)
    {
        uint8_t data = *send_cmd_ptr;
        send_cmd_ptr++;
        timeout = 1000;
        while(timeout--)
        {
            //if(!bsp_SpiBusBusy()) 
            //{
                
                bsp_spiWrite1(data);
                break;
            //}   
        }
    }

    #endif
    
    LCD_SET_DC();

    if(timeout > 0)
    {
        result = true;
    }
    lcd_SetCS(1);
    return result;
}

