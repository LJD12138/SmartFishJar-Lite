/*****************************************************************************************************************
*                                                                                                                *
 *                                         ADC任务****                                                          *
*                                                                                                                *
******************************************************************************************************************/
#include "board_config.h"

#if(boardADC_EN)
#include "Adc/adc_task.h"
#include "Adc/adc_iface.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"

#include "filtration.h"
#include "gpio_init.h"
#include "math.h"

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS


//****************************************************任务初始化**************************************************//
#if(boardUSE_OS)
#define       	ADC_TASK_PRIO                  			2         // 任务优先级 
#define       	ADC_TASK_STK_SIZE              			256       // 任务堆栈  实际字节数 *4
TaskHandle_t    tAdcTaskHandler = NULL; 
void           	vAdc_Task(void *pvParameters);
#endif  //boardUSE_OS


//****************************************************参数初始化**************************************************//

// VIN电压滤波器（6点均值）
#define     adcVIN_VOLT_FILTER_SIZE         6
static s32  s_vin_volt_buff[adcVIN_VOLT_FILTER_SIZE];
FilterHandler_T tAdc_VinVoltFilter = {s_vin_volt_buff, adcVIN_VOLT_FILTER_SIZE, 0, 0, 0, 0, 0};

// VIN电流滤波器（12点均值）
#define     adcVIN_CURR_FILTER_SIZE         12
static s32  s_vin_curr_buff[adcVIN_CURR_FILTER_SIZE];
FilterHandler_T tAdc_VinCurrFilter = {s_vin_curr_buff, adcVIN_CURR_FILTER_SIZE, 0, 0, 0, 0, 0};

// 12V电压滤波器（6点均值）
#define     adc12V_VOLT_FILTER_SIZE         6
static s32  s_12v_volt_buff[adc12V_VOLT_FILTER_SIZE];
FilterHandler_T tAdc_12VVoltFilter = {s_12v_volt_buff, adc12V_VOLT_FILTER_SIZE, 0, 0, 0, 0, 0};

// 12V_NTC滤波器（6点均值）
#define     adc12V_NTC_FILTER_SIZE          6
static s32  s_12v_ntc_buff[adc12V_NTC_FILTER_SIZE];
FilterHandler_T tAdc_12VNtcFilter = {s_12v_ntc_buff, adc12V_NTC_FILTER_SIZE, 0, 0, 0, 0, 0};

// 5V_NTC滤波器（6点均值）
#define     adc5V_NTC_FILTER_SIZE           6
static s32  s_5v_ntc_buff[adc5V_NTC_FILTER_SIZE];
FilterHandler_T tAdc_5VNtcFilter = {s_5v_ntc_buff, adc5V_NTC_FILTER_SIZE, 0, 0, 0, 0, 0};

// 加热棒电流滤波器（12点均值）
#define     adcHEAT_CURR_FILTER_SIZE        12
static s32  s_heat_curr_buff[adcHEAT_CURR_FILTER_SIZE];
FilterHandler_T tAdc_HeatCurrFilter = {s_heat_curr_buff, adcHEAT_CURR_FILTER_SIZE, 0, 0, 0, 0, 0};

// 氧气泵电流滤波器（12点均值）
#define     adcO2_CURR_FILTER_SIZE          12
static s32  s_o2_curr_buff[adcO2_CURR_FILTER_SIZE];
FilterHandler_T tAdc_O2CurrFilter = {s_o2_curr_buff, adcO2_CURR_FILTER_SIZE, 0, 0, 0, 0, 0};

// 水泵电流滤波器（12点均值）
#define     adcPUMP_CURR_FILTER_SIZE        12
static s32  s_pump_curr_buff[adcPUMP_CURR_FILTER_SIZE];
FilterHandler_T tAdc_PumpCurrFilter = {s_pump_curr_buff, adcPUMP_CURR_FILTER_SIZE, 0, 0, 0, 0, 0};

// 灯光电流滤波器（12点均值）
#define     adcLIGHT_CURR_FILTER_SIZE       12
static s32  s_light_curr_buff[adcLIGHT_CURR_FILTER_SIZE];
FilterHandler_T tAdc_LightCurrFilter = {s_light_curr_buff, adcLIGHT_CURR_FILTER_SIZE, 0, 0, 0, 0, 0};

// 光照传感器滤波器（6点均值）
#define     adcLIGHT_RES_FILTER_SIZE        6
static s32  s_light_res_buff[adcLIGHT_RES_FILTER_SIZE];
FilterHandler_T tAdc_LightResFilter = {s_light_res_buff, adcLIGHT_RES_FILTER_SIZE, 0, 0, 0, 0, 0};

// 水温NTC1滤波器（6点均值）
#define     adcWATER_NTC1_FILTER_SIZE       6
static s32  s_water_ntc1_buff[adcWATER_NTC1_FILTER_SIZE];
FilterHandler_T tAdc_WaterNtc1Filter = {s_water_ntc1_buff, adcWATER_NTC1_FILTER_SIZE, 0, 0, 0, 0, 0};

// 水温NTC2滤波器（6点均值）
#define     adcWATER_NTC2_FILTER_SIZE       6
static s32  s_water_ntc2_buff[adcWATER_NTC2_FILTER_SIZE];
FilterHandler_T tAdc_WaterNtc2Filter = {s_water_ntc2_buff, adcWATER_NTC2_FILTER_SIZE, 0, 0, 0, 0, 0};

AdcSamp_T 	tAdcSamp;


//****************************************************函数定义*****************************************************//
//static void v_power_select(bool en);
static void v_adc_param_init(void);


/***********************************************************************************************************************
-----函数功能    ADC任务初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vAdc_TaskInit(void)
{
	#if(boardLOW_POWER)
	vAdc_IoEnterLowPower();
	#endif
	
	vAdc_Init(); //AD初始化
	
	v_adc_param_init();
	
	#if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vAdc_Task,				// 任务函数 (1)
                (const char* )"AdcTask",				// 任务名称
                (uint16_t ) ADC_TASK_STK_SIZE,			// 任务堆栈大小
                (void* )NULL,							// 传递给任务函数的参数
                (UBaseType_t ) ADC_TASK_PRIO,			// 任务优先级
                (TaskHandle_t*)&tAdcTaskHandler);		// 任务句柄
	#endif  //boardUSE_OS
}

/***********************************************************************************************************************
-----函数功能    ADC参数初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_adc_param_init(void)
{
    memset((u8*)&s_vin_volt_buff,    0, sizeof(s_vin_volt_buff));
    memset((u8*)&s_vin_curr_buff,    0, sizeof(s_vin_curr_buff));
    memset((u8*)&s_12v_volt_buff,    0, sizeof(s_12v_volt_buff));
    memset((u8*)&s_12v_ntc_buff,     0, sizeof(s_12v_ntc_buff));
    memset((u8*)&s_5v_ntc_buff,      0, sizeof(s_5v_ntc_buff));
    memset((u8*)&s_heat_curr_buff,   0, sizeof(s_heat_curr_buff));
    memset((u8*)&s_o2_curr_buff,     0, sizeof(s_o2_curr_buff));
    memset((u8*)&s_pump_curr_buff,   0, sizeof(s_pump_curr_buff));
    memset((u8*)&s_light_curr_buff,  0, sizeof(s_light_curr_buff));
    memset((u8*)&s_light_res_buff,   0, sizeof(s_light_res_buff));
    memset((u8*)&s_water_ntc1_buff,  0, sizeof(s_water_ntc1_buff));
    memset((u8*)&s_water_ntc2_buff,  0, sizeof(s_water_ntc2_buff));
}

/***********************************************************************************************************************
-----函数功能    ADC循环任务
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vAdc_Task(void *pvParameters)
{
    s32 temp = 0;
    vu16 us_vin_volt = 0, us_vin_curr = 0;
    vu16 us_12v_volt = 0, us_12v_ntc = 0, us_5v_ntc = 0;
    vu16 us_heat_curr = 0, us_o2_curr = 0, us_pump_curr = 0;
    vu16 us_light_curr = 0, us_light_res = 0;
    vu16 us_water_ntc1 = 0, us_water_ntc2 = 0;

    static vu8  uc_init_adc_cnt = 0;
    static vu8  uc_delay_cnt = 0;

    #if(boardUSE_OS)
    for(;;)
    #endif  //boardUSE_OS
    {
        //*****************************滤波*****************************
        temp = usAdc_GetChannelValue(adcVIN_CURR);
        us_vin_curr = lFilter_MadianAverage(&tAdc_VinCurrFilter, &temp);

        temp = usAdc_GetChannelValue(adcVIN_VOLT);
        us_vin_volt = lFilter_MadianAverage(&tAdc_VinVoltFilter, &temp);

        temp = usAdc_GetChannelValue(adc5V_NTC);
        us_5v_ntc = lFilter_MadianAverage(&tAdc_5VNtcFilter, &temp);

        temp = usAdc_GetChannelValue(adcWATER_NTC2);
        us_water_ntc2 = lFilter_MadianAverage(&tAdc_WaterNtc2Filter, &temp);

        temp = usAdc_GetChannelValue(adcWATER_NTC1);
        us_water_ntc1 = lFilter_MadianAverage(&tAdc_WaterNtc1Filter, &temp);

        temp = usAdc_GetChannelValue(adcHEAT_CURR);
        us_heat_curr = lFilter_MadianAverage(&tAdc_HeatCurrFilter, &temp);

        temp = usAdc_GetChannelValue(adcO2_CURR);
        us_o2_curr = lFilter_MadianAverage(&tAdc_O2CurrFilter, &temp);

        temp = usAdc_GetChannelValue(adcLIGHT_CURR);
        us_light_curr = lFilter_MadianAverage(&tAdc_LightCurrFilter, &temp);

        temp = usAdc_GetChannelValue(adc12V_NTC);
        us_12v_ntc = lFilter_MadianAverage(&tAdc_12VNtcFilter, &temp);

        temp = usAdc_GetChannelValue(adc12V_VOLT);
        us_12v_volt = lFilter_MadianAverage(&tAdc_12VVoltFilter, &temp);

        temp = usAdc_GetChannelValue(adcLIGHT_RES);
        us_light_res = lFilter_MadianAverage(&tAdc_LightResFilter, &temp);

        temp = usAdc_GetChannelValue(adcPUMP_CURR);
        us_pump_curr = lFilter_MadianAverage(&tAdc_PumpCurrFilter, &temp);

        //*****************************物理量换算*****************************
        tAdcSamp.usVinVolt    = (vu16)(us_vin_volt * adcVIN_VOLT_RES_RATIO);
        tAdcSamp.fVinCurr     = us_vin_curr * adcCURR_RES_RATIO;

        tAdcSamp.us12VVolt    = (vu16)(us_12v_volt * adc12V_VOLT_RES_RATIO);
        tAdcSamp.s12VTemp     = (s16)LIMIT((307 - (37 * log((float)us_12v_ntc))), -128, 127);
        tAdcSamp.s5VTemp      = (s16)LIMIT((307 - (37 * log((float)us_5v_ntc))), -128, 127);

        tAdcSamp.fHeatCurr    = us_heat_curr * adcCURR_RES_RATIO;
        tAdcSamp.fO2Curr      = us_o2_curr   * adcCURR_RES_RATIO;
        tAdcSamp.fPumpCurr    = us_pump_curr * adcCURR_RES_RATIO;
        tAdcSamp.fLightCurr   = us_light_curr * adcCURR_RES_RATIO;

        tAdcSamp.usLightRes   = us_light_res;
        tAdcSamp.sWaterTemp1  = (s16)LIMIT((307 - (37 * log((float)us_water_ntc1))), -128, 127);
        tAdcSamp.sWaterTemp2  = (s16)LIMIT((307 - (37 * log((float)us_water_ntc2))), -128, 127);

        if(uPrint.tFlag.bAdcTask)
        {
            uc_delay_cnt++;
            if(uc_delay_cnt >= 100)
            {
                uc_delay_cnt = 0;
                sMyPrint("VIN电压=%.2fV, VIN电流=%.2fA\r\n", tAdcSamp.usVinVolt/10.0f, tAdcSamp.fVinCurr);
                sMyPrint("12V电压=%.2fV, 12V温度=%d°C\r\n", tAdcSamp.us12VVolt/10.0f, tAdcSamp.s12VTemp);
                sMyPrint("5V温度=%d°C\r\n", tAdcSamp.s5VTemp);
                sMyPrint("水温1=%d°C, 水温2=%d°C\r\n", tAdcSamp.sWaterTemp1, tAdcSamp.sWaterTemp2);
                sMyPrint("加热棒电流=%.2fA, 氧气泵电流=%.2fA\r\n", tAdcSamp.fHeatCurr, tAdcSamp.fO2Curr);
                sMyPrint("水泵电流=%.2fA, 灯光电流=%.2fA\r\n", tAdcSamp.fPumpCurr, tAdcSamp.fLightCurr);
                sMyPrint("光照ADC=%d\r\n", tAdcSamp.usLightRes);
            }
        }

        //***************************等待ADC采集稳定*****************************
        if(uc_init_adc_cnt < 0xff)
            uc_init_adc_cnt++;

        if(uc_init_adc_cnt == 10)
            tSysInfo.uInit.tFinish.bIF_AdcTask = true;

        #if(boardUSE_OS)
        if(tSysInfo.eDevState == DS_INIT)
            vTaskDelay(30);
        else
            vTaskDelay(100);
        #endif  //boardUSE_OS
    }
}
		
		
        //*************************************计算******************************************************************        
		
/***********************************************************************************************************************
-----函数功能    获取ADC通道原始值
-----说明(备注)  none
-----传入参数    channel: 通道索引
-----输出参数    none
-----返回值      选择通道的16位AD数据
************************************************************************************************************************/
u16 usAdc_GetChannelValue(u8 channel)	
{
	if(channel >= ADC_CHANNEL_NUM) return 0;
	
	return adc_value[channel];
}

#if(boardLOW_POWER)
/*****************************************************************************************************************
-----函数功能    进入低功耗
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true:执行成功   false:执行失败
*****************************************************************************************************************/
bool bAdc_EnterLowPower(void)
{
	vTaskSuspend(tAdcTaskHandler);  //先挂起任务
	vAdc_IoEnterLowPower();
	v_power_select(false); 
	return true;
}


/*****************************************************************************************************************
-----函数功能    退出低功耗
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      true:执行成功   false:执行失败
*****************************************************************************************************************/
bool bAdc_ExitLowPower(void)
{
	vAdc_Init();
	vTaskResume(tAdcTaskHandler);  //初始化外设后再恢复任务
	return true;
}
#endif  //boardLOW_POWER

#endif  //boardADC_EN

