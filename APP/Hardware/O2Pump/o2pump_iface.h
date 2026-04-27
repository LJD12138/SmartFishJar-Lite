#ifndef O2PUMP_IFACE_H_
#define O2PUMP_IFACE_H_
#include "board_config.h"

#if(boardO2PUMP_EN)

#define         o2PWM_MAX_VALUE                         1000

// 氧气泵  PC7 = TIMER2_CH1（全重映射，TIMER2由md_hm_iface.c统一初始化）
// 注意：此模块仅配置通道寄存器，不重复初始化TIMER2
#define         o2PWM_SET(x)                            TIMER_CH1CV(TIMER2) = (uint32_t)(x)

#endif  //boardO2PUMP_EN

#endif  //O2PUMP_IFACE_H_
