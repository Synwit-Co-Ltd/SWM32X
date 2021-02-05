/**
 * @file lv_refr.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf.h"
#include <stddef.h>
#include "lv_refr.h"
#include "lv_vdb.h"
#include "../lv_hal/lv_hal_tick.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_mem.h"
#include "SWM320.h"
#include "main.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    lv_area_t area;
    uint8_t joined;
}lv_join_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_refr_task(void * param);
static void lv_refr_join_area(void);
static void lv_refr_areas(void);
#if LV_VDB_SIZE == 0
static void lv_refr_area_no_vdb(const lv_area_t * area_p);
#else
static void lv_refr_area_with_vdb(const lv_area_t * area_p);
static void lv_refr_area_part_vdb(const lv_area_t * area_p);
#endif
static lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj);
static void lv_refr_obj_and_children(lv_obj_t * top_p, const lv_area_t * mask_p);
static void lv_refr_obj(lv_obj_t * obj, const lv_area_t * mask_ori_p);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_join_t inv_buf[LV_INV_FIFO_SIZE];
static uint16_t inv_buf_p;
static void (*monitor_cb)(uint32_t, uint32_t); /*Monitor the rendering time*/
static void (*round_cb)(lv_area_t*);           /*If set then called to modify invalidated areas for special display controllers*/
static uint32_t px_num;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the screen refresh subsystem
 */
void lv_refr_init(void)
{    
    inv_buf_p = 0;
    memset(inv_buf, 0, sizeof(inv_buf));

    lv_task_t* task;
    task = lv_task_create(lv_refr_task, LV_REFR_PERIOD, LV_TASK_PRIO_MID, NULL);
    lv_task_ready(task);        /*Be sure the screen will be refreshed immediately on start up*/
}

/**
 * Invalidate an area ʹ������Ч
 * @param area_p pointer to area which should be invalidated ָ��ָ��Ӧ����Ч������
 */
void lv_inv_area(const lv_area_t * area_p)
{
    /*Clear the invalidate buffer if the parameter is NULL �������Ϊ�գ������invalidate������*/
    if(area_p == NULL) {
        inv_buf_p = 0;
        return;
    }
    
    lv_area_t scr_area;
    scr_area.x1 = 0;
    scr_area.y1 = 0;
    scr_area.x2 = LV_HOR_RES - 1;
    scr_area.y2 = LV_VER_RES - 1;
    
    lv_area_t com_area;    
    bool suc;

    suc = lv_area_union(&com_area, area_p, &scr_area);

    /*The area is truncated to the screen�����򱻽ض̵���Ļ*/
    if(suc != false) {
        if(round_cb) round_cb(&com_area);

    	/*Save only if this area is not in one of the saved areas ֻ�е����������ѱ��������֮һʱ�Ž��б���*/
    	uint16_t i;
    	for(i = 0; i < inv_buf_p; i++) {
    	    if(lv_area_is_in(&com_area, &inv_buf[i].area) != false) return;
    	}

        /*Save the area��������*/
    	if(inv_buf_p < LV_INV_FIFO_SIZE) {
            lv_area_copy(&inv_buf[inv_buf_p].area,&com_area);
    	} else {/*If no place for the area add the screen���������û��λ�ã��������Ļ*/
    	    inv_buf_p = 0;
            lv_area_copy(&inv_buf[inv_buf_p].area,&scr_area);
        }
    	inv_buf_p ++;
    }
}

/**
 * Set a function to call after every refresh to announce the refresh time and the number of refreshed pixels
 * @param cb pointer to a callback function (void my_refr_cb(uint32_t time_ms, uint32_t px_num))
 *           time_ms: refresh time in [ms]
 *           px_num: not the drawn pixels but the number of affected pixels of the screen
 *                   (more pixels are drawn because of overlapping objects)
 */
void lv_refr_set_monitor_cb(void (*cb)(uint32_t, uint32_t))
{
    monitor_cb = cb;
}

/**
 * Called when an area is invalidated to modify the coordinates of the area.
 * Special display controllers may require special coordinate rounding
 * @param cb pointer to the a function which will modify the area
 */
void lv_refr_set_round_cb(void(*cb)(lv_area_t*))
{
    round_cb = cb;
}

/**
 * Get the number of areas in the buffer
 * @return number of invalid areas
 */
uint16_t lv_refr_get_buf_size(void)
{
    return inv_buf_p;
}

/**
 * Pop (delete) the last 'num' invalidated areas from the buffer
 * @param num number of areas to delete
 */
void lv_refr_pop_from_buf(uint16_t num)
{
    if(inv_buf_p < num) inv_buf_p = 0;
    else inv_buf_p -= num;
}

/**********************
 *   STATIC FUNCTIONS 
 **********************/

/**
 * Called periodically to handle the refreshing ���ڵ����Դ���ˢ��
 * @param param unused δʹ��
 */
static void lv_refr_task(void * param)
{
    (void)param;

    uint32_t start = lv_tick_get();


	lv_refr_join_area();

#ifndef DEBUG_FRESH_MODE
//	NVIC_DisableIRQ(LCD_IRQn);
//	while(LCD_IsBusy(LCD));
	lv_refr_areas();
//	NVIC_EnableIRQ(LCD_IRQn);
//	lv_refr_areas();
#else	
//	NVIC_DisableIRQ(LCD_IRQn);
//	while(LCD_IsBusy(LCD));
//	lv_refr_areas();
//	NVIC_EnableIRQ(LCD_IRQn);
#endif

    bool refr_done = false;
    if(inv_buf_p != 0) refr_done = true;
    memset(inv_buf, 0, sizeof(inv_buf));
    inv_buf_p = 0;

    /* In the callback lv_obj_inv can occur �ڻص�����lv_obj_inv�п��Է���
     * therefore be sure the inv_buf is cleared prior to it��ˣ�ȷ��in_buf����֮ǰ�����*/
    if(refr_done != false) {
        if(monitor_cb != NULL) {
            monitor_cb(lv_tick_elaps(start), px_num);
        }
    }
}


/**
 * Join the areas which has got common parts �����й������ֵ�����
 */
static void lv_refr_join_area(void)
{
    uint32_t join_from;
    uint32_t join_in;
    lv_area_t joined_area;
    for(join_in = 0; join_in < inv_buf_p; join_in++) {
        if(inv_buf[join_in].joined != 0) continue;
        
        /*Check all areas to join them in 'join_in'�������������join_in�м�������*/
        for(join_from = 0; join_from < inv_buf_p; join_from++) {
            /*Handle only unjoined areas and ignore itself ֻ����δ���ӵ����򣬺����䱾��*/
            if(inv_buf[join_from].joined != 0 || join_in == join_from) {
                continue;
            }

            /*Check if the areas are on each other �����Щ�����Ƿ��໥�ص�*/
            if(lv_area_is_on(&inv_buf[join_in].area,
                          &inv_buf[join_from].area) == false)
            {
                continue;
            }
            
            lv_area_join(&joined_area, &inv_buf[join_in].area,
                                    &inv_buf[join_from].area);

            /*Join two area only if the joined area size is smaller�������ӵ������С��Сʱ��������������*/
            if(lv_area_get_size(&joined_area) < 
             (lv_area_get_size(&inv_buf[join_in].area) + lv_area_get_size(&inv_buf[join_from].area))) {
                lv_area_copy(&inv_buf[join_in].area, &joined_area);

                /*Mark 'join_form' is joined into 'join_in'����join_form�����Ϊ��join_in��*/
                inv_buf[join_from].joined = 1;
            }
        }   
    }
}

/**
 * Refresh the joined areas ˢ����������
 */
static void lv_refr_areas(void)
{
    px_num = 0;
    uint32_t i;

    for(i = 0; i < inv_buf_p; i++) {
        /*Refresh the unjoined areasˢ��δ���������*/
        if(inv_buf[i].joined == 0) {
            /*If there is no VDB do simple drawing���û��VDB���򵥵Ļ�ͼ*/
#if LV_VDB_SIZE == 0
            lv_refr_area_no_vdb(&inv_buf[i].area);
#else
            /*If VDB is used...���ʹ��VDB��*/
//						NVIC_DisableIRQ(LCD_IRQn);
//           	while(LCD_IsBusy(LCD));
            lv_refr_area_with_vdb(&inv_buf[i].area);
//					  NVIC_EnableIRQ(LCD_IRQn);
#endif
            if(monitor_cb != NULL) px_num += lv_area_get_size(&inv_buf[i].area);
        }
    }

}

#if LV_VDB_SIZE == 0
/**
 * Refresh an area if there is no Virtual Display Buffer
 * @param area_p pointer to an area to refresh
 */
static void lv_refr_area_no_vdb(const lv_area_t * area_p)
{
    lv_obj_t * top_p;
    
    /*Get top object which is not covered by others*/    
    top_p = lv_refr_get_top_obj(area_p, lv_scr_act());
    
    /*Do the refreshing*/
    lv_refr_obj_and_children(top_p, area_p);
}

#else

/**
 * Refresh an area if there is Virtual Display Buffer �����������ʾ������ ��ˢ��һ������
 * @param area_p  pointer to an area to refresh ָ��Ҫˢ�µ������area_Pָ��
 */
static void lv_refr_area_with_vdb(const lv_area_t * area_p)
{
    /*Calculate the max row num�����������*/
    lv_coord_t w = lv_area_get_width(area_p);
    lv_coord_t h = lv_area_get_height(area_p);
    lv_coord_t y2 = area_p->y2 >= LV_VER_RES ? y2 = LV_VER_RES - 1 : area_p->y2;

    uint32_t max_row = (uint32_t) LV_VDB_SIZE / w;
    if(max_row > h) max_row = h;

    /*Always use the full rowʼ��ʹ������*/
    uint32_t row;
    lv_coord_t row_last = 0;
    for(row = area_p->y1; row  + max_row - 1 <= y2; row += max_row)  {
        lv_vdb_t * vdb_p = lv_vdb_get();

        /*Calc. the next y coordinates of VDB����VDB����һ��y����*/
        vdb_p->area.x1 = area_p->x1;
        vdb_p->area.x2 = area_p->x2;
        vdb_p->area.y1 = row;
        vdb_p->area.y2 = row + max_row - 1;
        if(vdb_p->area.y2 > y2) vdb_p->area.y2 = y2;
        row_last = vdb_p->area.y2;
        lv_refr_area_part_vdb(area_p);
    }
    
    /*If the last y coordinates are not handled yet�����û�д������һ��y���� ...*/
    if(y2 != row_last) {
        lv_vdb_t * vdb_p = lv_vdb_get();

        /*Calc. the next y coordinates of VDBVDB����VDB����һ��y����*/
        vdb_p->area.x1 = area_p->x1;
        vdb_p->area.x2 = area_p->x2;
        vdb_p->area.y1 = row;
        vdb_p->area.y2 = y2;

        /*Refresh this part tooҲˢ���ⲿ��*/
//	    	NVIC_DisableIRQ(LCD_IRQn);
//			  while(LCD_IsBusy(LCD));
        lv_refr_area_part_vdb(area_p);
//			  NVIC_EnableIRQ(LCD_IRQn);
    }
}

/**
 * Refresh a part of an area which is on the actual Virtual Display Buffer ˢ��λ��ʵ��������ʾ�������ϵ������һ����
 * @param area_p pointer to an area to refresh ָ��Ҫˢ�µ������area_pָ��
 */
static void lv_refr_area_part_vdb(const lv_area_t * area_p)
{
    lv_vdb_t * vdb_p = lv_vdb_get();
    lv_obj_t * top_p;

    /*Get the new mask from the original area and the act. VDB ��ԭ����������ж��õ��µ����� ������ʾ������
     It will be a part of 'area_p'������area_p��һ����*/
    lv_area_t start_mask;
    lv_area_union(&start_mask, area_p, &vdb_p->area);

    /*Get the most top object which is not covered by others��ȡ����û�и��ǵ�������Ķ���Ķ���*/
    top_p = lv_refr_get_top_obj(&start_mask, lv_scr_act());

    /*Do the refreshing from the top object�Ӷ�������ˢ��*/
    lv_refr_obj_and_children(top_p, &start_mask);

    /*Also refresh top and sys layer unconditionally ��������������ˢ��top��sys�� */
    lv_refr_obj_and_children(lv_layer_top(), &start_mask);
    lv_refr_obj_and_children(lv_layer_sys(), &start_mask);

    /*Flush the content of the VDB ˢ��VDB������*/ 
    lv_vdb_flush();
}

#endif /*LV_VDB_SIZE == 0*/

/**
 * Search the most top object which fully covers an area ��������Ķ�������ȫ������һ������
 * @param area_p pointer to an area area_pָ��һ������
 * @param obj the first object to start the searching (typically a screen) obj��һ�������Ķ���ͨ����һ����Ļ��
 * @return 
 */
static lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj)
{
    lv_obj_t * i;
    lv_obj_t * found_p = NULL;
    
    /*If this object is fully cover the draw area check the children too ������������ȫ�����˻�������Ҳ����Ӷ��� */
    if(lv_area_is_in(area_p, &obj->coords) && obj->hidden == 0)
    {
        LL_READ(obj->child_ll, i)        {
            found_p = lv_refr_get_top_obj(area_p, i);
            
            /*If a children is ok then break���������󷵻�OK �Ǿ�ֱ���˳�*/
            if(found_p != NULL) {
                break;
            }
        }
        
        /*If no better children check this object ���û�и��õ����� ����������*/
        if(found_p == NULL) {
            lv_style_t * style = lv_obj_get_style(obj);
            if(style->body.opa == LV_OPA_COVER &&
               obj->design_func(obj, area_p, LV_DESIGN_COVER_CHK) != false) {
                found_p = obj;
            }
        }
    }
    
    return found_p;
}

/**
 * Make the refreshing from an object. Draw all its children and the youngers too. ˢ�¶��� �������е����������ĵ���
 * @param top_p pointer to an objects. Start the drawing from it.  ָ������Top_Pָ�� �����￪ʼ��
 * @param mask_p pointer to an area, the objects will be drawn only here  mask_pָ��ָ��һ�����򣬶���ֻ�����������
 */
static void lv_refr_obj_and_children(lv_obj_t * top_p, const lv_area_t * mask_p)
{
    /* Normally always will be a top_obj (at least the screen) ͨ������top_obj(��������Ļ��)
     * but in special cases (e.g. if the screen has alpha) it won't. �������������(���磬�����Ļ��alphaֵ)���򲻻ᡣ
     * In this case use the screen directly �ڱ�����ֱ��ʹ����Ļ*/
    if(top_p == NULL) top_p = lv_scr_act();
    
    /*Refresh the top object and its childrenˢ�¶����������Ӷ���*/
    lv_refr_obj(top_p, mask_p);
    
    /*Draw the 'younger' sibling objects because they can be on top_obj���ơ���С�ġ�ͬ��������Ϊ���ǿ���λ��top_obj�� */
    lv_obj_t * par;
    lv_obj_t * i;
    lv_obj_t * border_p = top_p;

    par = lv_obj_get_parent(top_p);

    /*Do until not reach the screen ֱ��û�е�����Ļ*/
    while(par != NULL) {
        /*object before border_p has to be redrawn�������»���border_p֮ǰ�Ķ���*/
        i = lv_ll_get_prev(&(par->child_ll), border_p);

        while(i != NULL) { 
            /*Refresh the objects ˢ�¶���*/
            lv_refr_obj(i, mask_p);
            i = lv_ll_get_prev(&(par->child_ll), i);
        }  
        
        /*The new border will be there last parents,�±߽罫�����ĸ���
         *so the 'younger' brothers of parent will be refreshed ����������ġ��ֵ��ࡱ�ͻ�����ˢ��*/
        border_p = par;
        /*Go a level deeper����һ��*/
        par = lv_obj_get_parent(par);
    }

    /*Call the post draw design function of the parents of the to object���ö���ĸ�����ĺ������ƺ���*/
    par = lv_obj_get_parent(top_p);
    while(par != NULL) {
        par->design_func(par, mask_p, LV_DESIGN_DRAW_POST);
        par = lv_obj_get_parent(par);
    }
}

/**
 * Refresh an object an all of its children. (Called recursively) ˢ��һ�������������Ӷ���
 * @param obj pointer to an object to refresh ָ��Ҫˢ�µĶ����objָ��
 * @param mask_ori_p pointer to an area, the objects will be drawn only here mask_ori_pָ��ָ��һ�����򣬶���ֻ�����������
 */
static void lv_refr_obj(lv_obj_t * obj, const lv_area_t * mask_ori_p)
{
    /*Do not refresh hidden objects��Ҫˢ�����ض���*/
    if(obj->hidden != 0) return;
    
    bool union_ok;  /* Store the return value of area_union �洢area_union�ķ���ֵ */
    /* Truncate the original mask to the coordinates of the parent ��ԭʼ����ضϵ��������������
     * because the parent and its children are visible only here ��Ϊ�������ӽ��ֻ������ɼ� */
    lv_area_t obj_mask;
    lv_area_t obj_ext_mask;
    lv_area_t obj_area;
    lv_coord_t ext_size = obj->ext_size;
    lv_obj_get_coords(obj, &obj_area);
    obj_area.x1 -= ext_size;
    obj_area.y1 -= ext_size;
    obj_area.x2 += ext_size;
    obj_area.y2 += ext_size;
    union_ok = lv_area_union(&obj_ext_mask, mask_ori_p, &obj_area);
    
    /*Draw the parent and its children only if they ore on 'mask_parent'ֻ�е����༰������λ��'mask_parent'��ʱ���Ż��Ƹ��༰������*/
    if(union_ok != false) {

        /* Redraw the object �ػ�����*/
        lv_style_t * style = lv_obj_get_style(obj);
        if(style->body.opa != LV_OPA_TRANSP) {
            obj->design_func(obj, &obj_ext_mask, LV_DESIGN_DRAW_MAIN);
            //tick_wait_ms(100);  /*DEBUG: Wait after every object draw to see the order of drawing   tick_wait_ms (100);����:�ȴ�ÿ�����������ɺ�鿴����˳��*/
        }

        /*Create a new 'obj_mask' without 'ext_size' because the children can't be visible there����һ���µ�'obj_mask'����������'ext_size'����Ϊ���е���Ԫ�ز��ɼ�*/
        lv_obj_get_coords(obj, &obj_area);
        union_ok = lv_area_union(&obj_mask, mask_ori_p, &obj_area);
        if(union_ok != false) {
			lv_area_t mask_child; /*Mask from obj and its child ��obj�����������ȡ����*/
			lv_obj_t * child_p;
			lv_area_t child_area;
			LL_READ_BACK(obj->child_ll, child_p)
			{
				lv_obj_get_coords(child_p, &child_area);
				ext_size = child_p->ext_size;
				child_area.x1 -= ext_size;
				child_area.y1 -= ext_size;
				child_area.x2 += ext_size;
				child_area.y2 += ext_size;
				/* Get the union (common parts) of original mask (from obj) ��ȡԭʼ��ģ��union (common parts)(����obj)
				 * and its child ������*/
				union_ok = lv_area_union(&mask_child, &obj_mask, &child_area);

				/*If the parent and the child has common area then refresh the child ������ڵ���ӽڵ��й���������ˢ���ӽڵ�*/
				if(union_ok) {
					/*Refresh the next childrenˢ����һ����Ԫ��*/
					lv_refr_obj(child_p, &mask_child);
				}
			}
        }

        /* If all the children are redrawn make 'post draw' design ������е���Ԫ�ض������»��ƣ�����С�����ơ���� */
        if(style->body.opa != LV_OPA_TRANSP) {
		  obj->design_func(obj, &obj_ext_mask, LV_DESIGN_DRAW_POST);
		}
    }
}
