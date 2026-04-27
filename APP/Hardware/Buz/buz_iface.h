#ifndef BUZ_IFACE_H
#define BUZ_IFACE_H

#include "main.h"
#include "board_config.h"

#define    		buzPWM_GPIO_RCU     					RCU_GPIOB
#define    		buzPWM_GPIO_PORT    					GPIOB
#define    		buzPWM_GPIO_PIN     					GPIO_PIN_10

#define    		buzTIMER         						TIMER1
#define    		buzTIMER_RCU    						RCU_TIMER1
#define    		buzTIMER_CH     						TIMER_CH_2
#define    		buzTIMER_PWM_SET(x)    					TIMER_CH2CV(buzTIMER) = ((uint32_t)x)
// TIMER1 ²¿·ÖÖØÓ³Éä: PB10 -> TIMER1_CH2
#define    		buzTIMER_REMAP_EN   					1

void vBuz_Init(void);

#if(boardLOW_POWER)
void vBuz_IoEnterLowPower(void);
#endif

#endif  //BUZ_IFACE_H
