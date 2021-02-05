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
 * Get the elapsed milliseconds since start up ��ȡ�����󾭹��ĺ�����
 * @return the elapsed milliseconds ���ؾ����ĺ�����
 */
uint32_t lv_tick_get(void)
{
    uint32_t result;
    do {
        tick_irq_flag = 1;
        result = sys_time;
    } while(!tick_irq_flag);     /*'lv_tick_inc()' clears this flag which can be in an interrupt. Continue until make a non interrupted cycle ����lv_tick_inc()'������ܴ����ж��е������־��������ֱ���γ�һ������ϵ�ѭ�� */

    return result;
}

/**
 * Get the elapsed milliseconds science a previous time stamp �������ĺ����ѧ����Ϊ��ǰ��ʱ���
 * @param prev_tick a previous time stamp (return value of systick_get() ) prev_tick��ǰ��ʱ���(systick_get()�ķ���ֵ)
 * @return the elapsed milliseconds since 'prev_tick'���ش�'prev_tick'�𾭹��ĺ�����
 */
uint32_t lv_tick_elaps(uint32_t prev_tick)
{
	uint32_t act_time = lv_tick_get();

	/*If there is no overflow in sys_time simple subtract���sys_time��û���������򵥵ؼ�ȥ*/
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

