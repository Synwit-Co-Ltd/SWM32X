/**
 * @file lv_conf.h
 * 
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/*----------------
 * Dynamic memory 动态内存
 *----------------*/

/* Memory size which will be used by the library 库将使用的内存大小
 * to store the graphical objects and other data 存储图形对象和其他数据 */
#define LV_MEM_CUSTOM      0                /*1: use custom malloc/free, 0: use the built-in lv_mem_alloc/lv_mem_free 1:使用定制的mallocHeader for the dynamic memory function/free, 0:使用内置的lv_mem_alloc/lv_mem_free */
#if LV_MEM_CUSTOM == 0
#define LV_MEM_SIZE    (64U * 1024U)        /*Size memory used by `lv_mem_alloc` in bytes (>= 2kB) lv_mem_alloc使用的内存大小(以字节为单位)(>= 2kB)*/
#define LV_MEM_ATTR                         /*Complier prefix for big array declaration 用于大数组声明的编译器前缀 */
#define LV_MEM_AUTO_DEFRAG  1               /*Automatically defrag on free 自动整理免费*/
#else       /*LV_MEM_CUSTOM*/
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /**/
#define LV_MEM_CUSTOM_ALLOC   malloc       /*Wrapper to malloc 动态内存函数的头*/
#define LV_MEM_CUSTOM_FREE    free         /*Wrapper to free 包装器自由*/
#endif     /*LV_MEM_CUSTOM*/

/*===================
   Graphical settings 图形化设置
 *===================*/

/* Horizontal and vertical resolution of the library. 库的水平和垂直分辨率*/
#define LV_HOR_RES          (480)
#define LV_VER_RES          (272)
#define LV_DPI              100

/* Size of VDB (Virtual Display Buffer: the internal graphics buffer).VDB大小(虚拟显示缓冲区:内部图形缓冲区)。
 * Required for buffered drawing, opacity and anti-aliasing 需要缓冲绘图，不透明度和抗混叠
 * VDB makes the double buffering, you don't need to deal with it! VDB做了双重缓冲，你不需要处理它!
 * Typical size: ~1/10 screen 典型尺寸:~1/10屏幕 */
#define LV_VDB_SIZE         9600  /*Size of VDB in pixel count (1/10 screen size is good for first)VDB的大小(以像素为单位)(1/10屏幕大小优先)*/
#define LV_VDB_ADR          0                  /*Place VDB to a specific address (e.g. in external RAM) (0: allocate automatically into RAM)将VDB放置到特定的地址(例如外部RAM中)(0:自动分配到RAM中)*/

/* Use two Virtual Display buffers (VDB) parallelize rendering and flushing (optional) 使用两个虚拟显示缓冲区(VDB)并行化呈现和刷新(可选)
 * The flushing should use DMA to write the frame buffer in the background 刷新应该使用DMA在后台写入帧缓冲区*/
#define LV_VDB_DOUBLE       0       /*1: Enable the use of 2 VDBs 启用2个VDBs*/
#define LV_VDB2_ADR         0     /*Place VDB2 to a specific address (e.g. in external RAM) (0: allocate automatically into RAM) 将VDB2放置到特定的地址(例如在外部RAM中)(0:自动分配到RAM中) */

/* Enable anti-aliasing (lines, and radiuses will be smoothed) 启用抗混叠(线条和弧度将被平滑处理) */
#define LV_ANTIALIAS        0       /*1: Enable anti-aliasing 启用抗锯齿*/

/*Screen refresh settings 屏幕刷新设置*/
#define LV_REFR_PERIOD      50    /*Screen refresh period in milliseconds 屏幕刷新周期(以毫秒为单位) */
#define LV_INV_FIFO_SIZE    32    /*The average count of objects on a screen 屏幕上对象的平均计数  */

/*=================
   Misc. setting 混杂设置
 *=================*/

/*Input device settings输入设备的设置*/
#define LV_INDEV_READ_PERIOD            50                     /*Input device read period in milliseconds 输入设备读取周期(以毫秒为单位)*/
#define LV_INDEV_POINT_MARKER           0                      /*Mark the pressed points  (required: USE_LV_REAL_DRAW = 1)标记按压点(要求:USE_LV_REAL_DRAW = 1)*/
#define LV_INDEV_DRAG_LIMIT             10                     /*Drag threshold in pixels 拖动阈值(像素) */
#define LV_INDEV_DRAG_THROW             20                     /*Drag throw slow-down in [%]. Greater value means faster slow-down 在[%]中拖放慢速。更大的值意味着更快的减速*/
#define LV_INDEV_LONG_PRESS_TIME        400                    /*Long press time in milliseconds长按时间(以毫秒为单位)*/
#define LV_INDEV_LONG_PRESS_REP_TIME    100                    /*Repeated trigger period in long press [ms] 长按重复触发周期[ms]*/

/*Color settings*/
#define LV_COLOR_DEPTH     16                     /*Color depth: 1/8/16/24 颜色深度:1/8/16/24*/
#define LV_COLOR_TRANSP    LV_COLOR_LIME          /*Images pixels with this color will not be drawn (with chroma keying) 使用此颜色的图像像素将不会绘制(使用色度键控)*/

/*Text settings*/
#define LV_TXT_UTF8             1                /*Enable UTF-8 coded Unicode character usage启用UTF-8编码的Unicode字符使用 */
#define LV_TXT_BREAK_CHARS     " ,.;:-_"         /*Can break texts on these chars 可以在这些字符上中断文本*/

/*Graphics feature usage*/
#define USE_LV_ANIMATION        0               /*1: Enable all animations打开所有动画*/
#define USE_LV_SHADOW           0               /*1: Enable shadows 打开阴影*/
#define USE_LV_GROUP            1               /*1: Enable object groups (for keyboards) 启用对象组(用于键盘)*/
#define USE_LV_GPU              0               /*1: Enable GPU interface支持显卡接口*/
#define USE_LV_REAL_DRAW        1               /*1: Enable function which draw directly to the frame buffer instead of VDB (required if LV_VDB_SIZE = 0) 启用直接绘制到帧缓冲区而不是VDB的函数(如果LV_VDB_SIZE = 0，则需要)*/
#define USE_LV_FILESYSTEM       0               /*1: Enable file system (required by images 启用文件系统(图像需要)*/

/*Compiler attributes*/
#define LV_ATTRIBUTE_TICK_INC                 /* Define a custom attribute to tick increment function  定义一个自定义属性来标记增量函数*/
#define LV_ATTRIBUTE_TASK_HANDLER

/*================
 *  THEME USAGE 使用主题
 *================*/
#define USE_LV_THEME_TEMPL      0       /*Just for test 只是为了测试*/
#define USE_LV_THEME_DEFAULT    0       /*Built mainly from the built-in styles. Consumes very few RAM 主要由内置样式构建。只消耗很少的RAM*/
#define USE_LV_THEME_ALIEN      0       /*Dark futuristic theme 黑暗的未来主题*/
#define USE_LV_THEME_NIGHT      0       /*Dark elegant theme黑暗的优雅的主题*/
#define USE_LV_THEME_MONO       0       /*Mono color theme for monochrome displays 单色显示器的单色主题*/
#define USE_LV_THEME_MATERIAL   0       /*Flat theme with bold colors and light shadows平面主题与大胆的颜色和光影*/
#define USE_LV_THEME_ZEN        0       /*Peaceful, mainly light theme 和平，主要是亮光的主题*/

/*==================
 *    FONT USAGE 字体利用
 *===================*/

/* More info about fonts: https://littlevgl.com/basics#fonts 更多关于字体的信息:https://littlevgl.com/basics#字体
 * To enable a built-in font use 1,2,4 or 8 values 要启用内置字体，请使用1、2、4或8个值
 * which will determine the bit-per-pixel 将确定每像素位*/
#define LV_FONT_DEFAULT        &lv_font_dejavu_20     /*Always set a default font from the built-in fonts 始终从内置字体设置默认字体*/

#define USE_LV_FONT_DEJAVU_10              0
#define USE_LV_FONT_DEJAVU_10_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_10_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_10              0

#define USE_LV_FONT_DEJAVU_20              4
#define USE_LV_FONT_DEJAVU_20_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_20_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_20              4

#define USE_LV_FONT_DEJAVU_30              0
#define USE_LV_FONT_DEJAVU_30_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_30_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_30              0

#define USE_LV_FONT_DEJAVU_40              0
#define USE_LV_FONT_DEJAVU_40_LATIN_SUP    0
#define USE_LV_FONT_DEJAVU_40_CYRILLIC     0
#define USE_LV_FONT_SYMBOL_40              0

/*===================
 *  LV_OBJ SETTINGS LV_OBJ设置
 *==================*/
#define LV_OBJ_FREE_NUM_TYPE    uint32_t    /*Type of free number attribute (comment out disable free number) free number属性的类型(注释掉禁用free number)*/
#define LV_OBJ_FREE_PTR         1           /*Enable the free pointer attribute 启用free pointer属性*/

/*==================
 *  LV OBJ X USAGE LV OBJ X使用情况
 *================*/
/*
 * Documentation of the object types: https://littlevgl.com/object-types 对象类型的文档:https://littlevgl.com/object-types
 */

/*****************
 * Simple object 简单对象
 *****************/

/*Label (dependencies: - 标签(附件 */
#define USE_LV_LABEL    1
#if USE_LV_LABEL != 0
#define LV_LABEL_SCROLL_SPEED       25     /*Hor, or ver. scroll speed [px/sec] in 'LV_LABEL_LONG_SCROLL/ROLL' mode 或者是真的。在“lv_label_long - scroll / rolls”模式中滚动速度[px/sec]*/
#endif

/*Image (dependencies: lv_label 图像(依赖关系:lv_label*/
#define USE_LV_IMG      1

/*Line (dependencies: 线(依赖关系 -*/
#define USE_LV_LINE     1

/*******************
 * Container objects 容器对象
 *******************/

/*Container (dependencies: 容器(依赖关系 -*/
#define USE_LV_CONT     1

/*Page (dependencies: lv_cont) 页面(依赖关系:lv_cont)*/
#define USE_LV_PAGE     1

/*Window (dependencies: lv_cont, lv_btn, lv_label, lv_img, lv_page) 窗口(依赖性:lv_cont, lv_btn, lv_label, lv_img, lv_page)*/
#define USE_LV_WIN      1

/*Tab (dependencies: lv_page, lv_btnm) Tab(依赖项:lv_page, lv_btnm)*/
#define USE_LV_TABVIEW      1
#if USE_LV_TABVIEW != 0
#define LV_TABVIEW_ANIM_TIME    0     /*Time of slide animation [ms] (0: no animation) 幻灯片动画时间[ms](0:无动画)*/
#endif

/*************************
 * Data visualizer objects 数据可视化工具对象
 *************************/

/*Bar (dependencies: -) 条(依赖:-)*/
#define USE_LV_BAR      1

/*Line meter (dependencies: *;) 行计(依赖关系*/
#define USE_LV_LMETER   1

/*Gauge (dependencies:bar, lmeter) 计(依赖关系:酒吧,lmeter)*/
#define USE_LV_GAUGE    1

/*Chart (dependencies: -)*/
#define USE_LV_CHART    1

/*LED (dependencies: -)*/
#define USE_LV_LED      1

/*Message box (dependencies: lv_rect, lv_btnm, lv_label)*/
#define USE_LV_MBOX     1

/*Text area (dependencies: lv_label, lv_page)*/
#define USE_LV_TA       1
#if USE_LV_TA != 0
#define LV_TA_CURSOR_BLINK_TIME 400     /*ms*/
#define LV_TA_PWD_SHOW_TIME     1500    /*ms*/
#endif

/*************************
 * User input objects
 *************************/

/*Button (dependencies: lv_cont*/
#define USE_LV_BTN      1

/*Button matrix (dependencies: -)*/
#define USE_LV_BTNM     1

/*Keyboard (dependencies: lv_btnm)*/
#define USE_LV_KB       1

/*Check box (dependencies: lv_btn, lv_label)*/
#define USE_LV_CB       1

/*List (dependencies: lv_page, lv_btn, lv_label, (lv_img optionally for icons ))*/
#define USE_LV_LIST     1
#if USE_LV_LIST != 0
#define LV_LIST_FOCUS_TIME  100 /*Default animation time of focusing to a list element [ms] (0: no animation)  */
#endif

/*Drop down list (dependencies: lv_page, lv_label)*/
#define USE_LV_DDLIST    1
#if USE_LV_DDLIST != 0
#define LV_DDLIST_ANIM_TIME     200     /*Open and close default animation time [ms] (0: no animation)*/
#endif

/*Roller (dependencies: lv_ddlist)*/
#define USE_LV_ROLLER    1
#if USE_LV_ROLLER != 0
#define LV_ROLLER_ANIM_TIME     200     /*Focus animation time [ms] (0: no animation)*/
#endif

/*Slider (dependencies: lv_bar)*/
#define USE_LV_SLIDER    1

/*Switch (dependencies: lv_slider)*/
#define USE_LV_SW       1

#endif /*LV_CONF_H*/
