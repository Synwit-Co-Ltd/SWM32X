/**
 * @file lv_vdb.h
 * 
 */

#ifndef LV_VDB_H
#define LV_VDB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf.h"

#if LV_VDB_SIZE != 0

#include "../lv_misc/lv_color.h"
#include "../lv_misc/lv_area.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    lv_area_t area;
    lv_color_t *buf;
}lv_vdb_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Get the 'vdb' variable or allocate one in LV_VDB_DOUBLE mode ��ȡ��vdb��������������LV_VDB_DOUBLEģʽ����һ������
 * @return pointer to a 'vdb' variable ����ָ��vdb��������ָ��
 */
lv_vdb_t * lv_vdb_get(void);

/**
 * Flush the content of the vdb ˢ��vdb������
 */
void lv_vdb_flush(void);


/**
 * In 'LV_VDB_DOUBLE' mode  has to be called when 'disp_map()' �ڡ�LV_VDB_DOUBLE��ģʽ�£�����dis_map()��������ʱ��������á�LV_VDB_DOUBLE��ģʽ��
 * is ready with copying the map to a frame buffer. ��׼���ý�ӳ�临�Ƶ�֡��������
 */
void lv_flush_ready(void);

/**********************
 *      MACROS
 **********************/

#else /*LV_VDB_SIZE != 0*/

/*Just for compatibility ֻ��Ϊ�˼�����*/
void lv_flush_ready(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_VDB_H*/
