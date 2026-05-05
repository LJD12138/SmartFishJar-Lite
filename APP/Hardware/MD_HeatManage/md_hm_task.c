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

#endif



//****************************************************任务初始化**************************************************//

#if(boardUSE_OS)

#define        	HM_TASK_PRIO                  			1

#define        	HM_TASK_STK_SIZE              			192

TaskHandle_t    tHeatManageHandler = NULL;

void           	vHW_Task(void *pvParameters);

#endif  //boardUSE_OS



//****************************************************参数初始化**************************************************//

HM_T           tHM;



static bool b_fan_stop_to_run_flag = 0;
static bool b_fan_ui_override = 0;
static FanWorkMode_E e_fan_ui_mode = FWM_OFF;

static bool b_heat_ui_force_on = 0;

static u8   uc_updata_delay = 0;

static s16  s_water_temp = 0;     // 水温（取NTC1/NTC2均值）

static u16  us_heat_pwm  = 0;    // 加热棒PWM当前值



//****************************************************函数声明****************************************************//

static void v_fan_pwm_set(u8 fan_id, u16 level);

static void v_heat_pwm_set(u16 level);

static u16  us_fan_set_work_mode(FanWorkMode_E mode);





/*****************************************************************************************************************

-----函数功能    热管理任务初始化

******************************************************************************************************************/

bool bHM_TaskInit(void)

{

    vFan_PwmInit();

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

******************************************************************************************************************/

void vHW_Task(void *pvParameters)

{

    #if(boardUSE_OS)

    for(;;)

    #endif

    {

        if(bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR)

        {

            if(++uc_updata_delay >= 3)

            {

                uc_updata_delay = 0;

                #if(boardADC_EN && boardWATER_TEMP_EN)

                s_water_temp = (tAdcSamp.sWaterTemp1 + tAdcSamp.sWaterTemp2) / 2;

                #else

                s_water_temp = tSysInfo.sMaxTemp;

                #endif

            }



            {

                s16 pcb_temp = 0;

                #if(boardADC_EN)

                pcb_temp = (tAdcSamp.s5VTemp > tAdcSamp.s12VTemp) ? tAdcSamp.s5VTemp : tAdcSamp.s12VTemp;

                #endif



                if(b_fan_ui_override == true)

                    tHM.usValue = us_fan_set_work_mode(e_fan_ui_mode);

                else

                    switch (tHM.eWordMode)

                    {

                        default:

                        case FWM_OFF:

                            if(pcb_temp > 40)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_1);

                            break;

                        case FWM_GEAR_1:

                            if(pcb_temp < 38)

                                tHM.usValue = us_fan_set_work_mode(FWM_OFF);

                            else if(pcb_temp > 44)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_2);

                            break;

                        case FWM_GEAR_2:

                            if(pcb_temp < 42)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_1);

                            else if(pcb_temp > 48)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_3);

                            break;

                        case FWM_GEAR_3:

                            if(pcb_temp < 46)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_2);

                            else if(pcb_temp > 52)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_FULL);

                            break;

                        case FWM_GEAR_FULL:

                            if(pcb_temp < 50)

                                tHM.usValue = us_fan_set_work_mode(FWM_GEAR_3);

                            break;

                    }



                if((tHM.eWordMode < FWM_GEAR_2 && tHM.eWordMode > FWM_OFF) && b_fan_stop_to_run_flag == 0)

                {

                    b_fan_stop_to_run_flag = 1;

                    tHM.usValue = us_fan_set_work_mode(FWM_GEAR_2);

                }



                v_fan_pwm_set(0, tHM.usValue);

                #if(boardFAN2_EN)

                v_fan_pwm_set(1, tHM.usValue);

                #endif

            }



            #if(boardWATER_TEMP_EN)

            {

                u16 target_heat = 0;

                if(b_heat_ui_force_on == true)

                    target_heat = hmPWM_MAX_VALUE;

                else if(s_water_temp < 22)

                    target_heat = hmPWM_MAX_VALUE;

                else if(s_water_temp < 25)

                    target_heat = hmPWM_MAX_VALUE / 2;

                else

                    target_heat = 0;



                if(us_heat_pwm != target_heat)

                {

                    us_heat_pwm = target_heat;

                    v_heat_pwm_set(us_heat_pwm);

                }

            }

            #endif

        }

        else

        {

            if(tHM.eWordMode != FWM_OFF || tHM.usValue != 0)

            {

                tHM.usValue = us_fan_set_work_mode(FWM_OFF);

                v_fan_pwm_set(0, tHM.usValue);

                #if(boardFAN2_EN)

                v_fan_pwm_set(1, tHM.usValue);

                #endif

            }



            if(us_heat_pwm != 0)

            {

                us_heat_pwm = 0;

                v_heat_pwm_set(0);

            }

        }



        #if(boardUSE_OS)

        vTaskDelay(1000);

        #endif

    }

}



/*****************************************************************************************************************

-----函数功能    设置风扇PWM（支持双风扇独立控制）

-----传入参数    fan_id: 0=风扇1  1=风扇2

                level:  PWM值（0~hmPWM_MAX_VALUE）

******************************************************************************************************************/

static void v_fan_pwm_set(u8 fan_id, u16 level)

{

    level = LIMIT_MAX(level, hmPWM_MAX_VALUE);

    if(fan_id == 0)

        fanPWM1_SET(level);

    #if(boardFAN2_EN)

    else

        fanPWM2_SET(level);

    #endif

}



/*****************************************************************************************************************

-----函数功能    设置加热棒PWM

-----传入参数    level: PWM值（0~hmPWM_MAX_VALUE）

******************************************************************************************************************/

static void v_heat_pwm_set(u16 level)

{

    level = LIMIT_MAX(level, hmPWM_MAX_VALUE);

    heatPWM_SET(level);

}



/*****************************************************************************************************************

-----函数功能    设置风扇工作模式，返回对应PWM值

******************************************************************************************************************/

static u16 us_fan_set_work_mode(FanWorkMode_E mode)

{

    u16 temp = 0;



    switch(mode)

    {

        case FWM_GEAR_1:   temp = 200;            break;

        case FWM_GEAR_2:   temp = 500;            break;

        case FWM_GEAR_3:   temp = 800;            break;

        case FWM_GEAR_FULL: temp = hmPWM_MAX_VALUE; break;

        default:

            temp = 0;

            b_fan_stop_to_run_flag = 0;

            mode = FWM_OFF;

            break;

    }



    tHM.eWordMode = mode;

    return temp;

}





/************************************************************************************************************************

                                                  全局函数

*************************************************************************************************************************/



FanWorkMode_E eFan_GetWorkMode(void)

{
    return tHM.eWordMode;

}



bool bFan_CycleUiMode(bool add)

{

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

    #endif



    return true;

}



bool bFan_IsUiOverride(void)

{

    return b_fan_ui_override;

}



void vFan_ForceOpenFan(bool en)

{

    if(en)

        tHM.usValue = us_fan_set_work_mode(FWM_GEAR_FULL);

    else

        tHM.usValue = us_fan_set_work_mode(FWM_OFF);

    v_fan_pwm_set(0, tHM.usValue);

    #if(boardFAN2_EN)

    v_fan_pwm_set(1, tHM.usValue);

    #endif

}



bool bHeat_SetUiForce(bool en)

{

    b_heat_ui_force_on = en;



    if(bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR)

    {

        us_heat_pwm = (b_heat_ui_force_on == true) ? hmPWM_MAX_VALUE : 0;

        v_heat_pwm_set(us_heat_pwm);

    }



    return true;

}



bool bHeat_ToggleUiForce(void)

{

    return bHeat_SetUiForce(!b_heat_ui_force_on);

}



bool bHeat_IsUiForceOn(void)

{

    return b_heat_ui_force_on;

}



#if(boardLOW_POWER)

void vFan_EnterLowPower(void)

{

    vFan_IoEnterLowPower();

}



void vFan_ExitLowPower(void)

{

    vFan_PwmInit();

}

#endif



