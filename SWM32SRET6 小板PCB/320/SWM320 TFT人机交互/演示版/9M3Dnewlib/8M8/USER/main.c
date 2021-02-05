#include "SWM320.h"
#include "tft.h"
#include "lvgl/lvgl.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "BSP_Driv/bsp_SysTick.h"
#include "BSP_Driv/bsp_UART.h"
#include "../lv_examples/lv_tutorial/1_hello_world/lv_tutorial_hello_world.h"
#include "../lvgl/lv_hal/lv_hal_tick.h"
#include "touchpad/touchpad.h"
#include "bsp_sdio.h"
#include "ff.h"
#include "bsp_SPIFlash.h"
/*-------------------------------------------------------------------------------------------------------------- SPIFLASH的文件系统挂载与设置 ---------------------------------------------------------------------------------------*/
FATFS fs;							/* FatFs文件系统对象 */
FIL fnew;							/* 文件对象 */
FRESULT res_sd;                		/* 文件操作结果 */
FRESULT res_flash;                		/* 文件操作结果 */
UINT fnum;            				/* 文件成功读写数量 */

typedef union {                    /* 用于作为SPIFLASH转移到SDRAM的结构体 */ 
	
		unsigned long int  V;
	
		struct {
			unsigned char data1;
			unsigned char data2;
			unsigned char data3;
			unsigned char data4;
		
		
					}BYTE;

}	 IMG_DATA;

IMG_DATA save[256]={0};            /* 用于作为文件系统搬运的缓存 */


extern void lv_tutorial_hello_world(void);
extern void sysmon_create(void);
extern lv_obj_t * terminal_create(void);
extern int32_t GTP_Init_Panel(void);
extern void benchmark_create(void);
extern void lv_tutorial_fonts(void);
extern void lv_test_group_1(void);
extern void lv_test_slider_1(void);

static int touch_irq_flag = 0;
static int16_t  s_delay_time = 0,s_flag_delaytime = 0;
static  lv_res_t btn1_action(lv_obj_t * btn);
static  lv_res_t btn2_action(lv_obj_t * btn);
static  lv_res_t btn3_action(lv_obj_t * btn);

static lv_obj_t * page1;
static lv_style_t b1,b2,b3;

int mathout = 0;
uint32_t i;
uint32_t math;
int bg = 0;



	lv_img_t my_image1 = //刷背景图用到的函数
	{
  .header.w = 480,
  .header.h = 272,
  .header.alpha_byte = 0,
  .header.chroma_keyed = 0,	
  .header.format = LV_IMG_FORMAT_INTERNAL_RAW,
  .pixel_map = (uint8_t *)(0x70000000  + 0xBF400) //0x7F800 BF400（783360）
   };
	
	lv_img_t up = //刷png向上箭头用到的函数
	 { 
  .header.w = 190,
  .header.h = 210,
  .header.alpha_byte = 1,
  .header.chroma_keyed = 0,	
  .header.format = LV_IMG_FORMAT_INTERNAL_RAW,
  .pixel_map = (uint8_t *)(0x70000000 + 0x4 + 0x177000)
   };
	
	 
	 
void JumpToApp(uint32_t addr)  //跳转用函数
{	
	uint32_t sp = *((volatile uint32_t *)(addr));
	uint32_t pc = *((volatile uint32_t *)(addr + 4));
	
	typedef void (*Reset_Handler_t)(void);
	Reset_Handler_t Reset_Handler = (Reset_Handler_t)pc;
	
	__disable_irq();
	
	SCB->VTOR = addr;
	
	__set_MSP(sp);
	
	Reset_Handler();
	
	while(1) __NOP();
}

	void SPIFLASH2SDRAM(const TCHAR* path, uint32_t sdr_addr, uint32_t word_count)//从spiflash的文件系统读取到SDRAM 仿照从norflash读到sdram
{
	uint32_t n, i;

	
	res_flash = f_open(&fnew, path, FA_OPEN_EXISTING | FA_READ); 	  

	
	word_count = word_count / 4;
	
	for(n = 0; n < word_count/256; n++)
	{
		f_lseek(&fnew,256*4*n);
		res_flash = f_read(&fnew, save, sizeof(save), &fnum); 

		for(i = 0; i < 256; i++)
		{
				*((volatile uint32_t *)(SDRAMM_BASE + sdr_addr) + 256*n + i) = save[i].V;
		}
//	printf("现在是第：%d\r个循环\n",n);
	
	}
	
	if(word_count%256)
	{
		f_lseek(&fnew,256*4*n); //左边是打开的文件 右边是操作起始的位置
		res_flash = f_read(&fnew, save, sizeof(save), &fnum); 	
		for(i = 0; i < word_count%256; i++)
		{
			*((volatile uint32_t *)(SDRAMM_BASE + sdr_addr) + 256*n + i) = save[i].V;
		}
	}
	f_close(&fnew);	/* 不再读写，关闭文件 */
//	f_mount(NULL,"1:",1);/* 不再使用文件系统，取消挂载文件系统 */
//	printf("》SPIFLASH2SDRAM搬运完成。\r\n");
}
 
	 
	 

/*-------------------------------------------------------------------------------------------------------------- Main函数起始位 ------------------------------------------------------------------------------------------------------------------------------------*/

int main(void)
{
	BYTE work[FF_MAX_SS]; 			/* Work area (larger is better for processing time) */

	SystemInit();
	SysTick_Init();
	SerialInit(115200);
	lv_init();
	tft_init();
	
	__enable_irq();
	
	/*------------------- SPIFLASH ------------------------------------*/
	uint32_t DeviceID = 0; //SPIFLASH驱动ID
	uint32_t FlashID = 0;  //SPIFLASH ID
	SPI_Flash_Init(); //SPIFLASH设置
	printf("SPI Flash设置OK ...... \r\n");
	
	DeviceID = SPI_FLASH_ReadDeviceID();		// 获取 Flash Device ID
	
	FlashID = SPI_FLASH_ReadID();				// 获取 SPI Flash ID
	
	printf("\r\n FlashID is 0x%X,\
	Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);
	
	if (FlashID == sFLASH_ID) 
	{	
		printf("\r\n检测到SPI FLASH W25Q128 确认无误!\r\n");
	}	
	/*------------------- SPIFLASH ------------------------------------*/
res_flash = f_mount(&fs,"1:",1);
	/* 如果没有文件系统就格式化创建创建文件系统 */
	if(res_flash == FR_NO_FILESYSTEM)
	{
		printf("》SPIFLASH还没有文件系统，即将进行格式化...\r\n");
		res_flash = f_mkfs("1:",FM_ANY, 0, work, sizeof work);				/* 格式化 */						
		if(res_flash == FR_OK)
		{
			printf("》SPIFLASH已成功格式化文件系统。\r\n");
			res_flash = f_mount(NULL,"1:",1);		/* 格式化后，先取消挂载 */	
			res_flash = f_mount(&fs,"1:",1);		/* 重新挂载	*/	
		}
		else
		{
			printf("《《格式化失败。》》\r\n");
			while(1);
		}
	}
	else if(res_sd!=FR_OK)
	{
		printf("！！SPIFLASH挂载文件系统失败。(%d)\r\n",res_flash);
		printf("！！可能原因：SPIFLASH初始化不成功。\r\n");
		while(1);
	}
	else
	{
		printf("》文件系统挂载成功，可以进行读写测试\r\n");
	}	
		/*------------------------------------- 显示主程序 ------------------------------------*/
	SPIFLASH2SDRAM("1:synwit.bin", 0, 261120); //1FE000
	LCD_Start(LCD);
	NVIC_EnableIRQ(LCD_IRQn);
	
	SPIFLASH2SDRAM("1:png1.bin", 3394560, 119704); //从SPIFLASH搬运到SDRAM里面 33CC00
	SPIFLASH2SDRAM("1:png2.bin", 3514264, 119704); 
	SPIFLASH2SDRAM("1:png3.bin", 3633968, 119704); 
	SPIFLASH2SDRAM("1:png4.bin", 3753672, 119704); 
	SPIFLASH2SDRAM("1:png5.bin", 3873376, 119704); 
	SPIFLASH2SDRAM("1:png6.bin", 3993080, 119704); 
	SPIFLASH2SDRAM("1:png7.bin", 4112784, 119704); 
	SPIFLASH2SDRAM("1:png8.bin", 4232488, 119704); 
	SPIFLASH2SDRAM("1:png9.bin", 4352192, 119704); 
	SPIFLASH2SDRAM("1:png10.bin", 4471896, 119704); 

//	SPIFLASH2SDRAM("1:bg1.bin", 783360, 261120); //搬运图片到SDRAM中 分别是SPIFLASH中存放的文件名 SDRAM写入地址（带768000是为了不干扰显存） 所搬运的文件大小
//	SPIFLASH2SDRAM("1:bg2.bin", 1044480, 261120); //
//	SPIFLASH2SDRAM("1:bg3.bin", 1305600, 261120); //13EC00
//	SPIFLASH2SDRAM("1:bg4.bin", 1566720, 261120); //17E800
//	SPIFLASH2SDRAM("1:bg5.bin", 1827840, 261120); //1BE400
//	SPIFLASH2SDRAM("1:bg6.bin", 2088960, 261120); //1FE000
//	SPIFLASH2SDRAM("1:bg7.bin", 2350080, 261120); //23DC00
//	SPIFLASH2SDRAM("1:bg8.bin", 2611200, 261120); //27D800
//	SPIFLASH2SDRAM("1:bg9.bin", 2872320, 261120); //2BD400
//	SPIFLASH2SDRAM("1:bg10.bin", 3133440, 261120); //2FD000
	
	SPIFLASH2SDRAM("1:BMP1.bin", 783360, 261120); //搬运图片到SDRAM中 分别是SPIFLASH中存放的文件名 SDRAM写入地址（带768000是为了不干扰显存） 所搬运的文件大小
	SPIFLASH2SDRAM("1:BMP2.bin", 1044480, 261120); //
	SPIFLASH2SDRAM("1:BMP3.bin", 1305600, 261120); //13EC00
	SPIFLASH2SDRAM("1:BMP4.bin", 1566720, 261120); //17E800
	SPIFLASH2SDRAM("1:BMP5.bin", 1827840, 261120); //1BE400
//	SPIFLASH2SDRAM("1:BMP7.bin", 2350080, 261120); //23DC00
//	SPIFLASH2SDRAM("1:BMP8.bin", 2611200, 261120); //27D800
//	SPIFLASH2SDRAM("1:BMP9.bin", 2872320, 261120); //2BD400
//	SPIFLASH2SDRAM("1:BMP10.bin", 3133440, 261120); //2FD000
	
	
	printf("APP SPIFLASH文件系统部分已经执行完成\r\n");
	/*------------------------------------- 背景图 ------------------------------------*/
	lv_obj_t *wp = lv_img_create(lv_scr_act(), NULL);//背景图
  lv_img_set_src(wp, &my_image1); //背景图片设置
  lv_obj_set_width(wp, LV_HOR_RES * 4);
  lv_obj_set_protect(wp, LV_PROTECT_POS);	
	/*------------------------------------- PNG图 ------------------------------------*/	
	lv_obj_t *PNG1 = lv_img_create(lv_scr_act(), NULL); //PNG图位置
	lv_obj_set_pos(PNG1, 140, 0);     /*Align next to the source image*/
//	lv_img_set_src(PNG1,&up);
/*------------------------------------- 进度条 ------------------------------------*/	
	lv_obj_t * bar1 = lv_bar_create(lv_scr_act(), NULL);//固定样式 不用变更
  lv_obj_set_size(bar1, 200, 30);//分别是进度条对象 长度 宽度
  lv_obj_set_pos(bar1, 130, 220);//分别是进度条对象 X轴起始位置 y轴起始位置
  lv_bar_set_value(bar1, 100); //进度条对象 条子的填充百分比
/*------------------------------------- 进度条上文字 ------------------------------------*/	
  lv_obj_t * bar1_label = lv_label_create(lv_scr_act(), NULL);//固定样式 不用变更
	lv_label_set_recolor(bar1_label, true);
  lv_label_set_text(bar1_label, " #66FFE6 Hi from synwit"); //文字条对象 显示内容
  lv_obj_set_pos(bar1_label, 160, 200); //文字条对象 X轴起始位置 y轴起始位置 
	printf("APP LittleVGL配置部分已经执行完成\r\n");
	 
	 

//	printf("APP开始显示进度条0-256K区域正常执行\r\n");
//  demo_create();
//  for(i=0; i < 200000; i++);
	while(1)
	{
	for(bg = 0;bg < 5 ;bg++)
		{
	for(math = 0;math < 10 ;math++)
	{
		up.pixel_map = (uint8_t *)(0x70000000 + 0x4 + 0x33CC00 + math*119704); //显示png图用的
		lv_img_set_src(PNG1,&up);
//		my_image1.pixel_map = (uint8_t *)(0x70000000 + 0xBF400 + math*261120);
//		lv_img_set_src(wp, &my_image1); //背景图片设置
		lv_bar_set_value(bar1, 10 + math*10); //进度条对象 条子的填充百分比
		lv_task_handler();
		
		NVIC_DisableIRQ(LCD_IRQn);
		DrawImage(0, 0, 480, 272, 261120);
		NVIC_EnableIRQ(LCD_IRQn);
	 }
		my_image1.pixel_map = (uint8_t *)(0x70000000 + 0xBF400 + bg*261120);
		lv_img_set_src(wp, &my_image1); //背景图片设置
 }
//			switch(bg)
//		{
//			case 0:DrawImage(0, 0, 480, 272, 783360);bg = 1;break;
//			case 1:DrawImage(0, 0, 480, 272, 1044480);bg = 2;break;
//			case 2:DrawImage(0, 0, 480, 272, 1305600);bg = 3;break;
//			case 3:DrawImage(0, 0, 480, 272, 1566720);bg = 4;break;
//			case 4:DrawImage(0, 0, 480, 272, 1827840);bg = 0;break;	
//			case 5:DrawImage(0, 0, 480, 272, 2088960);bg = 6;break;
//			case 6:DrawImage(0, 0, 480, 272, 2350080);bg = 7;break;
//			case 7:DrawImage(0, 0, 480, 272, 2611200);bg = 8;break;
//			case 8:DrawImage(0, 0, 480, 272, 2872320);bg = 9;break;
//			case 9:DrawImage(0, 0, 480, 272, 3133440);bg = 0;break;
//		}
		
//	SPIFLASH2SDRAM("1:bg1.bin", 783360, 261120); // 对比上面用的 
//	SPIFLASH2SDRAM("1:bg2.bin", 1044480, 261120); //
//	SPIFLASH2SDRAM("1:bg3.bin", 1305600, 261120); //13EC00
//	SPIFLASH2SDRAM("1:bg4.bin", 1566720, 261120); //17E800
//	SPIFLASH2SDRAM("1:bg5.bin", 1827840, 261120); //1BE400
//	SPIFLASH2SDRAM("1:bg6.bin", 2088960, 261120); //1FE000
//	SPIFLASH2SDRAM("1:bg7.bin", 2350080, 261120); //23DC00
//	SPIFLASH2SDRAM("1:bg8.bin", 2611200, 261120); //27D800
//	SPIFLASH2SDRAM("1:bg9.bin", 2872320, 261120); //2BD400
//	SPIFLASH2SDRAM("1:bg10.bin", 3133440, 261120); //2FD000
		
//		lv_img_set_src(wp, &my_image1);
//		NVIC_DisableIRQ(LCD_IRQn);
//    while(LCD_IsBusy(LCD));
//		lv_task_handler();
//    NVIC_EnableIRQ(LCD_IRQn);
//		for(i=0; i < 200000; i++);
		}
	}



/**
  * @brief  This function handles Lcd Handler.
  * @param  None
  * @retval None
  */
void LCD_Handler(void)
{
	LCD_INTClr(LCD);
	LCD_Start(LCD);
}


/**
 * systick handler callback
 * @param none
 * @return none
 */
void SysTick_Handler_CallBack(void)
{	
	lv_tick_inc(1);
}

/**
 * gpioc2 interrupt handler
 * @param none
 * @return none
 */
void GPIOC2_Handler(void)
{
	EXTI_Clear(GPIOC, PIN2);					// 清除中断标志位
	touch_irq_flag = 1;			
}


static  lv_res_t btn1_action(lv_obj_t * btn)
{
	lv_page_set_style(page1, LV_PAGE_STYLE_BG, &b1);

    return LV_RES_OK;
}

static  lv_res_t btn2_action(lv_obj_t * btn)
{
	lv_page_set_style(page1, LV_PAGE_STYLE_BG, &b2);

    return LV_RES_OK;
}

static  lv_res_t btn3_action(lv_obj_t * btn)
{
	lv_page_set_style(page1, LV_PAGE_STYLE_BG, &b3);

    return LV_RES_OK;
}
