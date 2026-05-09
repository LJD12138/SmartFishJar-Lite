/*****************************************************************************************************************
*                                                                                                                *
*                                         Light task (RGBW 4 channels)                                           *
*                                                                                                                *
******************************************************************************************************************/
#include "MD_Light/md_light_task.h"

#if(boardLIGHT_EN)
#include "MD_Light/md_light_iface.h"
#include "Sys/sys_task.h"
#include "board_config.h"

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif  //boardADC_EN


//**************************************************** task init *************************************************//
#if(boardUSE_OS)
#define         LIGHT_TASK_PRIO                 1
#define         LIGHT_TASK_STK_SIZE             192
#define         LIGHT_TASK_CYCLE_MS             200
TaskHandle_t    tLightTaskHandler = NULL;
void            vLight_Task(void *pvParameters);
#endif  //boardUSE_OS


//**************************************************** runtime data **********************************************//
Light_T tLight;

#define LIGHT_OC_CURRENT                        2.5f
#define LIGHT_AUTO_PWM_BRIGHT                   200U
#define LIGHT_AUTO_PWM_MID                      500U


//**************************************************** local prototypes ******************************************//
static void v_light_param_init(void);
static bool b_light_is_active_state(void);
static void v_light_shutdown_output(void);
static void v_light_set_rgbw(u16 warm, u16 blue, u16 green, u16 red);
static u16 us_light_get_auto_pwm(void);
static u16 us_light_get_mode_pwm(LightWorkMode_E mode);
static u16 us_light_get_rgb_pwm(RGBWorkMode_E mode);
static void v_light_apply_mode(void);
static void v_light_refresh_power(void);


/*****************************************************************************************************************
----- function      init light parameters
******************************************************************************************************************/
static void v_light_param_init(void)
{
    tLight.usPower    = 0;
    tLight.usWarm     = 0;
    tLight.usBlue     = 0;
    tLight.usGreen    = 0;
    tLight.usRed      = 0;
    tLight.eLightMode = LWM_OFF;
    tLight.eRGBMode   = RWM_OFF;
    tLight.eDevState  = DS_SHUT_DOWN;
}


/*****************************************************************************************************************
----- function      check whether light task is allowed to run
******************************************************************************************************************/
static bool b_light_is_active_state(void)
{
    return (bSys_IsWorkState() == true || tSysInfo.eDevState == DS_ERR);
}


/*****************************************************************************************************************
----- function      get auto-mode white PWM from ambient light sensor
******************************************************************************************************************/
static u16 us_light_get_auto_pwm(void)
{
    #if(boardADC_EN && boardLIGHT_SENSOR_EN)
    u16 lux = tAdcSamp.usLightRes;

    if(lux < 100U)
        return LIGHT_AUTO_PWM_BRIGHT;
    if(lux < 400U)
        return LIGHT_AUTO_PWM_MID;
    return lightPWM_MAX_VALUE;
    #else
    return lightPWM_MAX_VALUE;
    #endif
}


/*****************************************************************************************************************
----- function      translate white mode to warm channel PWM
******************************************************************************************************************/
static u16 us_light_get_mode_pwm(LightWorkMode_E mode)
{
    switch(mode)
    {
        case LWM_LOW:
            return 250U;
        case LWM_HALF:
            return 500U;
        case LWM_FULL:
            return lightPWM_MAX_VALUE;
        case LWM_AUTO:
            return us_light_get_auto_pwm();
        default:
            return 0U;
    }
}


/*****************************************************************************************************************
----- function      translate RGB mode to color channel PWM
******************************************************************************************************************/
static u16 us_light_get_rgb_pwm(RGBWorkMode_E mode)
{
    switch(mode)
    {
        case RWM_LOW:
            return 250U;
        case RWM_HALF:
            return 500U;
        case RWM_FULL:
        case RWM_AUTO:
            return lightPWM_MAX_VALUE;
        default:
            return 0U;
    }
}


/*****************************************************************************************************************
----- function      refresh light power estimate
******************************************************************************************************************/
static void v_light_refresh_power(void)
{
    #if(boardADC_EN)
    u16 light_curr_ma = (u16)(tAdcSamp.fLightCurr * 1000.0f);
    tLight.usPower = (u16)(((u32)tAdcSamp.us12VVolt * light_curr_ma + 5000U) / 10000U);
    #else
    u32 total_pwm = (u32)tLight.usWarm + (u32)tLight.usBlue + (u32)tLight.usGreen + (u32)tLight.usRed;
    tLight.usPower = (u16)((total_pwm * 24U) / (4U * lightPWM_MAX_VALUE));
    #endif
}


/*****************************************************************************************************************
----- function      write RGBW outputs and sync state
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

    if((tLight.usWarm > 0U) || (tLight.usBlue > 0U) || (tLight.usGreen > 0U) || (tLight.usRed > 0U))
        tLight.eDevState = DS_WORK;
    else
        tLight.eDevState = DS_SHUT_DOWN;

    v_light_refresh_power();
}


/*****************************************************************************************************************
----- function      shutdown all light outputs
******************************************************************************************************************/
static void v_light_shutdown_output(void)
{
    v_light_set_rgbw(0, 0, 0, 0);
    tLight.usPower = 0;
    tLight.eDevState = DS_SHUT_DOWN;
}


/*****************************************************************************************************************
----- function      apply current light mode to hardware
******************************************************************************************************************/
static void v_light_apply_mode(void)
{
    u16 warm_pwm = 0U;
    u16 rgb_pwm = us_light_get_rgb_pwm(tLight.eRGBMode);

    static u8 uc_sos_step = 0U;
    static bool b_twinkle_on = false;

    switch(tLight.eLightMode)
    {
        case LWM_LOW:
        case LWM_HALF:
        case LWM_FULL:
        case LWM_AUTO:
            warm_pwm = us_light_get_mode_pwm(tLight.eLightMode);
            v_light_set_rgbw(warm_pwm, rgb_pwm, rgb_pwm, rgb_pwm);
            break;

        case LWM_SOS:
            uc_sos_step++;
            if(uc_sos_step >= 10U)
                uc_sos_step = 0U;

            if((uc_sos_step == 0U) || (uc_sos_step == 2U) || (uc_sos_step == 4U)
            || (uc_sos_step == 6U) || (uc_sos_step == 7U) || (uc_sos_step == 8U))
                v_light_set_rgbw(lightPWM_MAX_VALUE, 0U, 0U, lightPWM_MAX_VALUE);
            else
                v_light_set_rgbw(0U, 0U, 0U, 0U);
            break;

        case LWM_TWINKLE:
            b_twinkle_on = (bool)!b_twinkle_on;
            if(b_twinkle_on == true)
                v_light_set_rgbw(350U, rgb_pwm, 0U, rgb_pwm);
            else
                v_light_set_rgbw(0U, 0U, 0U, 0U);
            break;

        case LWM_OFF:
        default:
            v_light_shutdown_output();
            break;
    }
}


/*****************************************************************************************************************
----- function      init light task
******************************************************************************************************************/
void vLight_TaskInit(void)
{
    vLight_IfaceInit();
    v_light_param_init();
    v_light_shutdown_output();

    #if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vLight_Task,
                (const char*    )"LightTask",
                (uint16_t       )LIGHT_TASK_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LIGHT_TASK_PRIO,
                (TaskHandle_t*  )&tLightTaskHandler);
    #endif  //boardUSE_OS
}


/*****************************************************************************************************************
----- function      light task loop
******************************************************************************************************************/
void vLight_Task(void *pvParameters)
{
    (void)pvParameters;

    #if(boardUSE_OS)
    for(;;)
    #endif
    {
        #if(boardADC_EN)
        if(tAdcSamp.fLightCurr > LIGHT_OC_CURRENT)
        {
            tLight.eLightMode = LWM_OFF;
            tLight.eRGBMode = RWM_OFF;
            v_light_shutdown_output();

            #if(boardUSE_OS)
            vTaskDelay(500);
            #endif
            continue;
        }
        #endif

        if(b_light_is_active_state() == true)
            v_light_apply_mode();
        else
        {
            tLight.eLightMode = LWM_OFF;
            tLight.eRGBMode = RWM_OFF;
            v_light_shutdown_output();
        }

        #if(boardUSE_OS)
        vTaskDelay(LIGHT_TASK_CYCLE_MS);
        #endif
    }
}


/*****************************************************************************************************************
----- function      set white light mode
******************************************************************************************************************/
bool bLight_SetMode(LightWorkMode_E mode)
{
    tLight.eLightMode = mode;

    if(mode == LWM_OFF && tLight.eRGBMode == RWM_OFF)
        tLight.eDevState = DS_SHUT_DOWN;

    return true;
}


/*****************************************************************************************************************
----- function      set RGB light mode
******************************************************************************************************************/
bool bLight_SetRGBMode(RGBWorkMode_E mode)
{
    tLight.eRGBMode = mode;
    return true;
}


/*****************************************************************************************************************
----- function      switch light on/off/toggle
******************************************************************************************************************/
bool bLight_Switch(SwitchType_E type)
{
    switch(type)
    {
        case ST_ON:
            if(tLight.eLightMode == LWM_OFF)
                tLight.eLightMode = LWM_FULL;
            if(tLight.eRGBMode == RWM_OFF)
                tLight.eRGBMode = RWM_LOW;
            break;

        case ST_OFF:
            tLight.eLightMode = LWM_OFF;
            tLight.eRGBMode = RWM_OFF;
            v_light_shutdown_output();
            break;

        case ST_NULL:
            if((tLight.eLightMode == LWM_OFF) && (tLight.eRGBMode == RWM_OFF))
            {
                tLight.eLightMode = LWM_FULL;
                tLight.eRGBMode = RWM_LOW;
            }
            else
            {
                tLight.eLightMode = LWM_OFF;
                tLight.eRGBMode = RWM_OFF;
                v_light_shutdown_output();
            }
            break;

        default:
            return false;
    }

    return true;
}


/*****************************************************************************************************************
----- function      cycle through preset white light modes
******************************************************************************************************************/
void vLight_CircSelectMode(void)
{
    switch(tLight.eLightMode)
    {
        case LWM_OFF:
            tLight.eLightMode = LWM_LOW;
            break;

        case LWM_LOW:
            tLight.eLightMode = LWM_HALF;
            break;

        case LWM_HALF:
            tLight.eLightMode = LWM_FULL;
            break;

        case LWM_FULL:
            tLight.eLightMode = LWM_AUTO;
            break;

        case LWM_AUTO:
            tLight.eLightMode = LWM_SOS;
            break;

        case LWM_SOS:
            tLight.eLightMode = LWM_TWINKLE;
            break;

        case LWM_TWINKLE:
        default:
            tLight.eLightMode = LWM_OFF;
            break;
    }
}


#if(boardLOW_POWER)
void vLight_EnterLowPower(void)
{
    v_light_shutdown_output();
    vLight_IoEnterLowPower();
}

void vLight_ExitLowPower(void)
{
    vLight_IfaceInit();
    v_light_shutdown_output();
}
#endif

#endif  //boardLIGHT_EN
