/*****************************************************************************************************************
*                                                                                                                *
 *                                         显示工程模式                                                          *
*                                                                                                                *
******************************************************************************************************************/
#include "MD_Display/md_display_eng_mode.h"

#if(boardENG_MODE_EN && boardDISPLAY_EN)
#include "Sys/sys_task.h"
#include "Sys/sys_queue_task_eng.h"
#include "Print/print_task.h"
#include "MD_Display/md_display_task.h"
#include "MD_Display/md_display_api.h"
#include "MD_Display/md_display_iface.h"
#include "MD_Display/md_display_eng_mode.h"


#include "app_info.h"

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif

#if(boardBUZ_EN)
#include "Buz/buz_task.h"
#endif

#if(boardUSB_EN)
#include "Usb/usb_task.h"
#endif

#if(boardDC_EN)
#include "Dc/dc_task.h"
#endif

#if(boardHEAT_MANAGE_EN)
#include "MD_HeatManage/md_hm_task.h"
#endif  //boardHEAT_MANAGE_EN

#if(boardDCAC_EN)
#include "MD_Dcac/md_dcac_rec_task.h"
#include "MD_Dcac/md_dcac_task.h"
#endif

#if(boardBMS_EN)
#include "MD_Bms/md_bms_rec_task.h"
#include "MD_Bms/md_bms_task.h"
#endif

#if(boardMPPT_EN)
#include "MD_Mppt/md_mppt_rec_task.h"
#include "MD_Mppt/md_mppt_task.h"
#endif

#if(boardLIGHT_EN)
#include "MD_Light/md_light_task.h"
#endif


//****************************************************参数初始化**************************************************//
#if(boardDISPLAY_EN)
static vu8 S_ucHighLightTemp = 0;
static vu8 S_ucLowLightTemp = 0;
static vu16 S_usBLOffTimeTemp = 0;
#endif

//****************************************************函数声明****************************************************//
//显示工程模式模块和项目编号
void Display_EngModeObj(u16 obj, u8 index);
//显示工程模式等待时间和时钟图标
void Display_Time1(uint16_t min);

/***********************************************************************************************************************
-----函数功能    工程模式显示函数
-----说明(备注)  根据工程模式步骤和项目索引显示对应数值及图标
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_EnginModeDis(void)
{
	s16 temp = 0;
	
	//tpSysTask->ucStep选择工程模块, ucEngModeItem选择模块内参数
	switch(tpSysTask->ucStep)
	{
		case EMS_INIT:
		{
			Display_ShowAll();
		}break;
		
		case EMS_SYS:
		{
			if(tEngMode.ucEngModeItem == 0)  //显示版本
			{
				temp = (tAppMemParam.tVerInfo.saVersion[10] - 0x30) * 1000 +
						(tAppMemParam.tVerInfo.saVersion[12] - 0x30) * 100 +
						(tAppMemParam.tVerInfo.saVersion[14] - 0x30) * 10 +
						(tAppMemParam.tVerInfo.saVersion[16] - 0x30);
			}
			else if(tEngMode.ucEngModeItem == 1)  //控制风扇
				temp = tEngMode.cEngModeState;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tSYS.usAutoOffTime;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tSYS.sMaxTemp;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tSYS.sMinTemp;
			else if(tEngMode.ucEngModeItem == 5)
				temp = tAppMemParam.tSYS.usMinOpenVolt;
			else if(tEngMode.ucEngModeItem == 6)
			{
				#if(boardBUZ_EN)
				Display_IconBuz();
				if(tAppMemParam.tSYS.bBuzSwitchOff == 0)
					temp = 1;
				else 
					temp = 0;
				#endif
			}
			
		}break;
		
		#if(boardDISPLAY_EN)
		case EMS_LCD:
		{
			if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tDISP.ucHighLightValue;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tDISP.ucLowLightValue;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tDISP.usAutoOffTime;
		}break;
		#endif
		
		#if(boardBMS_EN)
		case EMS_BAT:
		{
			Display_Soc(100);
			Display_BAT(0,100);
			
			if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tBMS.cChgMaxTemp;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tBMS.cDisChgMaxTemp;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tBMS.cChgMinTemp;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tBMS.cDisChgMinTemp;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tBMS.usMaxVolt;
			else if(tEngMode.ucEngModeItem == 5)
				temp = tAppMemParam.tBMS.usMinVolt;
			
		}break;
		#endif
		
		#if(boardMPPT_EN)
		case EMS_MPPT:
		{
			//MPPT状态显示
			Display_IconDcIn();
			
			if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tMPPT.cAllowMaxTemp;
			else if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tMPPT.usAutoOffTime;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tMPPT.usMaxInVolt;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tMPPT.usMinInVolt;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tMPPT.usInPwrRating;
		}break;
		#endif
		
		#if(boardDCAC_EN)
		case EMS_DCAC:
		{
			//DCAC状态显示
			Display_IconAcIn();
			
			if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tDCAC.usAutoOffTime;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tDCAC.usMinOpenVolt;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tDCAC.usVoltRating;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tDCAC.usMaxInVolt;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tDCAC.usMinInVolt;
			else if(tEngMode.ucEngModeItem == 5)
				temp = tAppMemParam.tDCAC.usInPwrRating;
			else if(tEngMode.ucEngModeItem == 6)
				temp = tAppMemParam.tDCAC.usMinInPwr;
			else if(tEngMode.ucEngModeItem == 7)
				temp = tAppMemParam.tDCAC.usMaxInCurr;
			else if(tEngMode.ucEngModeItem == 8)
				temp = tAppMemParam.tDCAC.usOutPwrRating;
			else if(tEngMode.ucEngModeItem == 9)
				temp = tAppMemParam.tDCAC.usOverLoadPwr;
			else if(tEngMode.ucEngModeItem == 10)
				temp = tAppMemParam.tDCAC.usParaInPwr;
			else if(tEngMode.ucEngModeItem == 11)
				temp = tAppMemParam.tDCAC.usAcOutFreq;
			else if(tEngMode.ucEngModeItem == 12)
				temp = tAppMemParam.tDCAC.sMaxTemp;
		}break;
		#endif
		
		#if(boardADC_EN)
		case EMS_ADC:
		{
			if(tEngMode.ucEngModeItem == 0)
				temp = tAdcSamp.s12VTemp;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAdcSamp.s5VTemp;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAdcSamp.usVinVolt;
		}break;
		#endif
		
		#if(boardUSB_EN)
		case EMS_USB:
		{
			//USB状态显示
			Display_IconUsbOut();
			
			if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tUSB.usAutoOffTime;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tUSB.usMaxInVolt;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tUSB.usMinInVolt;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tUSB.usMinOpenVolt;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tUSB.sMaxTemp;
		}break;
		#endif
		
		#if(boardDC_EN)
		case EMS_DC:
		{
			//DC状态显示
			Display_IconDcOut();
			
			if(tEngMode.ucEngModeItem == 0)
				temp = tAppMemParam.tDC.usAutoOffTime;
			else if(tEngMode.ucEngModeItem == 1)
				temp = tAppMemParam.tDC.usMaxOutVolt;
			else if(tEngMode.ucEngModeItem == 2)
				temp = tAppMemParam.tDC.usMinOutVolt;
			else if(tEngMode.ucEngModeItem == 3)
				temp = tAppMemParam.tDC.usOverLoadPwr;
			else if(tEngMode.ucEngModeItem == 4)
				temp = tAppMemParam.tDC.usMinOpenVolt;
			else if(tEngMode.ucEngModeItem == 5)
				temp = tAppMemParam.tDC.sMaxTemp;
		}break;
		#endif
		
		#if(boardLIGHT_EN)
		case EMS_LIGHT:
		{
			//照明状态显示
			Display_IconLight();
		}break;
		#endif
		
		case EMS_SET:
		{
			if(tEngMode.ucEngModeItem == 0)  //保存
				Display_IconSave();
			else if(tEngMode.ucEngModeItem == 1)  //重置
				Display_IconSysErr();
			else if(tEngMode.ucEngModeItem == 2)  //升级
				Display_IconUpdata();
		}break;

		default:
		case EMS_FINISH:
		{
			
		}break;
	}
	
	Display_Time1(tpSysTask->usTaskWaitCnt/10);
	
	//散热开启
	if(eFan_GetWorkMode() > FWM_OFF)
		Display_IconFan();
	
	Display_EngModeObj(tpSysTask->ucStep,tEngMode.ucEngModeItem);
	Display_OutNum(temp);
	
	Display_RefreshData();
}


/***********************************************************************************************************************
-----函数功能    显示工程模式对象
-----说明(备注)  根据工程模式步骤显示对应对象名称和当前项目索引
-----传入参数    obj:工程模式步骤  index:项目索引
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void Display_EngModeObj(u16 obj, u8 index)
{
	switch(obj)
	{
		case EMS_INIT:
		{
			
		}break;
		
		case EMS_SYS:
		{
			DisplayNum(0,5);//0
			DisplayNum(1,27);//0
			DisplayNum(2,5);//0
			DisplayNum(3,index);//0
		}break;
		
	#if(boardDISPLAY_EN)
		case EMS_LCD:
		{
			DisplayNum(0,11);//0
			DisplayNum(1,20);//0
			DisplayNum(2,15);//0
			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardBMS_EN)
		case EMS_BAT:
		{
			DisplayNum(0,13);//0
			DisplayNum(1,12);//0
			DisplayNum(2,22);//0
			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardMPPT_EN)
		case EMS_MPPT:
		{
			DisplayNum(0,19);//0
			DisplayNum(1,29);//0

			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardDCAC_EN)
		case EMS_DCAC:
		{
			DisplayNum(0,12);//0
			DisplayNum(1,20);//0
			
			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardADC_EN)
		case EMS_ADC:
		{
			DisplayNum(0,12);//0
			DisplayNum(1,15);//0
			DisplayNum(2,14);//0
			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardUSB_EN)
		case EMS_USB:
		{
			DisplayNum(0,29);//0
			DisplayNum(1,5);//0
			DisplayNum(2,13);//0
			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardDC_EN)
		case EMS_DC:
		{
			DisplayNum(0,15);//0
			DisplayNum(1,14);//0

			DisplayNum(3,index);//0
		}break;
	#endif
		
	#if(boardLIGHT_EN)
		case EMS_LIGHT:  //led
		{
			DisplayNum(0,11);//0
			DisplayNum(1,16);//0
			DisplayNum(2,15);//0
			DisplayNum(3,index);//0
		}break;
	#endif

		case EMS_SET:
		{
			DisplayNum(0,5);//0
			DisplayNum(1,16);//0
			DisplayNum(2,22);//0
			DisplayNum(3,index);//0
		}
		break;
		
		default:
		case EMS_FINISH:
		{
			DisplayNum(1,1);//0
			DisplayNum(2,0);//0
			DisplayNum(3,0);//0
		}break;
	}

}


/***********************************************************************************************************************
-----函数功能    显示倒计时时间
-----说明(备注)  在工程模式页面显示任务等待倒计时
-----传入参数    min:分钟显示值
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void Display_Time1(uint16_t min)
{
	uint8_t ten_H,units_H;
	
	ten_H     = min/10;
	units_H   = min%10;	
		 
	DisplayNum(9,ten_H);
	DisplayNum(10,units_H);
			 
	Display_IconClock();//时钟标识
	 
} 


/***********************************************************************************************************************
-----函数功能    设置亮度编辑缓存
-----说明(备注)  进入亮度设置前暂存当前亮度和息屏时间参数
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void bDisp_SetLightness(void)
{
	S_ucHighLightTemp = tAppMemParam.tDISP.ucHighLightValue;
	S_ucLowLightTemp = tAppMemParam.tDISP.ucLowLightValue;
	S_usBLOffTimeTemp = tAppMemParam.tDISP.usAutoOffTime;
}

/***********************************************************************************************************************
-----函数功能    选择设置类型
-----说明(备注)  在高亮亮度、低亮亮度和息屏时间之间循环选择
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_TypeSelect(void)
{
	tDisp.eLightSetType++;
	if(tDisp.eLightSetType == LTS_NULL)
		tDisp.eLightSetType = LTS_HIGH;
}

/***********************************************************************************************************************
-----函数功能    设置记忆参数
-----说明(备注)  根据当前工程模式项目增减显示模块记忆参数
-----传入参数    add:true增加 false减少
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_MemParamSet(bool add)
{
	if(add == true)
	{
		if(tEngMode.ucEngModeItem == 0)
		{
			if(tAppMemParam.tDISP.ucHighLightValue < 0x8F)
				tAppMemParam.tDISP.ucHighLightValue++;
		}
		else if(tEngMode.ucEngModeItem == 1)
		{
			if(tAppMemParam.tDISP.ucLowLightValue < 0x8F)
				tAppMemParam.tDISP.ucLowLightValue++;
		}
		else if(tEngMode.ucEngModeItem == 2)
		{
			tAppMemParam.tDISP.usAutoOffTime++;
		}
		
	}
	else 
	{
		if(tEngMode.ucEngModeItem == 0)
		{
			if(tAppMemParam.tDISP.ucHighLightValue > 0x88)
				tAppMemParam.tDISP.ucHighLightValue--;
		}
		else if(tEngMode.ucEngModeItem == 1)
		{
			if(tAppMemParam.tDISP.ucLowLightValue > 0x88)
				tAppMemParam.tDISP.ucLowLightValue--;
		}
		else if(tEngMode.ucEngModeItem == 2)
		{
			tAppMemParam.tDISP.usAutoOffTime--;
		}
	}
}

/***********************************************************************************************************************
-----函数功能    退出亮度设置
-----说明(备注)  退出时恢复进入编辑前暂存的亮度和息屏参数
-----传入参数    none
-----输出参数    none
-----返回值      true:退出成功 false:退出失败
************************************************************************************************************************/
bool bDisp_ExitSetLightness(void)
{
	tAppMemParam.tDISP.ucHighLightValue = S_ucHighLightTemp;
	tAppMemParam.tDISP.ucLowLightValue = S_ucLowLightTemp;
	tAppMemParam.tDISP.usAutoOffTime = S_usBLOffTimeTemp;
//	bApp_MemParamUpdata(NULL,NULL,false);  //写入APP_INFO_FLASH
	return true;
}
#endif  //boardDISPLAY_EN  && boardENG_MODE_EN
