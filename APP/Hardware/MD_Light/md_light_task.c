/*****************************************************************************************************************
*                                                                                                                *
 *                                         照明灯任务（RGBW 4通道）                                            *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Light/md_light_task.h"

#if(boardLIGHT_EN)
#include "MD_Light/md_light_iface.h"
#include "Sys/sys_task.h"
#include "board_config.h"
#include "main.h"

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif

//****************************************************任务初始化**************************************************//
#if(boardUSE_OS)
#define        LIGHT_TASK_PRIO                  1
#define        LIGHT_TASK_STK_SIZE              192
TaskHandle_t    tLightTaskHandler = NULL;
void           vLight_Task(void *pvParameters);
#endif  //boardUSE_OS

//****************************************************参数初始化**************************************************//
Light_T         tLight;

//****************************************************函数声明****************************************************//
static void v_light_set_rgbw(u16 warm, u16 blue, u16 green, u16 red);


/*****************************************************************************************************************
-----函数功能    照明任务初始化
******************************************************************************************************************/
void vLight_TaskInit(void)
{
    vLight_IfaceInit();

    tLight.usWarm = 0;
    tLight.usBlue = 0;
    tLight.usGreen = 0;
    tLight.usRed = 0;
    tLight.eWordMode = LWM_OFF;

    #if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vLight_Task,
                (const char* )"LightTask",
                (uint16_t ) LIGHT_TASK_STK_SIZE,
                (void* )NULL,
                (UBaseType_t ) LIGHT_TASK_PRIO,
                (TaskHandle_t*)&tLightTaskHandler);
    #endif  //boardUSE_OS
}

/*****************************************************************************************************************
-----函数功能    照明任务运行
******************************************************************************************************************/
void vLight_Task(void *pvParameters)
{
    #if(boardUSE_OS)
    for(;;)
    #endif
    {
        if(bSys_IsWorkState() == true)
        {
            /* 光传感器自动调光系数（0~1000） */
            u16 dim_factor = lightPWM_MAX_VALUE;
            #if(boardADC_EN && boardLIGHT_SENSOR_EN)
            {
                /* usLightRes越大=环境越暗，提高亮度；越小=环境越亮，降低亮度 */
                u16 lux = tAdcSamp.usLightRes;
                if(lux < 100)
                    dim_factor = 200;               // 白天强光：20%
                else if(lux < 400)
                    dim_factor = 500;               // 正常室内：50%
                else
                    dim_factor = lightPWM_MAX_VALUE; // 暗环境：100%
            }
            #endif

            /* 过流保护：灯条电流超叙值，强制关闭 */
            #if(boardADC_EN)
            if(tAdcSamp.fLightCurr > 2.5f)
            {
                v_light_set_rgbw(0, 0, 0, 0);
                tLight.eWordMode = LWM_OFF;
                #if(boardUSE_OS)
                vTaskDelay(500);
                #endif
                continue;
            }
            #endif

            switch(tLight.eWordMode)
            {
                case LWM_HALF:
                {
                    u16 val = (lightPWM_MAX_VALUE / 2) * dim_factor / lightPWM_MAX_VALUE;
                    v_light_set_rgbw(val, val / 2, val / 3, 0);
                }
                break;

                case LWM_FULL:
                {
                    u16 val = lightPWM_MAX_VALUE * dim_factor / lightPWM_MAX_VALUE;
                    v_light_set_rgbw(val, val, val, val);
                }
                break;

                case LWM_OFF:
                default:
                    v_light_set_rgbw(0, 0, 0, 0);
                    break;
            }
        }
        else
        {
            if(tLight.eWordMode != LWM_OFF)
            {
                tLight.eWordMode = LWM_OFF;
                v_light_set_rgbw(0, 0, 0, 0);
            }
        }

        #if(boardUSE_OS)
        vTaskDelay(200);
        #endif
    }
}

/*****************************************************************************************************************
-----函数功能    同时设置RGBW四路PWM
******************************************************************************************************************/
static void v_light_set_rgbw(u16 warm, u16 blue, u16 green, u16 red)
{
    tLight.usWarm  = LIMIT_MAX(warm,  lightPWM_MAX_VALUE);
    tLight.usBlue  = LIMIT_MAX(blue,  lightPWM_MAX_VALUE);
    tLight.usGreen = LIMIT_MAX(green, lightPWM_MAX_VALUE);
    tLight.usRed   = LIMIT_MAX(red,   lightPWM_MAX_VALUE);
    lightW_PWM_SET(tLight.usWarm);
    lightB_PWM_SET(tLight.usBlue);
    lightG_PWM_SET(tLight.usGreen);
    lightR_PWM_SET(tLight.usRed);
}

/*****************************************************************************************************************
-----函数功能    开关灯控制（外部调用）
******************************************************************************************************************/
bool bLight_Switch(SwitchType_E type)
{
    if(type == ST_ON)
    {
        if(tLight.eWordMode == LWM_OFF)
            tLight.eWordMode = LWM_FULL;
    }
    else
    {
        tLight.eWordMode = LWM_OFF;
        v_light_set_rgbw(0, 0, 0, 0);
    }
    return true;
}

/*****************************************************************************************************************
-----函数功能    循环切换模式（外部调用）
******************************************************************************************************************/
void vLight_CircSelectMode(void)
{
    switch(tLight.eWordMode)
    {
        case LWM_OFF:  tLight.eWordMode = LWM_HALF; break;
        case LWM_HALF: tLight.eWordMode = LWM_FULL; break;
        case LWM_FULL: tLight.eWordMode = LWM_OFF;  break;
        default:       tLight.eWordMode = LWM_OFF;  break;
    }
}

#if(boardLOW_POWER)
void vLight_EnterLowPower(void)
{
    vLight_IoEnterLowPower();
}

void vLight_ExitLowPower(void)
{
    vLight_IfaceInit();
}
#endif

#endif  //boardLIGHT_EN
