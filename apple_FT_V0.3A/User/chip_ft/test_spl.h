#ifndef __TEST_SPL_H
#define __TEST_SPL_H

typedef struct
{
    union
	{
		uint32_t all;
		struct
		{
			uint32_t io_half1   : 1;
            uint32_t io_half2   : 1;
			uint32_t i2c        : 1;
			uint32_t uart       : 1;
			uint32_t spi        : 1;
            uint32_t gpadc      : 1;
            uint32_t audio      : 1;
			uint32_t unuse      : 25;
		};
	};
}ptn_item_t;


//获取pattern运行情况
typedef  struct
{
	ptn_item_t done;        //pattern 测试完毕
 
    uint32_t io_half1_err;  //有bit置位表示该bit位io有错；IO分为两部分测试，CH440R要切换side
    uint32_t io_half2_err;
    uint16_t gpadc_AGND;    //AGND的ADC值
    uint16_t gpadc_VREF_1P5;//GPADC内部参考1.5V的ADC值
    uint16_t gpadc_DVDD;   
    uint16_t gpadc_AVCC;
    uint16_t gpadc_VCC;     //不测试IOVCC（已尝试过测量），电压超量程，调节电压还要等待很长时间才稳定，否则测试不准确
    uint16_t gpadc_unuse;
    int32_t audio_index;    //音频回环测试的index
    int32_t audio_db;       //音频回环测试的信噪比
    uint32_t unuse1;
}ptn_query_t;

typedef struct
{
    uint32_t   chip_type;
    ptn_item_t en;
}ptn_en_t;

enum
{
    io_half1_ptn_bit_num = 0,
    io_half2_ptn_bit_num,
    i2c_ptn_bit_num,
    uart_ptn_bit_num,
    spi_ptn_bit_num,
    gpadc_ptn_bit_num,
    audio_ptn_bit_num
};

 
fsm_rt_t test_spl(void);
#endif
