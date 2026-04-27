#ifndef MD_LIGHT_IFACE_H_
#define MD_LIGHT_IFACE_H_
#include "board_config.h"

#if(boardLIGHT_EN)

#define 		lightPWM_MAX_VALUE                   	1000
#define 		lightPWM_PSC                         	32

// TIMER3 通道（PB6/PB7/PB8/PB9，无重映射，默认引脚）
#define 		lightTIMER                           	TIMER3
#define 		lightTIMER_RCU                       	RCU_TIMER3

// 暖白光  PB6 = TIMER3_CH0
#define 		lightW_GPIO_RCU                      	RCU_GPIOB
#define 		lightW_GPIO_PORT                     	GPIOB
#define 		lightW_PIN                           	GPIO_PIN_6
#define 		lightW_TIMER_CH                      	TIMER_CH_0
#define 		lightW_PWM_SET(x)                    	TIMER_CH0CV(lightTIMER) = (uint32_t)(x)

// 蓝光  PB7 = TIMER3_CH1
#define 		lightB_GPIO_RCU                      	RCU_GPIOB
#define 		lightB_GPIO_PORT                     	GPIOB
#define 		lightB_PIN                           	GPIO_PIN_7
#define 		lightB_TIMER_CH                      	TIMER_CH_1
#define 		lightB_PWM_SET(x)                    	TIMER_CH1CV(lightTIMER) = (uint32_t)(x)

// 绿光  PB8 = TIMER3_CH2
#define 		lightG_GPIO_RCU                      	RCU_GPIOB
#define 		lightG_GPIO_PORT                     	GPIOB
#define 		lightG_PIN                           	GPIO_PIN_8
#define 		lightG_TIMER_CH                      	TIMER_CH_2
#define 		lightG_PWM_SET(x)                    	TIMER_CH2CV(lightTIMER) = (uint32_t)(x)

// 红光  PB9 = TIMER3_CH3
#define 		lightR_GPIO_RCU                      	RCU_GPIOB
#define 		lightR_GPIO_PORT                     	GPIOB
#define 		lightR_PIN                           	GPIO_PIN_9
#define 		lightR_TIMER_CH                      	TIMER_CH_3
#define 		lightR_PWM_SET(x)                    	TIMER_CH3CV(lightTIMER) = (uint32_t)(x)

void vLight_IfaceInit(void);

#if(boardLOW_POWER)
void vLight_IoEnterLowPower(void);
#endif  //boardLOW_POWER

#endif  //boardLIGHT_EN

#endif  //MD_LIGHT_IFACE_H_
