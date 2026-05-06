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

#include <stdio.h>


#define     dispTASK_CLOSE_CYCLE_TIME           100 //任务时间

bool b_disp_render_closing_page(void);


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
        //初始化
        case 0:
        {
            if(tDisp.eDevState != DS_CLOSING)
                bDisp_SetDevState(DS_CLOSING);
                vDisp_PageSyncByState(DS_CLOSING);

            bDisp_Switch(ST_ON, true);

            cQueue_GotoStep(tp_task, STEP_NEXT);
        }
        break;

        //加载动画
        case 1:
        {
            if(b_disp_render_closing_page() == true)
            {
                bDisp_SetDevState(DS_SHUT_DOWN);
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
    vTaskDelay(dispTASK_CLOSE_CYCLE_TIME);
    #endif  //boardUSE_OS
}

/***********************************************************************************************************************
-----函数功能    渲染关机中页面
-----说明(备注)  关机页只属于关闭队列, 由统一渲染入口分发到这里执行
-----传入参数    none
-----输出参数    none
-----返回值      true:动画完成  false:继续显示
************************************************************************************************************************/
bool b_disp_render_closing_page(void)
{
    char line[24];

    vDisp_DrawPageFrame("P30 CLOSING");
    vDisp_DrawStatusTag("OFF");
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 30, 26, "Power Down");
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    sprintf(line, "ERR %03u FORCE %u", 0, 0);
    u8g2_DrawStr(&u8g2, 10, 40, line);
    u8g2_DrawStr(&u8g2, 20, 48, "Save state / stop io");

    vDisp_Refresh();
    tDispPageCtx.usDirtyMask = DDM_NONE;
    g_bDispPageDirty = false;
    return true;
}


