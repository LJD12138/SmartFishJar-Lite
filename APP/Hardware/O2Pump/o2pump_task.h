#ifndef O2PUMP_TASK_H_
#define O2PUMP_TASK_H_

#include "board_config.h"
#include "function.h"

#if(boardO2PUMP_EN)

typedef enum
{
    O2PUMP_OFF = 0,
    O2PUMP_LOW,
    O2PUMP_MID,
    O2PUMP_HIGH,
    O2PUMP_MAX,
}O2PumpWorkMode_E;

typedef struct
{
    vu16              usSpeed;
    O2PumpWorkMode_E  eMode;
    DevState_E        eDevState;      // 设备状态（替代bEnable管理功能使能）
}O2Pump_T;
extern O2Pump_T tO2Pump;

bool bO2Pump_TaskInit(void);
bool bO2Pump_SetLevel(u16 level);
bool bO2Pump_SetMode(O2PumpWorkMode_E mode);
void bO2Pump_SetDevState(DevState_E stat);

/*======================================== 开关控制接口（参考热管理任务） ========================================*/
// 统一开关接口：type=ST_ON/ST_OFF/ST_NULL
// fore_en: false=当前已是目标状态时直接跳过, true=强制执行
s8 cO2Pump_Switch(SwitchType_E type, bool fore_en);

#endif  //boardO2PUMP_EN

#endif  //O2PUMP_TASK_H_
