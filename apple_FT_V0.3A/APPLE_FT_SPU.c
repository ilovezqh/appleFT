/*
****************************************************************************************************
* 文件：	apple_ft_chip
* 作者：	plxc
* 版本：	v0.3
* 日期：		
* 简介：	
1）通道：配置13通道，绑定sensor0-12。
2）功能：在每次采样完成后，使用UART_TX上传1个BYTE的触摸通道数量数据。
				上位机在读取UART数据后，发起I2C通信，通过DEV8I2C读取2个BYTE的触摸通道分布数据。
3）触摸通道数量=多少个通道被按下；触摸通道分布=触摸通道对应的BIT置1，保存地址在0x2F00，供DEV8I2C读取。
4）DEV8I2C配置：从机7位地址为0x13，基地址为0x2F。
5) 主机加读13通道的校准电容值。在某个区间内为合理，如（-110~+110）。
****************************************************************************************************
*/

//CHIP
/*****************************************************************************************************
* 简介：保存按键触摸信息
* 注意：按下数量及按下通道分布都保存最大值
*****************************************************************************************************/
uint8_t g_pressing_ch_count =0;
uint16_t pressing_ch_bit =0;
xdata uint16_t g_pressing_ch_bit _at_ 0x2f00;
//
void Process_GetTouchInfo(void)
{
	uint8_t i,ch_cnt =0;
	uint16_t ch_bit=0;
	for(i=0;i<CHN_TOTAL;i++)
	{
		if((SPU_GetChStatus(i)&PRESSI_MSK) == PRESSI_MSK)
		{
			ch_cnt++;
			ch_bit |= (1<<i);
		}
	}
	//保存本次最大触摸数量
	if(g_pressing_ch_count < ch_cnt)
	{
		g_pressing_ch_count = ch_cnt;
	}
	if(pressing_ch_bit < ch_bit)
	{
		pressing_ch_bit = ch_bit;	
	}
	g_pressing_ch_bit = pressing_ch_bit;
}


//HOST
//pattern 10
/******************************************************************************
*功能：				
*注意：
*过程：
测试机使能芯片端自测程序运行；
在芯片端程序运行后，发出触摸控制信号；
在确定时间内监测UART_TX端口，如该时间段内收到UART数据，再启动DEV8I2C读取第二路触摸状态；
比较设计触摸状态及收到的两路触摸状态，匹配则测试通过。
加测读取校准电容并判断大小，以此来确定各通道是否被正确校准。
//芯片端程序运行自测程序，通过UART_TX输出触摸通道数量，响应DEV8I2C读取命令发送触摸通道分布。
*******************************************************************************/
uint8_t cali_cap[13]={0};
uint8_t APPLE_FT_SPU(void)
{	
	uint8_t temp;
	uint8_t uart_pass=0,spu_pass=0;
	
	uint16_t cnt =0;	
	
	g_pressing_ch_count =0;
	g_pressing_ch_bit =0;
	
	//代码下载完成
	if(APPLE_FT_DOWNLOAD())
		return TEST_DOWNLOAD_FAIL;
	else
	{
		//芯片开始单次工作,复位后保持在UPGRADE状态
		set_apple_soft_work();
		//等待芯片自测程序初始化等过程完成
		Delayms(10);

		//发出触摸控制信号
		press_down_apple();
		//等待芯片端完成检测
		Delayms(10);		
		//预清串口中断标志，等待系统稳定后的中断标志	
		flag_usart_received =0;
		
		//最多200ms时间段内等待串口中断及接收数据
		cnt =200;
		while(--cnt)
		{
			Delayms(1);
			if(flag_usart_received)
			{
				g_pressing_ch_count =UART_GetByte();
				flag_usart_received =0;//清标志等待下一次标志				
				//通过DEV8I2C读状态			
				i2c_read_register8(I2C_DEV8_ADDR,read_data,0,2);
				g_pressing_ch_bit = ((uint16_t)read_data[0]<<8) | read_data[1];					

				if(uart_pass == 0) //有接收说明UART正常
					uart_pass =1;
				if(g_pressing_ch_count == 8)//全部sensor都触发了
				{
					spu_pass =1;
					break;
				}			
			}		
		}
		
		//等待时间段内没有串口中断则判UART错误
		if(uart_pass == 0)
			return TEST_UART_FAIL;
		
		//多次接收数据但始终不等于0x08,说明有sensor没有被正确触发
		if(spu_pass == 0)
			return TEST_SPU_FAIL;	

		//释放触摸控制信号
		press_up_apple();
		
		//读取并判断校准电容是否在范围内
		for(cnt=0;cnt<13;cnt++)
		{
			i2c_read_register(I2C_DEV16_ADDR,&temp,0xC821+cnt*18,1);
			cali_cap[cnt] = temp;
			temp += 128;
			if((temp <= 20) || ( temp >= 238))//
			{
				return TEST_CALICAP_FAIL;
			}		
		}
		
		//比较DEV8I2C状态数据
		//匹配则测试通过，不匹配则报I2C错误		
		if(g_pressing_ch_bit == 0x1e0f)//XXX1 1110 0000 1111
			return TEST_PASS;
		else
			return TEST_DEV8I2C_FAIL;	
	}	
}