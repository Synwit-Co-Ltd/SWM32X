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
 * Invalidate an area 使区域无效
 * @param area_p pointer to area which should be invalidated 指针指向应该无效的区域
 */
void lv_inv_area(const lv_area_t * area_p)
{
    /*Clear the invalidate buffer if the parameter is NULL 如果参数为空，则清除invalidate缓冲区*/
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

    /*The area is truncated to the screen该区域被截短到屏幕*/
    if(suc != false) {
        if(round_cb) round_cb(&com_area);

    	/*Save only if this area is not in one of the saved areas 只有当此区域不在已保存的区域之一时才进行保存*/
    	uint16_t i;
    	for(i = 0; i < inv_buf_p; i++) {
    	    if(lv_area_is_in(&com_area, &inv_buf[i].area) != false) return;
    	}

        /*Save the area保存区域*/
    	if(inv_buf_p < LV_INV_FIFO_SIZE) {
            lv_area_copy(&inv_buf[inv_buf_p].area,&com_area);
    	} else {/*If no place for the area add the screen如果该区域没有位置，则添加屏幕*/
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
 * Called periodically to handle the refreshing 定期调用以处理刷新
 * @param param unused 未使用
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

    /* In the callback lv_obj_inv can occur 在回调函数lv_obj_inv中可以发生
     * therefore be sure the inv_buf is cleared prior to it因此，确保in_buf在它之前被清除*/
    if(refr_done != false) {
        if(monitor_cb != NULL) {
            monitor_cb(lv_tick_elaps(start), px_num);
        }
    }
}


/**
 * Join the areas which has got common parts 加入有公共部分的区域
 */
static void lv_refr_join_area(void)
{
    uint32_t join_from;
    uint32_t join_in;
    lv_area_t joined_area;
    for(join_in = 0; join_in < inv_buf_p; join_in++) {
        if(inv_buf[join_in].joined != 0) continue;
        
        /*Check all areas to join them in 'join_in'检查所有区域，在join_in中加入它们*/
        for(join_from = 0; join_from < inv_buf_p; join_from++) {
            /*Handle only unjoined areas and ignore itself 只处理未连接的区域，忽略其本身*/
            if(inv_buf[join_from].joined != 0 || join_in == join_from) {
                continue;
            }

            /*Check if the areas are on each other 检查这些区域是否相互重叠*/
            if(lv_area_is_on(&inv_buf[join_in].area,
                          &inv_buf[join_from].area) == false)
            {
                continue;
            }
            
            lv_area_join(&joined_area, &inv_buf[join_in].area,
                                    &inv_buf[join_from].area);

            /*Join two area only if the joined area size is smaller仅当连接的区域大小较小时才连接两个区域*/
            if(lv_area_get_size(&joined_area) < 
             (lv_area_get_size(&inv_buf[join_in].area) + lv_area_get_size(&inv_buf[join_from].area))) {
                lv_area_copy(&inv_buf[join_in].area, &joined_area);

                /*Mark 'join_form' is joined into 'join_in'将“join_form”标记为“join_in”*/
                inv_buf[join_from].joined = 1;
            }
        }   
    }
}

/**
 * Refresh the joined areas 刷新连接区域
 */
static void lv_refr_areas(void)
{
    px_num = 0;
    uint32_t i;

    for(i = 0; i < inv_buf_p; i++) {
        /*Refresh the unjoined areas刷新未加入的区域*/
        if(inv_buf[i].joined == 0) {
            /*If there is no VDB do simple drawing如果没有VDB做简单的绘图*/
#if LV_VDB_SIZE == 0
            lv_refr_area_no_vdb(&inv_buf[i].area);
#else
            /*If VDB is used...如果使用VDB…*/
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
 * Refresh an area if there is Virtual Display Buffer 如果有虚拟显示缓冲区 则刷新一个区域
 * @param area_p  pointer to an area to refresh 指向要刷新的区域的area_P指针
 */
static void lv_refr_area_with_vdb(const lv_area_t * area_p)
{
    /*Calculate the max row num计算最大行数*/
    lv_coord_t w = lv_area_get_width(area_p);
    lv_coord_t h = lv_area_get_height(area_p);
    lv_coord_t y2 = area_p->y2 >= LV_VER_RES ? y2 = LV_VER_RES - 1 : area_p->y2;

    uint32_t max_row = (uint32_t) LV_VDB_SIZE / w;
    if(max_row > h) max_row = h;

    /*Always use the full row始终使用整行*/
    uint32_t row;
    lv_coord_t row_last = 0;
    for(row = area_p->y1; row  + max_row - 1 <= y2; row += max_row)  {
        lv_vdb_t * vdb_p = lv_vdb_get();

        /*Calc. the next y coordinates of VDB计算VDB的下一个y坐标*/
        vdb_p->area.x1 = area_p->x1;
        vdb_p->area.x2 = area_p->x2;
        vdb_p->area.y1 = row;
        vdb_p->area.y2 = row + max_row - 1;
        if(vdb_p->area.y2 > y2) vdb_p->area.y2 = y2;
        row_last = vdb_p->area.y2;
        lv_refr_area_part_vdb(area_p);
    }
    
    /*If the last y coordinates are not handled yet如果还没有处理最后一个y坐标 ...*/
    if(y2 != row_last) {
        lv_vdb_t * vdb_p = lv_vdb_get();

        /*Calc. the next y coordinates of VDBVDB计算VDB的下一个y坐标*/
        vdb_p->area.x1 = area_p->x1;
        vdb_p->area.x2 = area_p->x2;
        vdb_p->area.y1 = row;
        vdb_p->area.y2 = y2;

        /*Refresh this part too也刷新这部分*/
//	    	NVIC_DisableIRQ(LCD_IRQn);
//			  while(LCD_IsBusy(LCD));
        lv_refr_area_part_vdb(area_p);
//			  NVIC_EnableIRQ(LCD_IRQn);
    }
}

/**
 * Refresh a part of an area which is on the actual Virtual Display Buffer 刷新位于实际虚拟显示缓冲区上的区域的一部分
 * @param area_p pointer to an area to refresh 指向要刷新的区域的area_p指针
 */
static void lv_refr_area_part_vdb(const lv_area_t * area_p)
{
    lv_vdb_t * vdb_p = lv_vdb_get();
    lv_obj_t * top_p;

    /*Get the new mask from the original area and the act. VDB 从原来的区域和行动得到新的掩码 虚拟显示缓存区
     It will be a part of 'area_p'它将是area_p的一部分*/
    lv_area_t start_mask;
    lv_area_union(&start_mask, area_p, &vdb_p->area);

    /*Get the most top object which is not covered by others获取其他没有覆盖的最上面的对象的对象*/
    top_p = lv_refr_get_top_obj(&start_mask, lv_scr_act());

    /*Do the refreshing from the top object从顶部对象刷新*/
    lv_refr_obj_and_children(top_p, &start_mask);

    /*Also refresh top and sys layer unconditionally 还可以无条件地刷新top和sys层 */
    lv_refr_obj_and_children(lv_layer_top(), &start_mask);
    lv_refr_obj_and_children(lv_layer_sys(), &start_mask);

    /*Flush the content of the VDB 刷新VDB的内容*/ 
    lv_vdb_flush();
}

#endif /*LV_VDB_SIZE == 0*/

/**
 * Search the most top object which fully covers an area 搜索最顶部的对象，它完全覆盖了一个区域
 * @param area_p pointer to an area area_p指向一个区域
 * @param obj the first object to start the searching (typically a screen) obj第一个搜索的对象（通常是一个屏幕）
 * @return 
 */
static lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj)
{
    lv_obj_t * i;
    lv_obj_t * found_p = NULL;
    
    /*If this object is fully cover the draw area check the children too 如果这个对象完全覆盖了绘制区域，也检查子对象 */
    if(lv_area_is_in(area_p, &obj->coords) && obj->hidden == 0)
    {
        LL_READ(obj->child_ll, i)        {
            found_p = lv_refr_get_top_obj(area_p, i);
            
            /*If a children is ok then break如果子类对象返回OK 那就直接退出*/
            if(found_p != NULL) {
                break;
            }
        }
        
        /*If no better children check this object 如果没有更好的子类 检查这个对象*/
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
 * Make the refreshing from an object. Draw all its children and the youngers too. 刷新对象 绘制所有的子类和子类的弟子
 * @param top_p pointer to an objects. Start the drawing from it.  指向对象的Top_P指针 从这里开始画
 * @param mask_p pointer to an area, the objects will be drawn only here  mask_p指针指向一个区域，对象只会在这里绘制
 */
static void lv_refr_obj_and_children(lv_obj_t * top_p, const lv_area_t * mask_p)
{
    /* Normally always will be a top_obj (at least the screen) 通常总是top_obj(至少在屏幕上)
     * but in special cases (e.g. if the screen has alpha) it won't. 但在特殊情况下(例如，如果屏幕有alpha值)，则不会。
     * In this case use the screen directly 在本例中直接使用屏幕*/
    if(top_p == NULL) top_p = lv_scr_act();
    
    /*Refresh the top object and its children刷新顶部对象及其子对象*/
    lv_refr_obj(top_p, mask_p);
    
    /*Draw the 'younger' sibling objects because they can be on top_obj绘制“较小的”同级对象，因为它们可以位于top_obj上 */
    lv_obj_t * par;
    lv_obj_t * i;
    lv_obj_t * border_p = top_p;

    par = lv_obj_get_parent(top_p);

    /*Do until not reach the screen 直到没有到达屏幕*/
    while(par != NULL) {
        /*object before border_p has to be redrawn必须重新绘制border_p之前的对象*/
        i = lv_ll_get_prev(&(par->child_ll), border_p);

        while(i != NULL) { 
            /*Refresh the objects 刷新对象*/
            lv_refr_obj(i, mask_p);
            i = lv_ll_get_prev(&(par->child_ll), i);
        }  
        
        /*The new border will be there last parents,新边界将是最后的父类
         *so the 'younger' brothers of parent will be refreshed 这样，父类的“兄弟类”就会重新刷新*/
        border_p = par;
        /*Go a level deeper更进一步*/
        par = lv_obj_get_parent(par);
    }

    /*Call the post draw design function of the parents of the to object调用对象的父对象的后绘制设计函数*/
    par = lv_obj_get_parent(top_p);
    while(par != NULL) {
        par->design_func(par, mask_p, LV_DESIGN_DRAW_POST);
        par = lv_obj_get_parent(par);
    }
}

/**
 * Refresh an object an all of its children. (Called recursively) 刷新一个对象及其所有子对象
 * @param obj pointer to an object to refresh 指向要刷新的对象的obj指针
 * @param mask_ori_p pointer to an area, the objects will be drawn only here mask_ori_p指针指向一个区域，对象只会在这里绘制
 */
static void lv_refr_obj(lv_obj_t * obj, const lv_area_t * mask_ori_p)
{
    /*Do not refresh hidden objects不要刷新隐藏对象*/
    if(obj->hidden != 0) return;
    
    bool union_ok;  /* Store the return value of area_union 存储area_union的返回值 */
    /* Truncate the original mask to the coordinates of the parent 将原始掩码截断到父类掩码的坐标
     * because the parent and its children are visible only here 因为父结点和子结点只在这里可见 */
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
    
    /*Draw the parent and its children only if they ore on 'mask_parent'只有当父类及其子类位于'mask_parent'上时，才绘制父类及其子类*/
    if(union_ok != false) {

        /* Redraw the object 重画对象*/
        lv_style_t * style = lv_obj_get_style(obj);
        if(style->body.opa != LV_OPA_TRANSP) {
            obj->design_func(obj, &obj_ext_mask, LV_DESIGN_DRAW_MAIN);
            //tick_wait_ms(100);  /*DEBUG: Wait after every object draw to see the order of drawing   tick_wait_ms (100);调试:等待每个对象绘制完成后查看绘制顺序*/
        }

        /*Create a new 'obj_mask' without 'ext_size' because the children can't be visible there创建一个新的'obj_mask'，但不包含'ext_size'，因为其中的子元素不可见*/
        lv_obj_get_coords(obj, &obj_area);
        union_ok = lv_area_union(&obj_mask, mask_ori_p, &obj_area);
        if(union_ok != false) {
			lv_area_t mask_child; /*Mask from obj and its child 从obj和它的子类读取掩码*/
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
				/* Get the union (common parts) of original mask (from obj) 获取原始掩模的union (common parts)(来自obj)
				 * and its child 及其子*/
				union_ok = lv_area_union(&mask_child, &obj_mask, &child_area);

				/*If the parent and the child has common area then refresh the child 如果父节点和子节点有公共区域，则刷新子节点*/
				if(union_ok) {
					/*Refresh the next children刷新下一个子元素*/
					lv_refr_obj(child_p, &mask_child);
				}
			}
        }

        /* If all the children are redrawn make 'post draw' design 如果所有的子元素都被重新绘制，则进行“后绘制”设计 */
        if(style->body.opa != LV_OPA_TRANSP) {
		  obj->design_func(obj, &obj_ext_mask, LV_DESIGN_DRAW_POST);
		}
    }
}
