#ifndef ADC_TASK_H
#define ADC_TASK_H

#include "board_config.h"

#if(boardADC_EN)
#include "Adc/adc_iface.h"

// VIN电压分压比 R13(68K)/R14(12K): Vout = Vadc * (R13+R14)/R14 * 0.1V
#define     	adcVIN_VOLT_R1                      	68.0f  //(Kohm) 分压上电阻
#define     	adcVIN_VOLT_R2                      	12.0f  //(Kohm) 分压下电阻
#define     	adcVIN_VOLT_RES_RATIO               	((((3.3f / 4095.0f) * (adcVIN_VOLT_R1 + adcVIN_VOLT_R2)) / adcVIN_VOLT_R2) * 10.0f)

// 12V电压分压比 R37/R38（参考原理图，暂用47K/5.1K）
#define     	adc12V_VOLT_R1                      	47.0f  //(Kohm)
#define     	adc12V_VOLT_R2                      	5.1f   //(Kohm)
#define     	adc12V_VOLT_RES_RATIO               	((((3.3f / 4095.0f) * (adc12V_VOLT_R1 + adc12V_VOLT_R2)) / adc12V_VOLT_R2) * 10.0f)

// 电流换算系数 (采样电阻R84，按原理图值，暂用0.01Ohm，运放增益10: I=Vadc/10/0.01)
#define     	adcCURR_RES_RATIO                   	0.0806f  // Vadc(0-3.3V)/4095 * 3.3 / 运放增益 / 采样电阻

typedef struct
{
    vu16           		usVinVolt;        //VIN输入电压  0.1V
    float          		fVinCurr;         //VIN输入电流  A

    vu16           		us12VVolt;        //12V输出电压  0.1V
    s16            		s12VTemp;         //12V板载温度  摄氏度
    s16            		s5VTemp;          //5V板载温度   摄氏度

    float          		fHeatCurr;        //加热棒电流   A
    float          		fO2Curr;          //氧气泵电流   A
    float          		fPumpCurr;        //水泵电流     A
    float          		fLightCurr;       //灯光总电流   A

    vu16           		usLightRes;       //光照传感器ADC原始值
    s16            		sWaterTemp1;      //水温探头1    摄氏度
    s16            		sWaterTemp2;      //水温探头2    摄氏度
}AdcSamp_T;
extern AdcSamp_T 	tAdcSamp;

void vAdc_TaskInit(void);
u16 usAdc_GetChannelValue(u8 channel);

#if(!boardUSE_OS)
void vAdc_Task(void *pvParameters);
#endif  //boardUSE_OS

#if(boardLOW_POWER)
bool bAdc_EnterLowPower(void);
bool bAdc_ExitLowPower(void);
#endif  //boardLOW_POWER

#endif  //boardADC_EN

#endif  //ADC_TASK_H
