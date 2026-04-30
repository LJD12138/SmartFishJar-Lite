#ifndef LED_IFACE_H_
#define LED_IFACE_H_

#include "board_config.h"

#if(boardLED_EN)
#define 		ledTIMER                           		TIMER1
#define 		ledTIMER_RCU                       		RCU_TIMER1
#define 		ledTIMER_CH                        		TIMER_CH_3

#define 		ledPWR_SW_PWM_SET(x)                  	TIMER_CH3CV(ledTIMER) = ((uint32_t)x)
#define 		ledPWM_MAX_VALUE     					999
#define 		ledPWM_PSC           					60

#define     	ledPWR_SW_RCU      						RCU_GPIOB
#define     	ledPWR_SW_PORT     						GPIOB
#define     	ledPWR_SW_PIN      						GPIO_PIN_11
#define     	ledPWR_SW_ON()     						ledPWR_SW_PWM_SET(ledPWM_MAX_VALUE)
#define     	ledPWR_SW_OFF()    						ledPWR_SW_PWM_SET(0)
//#define     	ledPWR_SW_ON()     						GPIO_BOP(ledPWR_SW_PORT) = ledPWR_SW_PIN
//#define     	ledPWR_SW_OFF()    						GPIO_BC(ledPWR_SW_PORT)  = ledPWR_SW_PIN

void vLed_IfaceInit(void);

void vLed_IfaceDeInit(void);
	
#if(boardLOW_POWER)
void vLed_IoEnterLowPower(void);
#endif  //boardLOW_POWER

#endif  //boardLED_EN
#endif  //LED_IFACE_H_
