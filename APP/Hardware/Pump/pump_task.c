/*****************************************************************************************************************
*                                                                                                                *
 *                                         水泵任务（TIMER0_CH0 PA8）                                          *
*                                                                                                                *
******************************************************************************************************************/
#include "Pump/pump_task.h"

#if(boardWATER_PUMP_EN)
#include "Pump/pump_iface.h"
#include "Sys/sys_task.h"
#include "board_config.h"

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif  //boardADC_EN


//****************************************************参数初始化**************************************************//
Pump_T tPump;

/*****************************************************************************************************************
-----函数功能    水泵任务初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 初始化成功
******************************************************************************************************************/
bool bPump_TaskInit(void)
{
    vPump_IfaceInit();
    tPump.usSpeed  = 0;
    tPump.eMode    = PUMP_OFF;
    tPump.eDevState = DS_SHUT_DOWN;
    pumpPWM_SET(0);
    return true;
}

/*****************************************************************************************************************
-----函数功能    设置水泵速度（直接PWM值）
-----说明(备注)  过流保护：ADC检测到水泵电流超过3.0A时，不允许继续增大PWM
-----传入参数    level: PWM值，范围 0~pumpPWM_MAX_VALUE
-----输出参数    none
-----返回值      true: 设置成功  false: 过流保护，设置失败
******************************************************************************************************************/
bool bPump_SetLevel(u16 level)
{
    /* 过流保护：水泵电流超过阈值，不允许继续增大 */
    #if(boardADC_EN)
    if(tAdcSamp.fPumpCurr > 3.0f && level > tPump.usSpeed)
        return false;
    #endif  //boardADC_EN

    tPump.usSpeed = LIMIT_MAX(level, pumpPWM_MAX_VALUE);
    pumpPWM_SET(tPump.usSpeed);
    tPump.eDevState = (tPump.usSpeed > 0) ? DS_WORK : DS_SHUT_DOWN;
    return true;
}

/*****************************************************************************************************************
-----函数功能    按预设模式设置水泵速度
-----说明(备注)  将工作模式转换为对应PWM值后调用bPump_SetLevel
-----传入参数    mode: 工作模式（PUMP_LOW/PUMP_MID/PUMP_HIGH/PUMP_MAX/PUMP_OFF）
-----输出参数    none
-----返回值      true: 设置成功  false: 设置失败
******************************************************************************************************************/
bool bPump_SetMode(PumpWorkMode_E mode)
{
    u16 pwm_val = 0;
    switch(mode)
    {
        case PUMP_LOW:  pwm_val = 300;  break;
        case PUMP_MID:  pwm_val = 600;  break;
        case PUMP_HIGH: pwm_val = 800;  break;
        case PUMP_MAX:  pwm_val = pumpPWM_MAX_VALUE; break;
        default:        pwm_val = 0;    break;
    }
    tPump.eMode = mode;
    return bPump_SetLevel(pwm_val);
}

#if(boardLOW_POWER)
/*****************************************************************************************************************
-----函数功能    水泵进入低功耗模式
-----说明(备注)  关闭水泵PWM并设置IO为低功耗状态
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vPump_EnterLowPower(void)
{
    bPump_SetLevel(0);
    vPump_IoEnterLowPower();
}

/*****************************************************************************************************************
-----函数功能    水泵退出低功耗模式
-----说明(备注)  重新初始化水泵接口
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vPump_ExitLowPower(void)
{
    vPump_IfaceInit();
}
#endif  //boardLOW_POWER

#endif  //boardWATER_PUMP_EN
