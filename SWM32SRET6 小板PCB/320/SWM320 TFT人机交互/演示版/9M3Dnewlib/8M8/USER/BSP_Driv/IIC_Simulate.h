
#ifndef __IIC_SIMULATE_H
#define __IIC_SIMULATE_H

#include "SWM3200.h"

//#define  PIN_SCL			PIN4				// PA4 IIC的时钟引脚
//#define  PIN_SDA			PIN5				// PA5 IIC的数据引脚
//#define  PORT_IIC			GPIOA				// IIC 的端口

//#define  SDA_Output()		GPIO_Init(PORT_IIC, PIN_SDA, 1, 1, 0)
//#define  SDA_Input()		GPIO_Init(PORT_IIC, PIN_SDA, 0, 1, 0)

//#define  I2C_SCL_SET() 		GPIO_SetBit(PORT_IIC,PIN_SCL)
//#define  I2C_SCL_RESET() 	GPIO_ClrBit(PORT_IIC,PIN_SCL)
//#define  I2C_SDA_SET() 		GPIO_SetBit(PORT_IIC,PIN_SDA)
//#define  I2C_SDA_RESET() 	GPIO_ClrBit(PORT_IIC,PIN_SDA)

//#define  Rd_SDA()  			GPIO_GetBit(PORT_IIC,PIN_SDA)

#define  PIN_SCL			PIN0				// PA4 IIC的时钟引脚
#define  PIN_SDA			PIN1				// PA5 IIC的数据引脚

#define  PORT_SDA			GPIOC				// IIC 的端口
#define  PORT_SCL			GPIOB				// IIC 的端口

#define  SDA_Output()		GPIO_Init( PORT_SDA, PIN_SDA, 1, 1, 0)
#define  SDA_Input()		GPIO_Init( PORT_SDA, PIN_SDA, 0, 1, 0)

#define  I2C_SCL_SET() 		GPIO_SetBit(PORT_SCL,PIN_SCL)
#define  I2C_SCL_RESET() 	GPIO_ClrBit(PORT_SCL,PIN_SCL)
#define  I2C_SDA_SET() 		GPIO_SetBit(PORT_SDA,PIN_SDA)
#define  I2C_SDA_RESET() 	GPIO_ClrBit(PORT_SDA,PIN_SDA)

#define  Rd_SDA()  			GPIO_GetBit(PORT_SDA,PIN_SDA)

#define  IIC_WRITE 0x00
#define  IIC_READ  0x01

void IIC_GPIO_Init(void);

void IIC_Delay5US(void);

void IIC_Start(void);

void IIC_Stop(void);

void IIC_Ack(void);

void IIC_Nack(void);

uint8_t IIC_RdAck(void);

uint8_t OutputOneByte(void);

void InputOneByte(uint8_t uByteVal);

void IIC_WrDatToAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t ucLenVal);

void IIC_RdDatFromAdd(uint8_t uDevID, uint8_t uStaAddVal, uint8_t *p, uint8_t uiLenVal);

#endif

