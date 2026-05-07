/*****************************************************************************************************************
*                                                                                                                *
*                                         热管理任务（风扇+加热棒）                                           *
*                                                                                                                *
******************************************************************************************************************/
#include "MD_HeatManage/md_hm_task.h"
#include "MD_HeatManage/md_hm_iface.h"
#include "freertos.h"
#include "task.h"
#include "Sys/sys_task.h"
#include "board_config.h"
#include "Print/print_task.h"

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif  //boardADC_EN


//****************************************************任务初始化**************************************************//
#if(boardUSE_OS)
#define         HM_TASK_PRIO                            1
#define         HM_TASK_STK_SIZE                        256
#define         HM_TASK_CYCLE_MS                        1000
TaskHandle_t    tHeatManageHandler = NULL;
void            vHW_Task(void *pvParameters);
#endif  //boardUSE_OS


//****************************************************参数初始化**************************************************//
HM_T tHM;

// 风扇相关
static bool b_fan_stop_to_run_flag = 0;
static bool b_fan_ui_override = 0;
static FanWorkMode_E e_fan_ui_mode = FWM_OFF;

// 加热相关
static bool b_heat_ui_force_on = 0;
static bool b_hm_ot_protect = 0;

static u8   uc_updata_delay = 0;

static s16  s_water_temp = 0;     // 水温（取NTC1/NTC2均值）
static u16  us_heat_pwm  = 0;    // 加热棒PWM当前值


//****************************************************PID控制器**************************************************//
static PID_Controller_t tHeatPid;

#define HEAT_PID_KP         15.0f       // 比例系数
#define HEAT_PID_KI         0.5f        // 积分系数
#define HEAT_PID_KD         2.0f        // 微分系数
#define HEAT_PID_OUT_MIN    0.0f
#define HEAT_PID_OUT_MAX    (float)hmPWM_MAX_VALUE

//****************************************************风扇无级调速参数*******************************************//
// 水温与风扇PWM映射：水温越高，风扇转速越快
// 低水温区：风扇不转，高水温区：全速
#define FAN_TEMP_START_DEFAULT      25   // 水温>=25°C开始启动风扇
#define FAN_TEMP_FULL_DEFAULT       40   // 水温>=40°C风扇全速

// 过温保护参数（参考USB任务：计数触发 + 迟滞恢复）
#define HM_OT_SET_CNT               5     // 连续5次超温触发保护
#define HM_OT_CLR_CNT               5     // 连续5次低于恢复阈值解除保护
#define HM_OT_SET_DELTA             8     // 超过目标温度+8°C触发
#define HM_OT_CLR_DELTA             5     // 回落到目标温度+5°C以下恢复

//****************************************************函数声明****************************************************//
static void v_fan_pwm_set(u8 fan_id, u16 level);
static void v_heat_pwm_set(u16 level);
static u16  us_fan_set_work_mode(FanWorkMode_E mode);

static float f_pid_compute(PID_Controller_t *pid, float measurement);
static void  v_pid_init(PID_Controller_t *pid, float kp, float ki, float kd, float setpoint, float out_min, float out_max);
static u16   us_fan_calc_stepless_pwm(s16 water_temp);
static bool  b_hm_is_active_state(void);
static void  v_hm_param_update(void);
static void  v_hm_check_prote(void);
static void  v_hm_shutdown_output(void);
static void  v_hm_run_fan_control(void);
static void  v_hm_run_heat_control(void);


/*****************************************************************************************************************
-----函数功能    参数初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static bool b_task_param_init(void)
{
    // 初始化热管理结构体
    tHM.usValue = 0;
    tHM.eWordMode = FWM_OFF;
    tHM.bFanEnable = false;             // 默认关闭风扇
    tHM.usFanTargetVolt = hmPWM_MAX_VALUE;  // 默认风扇PWM上限为最大值
    tHM.sFanTempStart = FAN_TEMP_START_DEFAULT;
    tHM.sFanTempFull = FAN_TEMP_FULL_DEFAULT;
    tHM.bHeatEnable = false;            // 默认关闭加热
    tHM.sHeatTargetTemp = 25;           // 默认目标温度25°C
    
    // 初始化PID控制器
    v_pid_init(&tHeatPid, HEAT_PID_KP, HEAT_PID_KI, HEAT_PID_KD, 
               (float)tHM.sHeatTargetTemp, HEAT_PID_OUT_MIN, HEAT_PID_OUT_MAX);
    return true;
}



/*****************************************************************************************************************
-----函数功能    热管理任务初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 初始化成功
******************************************************************************************************************/
bool bHM_TaskInit(void)
{
    vFan_PwmInit();

    b_task_param_init();

    #if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vHW_Task,
                (const char* )"bHeatManage",
                (uint16_t ) HM_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t ) HM_TASK_PRIO,
                (TaskHandle_t*)&tHeatManageHandler);
    #endif  //boardUSE_OS

    return true;
}

/*****************************************************************************************************************
-----函数功能    热管理循环任务
-----说明(备注)  1S周期运行，根据水温自动调节风扇（无级调速），根据水温PID控制加热棒
-----传入参数    pvParameters: FreeRTOS任务参数
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vHW_Task(void *pvParameters)
{
    #if(boardUSE_OS)
    for(;;)
    #endif  //boardUSE_OS
    {
        v_hm_param_update();
        v_hm_check_prote();

        if(b_hm_is_active_state() == true)
        {
            v_hm_run_fan_control();
            v_hm_run_heat_control();
        }
        else
        {
            v_hm_shutdown_output();
        }

        #if(boardUSE_OS)
        vTaskDelay(HM_TASK_CYCLE_MS);
        #endif  //boardUSE_OS
    }
}

/*****************************************************************************************************************
-----函数功能    设置风扇PWM（支持双风扇独立控制）
-----说明(备注)  对level进行限幅后写入对应风扇PWM寄存器
-----传入参数    fan_id: 0=风扇1  1=风扇2
                level:  PWM值（0~hmPWM_MAX_VALUE）
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_fan_pwm_set(u8 fan_id, u16 level)
{
    level = LIMIT_MAX(level, hmPWM_MAX_VALUE);
    if(fan_id == 0)
        fanPWM1_SET(level);
    #if(boardFAN2_EN)
    else
        fanPWM2_SET(level);
    #endif  //boardFAN2_EN
}

/*****************************************************************************************************************
-----函数功能    设置加热棒PWM
-----说明(备注)  对level进行限幅后写入加热棒PWM寄存器
-----传入参数    level: PWM值（0~hmPWM_MAX_VALUE）
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_heat_pwm_set(u16 level)
{
    level = LIMIT_MAX(level, hmPWM_MAX_VALUE);
    heatPWM_SET(level);
}

/*****************************************************************************************************************
-----函数功能    设置风扇工作模式，返回对应PWM值
-----说明(备注)  根据模式返回预设PWM值，并更新全局状态
-----传入参数    mode: 风扇工作模式
-----输出参数    none
-----返回值      对应模式的PWM值
******************************************************************************************************************/
static u16 us_fan_set_work_mode(FanWorkMode_E mode)
{
    u16 temp = 0;
    u16 target_max = (tHM.usFanTargetVolt > 0 && tHM.usFanTargetVolt <= hmPWM_MAX_VALUE) ? tHM.usFanTargetVolt : hmPWM_MAX_VALUE;

    switch(mode)
    {
        case FWM_GEAR_1:    temp = target_max / 4;      break;
        case FWM_GEAR_2:    temp = target_max / 2;      break;
        case FWM_GEAR_3:    temp = target_max * 3 / 4;  break;
        case FWM_GEAR_FULL: temp = target_max;          break;
        default:
            temp = 0;
            b_fan_stop_to_run_flag = 0;
            mode = FWM_OFF;
            break;
    }

    tHM.eWordMode = mode;
    return temp;
}

/*****************************************************************************************************************
-----函数功能    PID初始化
-----说明(备注)  none
-----传入参数    pid: PID控制器
                kp/ki/kd: PID参数
                setpoint: 目标值
                out_min/out_max: 输出限幅
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_pid_init(PID_Controller_t *pid, float kp, float ki, float kd, float setpoint, float out_min, float out_max)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->setpoint = setpoint;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_min = out_min;
    pid->output_max = out_max;
}

/*****************************************************************************************************************
-----函数功能    PID计算（位置式PID）
-----说明(备注)  根据当前测量值计算PID输出
-----传入参数    pid: PID控制器
                measurement: 当前测量值
-----输出参数    none
-----返回值      PID输出值
******************************************************************************************************************/
static float f_pid_compute(PID_Controller_t *pid, float measurement)
{
    float error = pid->setpoint - measurement;
    float proportional = pid->Kp * error;
    
    pid->integral += error;
    // 积分限幅（抗积分饱和）
    if(pid->Ki != 0.0f)
    {
        if(pid->integral > (pid->output_max / pid->Ki))
            pid->integral = pid->output_max / pid->Ki;
        else if(pid->integral < (pid->output_min / pid->Ki))
            pid->integral = pid->output_min / pid->Ki;
    }
    
    float integral = pid->Ki * pid->integral;
    float derivative = pid->Kd * (error - pid->prev_error);
    pid->prev_error = error;
    
    float output = proportional + integral + derivative;
    
    // 输出限幅
    if(output > pid->output_max)
        output = pid->output_max;
    else if(output < pid->output_min)
        output = pid->output_min;
    
    return output;
}

/*****************************************************************************************************************
-----函数功能    根据水温计算风扇无级调速PWM值
-----说明(备注)  水温线性映射到PWM值，支持目标电压上限限制
-----传入参数    water_temp: 当前水温(°C)
-----输出参数    none
-----返回值      计算后的PWM值
******************************************************************************************************************/
static u16 us_fan_calc_stepless_pwm(s16 water_temp)
{
    u16 pwm = 0;
    s16 temp_start = tHM.sFanTempStart;
    s16 temp_full = tHM.sFanTempFull;
    u16 target_max = (tHM.usFanTargetVolt > 0 && tHM.usFanTargetVolt <= hmPWM_MAX_VALUE) ? tHM.usFanTargetVolt : hmPWM_MAX_VALUE;

    // 防御式保护：避免温区配置异常导致除零
    if(temp_full <= temp_start)
    {
        temp_start = FAN_TEMP_START_DEFAULT;
        temp_full = FAN_TEMP_FULL_DEFAULT;
    }
    
    if(water_temp <= temp_start)
    {
        pwm = 0;
    }
    else if(water_temp >= temp_full)
    {
        pwm = target_max;
    }
    else
    {
        // 线性插值
        pwm = (u16)(((float)(water_temp - temp_start) / (float)(temp_full - temp_start)) * target_max);
    }
    
    return pwm;
}

/*****************************************************************************************************************
-----函数功能    判断HM是否处于工作状态
-----说明(备注)  工作态或错误态都允许热管理逻辑运行
-----传入参数    none
-----输出参数    none
-----返回值      true: 允许运行  false: 需要关断输出
******************************************************************************************************************/
static bool b_hm_is_active_state(void)
{
    return (bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR);
}

/*****************************************************************************************************************
-----函数功能    HM参数周期更新
-----说明(备注)  与USB任务一致，统一在主循环前半段更新运行参数
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_hm_param_update(void)
{
    if(++uc_updata_delay < 3)
        return;

    uc_updata_delay = 0;

    #if(boardADC_EN && boardWATER_TEMP_EN)
    s_water_temp = (tAdcSamp.sWaterTemp1 + tAdcSamp.sWaterTemp2) / 2;
    #else
    s_water_temp = tSysInfo.sMaxTemp;
    #endif
}

/*****************************************************************************************************************
-----函数功能    HM保护与参数防呆
-----说明(备注)  对关键阈值做保护，避免异常配置影响控制输出
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_hm_check_prote(void)
{
    static u8 uc_ot_set_cnt = 0;
    static u8 uc_ot_clr_cnt = 0;

    if(tHM.usFanTargetVolt > hmPWM_MAX_VALUE)
        tHM.usFanTargetVolt = hmPWM_MAX_VALUE;

    if(tHM.sFanTempFull <= tHM.sFanTempStart)
    {
        tHM.sFanTempStart = FAN_TEMP_START_DEFAULT;
        tHM.sFanTempFull = FAN_TEMP_FULL_DEFAULT;
    }

    if(b_hm_is_active_state() == false)
    {
        uc_ot_set_cnt = 0;
        uc_ot_clr_cnt = 0;
        b_hm_ot_protect = false;
        return;
    }

    if(s_water_temp >= (s16)(tHM.sHeatTargetTemp + HM_OT_SET_DELTA))
    {
        uc_ot_clr_cnt = 0;
        if(b_hm_ot_protect == false)
        {
            uc_ot_set_cnt++;
            if(uc_ot_set_cnt >= HM_OT_SET_CNT)
            {
                uc_ot_set_cnt = 0;
                b_hm_ot_protect = true;
            }
        }
    }
    else if(s_water_temp <= (s16)(tHM.sHeatTargetTemp + HM_OT_CLR_DELTA))
    {
        uc_ot_set_cnt = 0;
        if(b_hm_ot_protect == true)
        {
            uc_ot_clr_cnt++;
            if(uc_ot_clr_cnt >= HM_OT_CLR_CNT)
            {
                uc_ot_clr_cnt = 0;
                b_hm_ot_protect = false;
            }
        }
    }
}

/*****************************************************************************************************************
-----函数功能    HM非工作状态统一关断输出
-----说明(备注)  关闭风扇和加热，复位停止到启动标记
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_hm_shutdown_output(void)
{
    if(tHM.eWordMode != FWM_OFF || tHM.usValue != 0)
    {
        tHM.usValue = us_fan_set_work_mode(FWM_OFF);
        v_fan_pwm_set(0, tHM.usValue);
        #if(boardFAN2_EN)
        v_fan_pwm_set(1, tHM.usValue);
        #endif  //boardFAN2_EN
    }

    if(us_heat_pwm != 0)
    {
        us_heat_pwm = 0;
        v_heat_pwm_set(0);
    }

    b_hm_ot_protect = false;
}

/*****************************************************************************************************************
-----函数功能    HM风扇控制执行
-----说明(备注)  包含UI覆盖模式和自动无级调速模式
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_hm_run_fan_control(void)
{
    if(b_hm_ot_protect == true)
    {
        tHM.usValue = us_fan_set_work_mode(FWM_GEAR_FULL);
        v_fan_pwm_set(0, tHM.usValue);
        #if(boardFAN2_EN)
        v_fan_pwm_set(1, tHM.usValue);
        #endif  //boardFAN2_EN
        return;
    }

    if(tHM.bFanEnable == false)
    {
        if(tHM.eWordMode != FWM_OFF || tHM.usValue != 0)
        {
            tHM.usValue = us_fan_set_work_mode(FWM_OFF);
            v_fan_pwm_set(0, tHM.usValue);
            #if(boardFAN2_EN)
            v_fan_pwm_set(1, tHM.usValue);
            #endif  //boardFAN2_EN
        }
        return;
    }

    if(b_fan_ui_override == true)
    {
        tHM.usValue = us_fan_set_work_mode(e_fan_ui_mode);
    }
    else
    {
        tHM.usValue = us_fan_calc_stepless_pwm(s_water_temp);
        tHM.eWordMode = (tHM.usValue > 0) ? FWM_GEAR_FULL : FWM_OFF;
    }

    // 风扇从停止到启动时先给中高转速防止启动困难
    if(tHM.usValue > 0 && b_fan_stop_to_run_flag == 0)
    {
        b_fan_stop_to_run_flag = 1;
        tHM.usValue = (tHM.usValue < 300) ? 300 : tHM.usValue;
    }
    else if(tHM.usValue == 0)
    {
        b_fan_stop_to_run_flag = 0;
    }

    v_fan_pwm_set(0, tHM.usValue);
    #if(boardFAN2_EN)
    v_fan_pwm_set(1, tHM.usValue);
    #endif  //boardFAN2_EN
}

/*****************************************************************************************************************
-----函数功能    HM加热控制执行
-----说明(备注)  UI强制优先，否则采用PID自动控制
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_hm_run_heat_control(void)
{
    #if(boardWATER_TEMP_EN)
    if(b_hm_ot_protect == true)
    {
        if(us_heat_pwm != 0)
        {
            us_heat_pwm = 0;
            v_heat_pwm_set(0);
        }
        tHeatPid.integral = 0.0f;
        tHeatPid.prev_error = 0.0f;
        return;
    }

    if(tHM.bHeatEnable == false)
    {
        if(us_heat_pwm != 0)
        {
            us_heat_pwm = 0;
            v_heat_pwm_set(0);
        }
        return;
    }

    {
        u16 target_heat = 0;

        if(b_heat_ui_force_on == true)
        {
            target_heat = hmPWM_MAX_VALUE;
        }
        else
        {
            tHeatPid.setpoint = (float)tHM.sHeatTargetTemp;
            target_heat = (u16)f_pid_compute(&tHeatPid, (float)s_water_temp);
        }

        if(us_heat_pwm != target_heat)
        {
            us_heat_pwm = target_heat;
            v_heat_pwm_set(us_heat_pwm);
        }
    }
    #endif  //boardWATER_TEMP_EN
}












/*****************************************************************************************************************
                                                   全局函数
******************************************************************************************************************/

/*****************************************************************************************************************
-----函数功能    热管理统一开关控制（参考Dc模块cDc_Switch）
-----说明(备注)  通过传入不同对象实现一个接口控制FAN和HEAT两个对象
-----传入参数    obj:     控制对象（HM_OBJ_FAN=风扇, HM_OBJ_HEAT=加热）
                type:    开关类型（ST_ON=开启, ST_OFF=关闭, ST_NULL=取反）
                fore_en: false=当前已是目标状态时直接跳过, true=强制执行
-----输出参数    none
-----返回值      0: 操作成功  -1: 参数错误
******************************************************************************************************************/
s8 cHm_Switch(HM_Object_E obj, SwitchType_E type, bool fore_en)
{
    bool *p_enable = NULL;
    bool target_state = false;
    
    // 确定操作对象
    switch(obj)
    {
        case HM_OBJ_FAN:
            p_enable = &tHM.bFanEnable;
            break;
        case HM_OBJ_HEAT:
            p_enable = &tHM.bHeatEnable;
            break;
        default:
            return -1;  // 无效对象
    }
    
    // 确定目标状态
    switch(type)
    {
        case ST_ON:
            target_state = true;
            break;
        case ST_OFF:
            target_state = false;
            break;
        case ST_NULL:
            target_state = !(*p_enable);
            break;
        default:
            return -1;  // 无效类型
    }
    
    // 如果当前状态已经是目标状态且不需要强制执行，则直接跳过
    if((*p_enable) == target_state && fore_en == false)
    {
        return 0;
    }
    
    // 执行开关操作
    if(obj == HM_OBJ_FAN)
    {
        tHM.bFanEnable = target_state;
        if(target_state == false)
        {
            tHM.usValue = us_fan_set_work_mode(FWM_OFF);
            v_fan_pwm_set(0, tHM.usValue);
            #if(boardFAN2_EN)
            v_fan_pwm_set(1, tHM.usValue);
            #endif  //boardFAN2_EN
        }
    }
    else if(obj == HM_OBJ_HEAT)
    {
        tHM.bHeatEnable = target_state;
        if(target_state == false)
        {
            us_heat_pwm = 0;
            v_heat_pwm_set(0);
            // 重置PID积分项，避免下次开启时积分饱和
            tHeatPid.integral = 0.0f;
            tHeatPid.prev_error = 0.0f;
        }
    }
    
    return 0;
}

/*****************************************************************************************************************
-----函数功能    获取风扇当前工作模式
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      当前风扇工作模式
******************************************************************************************************************/
FanWorkMode_E eFan_GetWorkMode(void)
{
    return tHM.eWordMode;
}

/*****************************************************************************************************************
-----函数功能    循环切换风扇UI模式
-----说明(备注)  add=true时升档，add=false时降档，到达边界后循环
-----传入参数    add: true=加档  false=减档
-----输出参数    none
-----返回值      true: 操作成功
******************************************************************************************************************/
bool bFan_CycleUiMode(bool add)
{
    // 风扇功能未开启时不响应UI操作
    if(tHM.bFanEnable == false)
        return false;

    FanWorkMode_E next_mode;

    if(b_fan_ui_override == false)
        next_mode = tHM.eWordMode;
    else
        next_mode = e_fan_ui_mode;

    if(add == true)
    {
        if(next_mode < FWM_GEAR_FULL)
            next_mode = (FanWorkMode_E)(next_mode + 1);
        else
            next_mode = FWM_OFF;
    }
    else if(next_mode > FWM_OFF)
        next_mode = (FanWorkMode_E)(next_mode - 1);
    else
        next_mode = FWM_GEAR_FULL;

    e_fan_ui_mode = next_mode;
    b_fan_ui_override = true;
    tHM.usValue = us_fan_set_work_mode(e_fan_ui_mode);
    v_fan_pwm_set(0, tHM.usValue);
    #if(boardFAN2_EN)
    v_fan_pwm_set(1, tHM.usValue);
    #endif  //boardFAN2_EN

    return true;
}

/*****************************************************************************************************************
-----函数功能    查询风扇是否被UI强制覆盖
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: UI强制覆盖中  false: 自动模式
******************************************************************************************************************/
bool bFan_IsUiOverride(void)
{
    return b_fan_ui_override;
}

/*****************************************************************************************************************
-----函数功能    强制开关风扇
-----说明(备注)  直接设置风扇为全速或关闭，会覆盖自动温控逻辑
-----传入参数    en: true=全速  false=关闭
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_ForceOpenFan(bool en)
{
    if(en)
        tHM.usValue = us_fan_set_work_mode(FWM_GEAR_FULL);
    else
        tHM.usValue = us_fan_set_work_mode(FWM_OFF);
    v_fan_pwm_set(0, tHM.usValue);
    #if(boardFAN2_EN)
    v_fan_pwm_set(1, tHM.usValue);
    #endif  //boardFAN2_EN
}

/*****************************************************************************************************************
-----函数功能    查询风扇功能是否使能
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 风扇功能已开启  false: 风扇功能未开启
******************************************************************************************************************/
bool bFan_IsEnable(void)
{
    return tHM.bFanEnable;
}

/*****************************************************************************************************************
-----函数功能    设置风扇PWM上限（推荐接口）
-----说明(备注)  用于限制自动和档位模式下的最大风扇PWM
-----传入参数    pwm: PWM上限值（0~hmPWM_MAX_VALUE）
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_SetMaxPwm(u16 pwm)
{
    if(pwm > hmPWM_MAX_VALUE)
        pwm = hmPWM_MAX_VALUE;
    tHM.usFanTargetVolt = pwm;
}

/*****************************************************************************************************************
-----函数功能    获取风扇PWM上限（推荐接口）
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      当前设置的风扇PWM上限值
******************************************************************************************************************/
u16 usFan_GetMaxPwm(void)
{
    return tHM.usFanTargetVolt;
}

/*****************************************************************************************************************
-----函数功能    设置风扇无级调速温度区间
-----说明(备注)  temp_start为起转温度，temp_full为满速温度
-----传入参数    temp_start/temp_full: 温度(°C)
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_SetTargetTempRange(s16 temp_start, s16 temp_full)
{
    if(temp_full <= temp_start)
        return;

    tHM.sFanTempStart = temp_start;
    tHM.sFanTempFull = temp_full;
}

/*****************************************************************************************************************
-----函数功能    获取风扇无级调速温度区间
-----说明(备注)  none
-----传入参数    p_temp_start/p_temp_full: 输出指针，可为NULL
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_GetTargetTempRange(s16 *p_temp_start, s16 *p_temp_full)
{
    if(p_temp_start)
        *p_temp_start = tHM.sFanTempStart;
    if(p_temp_full)
        *p_temp_full = tHM.sFanTempFull;
}

/*****************************************************************************************************************
-----函数功能    设置加热棒UI强制开关
-----说明(备注)  在设备工作状态下立即生效
-----传入参数    en: true=强制开启  false=关闭强制
-----输出参数    none
-----返回值      true: 操作成功
******************************************************************************************************************/
bool bHeat_SetUiForce(bool en)
{
    b_heat_ui_force_on = en;

    if(bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR)
    {
        if(tHM.bHeatEnable)
        {
            us_heat_pwm = (b_heat_ui_force_on == true) ? hmPWM_MAX_VALUE : 0;
            v_heat_pwm_set(us_heat_pwm);
        }
    }

    return true;
}

/*****************************************************************************************************************
-----函数功能    切换加热棒UI强制开关状态
-----说明(备注)  对当前强制状态取反
-----传入参数    none
-----输出参数    none
-----返回值      true: 操作成功
******************************************************************************************************************/
bool bHeat_ToggleUiForce(void)
{
    return bHeat_SetUiForce(!b_heat_ui_force_on);
}

/*****************************************************************************************************************
-----函数功能    查询加热棒是否被UI强制开启
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: UI强制开启  false: 自动模式
******************************************************************************************************************/
bool bHeat_IsUiForceOn(void)
{
    return b_heat_ui_force_on;
}

/*****************************************************************************************************************
-----函数功能    设置加热功能使能（预留开启接口）
-----说明(备注)  默认关闭，调用此接口开启后加热才会工作
-----传入参数    en: true=开启加热功能  false=关闭加热功能
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vHeat_SetEnable(bool en)
{
    tHM.bHeatEnable = en;
    if(en == false)
    {
        us_heat_pwm = 0;
        v_heat_pwm_set(0);
        // 重置PID积分项，避免下次开启时积分饱和
        tHeatPid.integral = 0.0f;
        tHeatPid.prev_error = 0.0f;
    }
}

/*****************************************************************************************************************
-----函数功能    查询加热功能是否使能
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true: 加热功能已开启  false: 加热功能未开启
******************************************************************************************************************/
bool bHeat_IsEnable(void)
{
    return tHM.bHeatEnable;
}

/*****************************************************************************************************************
-----函数功能    设置加热目标温度（预留接口）
-----说明(备注)  设置PID控制的目标水温
-----传入参数    temp: 目标温度(°C)
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vHeat_SetTargetTemp(s16 temp)
{
    tHM.sHeatTargetTemp = temp;
    tHeatPid.setpoint = (float)temp;
}

/*****************************************************************************************************************
-----函数功能    获取加热目标温度
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      当前设置的目标温度(°C)
******************************************************************************************************************/
s16 sHeat_GetTargetTemp(void)
{
    return tHM.sHeatTargetTemp;
}

#if(boardLOW_POWER)
/*****************************************************************************************************************
-----函数功能    风扇进入低功耗模式
-----说明(备注)  关闭风扇PWM并设置IO为低功耗状态
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_EnterLowPower(void)
{
    vFan_IoEnterLowPower();
}

/*****************************************************************************************************************
-----函数功能    风扇退出低功耗模式
-----说明(备注)  重新初始化风扇PWM接口
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vFan_ExitLowPower(void)
{
    vFan_PwmInit();
}
#endif  //boardLOW_POWER
