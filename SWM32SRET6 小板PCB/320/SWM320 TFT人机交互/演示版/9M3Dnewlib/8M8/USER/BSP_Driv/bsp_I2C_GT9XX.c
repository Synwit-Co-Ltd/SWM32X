
#include "bsp_I2C_GT9XX.h"
#include "string.h" 
#include "bsp_SysTick.h"

void I2C_Mst_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN4, FUNMUX0_I2C0_SCL, 1);	// GPIOA.4配置为I2C0 SCL引脚
	PORT->PORTA_PULLU |= (1 << PIN4);				// 必须使能上拉，用于模拟开漏
	PORT_Init(PORTA, PIN5, FUNMUX1_I2C0_SDA, 1);	// GPIOA.5配置为I2C0 SDA引脚
	PORT->PORTA_PULLU |= (1 << PIN5);				// 必须使能上拉，用于模拟开漏
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.MstClk = 10000;
	I2C_initStruct.MstIEn = 0;
	I2C_Init(I2C0, &I2C_initStruct);
	
	I2C_Open(I2C0);
}

void bsp_GT9XX_InitRst(void)
{
	// 第一阶段设置端口，并拉低两个端口
	GPIO_Init(GPIO_PORT_GT_RST, GPIO_PIN_GT_RST, 1, 0, 0);				// 复位脚 输出
	GPIO_Init(GPIO_PORT_GT_INT, GPIO_PIN_GT_INT, 1, 0, 0);				// 中断脚 
	GPIO_ClrBit(GPIOC, PIN3);
	GPIO_ClrBit(GPIOC, PIN2);						// 拉低两个端口的电平，准备复位
	SWM_Delay(10);
	// 第二阶段复位芯片
	GPIO_SetBit(GPIOC, PIN3);						// 拉高开始复位芯片
	SWM_Delay(10);
	// 第三阶段设置中断引脚为 中断功能
	GPIO_Init(GPIOC, PIN2, 0, 0, 0);
	EXTI_Init(GPIOC, PIN2, EXTI_RISE_EDGE);			// 上升沿触发中断
	I2C_Mst_Init();									// 硬件IIC端口初始化
	SWM_Delay(10);	
}

void GT9XX_IRQEnable(void)
{
	NVIC_EnableIRQ(GPIOC2_IRQn);					// 使能GPIOC.2端口中断
	EXTI_Open(GPIOC, PIN2);							// 打开外部中断		

	NVIC_SetPriority(GPIOC2_IRQn, 1);
}

void GT9XX_IRQDisable(void)
{
	NVIC_DisableIRQ(GPIOC2_IRQn);					// 禁止GPIOC.2端口中断
	EXTI_Close(GPIOC, PIN2);						// 关闭外部中断	
	
	GPIO_Init(GPIOC, PIN2, 1, 0, 1);				// 回到普通输出端口
	GPIO_ClrBit(GPIOC, PIN2);
}

/********************************************************************************************************************** 
* 函数名称:	bsp_WrNumByte()
* 功能说明:	IIC写Num个字节 
* 输    入: reg 寄存器地址,*p数据，WrByteNum写入的数据个数
* 输    出: 0,正常     其他,失败
* 注意事项: 
**********************************************************************************************************************/
uint8_t bsp_GT9XX_WrReg(uint8_t IdAddr,uint8_t *p,uint8_t WrByteNum)
{
    I2C0->MSTDAT = IdAddr | 0; 						// 发送器件地址+写命令
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |		
				   (1 << I2C_MSTCMD_WR_Pos);		// 发送起始位和从机地址
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk) ;		// 等待发送完成
	if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)			// 判断是否接收到应答位
	{
		return 1;
	}
	
	for(uint8_t iCount = 0;iCount < WrByteNum; iCount++){
		I2C0->MSTDAT = *p++;						// 写入数据
		I2C0->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
		while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
		if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)		// 读应答位
		{
			return 1;
		}
	}
	
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STO_Pos);
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
	if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)
	{
		return 1;
	}	
    return 0;
}
/********************************************************************************************************************** 
* 函数名称:	bsp_RdNumByte()
* 功能说明:	IIC读Num个字节 
* 输    入: addr器件IIC地址,*p数据，WrByteNum写入的数据个数
* 输    出: 0,正常     其他,失败
* 注意事项: 
**********************************************************************************************************************/
uint8_t bsp_GT9XX_RdReg(uint8_t IdAddr,uint8_t *p,uint8_t RdByteNum)
{
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |
				   (1 << I2C_MSTCMD_WR_Pos);				// 发送Restart
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
	// 以上两句为该工程的特殊情况，重新发送一次启动信号
	I2C0->MSTDAT = IdAddr | 1; 						// 发送器件地址+读命令
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |
				   (1 << I2C_MSTCMD_WR_Pos);				// 发送Restart
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
	if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)
	{
		return 1;
	}	
	// 注意，这里只需循环（RdByteNum - 1）次，因为最后一个字节不需要应答
	for(uint8_t iCount = 0;iCount < RdByteNum - 1; iCount++){
		I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) |
					   (0 << I2C_MSTCMD_ACK_Pos);	// 产生ACK信号
		while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);	// 等待接收完成
		*p++ = I2C0->MSTDAT;
	}	
	
	I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos)  |
				   (1 << I2C_MSTCMD_ACK_Pos) |
				   (1 << I2C_MSTCMD_STO_Pos);		// 发送NACK响应,完成后发送STOP
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);		// 等待接收完成	
    *p++ = I2C0->MSTDAT;							// 最后一个字节，先发送命令，再读取即可
	return 0;  
}



