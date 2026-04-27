/***********************************************************************************************************************
 * OLED display API implementation for Lite hardware.
 **********************************************************************************************************************/
#include "MD_Display/md_display_api.h"

#if(boardDISPLAY_EN)

#include <string.h>
#include "MD_Display/md_display_iface.h"

#define DISP_TEST_PAGE_WIDTH          128U
#define DISP_TEST_PAGE_HEIGHT          64U
#define DISP_TEST_TEXT_X               31U
#define DISP_TEST_TEXT_Y               28U

static void v_disp_draw_test_frame(void)
{
	vDisp_OledDrawHLine(0U, 0U, DISP_TEST_PAGE_WIDTH, true);
	vDisp_OledDrawHLine(0U, (u8)(DISP_TEST_PAGE_HEIGHT - 1U), DISP_TEST_PAGE_WIDTH, true);
	vDisp_OledDrawVLine(0U, 0U, DISP_TEST_PAGE_HEIGHT, true);
	vDisp_OledDrawVLine((u8)(DISP_TEST_PAGE_WIDTH - 1U), 0U, DISP_TEST_PAGE_HEIGHT, true);
}

/***********************************************************************************************************************
-----函数功能    显示Hello World测试页
-----说明(备注)  清屏后绘制边框与测试字符串，用于验证OLED任务链路和字符显示是否正常。
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_ShowHelloWorldTestPage(void)
{
	vDisp_OledClearBuffer();
	v_disp_draw_test_frame();
	vDisp_OledDrawString6x8(DISP_TEST_TEXT_X, DISP_TEST_TEXT_Y, "HELLO WORLD");
	vDisp_OledRefresh();
}


#endif  // boardDISPLAY_EN
