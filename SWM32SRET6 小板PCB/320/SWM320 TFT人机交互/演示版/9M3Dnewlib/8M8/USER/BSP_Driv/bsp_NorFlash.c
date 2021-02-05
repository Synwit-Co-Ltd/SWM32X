#include "bsp_norflash.h"

void bsp_NorFlashInit(void)
{
	NORFL_InitStructure NORFL_InitStruct;
	
	PORT->PORTP_SEL0 = 0xAAAAAAAA;		//PP0-23 => ADDR0-23
	PORT->PORTP_SEL1 = 0x0000AAAA;
	
	PORT->PORTM_SEL0 = 0xAAAAAAAA;		//PM0-15 => DATA15-0
	PORT->PORTM_INEN = 0xFFFF;
	
	PORT->PORTM_SEL1 = 0xAAA;			//PM16 => OEN、PM17 => WEN、PM18 => NORFL_CSN、PM19 => SDRAM_CSN、PM20 => SRAM_CSN、PM21 => SDRAM_CKE
	
	NORFL_InitStruct.DataWidth = 16;
	NORFL_InitStruct.WELowPulseTime = 5;
	NORFL_InitStruct.OEPreValidTime = 12;
	NORFL_InitStruct.OperFinishIEn  = 0;
	NORFL_InitStruct.OperTimeoutIEn = 0;
	NORFL_Init(&NORFL_InitStruct);
}
