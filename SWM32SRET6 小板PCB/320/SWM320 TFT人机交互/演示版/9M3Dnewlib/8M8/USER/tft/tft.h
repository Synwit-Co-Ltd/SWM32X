/**
 * @file disp.h
 * 
 */

#ifndef TFT_H
#define TFT_H

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "lv_color.h"
#include "lv_area.h"
#include "SWM320.h"

/*********************
 *      DEFINES
 *********************/

#define TFT_EXT_FB					1		/*Frame buffer is located into an external SDRAM*/
#define TFT_USE_GPU				0		/*Enable hardware accelerator*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_init(void);
void DrawImage(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t addr);
/**********************
 *      MACROS
 **********************/

#endif

