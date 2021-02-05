/**
 * @file hal_disp.h
 *
 * @description Display Driver HAL interface header file ��ʾ��������HAL�ӿ�ͷ�ļ�
 *
 */

#ifndef HAL_DISP_H
#define HAL_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include "lv_hal.h"
#include "../lv_misc/lv_color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Display Driver structure to be registered by HAL ��ʾ HALL Ҫע�����������ṹ
 */
typedef struct _disp_drv_t {
    /*Write the internal buffer (VDB) to the display. 'lv_flush_ready()' has to be called when finished ���ڲ�������(VDB)д����ʾ�����������ʱ����'lv_flush_ready()'*/
    void (*disp_flush)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

    /*Fill an area with a color on the display ����ʾ���ϵ���ɫ���һ������*/
    void (*disp_fill)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);

    /*Write pixel map (e.g. image) to the display ��ͼƬд����ʾ��*/
    void (*disp_map)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

#if USE_LV_GPU
    /*Blend two memories using opacity (GPU only) ʹ�ò�͸���Ȼ�������ڴ�(��GPU)*/
    void (*mem_blend)(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);

    /*Fill a memory with a color (GPU only) ����ɫ����ڴ�(��GPU) */
    void (*mem_fill)(lv_color_t * dest, uint32_t length, lv_color_t color);
#endif

} lv_disp_drv_t;

typedef struct _disp_t {
    lv_disp_drv_t driver;
    struct _disp_t *next;
} lv_disp_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize a display driver with default values. ʹ��Ĭ��ֵ��ʼ����ʾ��������
 * It is used to surly have known values in the fields ant not memory junk.  ������ȷ���ֶ��е�ֵ�������ڴ�������
 * After it you can set the fields. ֮����������ֶΡ�
 * @param driver pointer to driver variable to initialize ��������ָ�룬ָ��Ҫ��ʼ���������������
 */
void lv_disp_drv_init(lv_disp_drv_t *driver);

/**
 * Register an initialized display driver. ע��һ����ʼ������ʾ��������
 * Automatically set the first display as active. �Զ����õ�һ����ʾΪ���
 * @param driver pointer to an initialized 'lv_disp_drv_t' variable (can be local variable) ��������ָ�룬ָ���ʼ���ġ�lv__drv_t������(�����Ǳ��ر���)
 * @return pointer to the new display or NULL on error ��ָ������ʾ��ָ�룬������ִ����򷵻�NULL
 */
lv_disp_t * lv_disp_drv_register(lv_disp_drv_t *driver);

/**
 * Set the active display ���û��ʾ
 * @param disp pointer to a display (return value of 'lv_disp_register') ָ����ʾ��@param dispָ��(����ֵΪ' lv__register ')
 */
void lv_disp_set_active(lv_disp_t * disp);

/**
 * Get a pointer to the active display ��ȡָ����ʾ��ָ��
 * @return pointer to the active display ���ػ��ʾ��ָ��
 */
lv_disp_t * lv_disp_get_active(void);

/**
 * Get the next display. ��ȡ��һ����ʾ 
 * @param disp pointer to the current display. NULL to initialize. ָ��ָ��ǰ��ʾ�����ʼ����
 * @return the next display or NULL if no more. Give the first display when the parameter is NULL ����һ����ʾ�����û�У�����NULL��������Ϊ��ʱ��������һ����ʾ
 */
lv_disp_t * lv_disp_next(lv_disp_t * disp);

/**
 * Fill a rectangular area with a color on the active display �ڻ��ʾ��������ɫ����������
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors ָ��ָ��һ����ɫ����
 */
void lv_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t *color_p);

/**
 * Fill a rectangular area with a color on the active display �ڻ��ʾ��������ɫ����������
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color fill color ��ɫ�����ɫ
 */
void lv_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);

/**
 * Put a color map to a rectangular area on the active display ����ɫ��ͼ���õ����ʾ�ľ�������
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_map pointer to an array of colors ָ��ָ��һ����ɫ����
 */
void lv_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map);

#if USE_LV_GPU
/**
 * Blend pixels to a destination memory from a source memory �����ش�Դ�ڴ��ϵ�Ŀ���ڴ�
 * In 'lv_disp_drv_t' 'mem_blend' is optional. (NULL if not available) ' lv__drv_t ' 'mem_blend'�ǿ�ѡ�ġ�(�粻���ã���Ϊ��)
 * @param dest a memory address. Blend 'src' here. ӵ��һ���ڴ��ַ����ϡ�src����
 * @param src pointer to pixel map. Blend it to 'dest'. ָ������ӳ���srcָ�롣��ϳ�dest��
 * @param length number of pixels in 'src'    src �����صĳ���
 * @param opa opacity (0, LV_OPA_TRANSP: transparent ... 255, LV_OPA_COVER, fully cover)  opa��͸����(0,LV_OPA_TRANSP:͸����255, LV_OPA_COVER��ȫ��)
 */
void lv_disp_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);

/**
 * Fill a memory with a color (GPUs may support it) ����ɫ����ڴ�(gpu����֧��)
 * In 'lv_disp_drv_t' 'mem_fill' is optional. (NULL if not available) ��' lv__drv_t ' 'mem_fill'�ǿ�ѡ�ġ�(�粻���ã���Ϊ��)
 * @param dest a memory address. Copy 'src' here. ӵ��һ���ڴ��ַ�����ơ�src����
 * @param src pointer to pixel map. Copy it to 'dest'. ָ������ӳ���srcָ�롣���Ƶ���dest����
 * @param length number of pixels in 'src'��src�������صĳ���
 * @param opa opacity (0, LV_OPA_TRANSP: transparent ... 255, LV_OPA_COVER, fully cover) opa��͸����(0,LV_OPA_TRANSP:͸����255, LV_OPA_COVER��ȫ��)
 */
void lv_disp_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color);
/**
 * Shows if memory blending (by GPU) is supported or not ��ʾ�Ƿ�֧��(GPU)�ڴ���
 * @return false: 'mem_blend' is not supported in the driver; true: 'mem_blend' is supported in the driver return false:���������в�֧��'mem_blend';true:����������֧��'mem_blend'
 */
bool lv_disp_is_mem_blend_supported(void);

/**
 * Shows if memory fill (by GPU) is supported or not ��ʾ�Ƿ�֧���ڴ����(��GPU)
 * @return false: 'mem_fill' is not supported in the drover; true: 'mem_fill' is supported in the driver eturn false: drover�в�֧��'mem_fill';true:����������֧��'mem_fill'
 */
bool lv_disp_is_mem_fill_supported(void);
#endif
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
