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
-----说明(备注)  保持亮屏显示错误页, 错误码变化时刷新, 故障清除后退出
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_err(Task_T *tp_task)
{
    static u16 s_last_err_code = 0xFFFFU;

    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_ERR)
                bDisp_SetDevState(DS_ERR);
			vDisp_PageSyncByState(DS_ERR);

            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            u16 err_code = usDisp_ErrCodeDisplay();

            if(err_code != s_last_err_code)
            {
				s_last_err_code = err_code;
                tDispPageCtx.usDirtyMask |= DDM_CONTENT | DDM_HEADER;
                g_bDispPageDirty = true;
            }

            if(g_bDispPageDirty)
                vDisp_RenderUi();

            if(err_code < 100)
            {
                // 有错误,保持显示
            }
            else
            {
                // 无错误,结束任务
				s_last_err_code = 0xFFFFU;
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
