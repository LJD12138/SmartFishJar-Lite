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


//****************************************************参数初始化**************************************************//
O2Pump_T tO2Pump;

/*****************************************************************************************************************
-----函数功能    氧气泵初始化（TIMER2已由热管理模块初始化，此处仅清零输出）
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 初始化成功
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
-----说明(备注)  过流保护：ADC检测到氧气泵电流超过2.0A时，不允许继续增大PWM
-----传入参数    level: PWM值，范围 0~o2PWM_MAX_VALUE
-----输出参数    none
-----返回值      true: 设置成功  false: 过流保护，设置失败
******************************************************************************************************************/
bool bO2Pump_SetLevel(u16 level)
{
    /* 过流保护 */
    #if(boardADC_EN)
    if(tAdcSamp.fO2Curr > 2.0f && level > tO2Pump.usSpeed)
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

#endif  //boardO2PUMP_EN
