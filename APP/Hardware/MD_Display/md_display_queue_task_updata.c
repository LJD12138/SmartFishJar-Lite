/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示队列任务-升级模式                                                 *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"


#define     dispTASK_UPDATA_CYCLE_TIME          100 //任务时间

/***********************************************************************************************************************
 -----函数功能    升级显示任务
 -----说明(备注)  none
 -----传入参数    none
 -----输出参数    none
 -----返回值      none
 ************************************************************************************************************************/
void v_disp_queue_task_updata(Task_T *tp_task)
{
    //新的任务
    if(lwrb_get_full(&tp_task->tQueueBuff))
        cQueue_GotoStep(tp_task, STEP_END);

    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_UPDATA_MODE)
                bDisp_SetDevState(DS_UPDATA_MODE);

            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            // 升级模式显示处理
            cQueue_GotoStep(tp_task, STEP_END);
        }
        break;
        
        default:
            cQueue_GotoStep(tp_task, STEP_END);
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_UPDATA_CYCLE_TIME);
    #endif  //boardUSE_OS
}
