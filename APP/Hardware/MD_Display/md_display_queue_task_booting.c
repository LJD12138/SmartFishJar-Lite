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

#include <stdio.h>


#define     dispTASK_BOOTING_CYCLE_TIME         100 //任务时间

bool b_disp_render_booting_page(void);



/***********************************************************************************************************************
-----函数功能    启动中显示任务
-----说明(备注)  打开显示并刷新启动进度页, 有新任务入队时退出当前任务
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_booting(Task_T *tp_task)
{
    switch (tp_task->ucStep)
    {
        //初始化
        case 0:
        {
            if(tDisp.eDevState != DS_BOOTING)
                bDisp_SetDevState(DS_BOOTING);
			vDisp_PageSyncByState(DS_BOOTING);

            if(g_bDispPageDirty && tDispPageCtx.usDirtyMask == DDM_FULL)
                vDisp_Init();

            bDisp_Switch(ST_ON, true);
            cQueue_GotoStep(tp_task, STEP_NEXT);
        }

        //加载动画
        case 1:
        {
            if(b_disp_render_booting_page() == true)
            {
                bDisp_SetDevState(DS_WORK);
                cQueue_GotoStep(tp_task, STEP_NEXT);
            }
        }
        break;

        //等待新任务
        case 2:
        {
            if(lwrb_get_full(&tp_task->tQueueBuff) > 0)
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

/***********************************************************************************************************************
-----函数功能    渲染启动中页面
-----说明(备注)  启动页只属于启动队列, 由统一渲染入口分发到这里执行
-----传入参数    none
-----输出参数    none
-----返回值      true:动画完成  false:继续显示
************************************************************************************************************************/
bool b_disp_render_booting_page(void)
{

    vDisp_DrawPageFrame("P10 BOOTING");
    vDisp_DrawStatusTag("BOOT");
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 26, 24, "Booting...");

    vDisp_Refresh();
    tDispPageCtx.usDirtyMask = DDM_NONE;
    g_bDispPageDirty = false;

    return true;
}

