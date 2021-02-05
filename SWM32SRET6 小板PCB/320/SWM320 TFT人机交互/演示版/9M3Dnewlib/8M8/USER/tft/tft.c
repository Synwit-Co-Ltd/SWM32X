/**
 * @file disp.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
//#include "lv_vdb.h"
#include "lv_hal.h"
#include <string.h>
#include "../../lvgl/lv_core/lv_vdb.h"
#include "tft.h"
#include "SWM320.h"
#include "SWM320_sdram.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/

/*These 3 functions are needed by LittlevGL*/
static void tft_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
static void tft_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
static void tft_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
static void tft_drawpoint(int16_t x, int16_t y, uint32_t c);
static void tft_rgbinit(void);
static void tft_setlcd_srcadd(void);
static void tft_memoryinit(void);

static uint32_t LCD_BufferSHOW[LV_HOR_RES*LV_VER_RES * 2 / 4] __attribute__((at(SDRAMM_BASE))) = {0x00000000};
static uint32_t LCD_BufferGUI[LV_HOR_RES*LV_VER_RES * 2 / 4] __attribute__((at(SDRAMM_BASE + 0x3FC00))) = {0x00000000};
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize your display here ��ʼ����ʾ���� 
 */
void tft_init(void)
{
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);

	tft_memoryinit();
	tft_rgbinit();

	disp_drv.disp_fill = tft_fill;
	disp_drv.disp_map = tft_map;
	disp_drv.disp_flush = tft_flush;
	lv_disp_drv_register(&disp_drv);

	memset(LCD_BufferSHOW, 0x00, sizeof(LCD_BufferSHOW));
	memset(LCD_BufferGUI, 0x00, sizeof(LCD_BufferGUI));
	tft_setlcd_srcadd();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Flush a color buffer  ˢ����ɫ������
 * @param x1 left coordinate of the rectangle X1 ��������ʾ����������
 * @param x2 right coordinate of the rectangle X2 ��������ʾ����������
 * @param y1 top coordinate of the rectangle Y1���εĶ�����
 * @param y2 bottom coordinate of the rectangle Y2 ���εĵײ�����
 * @param color_p pointer to an array of colors ָ�� ָ��һ����ɫ����
 */
static void tft_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*Return if the area is out the screen �����������Ļ�� �򷵻�*/  
	if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > LV_HOR_RES - 1) return;
	if(y1 > LV_VER_RES - 1) return;

	/*Truncate the area to the screen �ض���Ļ���� */
	int32_t act_x1 = x1 < 0 ? 0 : x1;
	int32_t act_y1 = y1 < 0 ? 0 : y1;
	int32_t act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
	int32_t act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;
//	NVIC_DisableIRQ(LCD_IRQn);
//	while(LCD_IsBusy(LCD));

	int32_t x;
	int32_t y;

	for(y = act_y1; y <= act_y2; y++) {
		for(x = act_x1; x <= act_x2; x++) {
			tft_drawpoint(x, y, color_p->full);
			color_p++;
		}

		color_p += x2 - act_x2;
	}
//			NVIC_EnableIRQ(LCD_IRQn);
    /*IMPORTANT! It must be called to tell the system the flush is ready  ��Ҫ ������ô˺�������ϵͳˢ��׼������*/
    lv_flush_ready();
}

/**
 * Fill a rectangular area with a color ����ɫ�����ʾ�ĳ���������
 * @param x1 left coordinate of the rectangle x1�Ǿ��ε�������
 * @param x2 right coordinate of the rectangle  x2���ε�������
 * @param y1 top coordinate of the rectangle  y1���εĶ�����
 * @param y2 bottom coordinate of the rectangle y2���εĵײ�����
 * @param color fill color ��ɫ�����ɫ
 */
static void tft_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    /*Return if the area is out the screen�����������Ļ�� �򷵻�*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > LV_HOR_RES - 1) return;
    if(y1 > LV_VER_RES - 1) return;

    /*Truncate the area to the screen �ض���Ļ����*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
    int32_t act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;

	uint32_t x;
	uint32_t y;

	/*Fill the remaining area ����ʣ������ */
	for(x = act_x1; x <= act_x2; x++) {
		for(y = act_y1; y <= act_y2; y++) {
			tft_drawpoint(x, y, color.full);
		}
	}
}
/**
 * �����ú��� ��GUI��ͼƬ���˵�SHOW��
 */
 void DrawImage(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t addr)
{
	uint32_t i, j;
	
	for(i = 0; i < h; i++)
	{
		for(j = 0; j < w; j+=2)
		{
			LCD_BufferSHOW[((y+i)*480 + (x+j))/2] = *((volatile uint32_t *)(SDRAMM_BASE + addr) + (i*w + j)/2);
		}
	}
}

/**
 * Put a color map to a rectangular area �ھ����������һ�Ų�ɫͼ
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colorsָ��ָ��һ����ɫ����
 */
static void tft_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*Return if the area is out the screen*/
	if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > LV_HOR_RES - 1) return;
	if(y1 > LV_VER_RES - 1) return;

	/*Truncate the area to the screen�ض���Ļ����*/
	int32_t act_x1 = x1 < 0 ? 0 : x1;
	int32_t act_y1 = y1 < 0 ? 0 : y1;
	int32_t act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
	int32_t act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;


	int32_t x;
	int32_t y;
	
	for(y = act_y1; y <= act_y2; y++) {
		for(x = act_x1; x <= act_x2; x++) {
//			LCD_Buffer[(y * LV_HOR_RES + x)/2] = *color_p;
			tft_drawpoint(x, y, color_p->full);
			color_p++;
		}

		color_p += x2 - act_x2;
	}

}

/**
 * Put color  to one point ����ɫ����һ����
 * @param x coordinate  x����
 * @param y coordinate  y����
 * @param color fill color ��ɫ�����ɫ
 */
static inline void tft_drawpoint(int16_t x, int16_t y, uint32_t c)
{	
	LCD_BufferGUI[(y*LV_HOR_RES + x)/2] &= ~(0xFFFF << ((x%2) == 0 ? 0 : 16));
	LCD_BufferGUI[(y*LV_HOR_RES + x)/2] |=  (c      << ((x%2) == 0 ? 0 : 16));
}


/**
 * tft initial
 */
static void tft_rgbinit(void)
{
	
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOC, PIN2, 1, 0, 0);	// �������	�ߵ�ƽ��Ч
	GPIO_SetBit(GPIOC, PIN2);			// ��������
	GPIO_Init(GPIOC, PIN1, 1, 0, 0);	// DISP��ʾ���ƶ� �ߵ�ƽ��Ч
	GPIO_SetBit(GPIOC, PIN1);			// ����ʾ
	
	PORT->PORTN_SEL0 = 0xAAAAAAAA;		//GPION.0��15  	LCD_DATA0��15
	PORT->PORTN_SEL1 = 0xAA;			//GPION.16��19ΪLCD_CLK��LCD_VSYNC��LCD_DE��LCD_HSYNC
	
	LCD_initStruct.Interface = LCD_INTERFACE_RGB;
	LCD_initStruct.HnPixel = LV_HOR_RES;		//ˮƽ800����
	LCD_initStruct.VnPixel = LV_VER_RES;		//��ֱ480����
	LCD_initStruct.Hfp = 5;
	LCD_initStruct.Hbp = 40;
	LCD_initStruct.Vfp = 8;
	LCD_initStruct.Vbp = 8;
	LCD_initStruct.ClkDiv = LCD_CLKDIV_12;
	LCD_initStruct.ClkAlways = 0;
	LCD_initStruct.SamplEdge = LCD_SAMPLEDGE_FALL;
	LCD_initStruct.HsyncWidth = LCD_HSYNC_1DOTCLK;
	LCD_initStruct.IntEOTEn = 1;
	LCD_Init(LCD, &LCD_initStruct);
}

static void tft_setlcd_srcadd(void)
{
	LCD->SRCADDR = (uint32_t)LCD_BufferSHOW;
}

static void tft_clrlcd_srcadd(void)
{
//	LCD->SRCADDR = ((void*) 0);
}

static void tft_memoryinit(void)
{
	uint32_t i;
	SDRAM_InitStructure SDRAM_InitStruct;
		
	
	PORT->PORTP_SEL0 = 0xAAAAAAAA;		//PP0-23 => ADDR0-23
	PORT->PORTP_SEL1 = 0x0000AAAA;
	
	PORT->PORTM_SEL0 = 0xAAAAAAAA;		//PM0-15 => DATA15-0
	PORT->PORTM_INEN = 0xFFFF;
	
	PORT->PORTM_SEL1 = 0xAAA;			//PM16 => OEN��PM17 => WEN��PM18 => NORFL_CSN��PM19 => SDRAM_CSN��PM20 => SRAM_CSN��PM21 => SDRAM_CKE
	
	SDRAM_InitStruct.CellSize = SDRAM_CELLSIZE_256Mb;
	SDRAM_InitStruct.CellBank = SDRAM_CELLBANK_4;
	SDRAM_InitStruct.CellWidth = SDRAM_CELLWIDTH_16;
	SDRAM_InitStruct.CASLatency = SDRAM_CASLATENCY_2;
	SDRAM_InitStruct.TimeTMRD = SDRAM_TMRD_3;
	SDRAM_InitStruct.TimeTRRD = SDRAM_TRRD_2;
	SDRAM_InitStruct.TimeTRAS = SDRAM_TRAS_6;
	SDRAM_InitStruct.TimeTRC  = SDRAM_TRC_8;
	SDRAM_InitStruct.TimeTRCD = SDRAM_TRCD_3;
	SDRAM_InitStruct.TimeTRP  = SDRAM_TRP_3;
	SDRAM_Init(&SDRAM_InitStruct);
}
