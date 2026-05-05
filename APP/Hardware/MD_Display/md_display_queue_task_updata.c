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

#if(boardUPDATA)
#include "Sys/sys_queue_task_updata.h"
#endif


#define     dispTASK_UPDATA_CYCLE_TIME          100 //任务时间

/***********************************************************************************************************************
-----函数功能    获取升级进度
-----说明(备注)  根据升级接收状态计算本地进度, 用于判断是否需要重绘
-----传入参数    none
-----输出参数    none
-----返回值      升级进度百分比
************************************************************************************************************************/
static u8 uc_disp_updata_percent_local(void)
{
    #if(boardUPDATA)
    if(tUpdata.usTotalFrmValue > 0)
    {
        u32 percent = (u32)tUpdata.usRecFrameCnt * 100U;
        percent /= tUpdata.usTotalFrmValue;
        if(percent > 100U)
            percent = 100U;
        return (u8)percent;
    }

    if(tUpdata.eProtoType != PT_NULL)
        return 15U;
    if(tUpdata.eChType != CT_NULL)
        return 8U;
    #endif

    return 0U;
}

/***********************************************************************************************************************
-----函数功能    升级显示任务
-----说明(备注)  保持亮屏显示升级页, 升级进度变化时刷新页面
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_updata(Task_T *tp_task)
{
    static u8 s_last_percent = 0xFFU;

    //新的任务
    if(lwrb_get_full(&tp_task->tQueueBuff))
        cQueue_GotoStep(tp_task, STEP_END);

    switch (tp_task->ucStep)
    {
        case 0:
        {
            if(tDisp.eDevState != DS_UPDATA_MODE)
                bDisp_SetDevState(DS_UPDATA_MODE);
			vDisp_PageSyncByState(DS_UPDATA_MODE);

            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            u8 percent = uc_disp_updata_percent_local();

            if(percent != s_last_percent)
            {
                s_last_percent = percent;
                tDispPageCtx.usDirtyMask |= DDM_CONTENT;
                g_bDispPageDirty = true;
            }

            if(g_bDispPageDirty)
                vDisp_RenderUi();

			s_last_percent = 0xFFU;
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
