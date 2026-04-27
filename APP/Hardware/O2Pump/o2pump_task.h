#ifndef O2PUMP_TASK_H_
#define O2PUMP_TASK_H_

#include "board_config.h"
#include "ComFunc/function.h"

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
    DevState_E        eDevState;
}O2Pump_T;
extern O2Pump_T tO2Pump;

bool bO2Pump_TaskInit(void);
bool bO2Pump_SetLevel(u16 level);
bool bO2Pump_SetMode(O2PumpWorkMode_E mode);

#endif  //boardO2PUMP_EN

#endif  //O2PUMP_TASK_H_
