#ifndef MD_LIGHT_TASK_H_
#define MD_LIGHT_TASK_H_

#include "board_config.h"

#if(boardLIGHT_EN)


//工作模式
typedef enum 
{   
    LWM_OFF = 0,
    LWM_LOW,
    LWM_HALF,
    LWM_FULL,
    LWM_AUTO,      // 根据 usLightRes 自动调节白光亮度
    LWM_SOS,
    LWM_TWINKLE,
}LampWorkMode_E;

typedef enum 
{   
    RWM_OFF = 0,
    RWM_LOW,
    RWM_HALF,
    RWM_FULL,
    RWM_AUTO,//根据usLightRes来调节亮度
}RGBWorkMode_E;


typedef struct
{
    vu16                usPower;
    vu16                usWarm;     // 暖白光通道 PWM（0~lightPWM_MAX_VALUE）
    vu16                usBlue;     // 蓝光通道 PWM
    vu16                usGreen;    // 绿光通道 PWM
    vu16                usRed;      // 红光通道 PWM
    LampWorkMode_E      eLampMode;
    RGBWorkMode_E  		eRGBMode;
    DevState_E          eDevState;
}Light_T;              
extern Light_T   		tLight;


void vLight_TaskInit(void);
bool bLight_Switch(SwitchType_E type);
bool bLight_SetMode(LampWorkMode_E mode);
bool bLight_SetRGBMode(RGBWorkMode_E mode);
void vLight_CircSelectMode(void);
void vLight_CircSelectRGBMode(void);

#if(boardLOW_POWER)
void vLight_EnterLowPower(void);
void vLight_ExitLowPower(void);
#endif

#endif  //boardLIGHT_EN

#endif  //MD_LIGHT_TASK_H_
