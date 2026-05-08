/*****************************************************************************************************************
*                                                                                                                *
*                                         显示队列任务-工作中                                                   *
*                                                                                                                *
******************************************************************************************************************/
#include "MD_Display/md_display_queue_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "MD_Display/icon_bitmaps.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"

#if(boardLIGHT_EN)
#include "MD_Light/md_light_task.h"
#endif

#if(boardHEAT_MANAGE_EN)
#include "MD_HeatManage/md_hm_task.h"
#include "MD_HeatManage/md_hm_iface.h"
#endif

#include <stdio.h>
#include <string.h>

#define     dispTASK_WORK_CYCLE_TIME            100 // 任务周期时间，单位：毫秒
#define     DISP_HEAT_TARGET_TEMP_C             25  // 目标加热温度，单位：摄氏度

static bool b_disp_render_work_page(void);
static void v_disp_work_draw_full_top_bar(const char *title, const char *tag);
static void v_disp_work_draw_hint_line(void);
static const char *pc_disp_work_fan_mode(void);
static const char *pc_disp_work_light_white_state(void);
static const char *pc_disp_work_light_rgb_state(void);
static void v_disp_work_format_curr_ma(char *dst, u16 ma);
static void v_disp_work_draw_field_row(u8 index, u8 y, const char *label, const char *value, bool edit_mark);
static const char *pc_disp_work_home_module_label(DispHomeModule_E module);
static const unsigned char *pc_disp_work_home_module_icon(DispHomeModule_E module);
static void v_disp_work_draw_home_module_block(const DispUiSnapshot_T *tp_ui, u8 x, u8 y, DispHomeModule_E module, bool selected);
static void v_disp_work_draw_home_page(const DispUiSnapshot_T *tp_ui);
static void v_disp_work_draw_light_page(const DispUiSnapshot_T *tp_ui);
static void v_disp_work_draw_heat_page(const DispUiSnapshot_T *tp_ui);
static void v_disp_work_draw_wpump_page(const DispUiSnapshot_T *tp_ui);
static void v_disp_work_draw_o2pump_page(const DispUiSnapshot_T *tp_ui);
static void v_disp_work_draw_setting_page(void);
static void v_disp_work_draw_adc_page(const DispUiSnapshot_T *tp_ui);

/***********************************************************************************************************************
-----函数功能    工作显示任务
-----说明(备注)  刷新工作页面、处理长文本滚动并周期刷新实时数据
-----传入参数    tp_task:任务对象指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void v_disp_queue_task_work(Task_T *tp_task)
{
    static u8 s_refresh_div = 0; // 页面刷新计数器
    static u8 s_scroll_div = 0;  // 文本滚动计数器

    // 检查任务队列是否有新任务
    if(lwrb_get_full(&tp_task->tQueueBuff))
        cQueue_GotoStep(tp_task, STEP_END);

    // 检查是否息屏
    if(tDisp.bLight == false && tp_task->ucStep != 0) 
    {
        tp_task->ucStep = 0;
        #if(boardUSE_OS)
        vTaskDelay(dispTASK_WORK_CYCLE_TIME); // 延迟任务周期时间
        #endif  //boardUSE_OS
        return;
    }
    
    switch (tp_task->ucStep)
    {
        case 0:
        {
            // 初始化工作页面
            if(tDisp.eDevState != DS_WORK)
                bDisp_SetDevState(DS_WORK);

            bDisp_Switch(ST_ON, true); // 打开显示
            vDisp_PageSyncByState(DS_WORK); // 同步页面状态
            tDispPageCtx.usDirtyMask = DDM_FULL; // 标记页面需要完全刷新
            g_bDispPageDirty = true;
            b_disp_render_work_page(); // 渲染工作页面
            cQueue_GotoStep(tp_task, STEP_NEXT); // 进入下一步
        }
        break;

        case 1:
        {
            // 处理文本滚动逻辑
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

                            tDispPageCtx.usDirtyMask |= DDM_CONTENT; // 标记内容需要刷新
                            g_bDispPageDirty = true;
                        }
                    }
                }
            }

            // 处理页面刷新逻辑
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
                                tDispPageCtx.usDirtyMask |= DDM_CONTENT; // 标记内容需要刷新
                                g_bDispPageDirty = true;
                            }
                            break;

                        default:
                            break;
                    }
                }
            }

            // 如果页面需要刷新，则重新渲染
            if(g_bDispPageDirty)
                b_disp_render_work_page();
            cQueue_GotoStep(tp_task, STEP_END); // 结束当前步骤
        }
        break;
        
        default:
            cQueue_GotoStep(tp_task, STEP_END); // 默认结束步骤
            break;
    }
    
    #if(boardUSE_OS)
    vTaskDelay(dispTASK_WORK_CYCLE_TIME); // 延迟任务周期时间
    #endif  //boardUSE_OS
}

/***********************************************************************************************************************
-----函数功能    渲染工作页面
-----说明(备注)  根据当前页面 ID 渲染对应的页面内容
-----传入参数    none
-----输出参数    none
-----返回值      true: 渲染成功
************************************************************************************************************************/
static bool b_disp_render_work_page(void)
{
    const DispUiSnapshot_T *tp_ui;

    vDisp_UpdateUiSnapshot(); // 更新 UI 快照
    tp_ui = ptDisp_GetUiSnapshot(); // 获取 UI 快照

    switch(tDispPageCtx.ePageId)
    {
        case DPI_HOME:
            v_disp_work_draw_home_page(tp_ui); // 渲染主页
            break;

        case DPI_LIGHT:
            v_disp_work_draw_light_page(tp_ui); // 渲染灯光页面
            break;

        case DPI_HEAT:
            v_disp_work_draw_heat_page(tp_ui); // 渲染加热页面
            break;

        case DPI_WPUMP:
            v_disp_work_draw_wpump_page(tp_ui); // 渲染水泵页面
            break;

        case DPI_O2PUMP:
            v_disp_work_draw_o2pump_page(tp_ui); // 渲染氧气泵页面
            break;

        case DPI_SETTING:
            v_disp_work_draw_setting_page(); // 渲染设置页面
            break;

        case DPI_ADC:
            v_disp_work_draw_adc_page(tp_ui); // 渲染 ADC 页面
            break;

        default:
            v_disp_work_draw_home_page(tp_ui); // 默认渲染主页
            break;
    }

    vDisp_Refresh(); // 刷新显示
    tDispPageCtx.usDirtyMask = DDM_NONE; // 清除刷新标记
    g_bDispPageDirty = false; // 页面不再脏
    return true;
}

/***********************************************************************************************************************
-----函数功能    绘制页面顶部的全宽标题栏（黑底白字）
-----说明(备注)  清除顶部区域并绘制标题与右侧短标签，使用 u8g2 反色绘制实现背景/文字反转
-----传入参数    title: 标题字符串，NULL 表示不绘制标题
-----传入参数    tag: 右侧短标签，NULL 表示不绘制标签
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_full_top_bar(const char *title, const char *tag)
{
    u8 title_len = (u8)((title != NULL) ? strlen(title) : 0U);
    u8 tag_len = (u8)((tag != NULL) ? strlen(tag) : 0U);

    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, 9);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawBox(&u8g2, 0, 0, OLED_WIDTH_PIXELS, 9);
    u8g2_SetDrawColor(&u8g2, 0);
    if(title_len > 0U)
        u8g2_DrawStr(&u8g2, 2, 7, title);
    if(tag_len > 0U)
        u8g2_DrawStr(&u8g2, (u8)(OLED_WIDTH_PIXELS - (tag_len * 6U) - 2U), 7, tag);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawLine(&u8g2, 0, 9, OLED_WIDTH_PIXELS - 1U, 9);
}

/***********************************************************************************************************************
-----函数功能    绘制底部提示行
-----说明(备注)  当 tDispPageCtx.usHintCnt>0 且提示文本非空时在底部显示提示内容
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_hint_line(void)
{
    if(tDispPageCtx.usHintCnt > 0U && tDispPageCtx.acHint[0] != '\0')
    {
        u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
        u8g2_DrawStr(&u8g2, 52, 61, tDispPageCtx.acHint);
    }
}

/***********************************************************************************************************************
-----函数功能    获取风扇/散热模式文本
-----说明(备注)  根据热管理模块 tHM.usValue 的值返回简短文本描述:
                - "OFF": 无输出
                - "AUTO": PWM 未达到最大
                - "FULL": PWM 达到最大值
-----传入参数    none
-----输出参数    none
-----返回值      const char*: 模式字符串
************************************************************************************************************************/
static const char *pc_disp_work_fan_mode(void)
{
    const char *pc_mode = "OFF";

#if(boardHEAT_MANAGE_EN)
    if(tHM.usValue == 0)
        pc_mode = "OFF";
    else
        pc_mode = (tHM.usValue >= hmPWM_MAX_VALUE) ? "FULL" : "AUTO";
#endif

    return pc_mode;
}

/***********************************************************************************************************************
-----函数功能    获取白光灯状态文本
-----说明(备注)  根据 tLight.eWordMode 返回短文本（如 DIM/ FUL/ SOS/ TWK），若不支持则返回 "OFF"
-----传入参数    none
-----输出参数    none
-----返回值      const char*: 状态字符串
************************************************************************************************************************/
static const char *pc_disp_work_light_white_state(void)
{
    #if(boardLIGHT_EN)
    switch(tLight.eWordMode)
    {
        case LWM_HALF: return "DIM";
        case LWM_FULL: return "FUL";
        case LWM_SOS: return "SOS";
        case LWM_TWINKLE: return "TWK";
        default: break;
    }
    #endif

    return "OFF";
}

/***********************************************************************************************************************
-----函数功能    获取 RGB 灯状态文本
-----说明(备注)  优先返回特殊模式 SOS/TWK，其次检测 RGB 分量是否非零返回 "ON"，否则返回 "OFF"
-----传入参数    none
-----输出参数    none
-----返回值      const char*: 状态字符串
************************************************************************************************************************/
static const char *pc_disp_work_light_rgb_state(void)
{
    #if(boardLIGHT_EN)
    if(tLight.eWordMode == LWM_SOS)
        return "SOS";
    if(tLight.eWordMode == LWM_TWINKLE)
        return "TWK";
    if(tLight.usBlue > 0U || tLight.usGreen > 0U || tLight.usRed > 0U)
        return "ON";
    #endif

    return "OFF";
}

/***********************************************************************************************************************
-----函数功能    格式化电流值为字符串
-----说明(备注)  将单位为 mA 的值格式化为带两位小数的安培表示，例如 1234 -> "1.23A"
-----传入参数    dst: 目标字符串缓冲区
-----传入参数    ma: 电流值（mA）
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_format_curr_ma(char *dst, u16 ma)
{
    sprintf(dst, "%u.%02uA", ma / 1000U, (ma % 1000U) / 10U);
}

/***********************************************************************************************************************
-----函数功能    绘制一行字段（标签-值）
-----说明(备注)  根据 index 判断是否为焦点行，焦点行采用反色背景；当 edit_mark 为 true 且为焦点时右侧显示 '*' 标记
-----传入参数    index: 字段索引
-----传入参数    y: 垂直坐标
-----传入参数    label: 左侧标签文本
-----传入参数    value: 右侧值文本
-----传入参数    edit_mark: 编辑标记开关
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_field_row(u8 index, u8 y, const char *label, const char *value, bool edit_mark)
{
    bool focus = (tDispPageCtx.ucFieldIndex == index);

    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    if(focus == true)
    {
        u8g2_DrawBox(&u8g2, 0, y - 7U, OLED_WIDTH_PIXELS, 8);
        u8g2_SetDrawColor(&u8g2, 0);
    }

    u8g2_DrawStr(&u8g2, 4, y, label);
    u8g2_DrawStr(&u8g2, 52, y, value);
    if(focus == true && edit_mark == true)
        u8g2_DrawStr(&u8g2, 122, y, "*");

    u8g2_SetDrawColor(&u8g2, 1);
}

/***********************************************************************************************************************
-----函数功能    获取首页模块标签文本
-----说明(备注)  根据模块类型返回对应的短标签（HEAT/WPUMP/O2PUMP/LIGHT）
-----传入参数    module: 模块枚举
-----输出参数    none
-----返回值      const char*: 模块标签字符串
************************************************************************************************************************/
static const char *pc_disp_work_home_module_label(DispHomeModule_E module)
{
    switch(module)
    {
        case DHM_HEAT: return "HEAT";
        case DHM_WPUMP: return "WPUMP";
        case DHM_O2PUMP: return "O2PUMP";
        case DHM_LIGHT:
        default: break;
    }

    return "LIGHT";
}

/***********************************************************************************************************************
-----函数功能    获取首页模块图标
-----说明(备注)  返回模块对应的 16x16 位图指针，用于绘制图标
-----传入参数    module: 模块枚举
-----输出参数    none
-----返回值      const unsigned char*: 图标数据指针
************************************************************************************************************************/
static const unsigned char *pc_disp_work_home_module_icon(DispHomeModule_E module)
{
    switch(module)
    {
        case DHM_HEAT: return icon_heat_16x16;
        case DHM_WPUMP: return icon_wpump_16x16;
        case DHM_O2PUMP: return icon_o2pump_16x16;
        case DHM_LIGHT:
        default: break;
    }

    return icon_light_16x16;
}

/***********************************************************************************************************************
-----函数功能    绘制首页单个模块块
-----说明(备注)  包含图标、模块名以及一行简短状态信息，选中时绘制加粗边框
-----传入参数    tp_ui: UI 快照指针
-----传入参数    x: 左上角 X 坐标
-----传入参数    y: 左上角 Y 坐标
-----传入参数    module: 模块类型
-----传入参数    selected: 是否选中
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_home_module_block(const DispUiSnapshot_T *tp_ui, u8 x, u8 y, DispHomeModule_E module, bool selected)
{
    char line1[18];

    switch(module)
    {
        case DHM_HEAT:
            sprintf(line1, "%dC %s", tp_ui->sWaterTemp, tp_ui->pcHeatMode);
            break;

        case DHM_WPUMP:
            sprintf(line1, "%s %u%%", tp_ui->pcPumpMode, tp_ui->usPumpSpeed / 10U);
            break;

        case DHM_O2PUMP:
            sprintf(line1, "%s %u%%", tp_ui->pcO2PumpMode, tp_ui->usO2PumpSpeed / 10U);
            break;

        case DHM_LIGHT:
        default:
            sprintf(line1, "W:%s R:%s", pc_disp_work_light_white_state(), pc_disp_work_light_rgb_state());
            break;
    }

    if(selected == true)
    {
        u8g2_DrawFrame(&u8g2, x, y, 64, 20);
        u8g2_DrawFrame(&u8g2, x + 1U, y + 1U, 62, 18);
    }
    else
        u8g2_DrawFrame(&u8g2, x, y, 64, 20);

    u8g2_DrawXBMP(&u8g2, x + 2U, y + 2U, 16, 16, pc_disp_work_home_module_icon(module));
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, x + 20U, y + 8U, pc_disp_work_home_module_label(module));
    u8g2_DrawStr(&u8g2, x + 20U, y + 15U, line1);
}

/***********************************************************************************************************************
-----函数功能    绘制 HOME 页面（工作态第一页）
-----说明(备注)  显示 VIN/I/P 信息，并在页面中绘制四个模块块（LIGHT/HEAT/WPUMP/O2PUMP）
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_home_page(const DispUiSnapshot_T *tp_ui)
{
    char line[32];

    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("HOME 1/3", "WORK");
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    sprintf(line, "VIN%u.%u I%u.%02u P%uW", tp_ui->usVinVolt / 10U,
            tp_ui->usVinVolt % 10U, tp_ui->usVinCurrMa / 1000U,
            (tp_ui->usVinCurrMa % 1000U) / 10U, tp_ui->usVinPowerW);
    u8g2_DrawStr(&u8g2, 1, 16, line);

    v_disp_work_draw_home_module_block(tp_ui, 0, 17, DHM_LIGHT, tDispPageCtx.eHomeModule == DHM_LIGHT);
    v_disp_work_draw_home_module_block(tp_ui, 64, 17, DHM_HEAT, tDispPageCtx.eHomeModule == DHM_HEAT);
    v_disp_work_draw_home_module_block(tp_ui, 0, 38, DHM_WPUMP, tDispPageCtx.eHomeModule == DHM_WPUMP);
    v_disp_work_draw_home_module_block(tp_ui, 64, 38, DHM_O2PUMP, tDispPageCtx.eHomeModule == DHM_O2PUMP);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制 LIGHT 页面（灯光详细信息）
-----说明(备注)  显示灯光功率、电流、白光与 RGB 状态、亮度与电流等字段
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_light_page(const DispUiSnapshot_T *tp_ui)
{
    char line1[24];
    char value[20];

    sprintf(line1, "P%uW I%umA", tp_ui->usLightPowerW, tp_ui->usLightCurrMa);
    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("LIGHT", "1/4");
    u8g2_DrawXBMP(&u8g2, 0, 10, 16, 16, icon_light_16x16);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 20, 16, line1);

    v_disp_work_draw_field_row(0U, 32, "WHITE", pc_disp_work_light_white_state(), false);
    v_disp_work_draw_field_row(1U, 40, "RGB", pc_disp_work_light_rgb_state(), false);
    sprintf(value, "%u%%", tp_ui->usLightWarm / 10U);
    v_disp_work_draw_field_row(2U, 48, "WARM", value, false);
    sprintf(value, "%umA", tp_ui->usLightCurrMa);
    v_disp_work_draw_field_row(3U, 56, "CURR", value, false);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制 HEAT 页面（加热控制信息）
-----说明(备注)  显示当前水温、两个水温采样、加热模式、风扇状态及目标温度
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_heat_page(const DispUiSnapshot_T *tp_ui)
{
    char line1[24];
    char value[20];

    sprintf(line1, "WATER %dC", tp_ui->sWaterTemp);
    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("HEAT", "1/4");
    u8g2_DrawXBMP(&u8g2, 0, 10, 16, 16, icon_heat_16x16);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 20, 16, line1);

    sprintf(value, "%d/%dC", tp_ui->sWaterTemp1, tp_ui->sWaterTemp2);
    v_disp_work_draw_field_row(0U, 32, "WATER", value, false);
    v_disp_work_draw_field_row(1U, 40, "HEAT", tp_ui->pcHeatMode, false);
    v_disp_work_draw_field_row(2U, 48, "FAN", pc_disp_work_fan_mode(), false);
    sprintf(value, "%dC", DISP_HEAT_TARGET_TEMP_C);
    v_disp_work_draw_field_row(3U, 56, "TARGET", value, false);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制 WPUMP 页面（水泵信息）
-----说明(备注)  显示泵的模式、速度百分比/原始值、运行状态及电流
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_wpump_page(const DispUiSnapshot_T *tp_ui)
{
    char line1[24];
    char value[20];

    sprintf(line1, "MODE %s", tp_ui->pcPumpMode);
    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("WPUMP", "1/4");
    u8g2_DrawXBMP(&u8g2, 0, 10, 16, 16, icon_wpump_16x16);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 20, 16, line1);

    v_disp_work_draw_field_row(0U, 32, "MODE", tp_ui->pcPumpMode, false);
    sprintf(value, "%u%%/%u", tp_ui->usPumpSpeed / 10U, tp_ui->usPumpSpeed);
    v_disp_work_draw_field_row(1U, 40, "SPEED", value, false);
    v_disp_work_draw_field_row(2U, 48, "STATE", tp_ui->usPumpSpeed > 0U ? "RUN" : "STOP", false);
    v_disp_work_format_curr_ma(value, tp_ui->usPumpCurrMa);
    v_disp_work_draw_field_row(3U, 56, "CURR", value, false);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制 O2PUMP 页面（氧气泵信息）
-----说明(备注)  显示氧气泵的模式、速度、运行状态及电流
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_o2pump_page(const DispUiSnapshot_T *tp_ui)
{
    char line1[24];
    char value[20];

    sprintf(line1, "MODE %s", tp_ui->pcO2PumpMode);
    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("O2PUMP", "1/4");
    u8g2_DrawXBMP(&u8g2, 0, 10, 16, 16, icon_o2pump_16x16);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 20, 16, line1);

    v_disp_work_draw_field_row(0U, 32, "MODE", tp_ui->pcO2PumpMode, false);
    sprintf(value, "%u%%/%u", tp_ui->usO2PumpSpeed / 10U, tp_ui->usO2PumpSpeed);
    v_disp_work_draw_field_row(1U, 40, "SPEED", value, false);
    v_disp_work_draw_field_row(2U, 48, "STATE", tp_ui->usO2PumpSpeed > 0U ? "RUN" : "STOP", false);
    v_disp_work_format_curr_ma(value, tp_ui->usO2CurrMa);
    v_disp_work_draw_field_row(3U, 56, "CURR", value, false);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制设置页面
-----说明(备注)  从 tDispPageCtx.tSettingCache 读取设置项并绘制，支持编辑态显示与 DIRTY 状态标签
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_setting_page(void)
{
    char value[20];
    const char *tag = tDispPageCtx.tSettingCache.bDirty ? "DIRTY" : (tDispPageCtx.bEditing ? "EDIT" : "VIEW");

    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("SET 2/3", tag);

    sprintf(value, "%us", tDispPageCtx.tSettingCache.usSleepTime);
    v_disp_work_draw_field_row(0U, 17, "SLEEP", value, tDispPageCtx.bEditing);
    sprintf(value, "%s", tDispPageCtx.tSettingCache.bBuzOff ? "OFF" : "ON");
    v_disp_work_draw_field_row(1U, 26, "BUZZER", value, tDispPageCtx.bEditing);
    sprintf(value, "%u", tDispPageCtx.tSettingCache.ucHighLightValue);
    v_disp_work_draw_field_row(2U, 35, "HIGH BL", value, tDispPageCtx.bEditing);
    sprintf(value, "%u", tDispPageCtx.tSettingCache.ucLowLightValue);
    v_disp_work_draw_field_row(3U, 44, "LOW BL", value, tDispPageCtx.bEditing);
    sprintf(value, "%s", tDispPageCtx.tSettingCache.bRestoreDefault ? "YES" : "NO");
    v_disp_work_draw_field_row(4U, 53, "RESET", value, tDispPageCtx.bEditing);
    v_disp_work_draw_hint_line();
}

/***********************************************************************************************************************
-----函数功能    绘制 ADC 页面（传感器/ADC 读数）
-----说明(备注)  根据 tDispPageCtx.ucAdcGroupIndex 选择显示温度组/电源组/灯光组的数据
-----传入参数    tp_ui: UI 快照指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_work_draw_adc_page(const DispUiSnapshot_T *tp_ui)
{
    char value[20];
    const char *tag = "TEMP";

    if(tDispPageCtx.ucAdcGroupIndex == 1U)
        tag = "PWR";
    else if(tDispPageCtx.ucAdcGroupIndex == 2U)
        tag = "LIGHT";

    vDisp_ClearRegion(0, 0, OLED_WIDTH_PIXELS, OLED_HEIGHT_PIXELS);
    v_disp_work_draw_full_top_bar("ADC 3/3", tag);

    switch(tDispPageCtx.ucAdcGroupIndex)
    {
        case 0U:
            sprintf(value, "%dC", tp_ui->sWaterTemp1);
            v_disp_work_draw_field_row(0U, 17, "WT1", value, false);
            sprintf(value, "%dC", tp_ui->sWaterTemp2);
            v_disp_work_draw_field_row(1U, 26, "WT2", value, false);
            sprintf(value, "%dC", tp_ui->sBoardTemp5V);
            v_disp_work_draw_field_row(2U, 35, "BOARD5", value, false);
            sprintf(value, "%dC", tp_ui->sBoardTemp12V);
            v_disp_work_draw_field_row(3U, 44, "BOARD12", value, false);
            sprintf(value, "%dC", tp_ui->sWaterTemp);
            v_disp_work_draw_field_row(4U, 53, "MAXT", value, false);
            break;

        case 1U:
            sprintf(value, "%u.%uV", tp_ui->us12VVolt / 10U, tp_ui->us12VVolt % 10U);
            v_disp_work_draw_field_row(0U, 17, "12V", value, false);
            sprintf(value, "%u.%uV", tp_ui->usVinVolt / 10U, tp_ui->usVinVolt % 10U);
            v_disp_work_draw_field_row(1U, 26, "VIN", value, false);
            v_disp_work_format_curr_ma(value, tp_ui->usVinCurrMa);
            v_disp_work_draw_field_row(2U, 35, "IIN", value, false);
            sprintf(value, "%uW", tp_ui->usVinPowerW);
            v_disp_work_draw_field_row(3U, 44, "PIN", value, false);
            sprintf(value, "%umA", tp_ui->usLightCurrMa);
            v_disp_work_draw_field_row(4U, 53, "LCURR", value, false);
            break;

        default:
            sprintf(value, "%u", tp_ui->usLightAdc);
            v_disp_work_draw_field_row(0U, 17, "LIGHT", value, false);
            sprintf(value, "%umA", tp_ui->usLightCurrMa);
            v_disp_work_draw_field_row(1U, 26, "LCURR", value, false);
            sprintf(value, "%u%%", tp_ui->usLightWarm / 10U);
            v_disp_work_draw_field_row(2U, 35, "WARM", value, false);
            sprintf(value, "%s", pc_disp_work_light_rgb_state());
            v_disp_work_draw_field_row(3U, 44, "RGB", value, false);
            sprintf(value, "%d/%dC", tp_ui->sBoardTemp5V, tp_ui->sBoardTemp12V);
            v_disp_work_draw_field_row(4U, 53, "BOARD", value, false);
            break;
    }

    v_disp_work_draw_hint_line();
}
