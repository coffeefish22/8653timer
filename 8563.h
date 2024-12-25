#include "stm32f10x.h"
#include <rtthread.h>

#define SCL_H() GPIO_SetBits(GPIOC, GPIO_Pin_3)
#define SCL_L() GPIO_ResetBits(GPIOC, GPIO_Pin_3)
#define SDA_H() GPIO_SetBits(GPIOC, GPIO_Pin_2)
#define SDA_L() GPIO_ResetBits(GPIOC, GPIO_Pin_2) 
//返回0，和1
#define Read_SDA() GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)
u8 IIC_ReadByte(void);
void IIC_WriteByte(u8 byte);
void IIC_WaitAck(void); 
void IIC_Stop(void);
void IIC_Start(void);
void IIC_Init(void);
void I2C_Ack(void);
void I2C_NoAck(void); 
//取回八个字节的数据:秒，分，时，天，星期，月份，年份。
#define ReadCode 0xa3
#define WriteCode 0xa2
void PCF8563_Init(void);
u8 PCF8563_ReaDAdress(u8 Adress);
void PCF8563_WriteAdress(u8 Adress, u8 DataTX);//取回7个字节的数据:秒，分，时，天，星期，月份，年份。
//全局变量
extern u8 PCF8563_Time[7];
void PCF8563_ReadTimes(void);
//在CLKOUT上定时1S输出一个下降沿脉冲
void PCF8563_CLKOUT_1s(void);





