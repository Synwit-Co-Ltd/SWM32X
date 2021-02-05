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
   /*Simple VDB��VDB*/
   static volatile lv_vdb_state_t vdb_state = LV_VDB_STATE_ACTIVE;
#  if LV_VDB_ADR == 0
     /*If the buffer address is not specified  simply allocate it �����������ַû��ָ������򵥵ط�����*/
     static lv_color_t vdb_buf[LV_VDB_SIZE];
     static lv_vdb_t vdb = {.buf = vdb_buf};
#  else
     /*If the buffer address is specified use that address ���ָ���˻�������ַ����ʹ�øõ�ַ*/
     static lv_vdb_t vdb = {.buf = (lv_color_t *)LV_VDB_ADR};
#  endif
		 
#else
   /*Double VDB ����VDB*/
   static volatile lv_vdb_state_t vdb_state[2] = {LV_VDB_STATE_FREE, LV_VDB_STATE_FREE};
#  if LV_VDB_ADR == 0
   /*If the buffer address is not specified  simply allocate it �����������ַû��ָ������򵥵ط�����*/
   static lv_color_t vdb_buf1[LV_VDB_SIZE];
   static lv_color_t vdb_buf2[LV_VDB_SIZE];
   static lv_vdb_t vdb[2] = {{.buf = vdb_buf1}, {.buf = vdb_buf2}};
#  else
   /*If the buffer address is specified use that address ���ָ���˻�������ַ����ʹ�øõ�ַ*/
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
 * Get the 'vdb' variable or allocate one in LV_VDB_DOUBLE mode ��ȡVDB���� ������LV_VDB_DOUBLEģʽ����һ������
 * @return pointer to a 'vdb' variable ����ָ��VDB������ָ��
 */
lv_vdb_t * lv_vdb_get(void)
{
#if LV_VDB_DOUBLE == 0
    /* Wait until VDB become ACTIVE from FLUSH by the �ȴ�VDB��ˢ�¼���
     * user call of 'lv_flush_ready()' in display drivers's flush function ����ʾ���������ˢ�º����е���'lv_flush_ready()'*/
    while(vdb_state != LV_VDB_STATE_ACTIVE);
    return &vdb;
#else
    /*If already there is an active do nothing ����Ѿ���һ������ʲôҲ����*/
    if(vdb_state[0] == LV_VDB_STATE_ACTIVE) return &vdb[0];
    if(vdb_state[1] == LV_VDB_STATE_ACTIVE) return &vdb[1];

    /*Try to allocate a free VDB ���Է���һ�����е�VDB*/
    if(vdb_state[0] == LV_VDB_STATE_FREE) {
        vdb_state[0] = LV_VDB_STATE_ACTIVE;
        return &vdb[0];
    }

    if(vdb_state[1] == LV_VDB_STATE_FREE) {
        vdb_state[1] = LV_VDB_STATE_ACTIVE;
        return &vdb[1];
    }

    return NULL;   /*There wasn't free VDB (never happen)û�п��е�VDB(��δ������)*/
#endif
}

/**
 * Flush the content of the VDB ˢ��VDB������
 */
void lv_vdb_flush(void)
{
    lv_vdb_t * vdb_act = lv_vdb_get();
    if(vdb_act == NULL) return;

#if LV_VDB_DOUBLE == 0
    vdb_state = LV_VDB_STATE_FLUSH;     /*User call to 'lv_flush_ready()' will set to ACTIVE 'disp_flush' �û����á�lv_flush_ready()��������Ϊ��ġ�dis_flush��*/
#else
    /* Wait the pending flush before starting this one ��������ˢ��֮ǰ����ȴ������ˢ��
     * (Don't forget: 'lv_flush_ready()' has to be called when flushing is ready) ��Ҫ����:��ˢ�¾���ʱ�������'lv_flush_ready()*/
    while(vdb_state[0] == LV_VDB_STATE_FLUSH || vdb_state[1] == LV_VDB_STATE_FLUSH);

    /*Turn the active VDB to flushing �����VDB����Ϊˢ��*/
    if(vdb_state[0] == LV_VDB_STATE_ACTIVE) vdb_state[0] = LV_VDB_STATE_FLUSH;
    if(vdb_state[1] == LV_VDB_STATE_ACTIVE) vdb_state[1] = LV_VDB_STATE_FLUSH;
#endif
    /*Flush the rendered content to the display �����ֵ�����ˢ�µ���ʾ��*/
//		NVIC_DisableIRQ(LCD_IRQn);
//	while(LCD_IsBusy(LCD));
	lv_disp_flush(vdb_act->area.x1, vdb_act->area.y1, vdb_act->area.x2, vdb_act->area.y2, vdb_act->buf);
//		NVIC_EnableIRQ(LCD_IRQn);

}

/**
 * Call in the display driver's  'disp_flush' function when the flushing is finished ��ˢ�����ʱ��������ʾ���������' dis_flush '����
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
 * Just for compatibility ֻ��Ϊ�˼�����
 */
void lv_flush_ready(void)
{
    /*Do nothing. It is used only for VDB ʲô����������ֻ����VDB*/
}
#endif
