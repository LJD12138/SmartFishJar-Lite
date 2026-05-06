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
-----说明(备注)  刷新工作页面、处理长文本滚动并周期刷新实时数据
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_work(Task_T *tp_task)
{
	static u8 s_refresh_div = 0;
    static u8 s_scroll_div = 0;

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
            vDisp_PageSyncByState(DS_WORK);
            tDispPageCtx.usDirtyMask = DDM_FULL;
            g_bDispPageDirty = true;
            bDisp_RenderUi();
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        case 1:
        {
            if(g_bDispPageDirty == false)
            {
                s_scroll_div++;
                if(s_scroll_div >= 1U)
                {
                    s_scroll_div = 0U;
                    if(tDispPageCtx.tTextScroll.ucMaxOffset > 0U)
                    {
                        if(tDispPageCtx.tTextScroll.ucHoldCnt < 7U)
                            tDispPageCtx.tTextScroll.ucHoldCnt++;
                        else
                        {
                            tDispPageCtx.tTextScroll.ucHoldCnt = 0U;
                            if(tDispPageCtx.tTextScroll.ucOffset < tDispPageCtx.tTextScroll.ucMaxOffset)
                                tDispPageCtx.tTextScroll.ucOffset++;
                            else
                                tDispPageCtx.tTextScroll.ucOffset = 0U;

                            tDispPageCtx.usDirtyMask |= DDM_CONTENT;
                            g_bDispPageDirty = true;
                        }
                    }
                }
            }

            if(g_bDispPageDirty == false)
            {
                s_refresh_div++;
                if(s_refresh_div >= 3U)
                {
                    s_refresh_div = 0U;
                    switch(tDispPageCtx.ePageId)
                    {
                        case DPI_HOME:
						case DPI_LIGHT:
						case DPI_HEAT:
						case DPI_WPUMP:
						case DPI_O2PUMP:
						case DPI_SETTING:
						case DPI_ADC:
                            if(tDispPageCtx.bEditing == false)
                            {
                                tDispPageCtx.usDirtyMask |= DDM_CONTENT;
                                g_bDispPageDirty = true;
                            }
                            break;

                        default:
                            break;
                    }
                }
            }

            if(g_bDispPageDirty)
                bDisp_RenderUi();
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
