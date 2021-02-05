
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
// �������ƣ�IIC_Start()
// �������ܣ�IIC��
// ��ڲ�������
// ���ڲ�������
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
// �������ƣ�IIC_Stop()
// �������ܣ�IICֹͣ
// ��ڲ�������
// ���ڲ�������
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
// �������ƣ�IIC_Ack()
// �������ܣ�IICӦ��
// ��ڲ�������
// ���ڲ�������
/* ***************************************************** */
void IIC_Ack(void)                      
{ 
	SDA_Output();
	I2C_SCL_RESET();		// Ϊ��������׼��
	I2C_SDA_RESET();		// ����Ӧ���ź�
	IIC_Delay5US();			// ��ʱ�㶮��
	I2C_SCL_SET();
	IIC_Delay5US();  
	I2C_SCL_RESET();
	IIC_Delay5US();			// ���������� 
	I2C_SDA_SET();			// �ͷ�����
}
/* ***************************************************** */
// �������ƣ�IIC_Nack()
// �������ܣ�IIC��Ӧ��
// ��ڲ�������
// ���ڲ�������
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
// �������ƣ�IIC_RdAck()
// �������ܣ���IICӦ��
// ��ڲ�������
// ���ڲ������Ƿ�Ӧ����ֵ
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
	return AckFlag;		// Ӧ�𷵻أ�0;��Ӧ�𷵻أ�1
}
/* ***************************************************** */
// �������ƣ�OutputOneByte()
// �������ܣ���IIC�����ж���һ���ֽ�
// ��ڲ�������
// ���ڲ�����������һ���ֽڣ�uByteVal��
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
// �������ƣ�InputOneByte()
// �������ܣ���IIC����д��һ���ֽ�
// ��ڲ�������д���һ���ֽڣ�uByteVal��
// ���ڲ�������
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
// �������ƣ�IIC_WrDevAddAndDatAdd()
// �������ܣ���IIC����д�����������ݵ�ַ
// ��ڲ�����������ַ��uDevAdd�������ݵ�ַ��uDatAdd��
// ���ڲ�����д���Ƿ�ɹ���ֵ
/* ***************************************************** */
void IIC_WrDevAddAndDatAdd(uint8_t uDevAdd,uint8_t uDatAdd)
{
	IIC_Start();			// ���Ϳ�ʼ�ź�
	InputOneByte(uDevAdd);	// ����������ַ
	IIC_RdAck();  			// ��Ӧ���ź�
	InputOneByte(uDatAdd);	// �������ݵ�ַ
	IIC_RdAck();			// ��Ӧ���ź�        
}
/* ***************************************************** */
// �������ƣ�IIC_WrDatToAdd()
// �������ܣ���IIC����д����
// ��ڲ���������ID(uDevID)�����ݴ洢��ʼ��ַ(uStaAddVal)
//			 ��������(*p)�������洢���ݵĸ���(uiLenVal)
// ���ڲ�������
/* ***************************************************** */
void IIC_WrDatToAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t ucLenVal)
{      
	uint8_t iCount;	  
	IIC_WrDevAddAndDatAdd(uDevID | IIC_WRITE,uStaAddVal);  
	// IIC_WRITE Ϊд�����׺��                                           
	for(iCount = 0;iCount < ucLenVal;iCount++)
	{
		InputOneByte(*p++);
		IIC_RdAck(); 
	}
	IIC_Stop(); 		
}
/* ***************************************************** */
// �������ƣ�IIC_RdDatFromAdd()
// �������ܣ���IIC����������
// ��ڲ���������ID(uDevID)�����ݴ洢��ַ(uStaAddVal)
//			 ��������(*p)�������洢���ݵĸ���(uiLenVal)
// ���ڲ�������
/* ***************************************************** */
void IIC_RdDatFromAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t uiLenVal)
{                                                    
	uint8_t iCount;
	IIC_WrDevAddAndDatAdd(uDevID | IIC_WRITE,uStaAddVal);  
	IIC_Start();
	InputOneByte(uDevID | IIC_READ);// IIC_READ Ϊд�����׺��
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
