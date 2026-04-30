/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示队列任务-错误状态                                                 *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"


#define     dispTASK_ERR_CYCLE_TIME             100 //任务时间

/***********************************************************************************************************************
 -----函数功能    错误显示任务
 -----说明(备注)  none
 -----传入参数    none
 -----输出参数    none
 -----返回值      none
 ************************************************************************************************************************/
void v_disp_queue_task_err(Task_T *tp_task)
{
    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_ERR)
                bDisp_SetDevState(DS_ERR);

            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            // 显示错误代码
            u16 err_code = usDisp_ErrCodeDisplay();
            if(err_code < 100)
            {
                // 有错误,保持显示
            }
            else
            {
                // 无错误,结束任务
                cQueue_GotoStep(tp_task, STEP_END);
            }
        }
        break;
        
        default:
            cQueue_GotoStep(tp_task, STEP_END);
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_ERR_CYCLE_TIME);
    #endif  //boardUSE_OS
}
