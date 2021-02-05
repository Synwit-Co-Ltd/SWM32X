/**
 * @file systick.c
 * Provide access to the system tick with 1 millisecond resolution
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_hal_tick.h"
#include <stddef.h>
#include "../../lv_conf.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static uint32_t sys_time = 0;
static volatile uint8_t tick_irq_flag;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * You have to call this function periodically
 * @param tick_period the call period of this function in milliseconds
 */
LV_ATTRIBUTE_TICK_INC void lv_tick_inc(uint32_t tick_period)
{
    tick_irq_flag = 0;
    sys_time += tick_period;
}

/**
 * Get the elapsed milliseconds since start up 获取启动后经过的毫秒数
 * @return the elapsed milliseconds 返回经过的毫秒数
 */
uint32_t lv_tick_get(void)
{
    uint32_t result;
    do {
        tick_irq_flag = 1;
        result = sys_time;
    } while(!tick_irq_flag);     /*'lv_tick_inc()' clears this flag which can be in an interrupt. Continue until make a non interrupted cycle 解释lv_tick_inc()'清除可能处于中断中的这个标志。继续，直到形成一个不间断的循环 */

    return result;
}

/**
 * Get the elapsed milliseconds science a previous time stamp 将经过的毫秒科学设置为以前的时间戳
 * @param prev_tick a previous time stamp (return value of systick_get() ) prev_tick以前的时间戳(systick_get()的返回值)
 * @return the elapsed milliseconds since 'prev_tick'返回从'prev_tick'起经过的毫秒数
 */
uint32_t lv_tick_elaps(uint32_t prev_tick)
{
	uint32_t act_time = lv_tick_get();

	/*If there is no overflow in sys_time simple subtract如果sys_time中没有溢出，则简单地减去*/
	if(act_time >= prev_tick) {
		prev_tick = act_time - prev_tick;
	} else {
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}

	return prev_tick;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

