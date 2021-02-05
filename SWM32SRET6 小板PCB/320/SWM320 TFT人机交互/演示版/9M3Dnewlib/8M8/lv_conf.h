/**
 * @file lv_conf.h
 * 
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/*----------------
 * Dynamic memory ��̬�ڴ�
 *----------------*/

/* Memory size which will be used by the library �⽫ʹ�õ��ڴ��С
 * to store the graphical objects and other data �洢ͼ�ζ������������ */
#define LV_MEM_CUSTOM      0                /*1: use custom malloc/free, 0: use the built-in lv_mem_alloc/lv_mem_free 1:ʹ�ö��Ƶ�mallocHeader for the dynamic memory function/free, 0:ʹ�����õ�lv_mem_alloc/lv_mem_free */
#if LV_MEM_CUSTOM == 0
#define LV_MEM_SIZE    (64U * 1024U)        /*Size memory used by `lv_mem_alloc` in bytes (>= 2kB) lv_mem_allocʹ�õ��ڴ��С(���ֽ�Ϊ��λ)(>= 2kB)*/
#define LV_MEM_ATTR                         /*Complier prefix for big array declaration ���ڴ����������ı�����ǰ׺ */
#define LV_MEM_AUTO_DEFRAG  1               /*Automatically defrag on free �Զ��������*/
#else       /*LV_MEM_CUSTOM*/
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /**/
#define LV_MEM_CUSTOM_ALLOC   malloc       /*Wrapper to malloc ��̬�ڴ溯����ͷ*/
#define LV_MEM_CUSTOM_FREE    free         /*Wrapper to free ��װ������*/
#endif     /*LV_MEM_CUSTOM*/

/*===================
   Graphical settings ͼ�λ�����
 *===================*/

/* Horizontal and vertical resolution of the library. ���ˮƽ�ʹ�ֱ�ֱ���*/
#define LV_HOR_RES          (480)
#define LV_VER_RES          (272)
#define LV_DPI              100

/* Size of VDB (Virtual Display Buffer: the internal graphics buffer).VDB��С(������ʾ������:�ڲ�ͼ�λ�����)��
 * Required for buffered drawing, opacity and anti-aliasing ��Ҫ�����ͼ����͸���ȺͿ����
 * VDB makes the double buffering, you don't need to deal with it! VDB����˫�ػ��壬�㲻��Ҫ������!
 * Typical size: ~1/10 screen ���ͳߴ�:~1/10��Ļ */
#define LV_VDB_SIZE         9600  /*Size of VDB in pixel count (1/10 screen size is good for first)VDB�Ĵ�С(������Ϊ��λ)(1/10��Ļ��С����)*/
#define LV_VDB_ADR          0                  /*Place VDB to a specific address (e.g. in external RAM) (0: allocate automatically into RAM)��VDB���õ��ض��ĵ�ַ(�����ⲿRAM��)(0:�Զ����䵽RAM��)*/

/* Use two Virtual Display buffers (VDB) parallelize rendering and flushing (optional) ʹ������������ʾ������(VDB)���л����ֺ�ˢ��(��ѡ)
 * The flushing should use DMA to write the frame buffer in the background ˢ��Ӧ��ʹ��DMA�ں�̨д��֡������*/
#define LV_VDB_DOUBLE       0       /*1: Enable the use of 2 VDBs ����2��VDBs*/
#define LV_VDB2_ADR         0     /*Place VDB2 to a specific address (e.g. in external RAM) (0: allocate automatically into RAM) ��VDB2���õ��ض��ĵ�ַ(�������ⲿRAM��)(0:�Զ����䵽RAM��) */

/* Enable anti-aliasing (lines, and radiuses will be smoothed) ���ÿ����(�����ͻ��Ƚ���ƽ������) */
#define LV_ANTIALIAS        0       /*1: Enable anti-aliasing ���ÿ����*/

/*Screen refresh settings ��Ļˢ������*/
#define LV_REFR_PERIOD      50    /*Screen refresh period in milliseconds ��Ļˢ������(�Ժ���Ϊ��λ) */
#define LV_INV_FIFO_SIZE    32    /*The average count of objects on a screen ��Ļ�϶����ƽ������  */

/*=================
   Misc. setting ��������
 *=================*/

/*Input device settings�����豸������*/
#define LV_INDEV_READ_PERIOD            50                     /*Input device read period in milliseconds �����豸��ȡ����(�Ժ���Ϊ��λ)*/
#define LV_INDEV_POINT_MARKER           0                      /*Mark the pressed points  (required: USE_LV_REAL_DRAW = 1)��ǰ�ѹ��(Ҫ��:USE_LV_REAL_DRAW = 1)*/
#define LV_INDEV_DRAG_LIMIT             10                     /*Drag threshold in pixels �϶���ֵ(����) */
#define LV_INDEV_DRAG_THROW             20                     /*Drag throw slow-down in [%]. Greater value means faster slow-down ��[%]���Ϸ����١������ֵ��ζ�Ÿ���ļ���*/
#define LV_INDEV_LONG_PRESS_TIME        400                    /*Long press time in milliseconds����ʱ��(�Ժ���Ϊ��λ)*/
#define LV_INDEV_LONG_PRESS_REP_TIME    100                    /*Repeated trigger period in long press [ms] �����ظ���������[ms]*/

/*Color settings*/
#define LV_COLOR_DEPTH     16                     /*Color depth: 1/8/16/24 ��ɫ���:1/8/16/24*/
#define LV_COLOR_TRANSP    LV_COLOR_LIME          /*Images pixels with this color will not be drawn (with chroma keying) ʹ�ô���ɫ��ͼ�����ؽ��������(ʹ��ɫ�ȼ���)*/

/*Text settings*/
#define LV_TXT_UTF8             1                /*Enable UTF-8 coded Unicode character usage����UTF-8�����Unicode�ַ�ʹ�� */
#define LV_TXT_BREAK_CHARS     " ,.;:-_"         /*Can break texts on these chars ��������Щ�ַ����ж��ı�*/

/*Graphics feature usage*/
#define USE_LV_ANIMATION        0               /*1: Enable all animations�����ж���*/
#define USE_LV_SHADOW           0               /*1: Enable shadows ����Ӱ*/
#define USE_LV_GROUP            1               /*1: Enable object groups (for keyboards) ���ö�����(���ڼ���)*/
#define USE_LV_GPU              0               /*1: Enable GPU interface֧���Կ��ӿ�*/
#define USE_LV_REAL_DRAW        1               /*1: Enable function which draw directly to the frame buffer instead of VDB (required if LV_VDB_SIZE = 0) ����ֱ�ӻ��Ƶ�֡������������VDB�ĺ���(���LV_VDB_SIZE = 0������Ҫ)*/
#define USE_LV_FILESYSTEM       0               /*1: Enable file system (required by images �����ļ�ϵͳ(ͼ����Ҫ)*/

/*Compiler attributes*/
#define LV_ATTRIBUTE_TICK_INC                 /* Define a custom attribute to tick increment function  ����һ���Զ��������������������*/
#define LV_ATTRIBUTE_TASK_HANDLER

/*================
 *  THEME USAGE ʹ������
 *================*/
#define USE_LV_THEME_TEMPL      0       /*Just for test ֻ��Ϊ�˲���*/
#define USE_LV_THEME_DEFAULT    0       /*Built mainly from the built-in styles. Consumes very few RAM ��Ҫ��������ʽ������ֻ���ĺ��ٵ�RAM*/
#define USE_LV_THEME_ALIEN      0       /*Dark futuristic theme �ڰ���δ������*/
#define USE_LV_THEME_NIGHT      0       /*Dark elegant theme�ڰ������ŵ�����*/
#define USE_LV_THEME_MONO       0       /*Mono color theme for monochrome displays ��ɫ��ʾ���ĵ�ɫ����*/
#define USE_LV_THEME_MATERIAL   0       /*Flat theme with bold colors and light shadowsƽ��������󵨵���ɫ�͹�Ӱ*/
#define USE_LV_THEME_ZEN        0       /*Peaceful, mainly light theme ��ƽ����Ҫ�����������*/

/*==================
 *    FONT USAGE ��������
 *===================*/

/* More info about fonts: https://littlevgl.com/basics#fonts ��������������Ϣ:https://littlevgl.com/basics#����
 * To enable a built-in font use 1,2,4 or 8 values Ҫ�����������壬��ʹ��1��2��4��8��ֵ
 * which will determine the bit-per-pixel ��ȷ��ÿ����λ*/
#define LV_FONT_DEFAULT        &lv_font_dejavu_20     /*Always set a default font from the built-in fonts ʼ�մ�������������Ĭ������*/

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
 *  LV_OBJ SETTINGS LV_OBJ����
 *==================*/
#define LV_OBJ_FREE_NUM_TYPE    uint32_t    /*Type of free number attribute (comment out disable free number) free number���Ե�����(ע�͵�����free number)*/
#define LV_OBJ_FREE_PTR         1           /*Enable the free pointer attribute ����free pointer����*/

/*==================
 *  LV OBJ X USAGE LV OBJ Xʹ�����
 *================*/
/*
 * Documentation of the object types: https://littlevgl.com/object-types �������͵��ĵ�:https://littlevgl.com/object-types
 */

/*****************
 * Simple object �򵥶���
 *****************/

/*Label (dependencies: - ��ǩ(���� */
#define USE_LV_LABEL    1
#if USE_LV_LABEL != 0
#define LV_LABEL_SCROLL_SPEED       25     /*Hor, or ver. scroll speed [px/sec] in 'LV_LABEL_LONG_SCROLL/ROLL' mode ��������ġ��ڡ�lv_label_long - scroll / rolls��ģʽ�й����ٶ�[px/sec]*/
#endif

/*Image (dependencies: lv_label ͼ��(������ϵ:lv_label*/
#define USE_LV_IMG      1

/*Line (dependencies: ��(������ϵ -*/
#define USE_LV_LINE     1

/*******************
 * Container objects ��������
 *******************/

/*Container (dependencies: ����(������ϵ -*/
#define USE_LV_CONT     1

/*Page (dependencies: lv_cont) ҳ��(������ϵ:lv_cont)*/
#define USE_LV_PAGE     1

/*Window (dependencies: lv_cont, lv_btn, lv_label, lv_img, lv_page) ����(������:lv_cont, lv_btn, lv_label, lv_img, lv_page)*/
#define USE_LV_WIN      1

/*Tab (dependencies: lv_page, lv_btnm) Tab(������:lv_page, lv_btnm)*/
#define USE_LV_TABVIEW      1
#if USE_LV_TABVIEW != 0
#define LV_TABVIEW_ANIM_TIME    0     /*Time of slide animation [ms] (0: no animation) �õ�Ƭ����ʱ��[ms](0:�޶���)*/
#endif

/*************************
 * Data visualizer objects ���ݿ��ӻ����߶���
 *************************/

/*Bar (dependencies: -) ��(����:-)*/
#define USE_LV_BAR      1

/*Line meter (dependencies: *;) �м�(������ϵ*/
#define USE_LV_LMETER   1

/*Gauge (dependencies:bar, lmeter) ��(������ϵ:�ư�,lmeter)*/
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
