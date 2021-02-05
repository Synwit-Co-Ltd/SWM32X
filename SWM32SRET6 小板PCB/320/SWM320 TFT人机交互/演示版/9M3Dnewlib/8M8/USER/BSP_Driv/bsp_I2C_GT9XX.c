
#include "bsp_I2C_GT9XX.h"
#include "string.h" 
#include "bsp_SysTick.h"

void I2C_Mst_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN4, FUNMUX0_I2C0_SCL, 1);	// GPIOA.4����ΪI2C0 SCL����
	PORT->PORTA_PULLU |= (1 << PIN4);				// ����ʹ������������ģ�⿪©
	PORT_Init(PORTA, PIN5, FUNMUX1_I2C0_SDA, 1);	// GPIOA.5����ΪI2C0 SDA����
	PORT->PORTA_PULLU |= (1 << PIN5);				// ����ʹ������������ģ�⿪©
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.MstClk = 10000;
	I2C_initStruct.MstIEn = 0;
	I2C_Init(I2C0, &I2C_initStruct);
	
	I2C_Open(I2C0);
}

void bsp_GT9XX_InitRst(void)
{
	// ��һ�׶����ö˿ڣ������������˿�
	GPIO_Init(GPIO_PORT_GT_RST, GPIO_PIN_GT_RST, 1, 0, 0);				// ��λ�� ���
	GPIO_Init(GPIO_PORT_GT_INT, GPIO_PIN_GT_INT, 1, 0, 0);				// �жϽ� 
	GPIO_ClrBit(GPIOC, PIN3);
	GPIO_ClrBit(GPIOC, PIN2);						// ���������˿ڵĵ�ƽ��׼����λ
	SWM_Delay(10);
	// �ڶ��׶θ�λоƬ
	GPIO_SetBit(GPIOC, PIN3);						// ���߿�ʼ��λоƬ
	SWM_Delay(10);
	// �����׶������ж�����Ϊ �жϹ���
	GPIO_Init(GPIOC, PIN2, 0, 0, 0);
	EXTI_Init(GPIOC, PIN2, EXTI_RISE_EDGE);			// �����ش����ж�
	I2C_Mst_Init();									// Ӳ��IIC�˿ڳ�ʼ��
	SWM_Delay(10);	
}

void GT9XX_IRQEnable(void)
{
	NVIC_EnableIRQ(GPIOC2_IRQn);					// ʹ��GPIOC.2�˿��ж�
	EXTI_Open(GPIOC, PIN2);							// ���ⲿ�ж�		

	NVIC_SetPriority(GPIOC2_IRQn, 1);
}

void GT9XX_IRQDisable(void)
{
	NVIC_DisableIRQ(GPIOC2_IRQn);					// ��ֹGPIOC.2�˿��ж�
	EXTI_Close(GPIOC, PIN2);						// �ر��ⲿ�ж�	
	
	GPIO_Init(GPIOC, PIN2, 1, 0, 1);				// �ص���ͨ����˿�
	GPIO_ClrBit(GPIOC, PIN2);
}

/********************************************************************************************************************** 
* ��������:	bsp_WrNumByte()
* ����˵��:	IICдNum���ֽ� 
* ��    ��: reg �Ĵ�����ַ,*p���ݣ�WrByteNumд������ݸ���
* ��    ��: 0,����     ����,ʧ��
* ע������: 
**********************************************************************************************************************/
uint8_t bsp_GT9XX_WrReg(uint8_t IdAddr,uint8_t *p,uint8_t WrByteNum)
{
    I2C0->MSTDAT = IdAddr | 0; 						// ����������ַ+д����
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |		
				   (1 << I2C_MSTCMD_WR_Pos);		// ������ʼλ�ʹӻ���ַ
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk) ;		// �ȴ��������
	if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)			// �ж��Ƿ���յ�Ӧ��λ
	{
		return 1;
	}
	
	for(uint8_t iCount = 0;iCount < WrByteNum; iCount++){
		I2C0->MSTDAT = *p++;						// д������
		I2C0->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
		while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
		if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)		// ��Ӧ��λ
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
* ��������:	bsp_RdNumByte()
* ����˵��:	IIC��Num���ֽ� 
* ��    ��: addr����IIC��ַ,*p���ݣ�WrByteNumд������ݸ���
* ��    ��: 0,����     ����,ʧ��
* ע������: 
**********************************************************************************************************************/
uint8_t bsp_GT9XX_RdReg(uint8_t IdAddr,uint8_t *p,uint8_t RdByteNum)
{
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |
				   (1 << I2C_MSTCMD_WR_Pos);				// ����Restart
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
	// ��������Ϊ�ù��̵�������������·���һ�������ź�
	I2C0->MSTDAT = IdAddr | 1; 						// ����������ַ+������
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |
				   (1 << I2C_MSTCMD_WR_Pos);				// ����Restart
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);
	if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)
	{
		return 1;
	}	
	// ע�⣬����ֻ��ѭ����RdByteNum - 1���Σ���Ϊ���һ���ֽڲ���ҪӦ��
	for(uint8_t iCount = 0;iCount < RdByteNum - 1; iCount++){
		I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) |
					   (0 << I2C_MSTCMD_ACK_Pos);	// ����ACK�ź�
		while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);	// �ȴ��������
		*p++ = I2C0->MSTDAT;
	}	
	
	I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos)  |
				   (1 << I2C_MSTCMD_ACK_Pos) |
				   (1 << I2C_MSTCMD_STO_Pos);		// ����NACK��Ӧ,��ɺ���STOP
	while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk);		// �ȴ��������	
    *p++ = I2C0->MSTDAT;							// ���һ���ֽڣ��ȷ�������ٶ�ȡ����
	return 0;  
}



