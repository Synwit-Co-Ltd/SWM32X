#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include "SWM320.h"

/*设定使用的电容屏IIC设备地址*/
#define GTP_ADDRESS            0xBA  

/*复位引脚*/
#define GPIO_PORT_GT_RST                GPIOC
#define GPIO_PIN_GT_RST                 PIN3
/*中断引脚*/
#define GPIO_PORT_GT_INT                GPIOC
#define GPIO_PIN_GT_INT                 PIN2

void bsp_GT9XX_InitRst(void);

void GT9XX_IRQEnable(void);

void GT9XX_IRQDisable(void);

uint8_t bsp_GT9XX_WrReg(uint8_t IdAddr,uint8_t *p,uint8_t WrByteNum);

uint8_t bsp_GT9XX_RdReg(uint8_t IdAddr,uint8_t *p,uint8_t RdByteNum);

#endif 
