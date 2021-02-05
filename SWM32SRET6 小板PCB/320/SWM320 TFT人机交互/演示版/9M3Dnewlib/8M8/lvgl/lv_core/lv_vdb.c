/**
 * @file lv_vdb.c
 * 
 */
#include "../../lv_conf.h"
#if LV_VDB_SIZE != 0

#include "../lv_hal/lv_hal_disp.h"
#include <stddef.h>
#include "lv_vdb.h"
#include "../lv_hal/lv_hal_tick.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_mem.h"
#include "SWM320.h"
#include "main.h"
/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_VDB_STATE_FREE = 0,
    LV_VDB_STATE_ACTIVE,
    LV_VDB_STATE_FLUSH,
} lv_vdb_state_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


#if LV_VDB_DOUBLE == 0
   /*Simple VDB简单VDB*/
   static volatile lv_vdb_state_t vdb_state = LV_VDB_STATE_ACTIVE;
#  if LV_VDB_ADR == 0
     /*If the buffer address is not specified  simply allocate it 如果缓冲区地址没有指定，则简单地分配它*/
     static lv_color_t vdb_buf[LV_VDB_SIZE];
     static lv_vdb_t vdb = {.buf = vdb_buf};
#  else
     /*If the buffer address is specified use that address 如果指定了缓冲区地址，则使用该地址*/
     static lv_vdb_t vdb = {.buf = (lv_color_t *)LV_VDB_ADR};
#  endif
		 
#else
   /*Double VDB 两个VDB*/
   static volatile lv_vdb_state_t vdb_state[2] = {LV_VDB_STATE_FREE, LV_VDB_STATE_FREE};
#  if LV_VDB_ADR == 0
   /*If the buffer address is not specified  simply allocate it 如果缓冲区地址没有指定，则简单地分配它*/
   static lv_color_t vdb_buf1[LV_VDB_SIZE];
   static lv_color_t vdb_buf2[LV_VDB_SIZE];
   static lv_vdb_t vdb[2] = {{.buf = vdb_buf1}, {.buf = vdb_buf2}};
#  else
   /*If the buffer address is specified use that address 如果指定了缓冲区地址，则使用该地址*/
   static lv_vdb_t vdb[2] = {{.buf = (lv_color_t *)LV_VDB_ADR}, {.buf = (lv_color_t *)LV_VDB2_ADR}};
#  endif
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Get the 'vdb' variable or allocate one in LV_VDB_DOUBLE mode 获取VDB变量 或者以LV_VDB_DOUBLE模式分配一个变量
 * @return pointer to a 'vdb' variable 返回指向VDB变量的指针
 */
lv_vdb_t * lv_vdb_get(void)
{
#if LV_VDB_DOUBLE == 0
    /* Wait until VDB become ACTIVE from FLUSH by the 等待VDB从刷新激活
     * user call of 'lv_flush_ready()' in display drivers's flush function 在显示驱动程序的刷新函数中调用'lv_flush_ready()'*/
    while(vdb_state != LV_VDB_STATE_ACTIVE);
    return &vdb;
#else
    /*If already there is an active do nothing 如果已经有一个动作什么也不做*/
    if(vdb_state[0] == LV_VDB_STATE_ACTIVE) return &vdb[0];
    if(vdb_state[1] == LV_VDB_STATE_ACTIVE) return &vdb[1];

    /*Try to allocate a free VDB 尝试分配一个空闲的VDB*/
    if(vdb_state[0] == LV_VDB_STATE_FREE) {
        vdb_state[0] = LV_VDB_STATE_ACTIVE;
        return &vdb[0];
    }

    if(vdb_state[1] == LV_VDB_STATE_FREE) {
        vdb_state[1] = LV_VDB_STATE_ACTIVE;
        return &vdb[1];
    }

    return NULL;   /*There wasn't free VDB (never happen)没有空闲的VDB(从未发生过)*/
#endif
}

/**
 * Flush the content of the VDB 刷新VDB的内容
 */
void lv_vdb_flush(void)
{
    lv_vdb_t * vdb_act = lv_vdb_get();
    if(vdb_act == NULL) return;

#if LV_VDB_DOUBLE == 0
    vdb_state = LV_VDB_STATE_FLUSH;     /*User call to 'lv_flush_ready()' will set to ACTIVE 'disp_flush' 用户调用“lv_flush_ready()”将设置为活动的“dis_flush”*/
#else
    /* Wait the pending flush before starting this one 在启动此刷新之前，请等待挂起的刷新
     * (Don't forget: 'lv_flush_ready()' has to be called when flushing is ready) 不要忘记:当刷新就绪时必须调用'lv_flush_ready()*/
    while(vdb_state[0] == LV_VDB_STATE_FLUSH || vdb_state[1] == LV_VDB_STATE_FLUSH);

    /*Turn the active VDB to flushing 将活动的VDB设置为刷新*/
    if(vdb_state[0] == LV_VDB_STATE_ACTIVE) vdb_state[0] = LV_VDB_STATE_FLUSH;
    if(vdb_state[1] == LV_VDB_STATE_ACTIVE) vdb_state[1] = LV_VDB_STATE_FLUSH;
#endif
    /*Flush the rendered content to the display 将呈现的内容刷新到显示中*/
//		NVIC_DisableIRQ(LCD_IRQn);
//	while(LCD_IsBusy(LCD));
	lv_disp_flush(vdb_act->area.x1, vdb_act->area.y1, vdb_act->area.x2, vdb_act->area.y2, vdb_act->buf);
//		NVIC_EnableIRQ(LCD_IRQn);

}

/**
 * Call in the display driver's  'disp_flush' function when the flushing is finished 当刷新完成时，调用显示驱动程序的' dis_flush '函数
 */
void lv_flush_ready(void)
{	
//	  NVIC_DisableIRQ(LCD_IRQn);
//	  while(LCD_IsBusy(LCD));
#if LV_VDB_DOUBLE == 0
    vdb_state = LV_VDB_STATE_ACTIVE;
#else
    if(vdb_state[0] == LV_VDB_STATE_FLUSH)  vdb_state[0] = LV_VDB_STATE_FREE;
    if(vdb_state[1] == LV_VDB_STATE_FLUSH)  vdb_state[1] = LV_VDB_STATE_FREE;
#endif
//	  NVIC_EnableIRQ(LCD_IRQn);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#else

/**
 * Just for compatibility 只是为了兼容性
 */
void lv_flush_ready(void)
{
    /*Do nothing. It is used only for VDB 什么都不做。它只用于VDB*/
}
#endif
