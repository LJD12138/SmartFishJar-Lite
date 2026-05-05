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
-----函数功能    获取启动进度
-----说明(备注)  根据系统初始化完成位计算启动进度, 用于判断是否需要重绘
-----传入参数    none
-----输出参数    none
-----返回值      启动进度百分比
************************************************************************************************************************/
static u16 us_disp_booting_progress(void)
{
    u16 state = tSysInfo.uInit.State;
    u16 done_count = 0;
    u16 total_count = 15;

    while(state)
    {
        done_count += (u16)(state & 0x0001U);
        state >>= 1;
    }

    return (u16)((done_count * 100U) / total_count);
}

/***********************************************************************************************************************
-----函数功能    启动中显示任务
-----说明(备注)  打开显示并刷新启动进度页, 有新任务入队时退出当前任务
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_booting(Task_T *tp_task)
{
    static u16 s_last_progress = 0xFFFFU;

    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_BOOTING)
                bDisp_SetDevState(DS_BOOTING);
			vDisp_PageSyncByState(DS_BOOTING);

            if(g_bDispPageDirty && tDispPageCtx.usDirtyMask == DDM_FULL)
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
            u16 progress = us_disp_booting_progress();

            if(progress != s_last_progress)
            {
                s_last_progress = progress;
                tDispPageCtx.usDirtyMask |= DDM_CONTENT;
                g_bDispPageDirty = true;
            }

            if(g_bDispPageDirty)
                vDisp_RenderUi();

            if(lwrb_get_full(&tp_task->tQueueBuff))
            {
				s_last_progress = 0xFFFFU;
                cQueue_GotoStep(tp_task, STEP_END);
			}
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
