#include "__test_common.h"
 
uint8_t apple_CheckOk(void)
{
	if (asu_CheckDevice(APPLE_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		asu_Stop();
		return 0;
	}
}

fsm_rt_t apple_read_addr16(uint8_t DevAddr,uint8_t *ReadData,uint16_t RegAddr,uint16_t length)
{
	uint16_t num = 0x00;
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ ASU_WR);	//设备地址 + 写
	if(asu_WaitAck())
	{
		return fsm_rt_err_asu_NoAck;
	}
	asu_SendByte((uint8_t)(RegAddr));	//寄存器地址低字节
	asu_WaitAck();					
	asu_SendByte((uint8_t)(RegAddr>>8));	//寄存器地址高字节
	asu_WaitAck();					
	asu_Stop();		
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ASU_RD);	//设备地址+读
	asu_WaitAck();	
	for(num = 0;num<length;num++)
	{
		ReadData[num] = asu_ReadByte();
		if (num < (length -1)){asu_Ack();}			
		else{asu_NAck();}	//最后一个读数据，主机不反回ACK						
	}
	asu_Stop();	
	
	return fsm_rt_cpl;
}

fsm_rt_t apple_read_addr8(uint8_t DevAddr,uint8_t *ReadData,uint8_t RegAddr,uint16_t length)
{
	uint16_t num = 0x00;
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ ASU_WR);	//设备地址 + 写
	if(asu_WaitAck())
	{
		return fsm_rt_err_asu_NoAck;
	}
	asu_SendByte(RegAddr);	//寄存器地址低字节
	asu_WaitAck();					
	 		
	asu_Stop();		
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ASU_RD);	//设备地址+读
	asu_WaitAck();	
	for(num = 0;num<length;num++)
	{
		ReadData[num] = asu_ReadByte();
		if (num < (length -1)){asu_Ack();}			
		else{asu_NAck();}	//最后一个读数据，主机不反回ACK						
	}
	asu_Stop();	
	
	return fsm_rt_cpl;
}

fsm_rt_t apple_write_addr8(uint8_t DevAddr,uint8_t *WriteData,uint8_t RegAddr,uint16_t length)
{
	uint16_t num = 0x00;
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ ASU_WR);	//设备地址 + 写
	if(asu_WaitAck())
	{
		return fsm_rt_err_asu_NoAck;
	}
	asu_SendByte(RegAddr);	//寄存器地址低字节
	asu_WaitAck();					
	 		
	 
	for(num = 0;num<length;num++)
	{
		asu_SendByte(WriteData[num]);
		asu_WaitAck();					
	}
	asu_Stop();	
	
	return fsm_rt_cpl;
}


fsm_rt_t apple_write_addr16(uint8_t DevAddr,uint8_t *WriteData,uint16_t RegAddr,uint16_t length)
{
	uint16_t num = 0x00;
	
	asu_Start();
	asu_SendByte((DevAddr<<1)+ ASU_WR);	//设备地址 + 写
	if(asu_WaitAck())
	{
		return fsm_rt_err_asu_NoAck;
	}
	asu_SendByte(RegAddr);	//寄存器地址低字节
    asu_WaitAck();
    asu_SendByte((RegAddr>>8));	//寄存器地址低字节
	asu_WaitAck();					
	 		
	
	for(num = 0;num<length;num++)
	{
		asu_SendByte(WriteData[num]);
		asu_WaitAck();					
	}
	asu_Stop();	
	
	return fsm_rt_cpl;
}
