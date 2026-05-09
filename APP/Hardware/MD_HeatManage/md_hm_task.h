#ifndef MD_HM_TASK_H_
#define MD_HM_TASK_H_

#include "main.h"

#define  		fanSIMPLE_MODE      					1   //0:简单模式   1:全功能

/*======================================== PID控制器定义 ========================================*/
typedef struct
{
    float   Kp;             // 比例系数
    float   Ki;             // 积分系数
    float   Kd;             // 微分系数
    float   setpoint;       // 目标值
    float   integral;       // 积分累积
    float   prev_error;     // 上一次误差
    float   output_min;     // 输出下限
    float   output_max;     // 输出上限
} PID_Controller_t;

/*======================================== 热管理对象枚举 ========================================*/
typedef enum
{
    HM_OBJ_FAN = 0,     // 风扇对象
    HM_OBJ_HEAT,        // 加热对象
    HM_OBJ_ALL,        // 所有对象
}HM_Object_E;

/*======================================== 热管理全局结构 ========================================*/
typedef struct
{
    vu16                usPower;
    // 温度
    s16                 sMaxTemp;              // 最高温度
    s16                 sMinTemp;              // 最低温度

    // 风扇控制参数
    bool                bFanEnable;         // 风扇功能使能（默认关闭）
    s16                 sFanTempStart;      // 风扇起转温度(>=该温度开始调速)
    s16                 sFanTempFull;       // 风扇满速温度(>=该温度满速)
    
    // 加热控制参数
    bool                bHeatEnable;        // 加热功能使能（默认关闭）
    s16                 sHeatTargetTemp;    // 加热目标温度（默认25°C）
}HM_T;              
extern HM_T				tHM;


/*======================================== 任务初始化 ========================================*/
bool bHM_TaskInit(void);

/*======================================== 统一开关接口（参考Dc模块） ========================================*/
// 通过传入不同对象实现一个接口控制FAN和HEAT两个对象
// fore_en: false=当前已是目标状态时直接跳过, true=强制执行
s8 cHm_Switch(HM_Object_E obj, SwitchType_E type, bool fore_en);

void vHM_FanSetTargetTemp(s16 temp_start, s16 temp_full);
void vHM_HeatSetTargetTemp(s16 temp);
bool bHM_IsForceOn(void);
vs16 usHM_GetDevPwm(HM_Object_E obj);


#if(boardLOW_POWER)
void vFan_EnterLowPower(void);
void vFan_ExitLowPower(void);
#endif

#endif  //MD_HM_TASK_H_
