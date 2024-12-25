

#include <rtthread.h>
#include "stm32f10x.h"
#include "globalval.h"
#include "myconfig.h"
#include <math.h> //provide sqrt() function
#include "soft_iic_core.h"
#include "8563.h" //create delay(us)
extern u32 temp_t;
extern u32	insign;
void delay_nus(int us)
{
	rt_hw_us_delay(us);
}

void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	//开漏输出，可以在不用改变成输入的情况下读取IO的电平
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	SCL_H();//拉高
	SDA_H();
}
void IIC_Start_8563(void)
{
	SDA_H();
	SCL_H();
	delay_nus(20);
	SDA_L();
	delay_nus(20);
	SDA_L();
	delay_nus(20);
}
void IIC_Stop_8563(void)
{
	SCL_L();
	delay_nus(20);
	SDA_L();
	delay_nus(20);
	SCL_H();
	delay_nus(20);
	SDA_H();
	delay_nus(20);
}

void IIC_WaitAck(void)
{ 
	u16 k;
	SCL_L();
	SDA_H();
	delay_nus(20);
	SCL_H();
	k=0;
	while((Read_SDA()!=0)&&(k<60000))
		k++;
	delay_nus(20);
	SCL_L();
	delay_nus(20);
}
void IIC_WriteByte(u8 byte)
{
	u8 i=0;
	for(i=0;i<8;i++)
	{
		SCL_L();
		delay_nus(20);
		if(byte&0x80)
		{
			SDA_H();
		}
		else
		{
			SDA_L();
		}
		delay_nus(20);
		SCL_H();
		delay_nus(20);
		byte<<=1;
	}
	SCL_L();
	delay_nus(20);
}		

u8 IIC_ReadByte(void)
{
	u8 i,ReadByte;
	SDA_H();
	for(i=0;i<8;i++)
	{
		ReadByte<<=1;
		SCL_L();
		delay_nus(20);
		SCL_H();
		delay_nus(20);
		if(Read_SDA())
		{
			ReadByte|=0x01;
		}
		else
		{
			ReadByte&=~(0x01);
		}
	}
	return ReadByte;
}

void I2C_Ack(void)
{
	SCL_L();
	delay_nus(20);
	SDA_L();
	delay_nus(20);
	SCL_H();
	delay_nus(20);
	SCL_L();
	delay_nus(20);
}

void I2C_NoAck(void)
{
	SCL_L();
	delay_nus(20);
	SDA_H();
	delay_nus(20);
	SCL_H();
	delay_nus(20);
	SCL_L();
	delay_nus(20);
}

u8 PCF8563_Time[7] ={50,59,23,31,6,12,12};
void PCF8563_Init(void)
{
	IIC_Init();
	//十进制码转换成BCD码
	PCF8563_Time[0]=((PCF8563_Time[0]/10)<<4)|(PCF8563_Time[0]%10);
	PCF8563_Time[1]=((PCF8563_Time[1]/10)<<4)|(PCF8563_Time[1]%10);
	PCF8563_Time[2]=((PCF8563_Time[2]/10)<<4)|(PCF8563_Time[2]%10);
	PCF8563_Time[3]=((PCF8563_Time[3]/10)<<4)|(PCF8563_Time[3]%10);
	PCF8563_Time[5]=((PCF8563_Time[5]/10<<4))|(PCF8563_Time[5]%10);
	PCF8563_Time[6]=((PCF8563_Time[6]/10<<4))|(PCF8563_Time[6]%10);
	PCF8563_CLKOUT_1s();
	PCF8563_WriteAdress(0x00,0x20);//禁止RTCsourceclock
	//初始化PCF8563的时间
	PCF8563_WriteAdress(0x02,PCF8563_Time[0]);
	PCF8563_WriteAdress(0x03,PCF8563_Time[1]);
	PCF8563_WriteAdress(0x04,PCF8563_Time[2]);
	PCF8563_WriteAdress(0x05,PCF8563_Time[3]);
	PCF8563_WriteAdress(0x06,PCF8563_Time[4]);
	PCF8563_WriteAdress(0x07,PCF8563_Time[5]);
	PCF8563_WriteAdress(0x08,PCF8563_Time[6]);
	PCF8563_WriteAdress(0x00,0x00);
	//EnableRTCsorceclock
}

u8 PCF8563_ReaDAdress(u8 Adress)
	{
	u8 ReadData;
	IIC_Start_8563();
	IIC_WriteByte(0xa2);
	IIC_WaitAck();
	IIC_WriteByte(Adress);
	IIC_WaitAck();
	IIC_Start_8563();
	IIC_WriteByte(0xa3);
	IIC_WaitAck();
	ReadData=IIC_ReadByte();
	IIC_Stop_8563();
	return ReadData;
}

void PCF8563_WriteAdress(u8 Adress,u8 DataTX)
{
	IIC_Start_8563();
	IIC_WriteByte(0xa2);
	IIC_WaitAck();
	IIC_WriteByte(Adress);
	IIC_WaitAck();
	IIC_WriteByte(DataTX);
	IIC_WaitAck();
	IIC_Stop_8563();
}	

void PCF8563_ReadTimes(void)
{
	IIC_Start_8563();
	IIC_WriteByte(0xa2);
	IIC_WaitAck();
	IIC_WriteByte(0x02);
	IIC_WaitAck();
	IIC_Start_8563();
	IIC_WriteByte(0xa3);
	IIC_WaitAck();
	PCF8563_Time[0]=IIC_ReadByte()&0x7f;
	I2C_Ack();
	PCF8563_Time[1]=IIC_ReadByte()&0x7f;
	I2C_Ack();
	PCF8563_Time[2]=IIC_ReadByte()&0x3f;
	I2C_Ack();
	PCF8563_Time[3]=IIC_ReadByte()&0x3f;
	I2C_Ack();
	PCF8563_Time[4]=IIC_ReadByte()&0x07;
	I2C_Ack();
	PCF8563_Time[5]=IIC_ReadByte()&0x1f;
	I2C_Ack();
	PCF8563_Time[6]=IIC_ReadByte();
	I2C_NoAck();
	IIC_Stop_8563();
}

void PCF8563_CLKOUT_1s(void)
{
	PCF8563_WriteAdress(0x01,0);//禁止定时器输出，闹铃输出
	PCF8563_WriteAdress(0x0e,0);//关闭定时器等等
	//PCF8563_WriteAdress(0x0e,0);//写入1
	PCF8563_WriteAdress(0x0d,0x83);//打开输出脉冲
}
static void cal_PCF8563_task(void* parameter)
{


	int sec=0;
	u8 bak_sec=0;
	static unsigned char InitOkFlag=0;
	static unsigned char ReadErrNum=0;/*读数据连续出错的次数*/
	
	while(RT_TRUE)
	{                    
		if(!InitOkFlag)
		{
			PCF8563_Init();
			InitOkFlag=1;
			ReadErrNum=0;
		}
		rt_thread_delay(100);	
		while(InitOkFlag)
		{
			PCF8563_ReadTimes();
			{
				if(PCF8563_Time[0]!=bak_sec)   //有秒发生改变
				{
					bak_sec=PCF8563_Time[0];

					if(second_select==1)
					{
						rt_sem_release(&sem_rtc);//通知实时时钟已经发生了变化
						insign=temp_t;
						temp_t=0;			
					}
					ReadErrNum=0;
				}
			}
			if(1)
			{
				ReadErrNum++;
				if(ReadErrNum>=1000)
				{
					ReadErrNum=0;
					/*如果连续5次读不到数据，下次读数据之前会重新初始化器件*/
					InitOkFlag=0;
					if(rtc_error_flag)
						second_select=2;
					else
						second_select=0;
				}
			}
			rt_thread_delay(10);		
		}
	}
}

void cal_PCF8563_init()
{
	rt_thread_t thread;

    thread = rt_thread_create(
                   "8563",
                   cal_PCF8563_task,
                   RT_NULL,
                   		512,
                   THREAD_CAL_8563_PRIORITY,15);
    if (thread != RT_NULL)
        rt_thread_startup(thread);
}




