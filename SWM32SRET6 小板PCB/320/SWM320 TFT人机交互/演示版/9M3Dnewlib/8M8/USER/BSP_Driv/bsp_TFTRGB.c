
#include "bsp_tftrgb.h"

//注意：如果RGB频率与内存频率不匹配，会花屏
void bsp_RGBInit(void)
{
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOB, PIN0, 1, 0, 0);	// 背光控制	高电平有效
	GPIO_SetBit(GPIOB, PIN0);			// 点亮背光
	GPIO_Init(GPIOC, PIN1, 1, 0, 0);	// DISP显示控制端 高电平有效
	GPIO_SetBit(GPIOC, PIN1);			// 打开显示
	
	PORT->PORTN_SEL0 = 0xAAAAAAAA;		//GPION.0～15  	LCD_DATA0～15
	PORT->PORTN_SEL1 = 0xAA;			//GPION.16～19为LCD_CLK、LCD_VSYNC、LCD_DE、LCD_HSYNC
	
	LCD_initStruct.Interface = LCD_INTERFACE_RGB;
	LCD_initStruct.Dir = LCD_DIR_LANDSCAPE;
	LCD_initStruct.HnPixel = 800;		//水平800像素
	LCD_initStruct.VnPixel = 480;		//垂直480像素
	LCD_initStruct.Hfp = 5;
	LCD_initStruct.Hbp = 40;
	LCD_initStruct.Vfp = 8;
	LCD_initStruct.Vbp = 8;
	LCD_initStruct.ClkDiv = LCD_CLKDIV_8;
	LCD_initStruct.ClkAlways = 0;
	LCD_initStruct.SamplEdge = LCD_SAMPLEDGE_RISE;
	LCD_initStruct.HsyncWidth = LCD_HSYNC_1DOTCLK;
	LCD_initStruct.IntEOTEn = 1;
	LCD_Init(LCD, &LCD_initStruct);
}

