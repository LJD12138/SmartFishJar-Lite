/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示队列任务-关闭中                                                   *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"


#define     dispTASK_CLOSE_CYCLE_TIME           100 //任务时间

/***********************************************************************************************************************
-----函数功能    关闭中显示任务
-----说明(备注)  打开显示并刷新关机提示页面
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_closing(Task_T *tp_task)
{
    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_CLOSING)
                bDisp_SetDevState(DS_CLOSING);

            bDisp_Switch(ST_ON, false);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            vDisp_PageSyncByState(DS_CLOSING);
            vDisp_RenderUi();
            cQueue_GotoStep(tp_task, STEP_END);
        }
        break;
        
        default:
            cQueue_GotoStep(tp_task, STEP_END);
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_CLOSE_CYCLE_TIME);
    #endif  //boardUSE_OS
}
