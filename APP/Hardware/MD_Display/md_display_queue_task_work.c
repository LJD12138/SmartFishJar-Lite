/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示队列任务-工作中                                                   *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"


#define     dispTASK_WORK_CYCLE_TIME            100 //任务时间

/***********************************************************************************************************************
 -----函数功能    工作显示任务
 -----说明(备注)  none
 -----传入参数    none
 -----输出参数    none
 -----返回值      none
 ************************************************************************************************************************/
void v_disp_queue_task_work(Task_T *tp_task)
{
    //新的任务
    if(lwrb_get_full(&tp_task->tQueueBuff))
        cQueue_GotoStep(tp_task, STEP_END);

    //息屏
    if(tDisp.bLight == false) 
    {
        tp_task->ucStep = 0;
        #if(boardUSE_OS)
        vTaskDelay(dispTASK_WORK_CYCLE_TIME);
        #endif  //boardUSE_OS
        return;
    }
    
    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_WORK)
                bDisp_SetDevState(DS_WORK);

            bDisp_Switch(ST_ON, true);

            if(g_bDispPageDirty)
                vDisp_Init();
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            // 更新工作显示
            vDisp_UiTest();
            cQueue_GotoStep(tp_task, STEP_END);
        }
        break;
        
        default:
            cQueue_GotoStep(tp_task, STEP_END);
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_WORK_CYCLE_TIME);
    #endif  //boardUSE_OS
}
