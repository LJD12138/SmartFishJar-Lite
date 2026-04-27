#ifndef PUMP_IFACE_H_
#define PUMP_IFACE_H_
#include "board_config.h"

#if(boardWATER_PUMP_EN)

#define         pumpPWM_MAX_VALUE                       1000
#define         pumpPWM_PSC                             32

// 水泵  PA8 = TIMER0_CH0（无重映射，默认引脚）
#define         pumpPWM_GPIO_RCU                        RCU_GPIOA
#define         pumpPWM_GPIO_PORT                       GPIOA
#define         pumpPWM_PIN                             GPIO_PIN_8

#define         pumpTIMER                               TIMER0
#define         pumpTIMER_RCU                           RCU_TIMER0
#define         pumpTIMER_CH                            TIMER_CH_0
#define         pumpPWM_SET(x)                          TIMER_CH0CV(pumpTIMER) = (uint32_t)(x)

void vPump_IfaceInit(void);

#if(boardLOW_POWER)
void vPump_IoEnterLowPower(void);
#endif

#endif  //boardWATER_PUMP_EN

#endif  //PUMP_IFACE_H_
