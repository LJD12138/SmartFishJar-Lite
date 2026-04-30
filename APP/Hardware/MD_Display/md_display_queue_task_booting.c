/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示队列任务-启动中                                                   *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"


#define     dispTASK_BOOTING_CYCLE_TIME         100 //任务时间

/***********************************************************************************************************************
 -----函数功能    启动中显示任务
 -----说明(备注)  none
 -----传入参数    none
 -----输出参数    none
 -----返回值      none
 ************************************************************************************************************************/
void v_disp_queue_task_booting(Task_T *tp_task)
{
    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_BOOTING)
                bDisp_SetDevState(DS_BOOTING);

            if(g_bDispPageDirty)
                vDisp_Init();
            
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }

        case 1:
        {
            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }

        case 2:
        {
            //显示启动中界面

            if(lwrb_get_full(&tp_task->tQueueBuff))
                cQueue_GotoStep(tp_task, STEP_END);
        }
        break;

        default:
            cQueue_GotoStep(tp_task, STEP_END);
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_BOOTING_CYCLE_TIME);
    #endif  //boardUSE_OS
}
