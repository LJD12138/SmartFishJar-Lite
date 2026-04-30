/***********************************************************************************************************************
 * Project : ProjectTeam
 * Module  : G2404-3/APP/Hardware/MD_Display
 * File    : md_display_api.h
 * Date    : 2026-03-19
 * Author  : LJD(291483914@qq.com)
 * Description: MD Display API Header File - Optimized version based on M5004-3
 * -------------------------------------------------------
 * todo    :
 * 1.
 * -------------------------------------------------------
 * Copyright (c) 2026 -inc
************************************************************************************************************************/
#ifndef MD_DISPLAY_API_H
#define MD_DISPLAY_API_H


#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================includes====================================*/
#include "board_config.h"

#if(boardDISPLAY_EN)

/* ==========================================macros======================================*/
#define     OLED_CMD                        0U
#define     OLED_DATA                       1U
#define     OLED_WIDTH_PIXELS               128U
#define     OLED_HEIGHT_PIXELS              64U
#define     OLED_PAGE_COUNT                 8U
#define     OLED_SPI_TIMEOUT                0x0000FFFFUL

/* ==========================================globals=====================================*/


/* ==========================================types=======================================*/


/* ==========================================extern======================================*/

#if(dispUSE_U8G2 == 0)
void vDisp_OledDrawPixel(u8 x, u8 y, bool on);
void vDisp_OledDrawHLine(u8 x, u8 y, u8 len, bool on);
void vDisp_OledDrawVLine(u8 x, u8 y, u8 len, bool on);
void vDisp_OledDrawChar6x8(u8 x, u8 y, char c);
void vDisp_OledDrawString6x8(u8 x, u8 y, const char *str);
void vDisp_OledFillBuffer(u8 value);
#endif  // dispUSE_U8G2 == 0

void vDisp_Init(void);
void vDisp_Refresh(void);
void vDisp_SetPower(bool on);
void vDisp_ClearBuffer(void);
void vDisp_UiTest(void);

#ifdef __cplusplus
}
#endif

#endif //boardDISPLAY_EN

#endif  //MD_DISPLAY_API_H
