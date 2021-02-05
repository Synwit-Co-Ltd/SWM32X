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
 * Get the 'vdb' variable or allocate one in LV_VDB_DOUBLE mode 获取“vdb”变量，或者以LV_VDB_DOUBLE模式分配一个变量
 * @return pointer to a 'vdb' variable 返回指向“vdb”变量的指针
 */
lv_vdb_t * lv_vdb_get(void);

/**
 * Flush the content of the vdb 刷新vdb的内容
 */
void lv_vdb_flush(void);


/**
 * In 'LV_VDB_DOUBLE' mode  has to be called when 'disp_map()' 在“LV_VDB_DOUBLE”模式下，当“dis_map()”被调用时，必须调用“LV_VDB_DOUBLE”模式。
 * is ready with copying the map to a frame buffer. 已准备好将映射复制到帧缓冲区。
 */
void lv_flush_ready(void);

/**********************
 *      MACROS
 **********************/

#else /*LV_VDB_SIZE != 0*/

/*Just for compatibility 只是为了兼容性*/
void lv_flush_ready(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_VDB_H*/
