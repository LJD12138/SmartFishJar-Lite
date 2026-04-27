#ifndef MD_HM_IFACE_H_
#define MD_HM_IFACE_H_
#include "main.h"
#include "board_config.h"

#define 		hmPWM_MAX_VALUE      					1000
#define 		hmPWM_PSC            					32

// TIMER2 全重映射（PC6/PC7/PC8/PC9）
#define 		hmTIMER                            		TIMER2
#define 		hmTIMER_RCU                        		RCU_TIMER2

// 加热棒 PC6 = TIMER2_CH0（全重映射）
#define 		heatPWM_GPIO_RCU                    	RCU_GPIOC
#define 		heatPWM_GPIO_PORT                   	GPIOC
#define 		heatPWM_PIN                         	GPIO_PIN_6
#define 		heatTIMER_CH                        	TIMER_CH_0
#define 		heatPWM_SET(x)                      	TIMER_CH0CV(hmTIMER) = (uint32_t)(x)

// 风扇1 PC9 = TIMER2_CH3（全重映射）
#define 		fanPWM1_GPIO_RCU                    	RCU_GPIOC
#define 		fanPWM1_GPIO_PORT                   	GPIOC
#define 		fanPWM1_PIN                         	GPIO_PIN_9
#define 		fanTIMER_CH1                        	TIMER_CH_3
#define 		fanPWM1_SET(x)                      	TIMER_CH3CV(hmTIMER) = (uint32_t)(x)

// 风扇2 PC8 = TIMER2_CH2（全重映射）
#if(boardFAN2_EN)
#define 		fanPWM2_GPIO_RCU                    	RCU_GPIOC
#define 		fanPWM2_GPIO_PORT                   	GPIOC
#define 		fanPWM2_PIN                         	GPIO_PIN_8
#define 		fanTIMER_CH2                        	TIMER_CH_2
#define 		fanPWM2_SET(x)                      	TIMER_CH2CV(hmTIMER) = (uint32_t)(x)
#endif  //boardFAN2_EN

void vFan_PwmInit(void);

#if(boardLOW_POWER)
void vFan_IoEnterLowPower(void);
#endif

#endif  //MD_HM_IFACE_H_
