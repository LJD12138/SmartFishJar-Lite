/*****************************************************************************************************************
*                                                                                                                *
 *                                         按键功能                                                             *
*                                                                                                                *
******************************************************************************************************************/
#include "Key/key_func.h"

#if(boardKEY_EN)
#include "Key/key_task.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"
#include "..\..\BOOT\Application\flash_allot_table.h"

#include "function.h"

#if(boardUSB_EN)
#include "Usb/usb_task.h"
#endif  //boardUSB_EN

#if(boardDC_EN)
#include "Dc/dc_task.h"
#endif  //boardDC_EN

#if(boardLIGHT_EN)
#include "MD_Light/md_light_task.h"
#endif  //boardLIGHT_EN

#if(boardDISPLAY_EN)
#include "MD_Display/md_display_task.h"
#endif  //boardDISPLAY_EN

#if(boardBUZ_EN)
#include "Buz/buz_task.h"
#endif  //boardBUZ_EN

#if(boardDCAC_EN)
#include "MD_Dcac/md_dcac_task.h"
#endif  //boardDCAC_EN

#if(boardENG_MODE_EN)
#include "key_func_eng.h"
#endif  //boardENG_MODE_EN

//****************************************************参数初始化**************************************************//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////按键功能数组要求:不满十个触发类型的要加KTE_FUN_NULL作为结束符///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//长按	开关机
u8 const KeyTriType_SysOnOffBuff[ 2 ] = { KTE_ENTER_LONG, KTE_FUN_NULL};
//点按	开关机
u8 const KeyTriType_SysOnOffBuff1[ 2 ] = { KTE_ENTER_LONG, KTE_FUN_NULL};
//连击	开关机
u8 const KeyTriType_SysProteOnOffBuff[ 4 ] = { KTE_ENTER_LONG, KTE_ENTER_LONG, KTE_ENTER_LONG,KTE_FUN_NULL};
											
#if(boardDISPLAY_EN)
//单击 	开关背光
u8 const KeyTriType_BLOnOffBuff[ 2 ] = { KTE_ENTER_SHORT, KTE_FUN_NULL};
//组合 	强制开关背光
u8 const KeyTriType_ForceOpenBLBuff1[ 3 ] = { KTE_ENTER_LONG, KTE_ENTER_LONG,KTE_FUN_NULL};
u8 const KeyTriType_ForceOpenBLBuff2[ 3 ] = { KTE_ENTER_LONG,KTE_ENTER_LONG ,KTE_FUN_NULL};
#endif  //boardDISPLAY_EN



/***********************************************************************************************************************
-----函数功能    按键功能处理函数
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vKey_ProcKeyFunc(u8* pKeyTriTypeBuff)
{
	//******************************************开关机************************************************
	if( bFun_DataCompare( pKeyTriTypeBuff, (u8*)&KeyTriType_SysOnOffBuff, sizeof(KeyTriType_SysOnOffBuff)))  
	{
		cSys_Switch(SO_KEY, ST_NULL, false);
		
		if(uPrint.tFlag.bKeyTask)
			sMyPrint("Key_Task:开关机 \r\n");
	}
	#if(boardENG_MODE_EN)
	//******************************************工程模式************************************************************
	else if(tSysInfo.eDevState == DS_ENG_MODE)
	{
		v_key_func_eng(pKeyTriTypeBuff);
	}
	#endif
	//*****************************************************工作状态下**************************************************//
	else if ( tSysInfo.eDevState == DS_WORK || tSysInfo.eDevState == DS_ERR)
	{
		if(bFun_DataCompare(pKeyTriTypeBuff, (u8*)&KeyTriType_SysProteOnOffBuff, sizeof(KeyTriType_SysProteOnOffBuff))) //连击Power按键
		{
			bSys_SetPerm(SPO_FORCE_CLOSE, true);

			#if(boardKEY_EN)
			bBuz_Tweet(LONG_1);
			#endif  //boardKEY_EN

			cSys_Switch(SO_KEY, ST_OFF, false);
			if(uPrint.tFlag.bKeyTask)
				sMyPrint("Key_Task:系统充放保护\r\n");
		}
		#if(boardDISPLAY_EN)
		else if(bFun_DataCompare( pKeyTriTypeBuff, (u8*)&KeyTriType_BLOnOffBuff, sizeof(KeyTriType_BLOnOffBuff))) //单击POWER按键　
		{
			bDisp_Switch(ST_NULL, true);
			
			if(uPrint.tFlag.bKeyTask)
				sMyPrint("Key_Task:开关背光\r\n");
		}
		else if(bFun_DataCompare(pKeyTriTypeBuff, (u8*)&KeyTriType_ForceOpenBLBuff1, sizeof(KeyTriType_ForceOpenBLBuff1)) ||
			    bFun_DataCompare(pKeyTriTypeBuff, (u8*)&KeyTriType_ForceOpenBLBuff2, sizeof(KeyTriType_ForceOpenBLBuff2))
		) //连击Power按键
		{
			bDisp_Switch(ST_ON, true);
			
			if(uPrint.tFlag.bKeyTask)
				sMyPrint("Key_Task:强制开启背光\r\n");
		}
		#endif  //boardDISPLAY_EN
	}
	vKey_ParamInit();
}
#endif  //boardKEY_EN

