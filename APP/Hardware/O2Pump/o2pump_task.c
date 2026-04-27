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
#endif

//****************************************************参数初始化**************************************************//
O2Pump_T tO2Pump;

/*****************************************************************************************************************
-----函数功能    氧气泵初始化（TIMER2已由热管理模块初始化，此处仅清零输出）
******************************************************************************************************************/
bool bO2Pump_TaskInit(void)
{
    tO2Pump.usSpeed   = 0;
    tO2Pump.eMode     = O2PUMP_OFF;
    tO2Pump.eDevState = DS_SHUT_DOWN;
    o2PWM_SET(0);    // 只写寄存器，TIMER2已经运行
    return true;
}

/*****************************************************************************************************************
-----函数功能    设置氧气泵速度
-----传入参数    level: 0~o2PWM_MAX_VALUE
******************************************************************************************************************/
bool bO2Pump_SetLevel(u16 level)
{
    /* 过流保护 */
    #if(boardADC_EN)
    if(tAdcSamp.fO2Curr > 2.0f && level > tO2Pump.usSpeed)
        return false;
    #endif

    tO2Pump.usSpeed = LIMIT_MAX(level, o2PWM_MAX_VALUE);
    o2PWM_SET(tO2Pump.usSpeed);
    tO2Pump.eDevState = (tO2Pump.usSpeed > 0) ? DS_WORK : DS_SHUT_DOWN;
    return true;
}

/*****************************************************************************************************************
-----函数功能    按预设模式设置氧气泵速度
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

#endif  //boardO2PUMP_EN
