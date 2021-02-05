/**
 * @file indev.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "tft.h"
#include "lv_hal.h"
#include "../BSP_Driv/bsp_GT9XX.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
  uint16_t TouchDetected;
  int16_t X;
  uint16_t Y;
//  int16_t Z;
}TS_StateTypeDef;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool touchpad_read(lv_indev_data_t *data);

/**********************
 *  STATIC VARIABLES
 **********************/
static TS_StateTypeDef  TS_State;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize your input devices here
 */
void touchpad_init(void)
{
//  BSP_TS_Init(LV_HOR_RES, LV_VER_RES);
  GTP_Init_Panel();

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read = touchpad_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static bool touchpad_read(lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;

	if(TS_State.TouchDetected != 0) {
		data->point.x = TS_State.X;
		data->point.y = TS_State.Y;
		last_x = data->point.x;
		last_y = data->point.y;
		data->state = LV_INDEV_STATE_PR;
	} else {
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}
	
	return false;
}


/**
  * @brief  Set status and positions of the touch screen.
  * @param  xp put the x coordinate here
  * @param  yp put the y coordinate here
  * @return none
  */
void touchpad_setstate( int16_t xp, int16_t yp, uint16_t state)
{
	TS_State.TouchDetected = state;
	TS_State.X = xp;
	TS_State.Y = yp;
}

/**
  * @brief  Set status and positions of the touch screen.
  * @param  xp put the x coordinate here
  * @param  yp put the y coordinate here
  * @return none
  */
void touchpad_process(void)
{
	GTP_TouchProcess();
}
