
#include "IIC_Simulate.h"

void IIC_GPIO_Init(void)
{
	GPIO_Init(PORT_SDA, PIN_SDA, 1, 1, 0);
	GPIO_Init(PORT_SCL, PIN_SCL, 1, 1, 0);
	I2C_SCL_SET();
	I2C_SDA_SET();
}

void IIC_Delay5US(void)
{
	uint8_t delay;
	delay = 125;			/*Oscilloscope measurement  34*/
	while(delay)
		delay--;
}

/* ***************************************************** */
// 函数名称：IIC_Start()
// 函数功能：IIC起动
// 入口参数：无
// 出口参数：无
/* ***************************************************** */
void IIC_Start(void)
{
	SDA_Output();
	I2C_SDA_SET();
	IIC_Delay5US();
	I2C_SCL_SET();
	IIC_Delay5US();
	I2C_SDA_RESET();
	IIC_Delay5US();
}
/* ***************************************************** */
// 函数名称：IIC_Stop()
// 函数功能：IIC停止
// 入口参数：无
// 出口参数：无
/* ***************************************************** */
void IIC_Stop(void)                     
{
	SDA_Output();
	I2C_SDA_RESET();
	IIC_Delay5US();
	I2C_SCL_SET();
	IIC_Delay5US();
	I2C_SDA_SET();
}
/* ***************************************************** */
// 函数名称：IIC_Ack()
// 函数功能：IIC应答
// 入口参数：无
// 出口参数：无
/* ***************************************************** */
void IIC_Ack(void)                      
{ 
	SDA_Output();
	I2C_SCL_RESET();		// 为产生脉冲准备
	I2C_SDA_RESET();		// 产生应答信号
	IIC_Delay5US();			// 延时你懂得
	I2C_SCL_SET();
	IIC_Delay5US();  
	I2C_SCL_RESET();
	IIC_Delay5US();			// 产生高脉冲 
	I2C_SDA_SET();			// 释放总线
}
/* ***************************************************** */
// 函数名称：IIC_Nack()
// 函数功能：IIC不应答
// 入口参数：无
// 出口参数：无
/* ***************************************************** */
void IIC_Nack(void)                    
{   
	SDA_Output();
	I2C_SDA_SET();
	I2C_SCL_RESET();
	IIC_Delay5US();      
	I2C_SCL_SET();
	IIC_Delay5US();      
	I2C_SCL_RESET();
}
/* ***************************************************** */
// 函数名称：IIC_RdAck()
// 函数功能：读IIC应答
// 入口参数：无
// 出口参数：是否应答真值
/* ***************************************************** */
uint8_t IIC_RdAck(void)                      
{ 
	uint8_t AckFlag;
	uint8_t uiVal = 0;
	SDA_Input();
	I2C_SCL_RESET();
	IIC_Delay5US();    
	I2C_SDA_SET();
	I2C_SCL_SET();
	IIC_Delay5US();
	while((Rd_SDA()) && (uiVal < 255))
	{
		uiVal ++;
		AckFlag = Rd_SDA();
	} 
	AckFlag = Rd_SDA();
	I2C_SCL_RESET();  
	return AckFlag;		// 应答返回：0;不应答返回：1
}
/* ***************************************************** */
// 函数名称：OutputOneByte()
// 函数功能：从IIC器件中读出一个字节
// 入口参数：无
// 出口参数：读出的一个字节（uByteVal）
/* ***************************************************** */
uint8_t OutputOneByte(void)        
{
	uint8_t uByteVal = 0;
	uint8_t iCount;
	SDA_Input();
	I2C_SDA_SET();
	for (iCount = 0;iCount < 8;iCount++)
	{
		I2C_SCL_RESET();
		IIC_Delay5US();
		I2C_SCL_SET();
		IIC_Delay5US(); 
		uByteVal <<= 1;
		if(Rd_SDA())
			uByteVal |= 0x01;
	}         
	I2C_SCL_RESET();
	return(uByteVal);
}
/* ***************************************************** */
// 函数名称：InputOneByte()
// 函数功能：向IIC器件写入一个字节
// 入口参数：待写入的一个字节（uByteVal）
// 出口参数：无
/* ***************************************************** */
void InputOneByte(uint8_t uByteVal)
{
    uint8_t iCount;
	SDA_Output();
    for(iCount = 0;iCount < 8;iCount++)
	{  
		I2C_SCL_RESET();
		IIC_Delay5US();			
		if((uByteVal & 0x80) >> 7)
			I2C_SDA_SET();
		else
			I2C_SDA_RESET();
		IIC_Delay5US();         
		I2C_SCL_SET();
		IIC_Delay5US();
		uByteVal <<= 1;
    } 
	I2C_SCL_RESET();   	  
}
/* ***************************************************** */
// 函数名称：IIC_WrDevAddAndDatAdd()
// 函数功能：向IIC器件写入器件和数据地址
// 入口参数：器件地址（uDevAdd），数据地址（uDatAdd）
// 出口参数：写入是否成功真值
/* ***************************************************** */
void IIC_WrDevAddAndDatAdd(uint8_t uDevAdd,uint8_t uDatAdd)
{
	IIC_Start();			// 发送开始信号
	InputOneByte(uDevAdd);	// 输入器件地址
	IIC_RdAck();  			// 读应答信号
	InputOneByte(uDatAdd);	// 输入数据地址
	IIC_RdAck();			// 读应答信号        
}
/* ***************************************************** */
// 函数名称：IIC_WrDatToAdd()
// 函数功能：向IIC器件写数据
// 入口参数：器件ID(uDevID)、数据存储起始地址(uStaAddVal)
//			 待存数据(*p)、连续存储数据的个数(uiLenVal)
// 出口参数：无
/* ***************************************************** */
void IIC_WrDatToAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t ucLenVal)
{      
	uint8_t iCount;	  
	IIC_WrDevAddAndDatAdd(uDevID | IIC_WRITE,uStaAddVal);  
	// IIC_WRITE 为写命令后缀符                                           
	for(iCount = 0;iCount < ucLenVal;iCount++)
	{
		InputOneByte(*p++);
		IIC_RdAck(); 
	}
	IIC_Stop(); 		
}
/* ***************************************************** */
// 函数名称：IIC_RdDatFromAdd()
// 函数功能：向IIC器件读数据
// 入口参数：器件ID(uDevID)、数据存储地址(uStaAddVal)
//			 待存数据(*p)、连续存储数据的个数(uiLenVal)
// 出口参数：无
/* ***************************************************** */
void IIC_RdDatFromAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t uiLenVal)
{                                                    
	uint8_t iCount;
	IIC_WrDevAddAndDatAdd(uDevID | IIC_WRITE,uStaAddVal);  
	IIC_Start();
	InputOneByte(uDevID | IIC_READ);// IIC_READ 为写命令后缀符
	IIC_RdAck();     
	for(iCount = 0;iCount < uiLenVal;iCount++)
	{
		*p++ = OutputOneByte();
	   	if(iCount != (uiLenVal - 1))
			IIC_Ack();      	   
	}       
	IIC_Nack();        
	IIC_Stop();	  
}
