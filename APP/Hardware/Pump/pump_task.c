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
#endif

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif

//****************************************************参数初始化**************************************************//
Pump_T tPump;

/*****************************************************************************************************************
-----函数功能    水泵任务初始化
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
-----传入参数    level: 0~pumpPWM_MAX_VALUE
******************************************************************************************************************/
bool bPump_SetLevel(u16 level)
{
    /* 过流保护：水泵电流超过阈值，不允许继续增大 */
    #if(boardADC_EN)
    if(tAdcSamp.fPumpCurr > 3.0f && level > tPump.usSpeed)
        return false;
    #endif

    tPump.usSpeed = LIMIT_MAX(level, pumpPWM_MAX_VALUE);
    pumpPWM_SET(tPump.usSpeed);
    tPump.eDevState = (tPump.usSpeed > 0) ? DS_WORK : DS_SHUT_DOWN;
    return true;
}

/*****************************************************************************************************************
-----函数功能    按预设模式设置水泵速度
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
void vPump_EnterLowPower(void)
{
    bPump_SetLevel(0);
    vPump_IoEnterLowPower();
}

void vPump_ExitLowPower(void)
{
    vPump_IfaceInit();
}
#endif

#endif  //boardWATER_PUMP_EN
