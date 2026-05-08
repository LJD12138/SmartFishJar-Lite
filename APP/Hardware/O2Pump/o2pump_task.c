/*****************************************************************************************************************
*                                                                                                                *
 *                                         氧气泵任务（TIMER2_CH1 PC7）                                        *
 * 重要：TIMER2由md_hm_iface.c统一初始化，本模块只写CH1比较寄存器                                              *
 *                                                                                                                *
 ******************************************************************************************************************/
#include "O2Pump/o2pump_task.h"

#if(boardO2PUMP_EN)
#include "O2Pump/o2pump_iface.h"
#include "Sys/sys_task.h"
#include "board_config.h"

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif  //boardADC_EN

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS


//****************************************************任务初始化**************************************************//
#if(boardUSE_OS)
#define         O2PUMP_TASK_PRIO                            1
#define         O2PUMP_TASK_STK_SIZE                        128
#define         O2PUMP_TASK_CYCLE_MS                        500
TaskHandle_t    tO2PumpTaskHandler = NULL;
void            vO2Pump_Task(void *pvParameters);
#endif  //boardUSE_OS


//****************************************************参数初始化**************************************************//
O2Pump_T tO2Pump;

// 过流保护参数（参考热管理：计数触发 + 迟滞恢复）
#define O2PUMP_OC_SET_CNT               3     // 连续3次过流触发保护
#define O2PUMP_OC_CLR_CNT               5     // 连续5次正常解除保护
#define O2PUMP_OC_CURRENT               2.0f  // 过流阈值2.0A

static bool b_o2p_oc_protect = 0;


//****************************************************函数声明****************************************************//
static bool b_o2p_is_active_state(void);
static void v_o2p_check_prote(void);
static void v_o2p_shutdown_output(void);


/*****************************************************************************************************************
-----函数功能    参数初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static bool b_task_param_init(void)
{
    tO2Pump.usSpeed   = 0;
    tO2Pump.eMode     = O2PUMP_OFF;
    tO2Pump.eDevState = DS_SHUT_DOWN;   // 默认关闭
    o2PWM_SET(0);    // 只写寄存器，TIMER2已经运行
    return true;
}


/*****************************************************************************************************************
-----函数功能    氧气泵任务初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 初始化成功
******************************************************************************************************************/
bool bO2Pump_TaskInit(void)
{
    b_task_param_init();

    #if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vO2Pump_Task,
                (const char*    )"o2PumpTask",
                (uint16_t       )O2PUMP_TASK_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )O2PUMP_TASK_PRIO,
                (TaskHandle_t*  )&tO2PumpTaskHandler);
    #endif  //boardUSE_OS

    return true;
}


/*****************************************************************************************************************
-----函数功能    氧气泵循环任务
-----说明(备注)  500ms周期运行，持续监控过流保护，非工作状态自动关断
-----传入参数    pvParameters: FreeRTOS任务参数
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
#if(boardUSE_OS)
void vO2Pump_Task(void *pvParameters)
{
    for(;;)
    {
        v_o2p_check_prote();

        if(b_o2p_is_active_state() == true && tO2Pump.eDevState == DS_WORK && b_o2p_oc_protect == false)
        {
            // 工作态且使能且未触发过流保护，保持当前输出
        }
        else
        {
            v_o2p_shutdown_output();
        }

        vTaskDelay(O2PUMP_TASK_CYCLE_MS);
    }
}
#endif  //boardUSE_OS


/*****************************************************************************************************************
-----函数功能    判断氧气泵是否处于允许运行状态
-----说明(备注)  工作态或错误态允许运行（参考热管理b_hm_is_active_state）
-----传入参数    none
-----输出参数    none
-----返回值      true: 允许运行  false: 需要关断输出
******************************************************************************************************************/
static bool b_o2p_is_active_state(void)
{
    return (bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR);
}


/*****************************************************************************************************************
-----函数功能    氧气泵过流保护检查
-----说明(备注)  参考热管理过温保护：计数触发 + 迟滞恢复
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_o2p_check_prote(void)
{
    static u8 uc_oc_set_cnt = 0;
    static u8 uc_oc_clr_cnt = 0;

    if(b_o2p_is_active_state() == false)
    {
        uc_oc_set_cnt = 0;
        uc_oc_clr_cnt = 0;
        b_o2p_oc_protect = false;
        return;
    }

    #if(boardADC_EN)
    if(tAdcSamp.fO2Curr > O2PUMP_OC_CURRENT)
    {
        uc_oc_clr_cnt = 0;
        if(b_o2p_oc_protect == false)
        {
            uc_oc_set_cnt++;
            if(uc_oc_set_cnt >= O2PUMP_OC_SET_CNT)
            {
                uc_oc_set_cnt = 0;
                b_o2p_oc_protect = true;
                v_o2p_shutdown_output();
            }
        }
    }
    else
    {
        uc_oc_set_cnt = 0;
        if(b_o2p_oc_protect == true)
        {
            uc_oc_clr_cnt++;
            if(uc_oc_clr_cnt >= O2PUMP_OC_CLR_CNT)
            {
                uc_oc_clr_cnt = 0;
                b_o2p_oc_protect = false;
            }
        }
    }
    #endif  //boardADC_EN
}


/*****************************************************************************************************************
-----函数功能    氧气泵统一关断输出
-----说明(备注)  关闭PWM输出并复位状态
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_o2p_shutdown_output(void)
{
    if(tO2Pump.usSpeed != 0)
    {
        tO2Pump.usSpeed = 0;
        o2PWM_SET(0);
        tO2Pump.eDevState = DS_SHUT_DOWN;
        tO2Pump.eMode = O2PUMP_OFF;
    }
}


/*****************************************************************************************************************
-----函数功能    设置氧气泵速度
-----说明(备注)  过流保护：ADC检测到氧气泵电流超过阈值时，不允许继续增大PWM
                 任务级过流保护 + 设置时即时保护双重保障
-----传入参数    level: PWM值，范围 0~o2PWM_MAX_VALUE
-----输出参数    none
-----返回值      true: 设置成功  false: 过流保护，设置失败
******************************************************************************************************************/
bool bO2Pump_SetLevel(u16 level)
{
    /* 任务级过流保护 */
    if(b_o2p_oc_protect == true && level > tO2Pump.usSpeed)
        return false;

    /* 即时过流保护 */
    #if(boardADC_EN)
    if(tAdcSamp.fO2Curr > O2PUMP_OC_CURRENT && level > tO2Pump.usSpeed)
        return false;
    #endif  //boardADC_EN

    tO2Pump.usSpeed = LIMIT_MAX(level, o2PWM_MAX_VALUE);
    o2PWM_SET(tO2Pump.usSpeed);
    tO2Pump.eDevState = (tO2Pump.usSpeed > 0) ? DS_WORK : DS_SHUT_DOWN;
    return true;
}


/*****************************************************************************************************************
-----函数功能    按预设模式设置氧气泵速度
-----说明(备注)  将工作模式转换为对应PWM值后调用bO2Pump_SetLevel
-----传入参数    mode: 工作模式（O2PUMP_LOW/O2PUMP_MID/O2PUMP_HIGH/O2PUMP_MAX/O2PUMP_OFF）
-----输出参数    none
-----返回值      true: 设置成功  false: 设置失败
******************************************************************************************************************/
bool bO2Pump_SetMode(O2PumpWorkMode_E mode)
{
    u16 pwm_val = 0;
    switch(mode)
    {
        case O2PUMP_LOW:  pwm_val = 300;  break;
        case O2PUMP_MID:  pwm_val = 600;  break;
        case O2PUMP_HIGH: pwm_val = 800;  break;
        case O2PUMP_MAX:  pwm_val = o2PWM_MAX_VALUE; break;
        default:          pwm_val = 0;    break;
    }
    tO2Pump.eMode = mode;
    return bO2Pump_SetLevel(pwm_val);
}


/*****************************************************************************************************************
                                                     全局函数
 ******************************************************************************************************************/

/*****************************************************************************************************************
-----函数功能    设置氧气泵设备状态（参考bUsb_SetDevState）
-----说明(备注)  统一管理设备状态切换，根据状态执行相应硬件操作
-----传入参数    stat: 目标设备状态
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void bO2Pump_SetDevState(DevState_E stat)
{
    tO2Pump.eDevState = stat;

    // 关闭和关机状态统一关断输出
    if(stat == DS_CLOSING || stat == DS_SHUT_DOWN)
    {
        v_o2p_shutdown_output();
    }
}


/*****************************************************************************************************************
-----函数功能    氧气泵统一开关控制（参考热管理任务cHm_Switch）
-----说明(备注)  通过传入不同类型实现开启/关闭/取反
-----传入参数    type:    开关类型（ST_ON=开启, ST_OFF=关闭, ST_NULL=取反）
                 fore_en: false=当前已是目标状态时直接跳过, true=强制执行
-----输出参数    none
-----返回值      0: 操作成功  -1: 参数错误
******************************************************************************************************************/
s8 cO2Pump_Switch(SwitchType_E type, bool fore_en)
{
    DevState_E target_state = DS_SHUT_DOWN;

    switch(type)
    {
        case ST_ON:
            target_state = DS_WORK;
            break;
        case ST_OFF:
            target_state = DS_SHUT_DOWN;
            break;
        case ST_NULL:
            target_state = (tO2Pump.eDevState == DS_WORK) ? DS_SHUT_DOWN : DS_WORK;
            break;
        default:
            return -1;
    }

    if(tO2Pump.eDevState == target_state && fore_en == false)
        return 0;

    bO2Pump_SetDevState(target_state);

    return 0;
}

#endif  //boardO2PUMP_EN
