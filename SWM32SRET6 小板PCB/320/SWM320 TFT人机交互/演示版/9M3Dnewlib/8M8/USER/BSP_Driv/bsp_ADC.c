#include "bsp_adc.h"

void bsp_ADC02_Init(void)
{
	ADC_InitStructure ADC_initStruct;
//	PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC0_IN4, 0);	//PA.12 => ADC.CH4
	
	ADC_initStruct.clk_src = ADC_CLKSRC_VCO_DIV64;		// ADC转换时钟源 
	ADC_initStruct.clk_div = 25;
	ADC_initStruct.pga_gain = ADC_IN_RANGE_3V3;
	ADC_initStruct.channels = ADC_CH2;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.trig_src = ADC_TRIGSRC_SW;
	ADC_initStruct.Continue = 0;						// 非连续模式，即单次模式
	ADC_initStruct.EOC_IEn = 0;	
	ADC_initStruct.OVF_IEn = 0;
	ADC_initStruct.HFULL_IEn = 0;
	ADC_initStruct.FULL_IEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);					// 配置ADC
	
	ADC_Open(ADC0);										// 使能ADC
}

