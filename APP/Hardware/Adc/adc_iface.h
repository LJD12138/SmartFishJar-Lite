#ifndef ADC_IFACE_H
#define ADC_IFACE_H


#include "board_config.h"
#if(boardADC_EN)

#define     	ADC_DMAX              					2
#define     	ADC_CHANNEL_NUM       					12   //DMA缓存大小 (12路ADC)

// 通道索引定义（DMA采样顺序严格对应）
#define     	adcVIN_CURR           					0    //PA0  ADC_CH0  输入电流
#define     	adcVIN_VOLT           					1    //PA1  ADC_CH1  输入电压
#define     	adc5V_NTC             					2    //PA2  ADC_CH2  5V板载温度
#define     	adcWATER_NTC2         					3    //PA6  ADC_CH6  水温探头2
#define     	adcWATER_NTC1         					4    //PA7  ADC_CH7  水温探头1
#define     	adcHEAT_CURR          					5    //PB0  ADC_CH8  加热棒电流
#define     	adcO2_CURR            					6    //PB1  ADC_CH9  氧气泵电流
#define     	adcLIGHT_CURR         					7    //PC0  ADC_CH10 灯光总电流
#define     	adc12V_NTC            					8    //PC1  ADC_CH11 12V板载温度
#define     	adc12V_VOLT           					9    //PC3  ADC_CH13 12V输出电压
#define     	adcLIGHT_RES          					10   //PC4  ADC_CH14 光照传感器
#define     	adcPUMP_CURR          					11   //PC5  ADC_CH15 水泵电流

// ADC GPIO引脚定义
#define     	adcVIN_CURR_RCU       					RCU_GPIOA
#define     	adcVIN_CURR_PORT      					GPIOA
#define     	adcVIN_CURR_PIN       					GPIO_PIN_0
#define     	adcVIN_CURR_CH        					ADC_CHANNEL_0

#define     	adcVIN_VOLT_RCU       					RCU_GPIOA
#define     	adcVIN_VOLT_PORT      					GPIOA
#define     	adcVIN_VOLT_PIN       					GPIO_PIN_1
#define     	adcVIN_VOLT_CH        					ADC_CHANNEL_1

#define     	adc5V_NTC_RCU         					RCU_GPIOA
#define     	adc5V_NTC_PORT        					GPIOA
#define     	adc5V_NTC_PIN         					GPIO_PIN_2
#define     	adc5V_NTC_CH          					ADC_CHANNEL_2

#define     	adcWATER_NTC2_RCU     					RCU_GPIOA
#define     	adcWATER_NTC2_PORT    					GPIOA
#define     	adcWATER_NTC2_PIN     					GPIO_PIN_6
#define     	adcWATER_NTC2_CH      					ADC_CHANNEL_6

#define     	adcWATER_NTC1_RCU     					RCU_GPIOA
#define     	adcWATER_NTC1_PORT    					GPIOA
#define     	adcWATER_NTC1_PIN     					GPIO_PIN_7
#define     	adcWATER_NTC1_CH      					ADC_CHANNEL_7

#define     	adcHEAT_CURR_RCU      					RCU_GPIOB
#define     	adcHEAT_CURR_PORT     					GPIOB
#define     	adcHEAT_CURR_PIN      					GPIO_PIN_0
#define     	adcHEAT_CURR_CH       					ADC_CHANNEL_8

#define     	adcO2_CURR_RCU        					RCU_GPIOB
#define     	adcO2_CURR_PORT       					GPIOB
#define     	adcO2_CURR_PIN        					GPIO_PIN_1
#define     	adcO2_CURR_CH         					ADC_CHANNEL_9

#define     	adcLIGHT_CURR_RCU     					RCU_GPIOC
#define     	adcLIGHT_CURR_PORT    					GPIOC
#define     	adcLIGHT_CURR_PIN     					GPIO_PIN_0
#define     	adcLIGHT_CURR_CH      					ADC_CHANNEL_10

#define     	adc12V_NTC_RCU        					RCU_GPIOC
#define     	adc12V_NTC_PORT       					GPIOC
#define     	adc12V_NTC_PIN        					GPIO_PIN_1
#define     	adc12V_NTC_CH         					ADC_CHANNEL_11

#define     	adc12V_VOLT_RCU       					RCU_GPIOC
#define     	adc12V_VOLT_PORT      					GPIOC
#define     	adc12V_VOLT_PIN       					GPIO_PIN_3
#define     	adc12V_VOLT_CH        					ADC_CHANNEL_13

#define     	adcLIGHT_RES_RCU      					RCU_GPIOC
#define     	adcLIGHT_RES_PORT     					GPIOC
#define     	adcLIGHT_RES_PIN      					GPIO_PIN_4
#define     	adcLIGHT_RES_CH       					ADC_CHANNEL_14

#define     	adcPUMP_CURR_RCU      					RCU_GPIOC
#define     	adcPUMP_CURR_PORT     					GPIOC
#define     	adcPUMP_CURR_PIN      					GPIO_PIN_5
#define     	adcPUMP_CURR_CH       					ADC_CHANNEL_15


#if (ADC_DMAX == 1)
#define     	ADCX_RCU                    			RCU_ADC0
#define     	ADCX                        			ADC0
#define     	adcDMA_RCU                    			RCU_DMA1
#define     	adcDMA                        			DMA1
#define     	adcDMA_CH                     			DMA_CH4

#define     	DMA_SUBPERIX                			DMA_SUBPERI0
#elif (ADC_DMAX == 2)
#define     	ADCX_RCU                    			RCU_ADC0
#define     	ADCX                        			ADC0
#define     	adcDMA_RCU                    			RCU_DMA0
#define     	adcDMA                        			DMA0
#define     	adcDMA_CH                     			DMA_CH0
//#define     	DMA_SUBPERIX                			DMA_SUBPERI0
#endif

extern u16 adc_value[];

void vAdc_Init(void);
void vAdc_DeInit(void);

#if(boardLOW_POWER)
void vAdc_IoEnterLowPower(void);
#endif

#endif  //boardADC_EN

#endif  //ADC_IFACE_H

