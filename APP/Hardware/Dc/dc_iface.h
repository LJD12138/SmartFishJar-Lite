#ifndef DC_IFACE_H_
#define DC_IFACE_H_

#include "board_config.h"

#if(boardDC_EN)

#define     	dcPOWER_EN_RCU          					RCU_GPIOB
#define     	dcPOWER_EN_PORT         					GPIOB
#define     	dcPOWER_EN_PIN          					GPIO_PIN_6
#define     	dcPOWER_EN_ON()         					GPIO_BOP(dcPOWER_EN_PORT) = (uint32_t)dcPOWER_EN_PIN
#define     	dcPOWER_EN_OFF()        					GPIO_BC(dcPOWER_EN_PORT) = (uint32_t)dcPOWER_EN_PIN

void vDc_IfaceInit(void);

#endif  //boardDC_EN

#endif  //DC_IFACE_H_

