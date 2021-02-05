/**
 * @file hal_disp.h
 *
 * @description Display Driver HAL interface header file 显示驱动程序HAL接口头文件
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
 * Display Driver structure to be registered by HAL 显示 HALL 要注册的驱动程序结构
 */
typedef struct _disp_drv_t {
    /*Write the internal buffer (VDB) to the display. 'lv_flush_ready()' has to be called when finished 将内部缓冲区(VDB)写入显示。必须在完成时调用'lv_flush_ready()'*/
    void (*disp_flush)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

    /*Fill an area with a color on the display 用显示器上的颜色填充一个区域*/
    void (*disp_fill)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);

    /*Write pixel map (e.g. image) to the display 将图片写入显示区*/
    void (*disp_map)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

#if USE_LV_GPU
    /*Blend two memories using opacity (GPU only) 使用不透明度混合两个内存(仅GPU)*/
    void (*mem_blend)(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);

    /*Fill a memory with a color (GPU only) 用颜色填充内存(仅GPU) */
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
 * Initialize a display driver with default values. 使用默认值初始化显示驱动程序。
 * It is used to surly have known values in the fields ant not memory junk.  它用于确定字段中的值而不是内存垃圾。
 * After it you can set the fields. 之后可以设置字段。
 * @param driver pointer to driver variable to initialize 驱动程序指针，指向要初始化的驱动程序变量
 */
void lv_disp_drv_init(lv_disp_drv_t *driver);

/**
 * Register an initialized display driver. 注册一个初始化的显示驱动程序。
 * Automatically set the first display as active. 自动设置第一个显示为活动。
 * @param driver pointer to an initialized 'lv_disp_drv_t' variable (can be local variable) 驱动程序指针，指向初始化的“lv__drv_t”变量(可以是本地变量)
 * @return pointer to the new display or NULL on error 回指向新显示的指针，如果出现错误则返回NULL
 */
lv_disp_t * lv_disp_drv_register(lv_disp_drv_t *driver);

/**
 * Set the active display 设置活动显示
 * @param disp pointer to a display (return value of 'lv_disp_register') 指向显示的@param disp指针(返回值为' lv__register ')
 */
void lv_disp_set_active(lv_disp_t * disp);

/**
 * Get a pointer to the active display 获取指向活动显示的指针
 * @return pointer to the active display 返回活动显示的指针
 */
lv_disp_t * lv_disp_get_active(void);

/**
 * Get the next display. 获取下一个显示 
 * @param disp pointer to the current display. NULL to initialize. 指针指向当前显示。零初始化。
 * @return the next display or NULL if no more. Give the first display when the parameter is NULL 回下一个显示，如果没有，返回NULL。当参数为空时，给出第一个显示
 */
lv_disp_t * lv_disp_next(lv_disp_t * disp);

/**
 * Fill a rectangular area with a color on the active display 在活动显示器上用颜色填充矩形区域
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors 指针指向一个颜色数组
 */
void lv_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t *color_p);

/**
 * Fill a rectangular area with a color on the active display 在活动显示器上用颜色填充矩形区域
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color fill color 颜色填充颜色
 */
void lv_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);

/**
 * Put a color map to a rectangular area on the active display 将彩色地图放置到活动显示的矩形区域
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_map pointer to an array of colors 指针指向一个颜色数组
 */
void lv_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map);

#if USE_LV_GPU
/**
 * Blend pixels to a destination memory from a source memory 将像素从源内存混合到目标内存
 * In 'lv_disp_drv_t' 'mem_blend' is optional. (NULL if not available) ' lv__drv_t ' 'mem_blend'是可选的。(如不可用，则为空)
 * @param dest a memory address. Blend 'src' here. 拥有一个内存地址。结合“src”。
 * @param src pointer to pixel map. Blend it to 'dest'. 指向像素映射的src指针。混合成dest。
 * @param length number of pixels in 'src'    src 中像素的长度
 * @param opa opacity (0, LV_OPA_TRANSP: transparent ... 255, LV_OPA_COVER, fully cover)  opa不透明度(0,LV_OPA_TRANSP:透明…255, LV_OPA_COVER，全盖)
 */
void lv_disp_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);

/**
 * Fill a memory with a color (GPUs may support it) 用颜色填充内存(gpu可能支持)
 * In 'lv_disp_drv_t' 'mem_fill' is optional. (NULL if not available) 在' lv__drv_t ' 'mem_fill'是可选的。(如不可用，则为空)
 * @param dest a memory address. Copy 'src' here. 拥有一个内存地址。复制“src”。
 * @param src pointer to pixel map. Copy it to 'dest'. 指向像素映射的src指针。复制到“dest”。
 * @param length number of pixels in 'src'“src”中像素的长度
 * @param opa opacity (0, LV_OPA_TRANSP: transparent ... 255, LV_OPA_COVER, fully cover) opa不透明度(0,LV_OPA_TRANSP:透明…255, LV_OPA_COVER，全盖)
 */
void lv_disp_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color);
/**
 * Shows if memory blending (by GPU) is supported or not 显示是否支持(GPU)内存混合
 * @return false: 'mem_blend' is not supported in the driver; true: 'mem_blend' is supported in the driver return false:驱动程序中不支持'mem_blend';true:驱动程序中支持'mem_blend'
 */
bool lv_disp_is_mem_blend_supported(void);

/**
 * Shows if memory fill (by GPU) is supported or not 显示是否支持内存填充(由GPU)
 * @return false: 'mem_fill' is not supported in the drover; true: 'mem_fill' is supported in the driver eturn false: drover中不支持'mem_fill';true:驱动程序中支持'mem_fill'
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
