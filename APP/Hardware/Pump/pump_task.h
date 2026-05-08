#ifndef PUMP_TASK_H_
#define PUMP_TASK_H_

#include "board_config.h"
#include "function.h"

#if(boardWATER_PUMP_EN)

typedef enum
{
    PUMP_OFF = 0,
    PUMP_LOW,
    PUMP_MID,
    PUMP_HIGH,
    PUMP_MAX,
}PumpWorkMode_E;

typedef struct
{
    vu16            usSpeed;        // 当前PWM占空比（0~pumpPWM_MAX_VALUE）
    PumpWorkMode_E  eMode;
    DevState_E      eDevState;      // 设备状态（替代bEnable管理功能使能）
}Pump_T;
extern Pump_T tPump;

bool bPump_TaskInit(void);
bool bPump_SetLevel(u16 level);
bool bPump_SetMode(PumpWorkMode_E mode);
void bPump_SetDevState(DevState_E stat);
s8 cPump_Switch(SwitchType_E type, bool fore_en);

#if(boardLOW_POWER)
void vPump_EnterLowPower(void);
void vPump_ExitLowPower(void);
#endif

#endif  //boardWATER_PUMP_EN

#endif  //PUMP_TASK_H_
