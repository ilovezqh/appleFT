#include "__test_common.h"

#define EE_HEAD  0xAAAAAAAA
#define EE_TAIL  0xBBBBBBBB

 
ee_data_t ee_data;

int32_t ee_data_init(void)
{
    uint8_t fail_cnt=0;

    uint32_t ee_crc32;
   
    do
    {
        uint8_t sta = ee_ReadBytes((uint8_t*)&ee_data, 0, sizeof(ee_data_t));
        if(sta == 0) return -1; //read ee fail
        
        for(uint16_t i=0; i<sizeof(ee_data_t); i++)
        {
            if((i%16) == 0) debug("\r\n");
            debug("%02X ",((uint8_t*)&ee_data)[i]);
            if((i%8 == 0) && (i/8%2))  debug(" - ");
            
        }
        debug("\r\n");
        ee_crc32 = crc32_entire_calcu((uint8_t*)&ee_data, sizeof(ee_data_t)-4);
        if((ee_data.head != EE_HEAD) || (ee_data.tail != EE_TAIL) || (ee_crc32!= ee_data.crc))
        {
            fail_cnt++;
            memset(&ee_data,0,sizeof(ee_data_t));
            ee_data.head            = EE_HEAD;
            ee_data.tail            = EE_TAIL;
            ee_data.crc             = crc32_entire_calcu((uint8_t*)&ee_data, sizeof(ee_data_t)-4);
            sta = ee_WriteBytes((uint8_t*)&ee_data, 0, sizeof(ee_data_t));
            if(sta == 0) return -2; //write ee fail
            delay_ms(3); //must delay for read operation  after write operation
        }
        else    
        {
            return 0;  //ee success
        }
    }
    while(fail_cnt<3);
    
    return -3; //try max error
}

int32_t ee_data_save(void)
{
    ee_data.crc = crc32_entire_calcu((uint8_t*)&ee_data, sizeof(ee_data_t)-4);
    if(ee_WriteBytes((uint8_t *)&ee_data,0, sizeof(ee_data_t))) return 0;
    return -1;
}


