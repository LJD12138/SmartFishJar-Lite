#ifndef MD_DISPLAY_IFACE_H_
#define MD_DISPLAY_IFACE_H_

#include "main.h"
#include "board_config.h"

#if(boardDISPLAY_EN)

// OLED SPI Òý½Å¶¨Òå
#define     	dispOLED_NSS_RCU              			RCU_GPIOB
#define     	dispOLED_NSS_PORT             			GPIOB
#define     	dispOLED_NSS_PIN              			GPIO_PIN_12
#define     	dispOLED_NSS_H()              			GPIO_BOP(dispOLED_NSS_PORT) = (uint32_t)dispOLED_NSS_PIN
#define     	dispOLED_NSS_L()              			GPIO_BC(dispOLED_NSS_PORT) = (uint32_t)dispOLED_NSS_PIN

#define     	dispOLED_SCK_RCU              			RCU_GPIOB
#define     	dispOLED_SCK_PORT             			GPIOB
#define     	dispOLED_SCK_PIN              			GPIO_PIN_13
#define     	dispOLED_SCK_H()              			GPIO_BOP(dispOLED_SCK_PORT) = (uint32_t)dispOLED_SCK_PIN
#define     	dispOLED_SCK_L()              			GPIO_BC(dispOLED_SCK_PORT) = (uint32_t)dispOLED_SCK_PIN

#define     	dispOLED_MOSI_RCU             			RCU_GPIOB
#define     	dispOLED_MOSI_PORT            			GPIOB
#define     	dispOLED_MOSI_PIN             			GPIO_PIN_15
#define     	dispOLED_MOSI_H()             			GPIO_BOP(dispOLED_MOSI_PORT) = (uint32_t)dispOLED_MOSI_PIN
#define     	dispOLED_MOSI_L()             			GPIO_BC(dispOLED_MOSI_PORT) = (uint32_t)dispOLED_MOSI_PIN

#define     	dispOLED_RES_RCU              			RCU_GPIOA
#define     	dispOLED_RES_PORT             			GPIOA
#define     	dispOLED_RES_PIN              			GPIO_PIN_11
#define     	dispOLED_RES_H()              			GPIO_BOP(dispOLED_RES_PORT) = (uint32_t)dispOLED_RES_PIN
#define     	dispOLED_RES_L()              			GPIO_BC(dispOLED_RES_PORT) = (uint32_t)dispOLED_RES_PIN

#define     	dispOLED_DC_RCU               			RCU_GPIOA
#define     	dispOLED_DC_PORT              			GPIOA
#define     	dispOLED_DC_PIN               			GPIO_PIN_12
#define     	dispOLED_DC_H()               			GPIO_BOP(dispOLED_DC_PORT) = (uint32_t)dispOLED_DC_PIN
#define     	dispOLED_DC_L()               			GPIO_BC(dispOLED_DC_PORT) = (uint32_t)dispOLED_DC_PIN

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_HARD_DMA)
#define     	dispOLED_SPI_PERIPH           			SPI2
#define     	dispOLED_SPI_RCU              			RCU_SPI2
#define     	dispOLED_SPI_DMA_PERIPH       			DMA1
#define     	dispOLED_SPI_DMA_RCU          			RCU_DMA1
#define     	dispOLED_SPI_DMA_TX_CH        			DMA_CH1
#define     	dispOLED_SPI_PRESCALE         			SPI_PSC_16
#endif

void vDisp_OledIfaceInit(void);
void vDisp_OledReInit(void);
void vDisp_OledSetPower(bool on);

void vDisp_OledClearBuffer(void);
void vDisp_OledFillBuffer(u8 value);
void vDisp_OledRefresh(void);

void vDisp_OledDrawPixel(u8 x, u8 y, bool on);
void vDisp_OledDrawHLine(u8 x, u8 y, u8 len, bool on);
void vDisp_OledDrawVLine(u8 x, u8 y, u8 len, bool on);
void vDisp_OledDrawChar6x8(u8 x, u8 y, char c);
void vDisp_OledDrawString6x8(u8 x, u8 y, const char *str);

#endif  // boardDISPLAY_EN

#endif  // MD_DISPLAY_IFACE_H_
