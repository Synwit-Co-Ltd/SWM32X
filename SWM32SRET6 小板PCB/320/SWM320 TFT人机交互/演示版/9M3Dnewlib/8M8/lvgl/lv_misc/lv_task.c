/**
 * @file lv_task.c
 * An 'lv_task'  is a void (*fp) (void* param) type function which will be called periodically. lv_task是一个void (*fp) (void* param)类型的函数，它将定期调用。
 * A priority (5 levels + disable) can be assigned to lv_tasks.  可以为lv_tasks分配一个优先级(5级+禁用)。
 */

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>
#include "lv_task.h"
#include "../lv_hal/lv_hal_tick.h"
#include "../../lv_conf.h"

/*********************
 *      DEFINES
 *********************/
#define IDLE_MEAS_PERIOD    500     /*[ms]*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool lv_task_exec (lv_task_t* lv_task_p); //原本是C+中的定义形式 名称为布尔型 返回值只有1和0 ture或false

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_ll_t lv_task_ll;  /*Linked list to store the lv_tasks 链接列表来存储lv_tasks*/
static bool lv_task_run = false;
static uint8_t idle_last = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Init the lv_task module 初始化lv_task模块
 */
void lv_task_init(void)
{
    lv_ll_init(&lv_task_ll, sizeof(lv_task_t));
    
    /*Initially enable the lv_task handling*/
    lv_task_enable(true);
}

/**
 * Call it  periodically to handle lv_tasks. 定期调用用来处理 lv_tasks.
 */
LV_ATTRIBUTE_TASK_HANDLER void lv_task_handler(void)
{
    static uint32_t idle_period_start = 0;
    static uint32_t handler_start = 0;
    static uint32_t busy_time = 0;

	if(lv_task_run == false) return;

	handler_start = lv_tick_get(); //右边：获取启动后经过的毫秒数，并且返回值

	/* Run all task from the highest to the lowest priority          从高优先级到最低优先级运行所有任务 
	 * If a lower priority task is executed check task again from the highest priority 执行完一个低优先级任务后会从高优先级再次检查
	 * but on the priority of executed tasks don't run tasks before the executed  但在执行任务的优先级上，不要在执行任务之前运行任务*/
	lv_task_t * task_interruper = NULL;
    lv_task_t * next;
	bool end_flag;
	do {
	    end_flag = true;
	    lv_task_t * act = lv_ll_get_head(&lv_task_ll);
	    while(act){
	        /* The task might be deleted if it runs only once ('once = 1') 如果任务只运行一次，则可能被删除('once = 1')
	         * So get next element until the current is surely valid 因此，不停获取下一个元素，直到当前元素肯定是有效的*/
	        next = lv_ll_get_next(&lv_task_ll, act);

	        /*Here is the interrupter task. Don't execute it again. 这是中断器任务。不要再这样做了*/
	        if(act == task_interruper) {
	            task_interruper = NULL;     /*From this point only task after the interrupter comes, so the interrupter is not interesting anymore从这一点来看，只有在中断出现之后才执行任务，所以中断就不再有趣了*/
	            act = next;
	            continue;                   /*Load the next task 加载下一个任务*/
	        }

	        /*Just try to run the tasks with highest priority.试着以最高优先级运行任务*/
	        if(act->prio == LV_TASK_PRIO_HIGHEST) {
	            lv_task_exec(act);
	        }
	        /*Tasks with higher priority then the interrupted shall be run in every case优先级较高的任务则应在每种情况下运行中断*/
	        else if(task_interruper) {
	            if(act->prio > task_interruper->prio) {
	                if(lv_task_exec(act)) {
	                    task_interruper = act;  /*Check all tasks again from the highest priority再次从最高优先级检查所有任务 */
	                    end_flag = false;
	                    break;
	                }
	            }
	        }
	        /* It is no interrupter task or we already reached it earlier.这不是中断器任务，或者我们已经提前到达了它。
	         * Just run the remaining tasks运行剩余的任务*/
	        else {
	            if(lv_task_exec(act)) {
	                task_interruper = act;  /*Check all tasks again from the highest priority 再次从最高优先级检查所有任务*/
	                end_flag = false;
	                break;
	            }
	        }
	        act = next;         /*Load the next task加载下一个任务*/
	    }
	} while(!end_flag);

    busy_time += lv_tick_elaps(handler_start);
    uint32_t idle_period_time = lv_tick_elaps(idle_period_start);
    if(idle_period_time >= IDLE_MEAS_PERIOD) {

        idle_last = (uint32_t)((uint32_t)busy_time * 100) / IDLE_MEAS_PERIOD;   /*Calculate the busy percentage计算繁忙百分比*/
        idle_last = idle_last > 100 ? 0 : 100 - idle_last;                      /*But we need idle time但是我们需要空闲时间*/
        busy_time = 0;
        idle_period_start = lv_tick_get();


    }
}

/**
 * Create a new lv_task 创建一个新的lv_task
 * @param task a function which is the task itself 任务是任务本身的函数
 * @param period call period in ms unit  在ms单元中的周期调用周期
 * @param prio priority of the task (LV_TASK_PRIO_OFF means the task is stopped) 优先级任务(LV_TASK_PRIO_OFF表示任务已停止)
 * @param param free parameter 参数
 * @return pointer to the new task 返回 指向新任务的指针
 */
lv_task_t* lv_task_create(void (*task) (void *), uint32_t period, lv_task_prio_t prio, void * param)
{
    lv_task_t* new_lv_task = NULL;
    lv_task_t* tmp;

    /*Create task lists in order of priority from high to low 按照从高到低的优先级顺序创建任务列表*/
    tmp = lv_ll_get_head(&lv_task_ll);
    if(NULL == tmp) {                               /*First task*/
        new_lv_task = lv_ll_ins_head(&lv_task_ll);
    }
    else{
        do{
            if(tmp->prio <= prio){
                new_lv_task = lv_ll_ins_prev(&lv_task_ll, tmp);
                break;
            }
        tmp = lv_ll_get_next(&lv_task_ll,tmp);
        }while(tmp != NULL);

        if(tmp == NULL) {   /*Only too high priority tasks were found 只发现优先级过高的任务*/
            new_lv_task = lv_ll_ins_tail(&lv_task_ll);
        }
    }

    lv_mem_assert(new_lv_task);

    new_lv_task->period = period;
    new_lv_task->task = task;
    new_lv_task->prio = prio;
    new_lv_task->param = param;
    new_lv_task->once = 0;
    new_lv_task->last_run = lv_tick_get();

    return new_lv_task;

}

/**
 * Delete a lv_task 删除lv_task
 * @param lv_task_p pointer to task created by lv_task_p lv_task_p指针指向lv_task_p创建的任务
 */
void lv_task_del(lv_task_t* lv_task_p) 
{
    lv_ll_rem(&lv_task_ll, lv_task_p);
    
    lv_mem_free(lv_task_p);
}

/**
 * Set new priority for a lv_task 为lv_task设置新的优先级
 * @param lv_task_p pointer to a lv_task 指向lv_task的lv_task_p指针
 * @param prio the new priority  prio 新优先级
 */
void lv_task_set_prio(lv_task_t* lv_task_p, lv_task_prio_t prio)
{
    /*Find the tasks with new priority 找到具有新优先级的任务*/
    lv_task_t * i;
    LL_READ(lv_task_ll, i) {
        if(i->prio <= prio) {
            if(i != lv_task_p) lv_ll_move_before(&lv_task_ll, lv_task_p, i);
            break;
        }
    }

    /*There was no such a low priority so far then add the node to the tail 目前还没有这么低的优先级，然后将节点添加到尾部 */
    if(i == NULL) {
        lv_ll_move_before(&lv_task_ll, lv_task_p, NULL);
    }


    lv_task_p->prio = prio;
}

/**
 * Set new period for a lv_task 为lv_task设置新周期
 * @param lv_task_p pointer to a lv_task 指向lv_task的 lv_task_p指针
 * @param period the new period
 */
void lv_task_set_period(lv_task_t* lv_task_p, uint32_t period)
{
    lv_task_p->period = period;
}

/**
 * Make a lv_task ready. It will not wait its period.
 * @param lv_task_p pointer to a lv_task.
 */
void lv_task_ready(lv_task_t* lv_task_p)
{
    lv_task_p->last_run = lv_tick_get() - lv_task_p->period - 1;
}

/**
 * Delete the lv_task after one call
 * @param lv_task_p pointer to a lv_task.
 */
void lv_task_once(lv_task_t * lv_task_p)
{
    lv_task_p->once = 1;
}

/**
 * Reset a lv_task. 
 * It will be called the previously set period milliseconds later.
 * @param lv_task_p pointer to a lv_task.
 */
void lv_task_reset(lv_task_t* lv_task_p)
{
    lv_task_p->last_run = lv_tick_get();
}

/**
 * Enable or disable the whole lv_task handling
 * @param en: true: lv_task handling is running, false: lv_task handling is suspended
 */
void lv_task_enable(bool en)
{
	lv_task_run = en;
}

/**
 * Get idle percentage
 * @return the lv_task idle in percentage
 */
uint8_t lv_task_get_idle(void)
{
    return idle_last;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Execute task if its the priority is appropriate  执行任务，如果它的优先级是适当的
 * @param lv_task_p pointer to lv_task 指向lv_task的 lv_task_p指针
 * @return true: execute, false: not executed return true:执行，false:未执行
 */
static bool lv_task_exec (lv_task_t* lv_task_p)
{
    bool exec = false;
    
    /*Execute if at least 'period' time elapsed如果至少经过“一段时间”，则执行*/
    uint32_t elp = lv_tick_elaps(lv_task_p->last_run);
    if(elp >= lv_task_p->period) {
        lv_task_p->last_run = lv_tick_get();
        lv_task_p->task(lv_task_p->param);

        /*Delete if it was a one shot lv_task如果是一次性的lv_task，则删除它*/
        if(lv_task_p->once != 0) lv_task_del(lv_task_p);

        exec = true;
    }
    
    return exec;
}

