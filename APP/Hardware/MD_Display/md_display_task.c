/*****************************************************************************************************************
*                                                                                                                *
 *                                         Disp显示任务                                                          *
*                                                                                                                *
******************************************************************************************************************/
#include "MD_Display/md_display_task.h"

#if(boardDISPLAY_EN)
#include "MD_Display/md_display_api.h"
#include "MD_Display/md_display_iface.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"

#include "app_info.h"


#if(boardKEY_EN)
#include "Key/key_task.h"
#endif  //boardKEY_EN

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

#if(boardBMS_EN)
#include "MD_Bms/md_bms_rec_task.h"
#include "MD_Bms/md_bms_task.h"
#endif  //boardBMS_EN

#if(boardMPPT_EN)
#include "MD_Mppt/md_mppt_rec_task.h"
#include "MD_Mppt/md_mppt_task.h"
#endif  //boardMPPT_EN

#if(boardDCAC_EN)
#include "MD_Dcac/md_dcac_task.h"
#include "MD_Dcac/md_dcac_rec_task.h"
#endif  //DCAC使能

#if(boardUPDATA)
#include "Sys/sys_queue_task_updata.h"
#endif  //boardUPDATA

#if(boardHEAT_MANAGE_EN)
#include "MD_HeatManage/md_hm_task.h"
#endif  //boardHEAT_MANAGE_EN


//****************************************************任务参数初始化**********************************************//
#if(boardUSE_OS)
#define			dispTASK_PRIO                   2       //任务优先级 
#define			dispTASK_STK_SIZE               256     //任务堆栈  实际字节数 *4
TaskHandle_t	tDispTaskHandler = NULL; 
void vDisp_Task(void *pvParameters);
#endif  //boardUSE_OS

//****************************************************参数初始化**************************************************//
Disp_T   tDisp; 
static bool S_bDispPageDirty = true;

//****************************************************局部函数定义************************************************//
static void v_disp_init(void);
static void v_disp_closing(void);
static void v_disp_shut_down(void);
static void v_disp_booting(void);
static void v_disp_work(void);

/***********************************************************************************************************************
-----函数功能    参数初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
static void v_disp_param_init(void)
{
	memset(&tDisp, 0, sizeof(tDisp));
	
	tDisp.usAutoOffTime = boardDISP_OFF_TIME;
	tDisp.bSleepShow =true;//待机强制打开亮屏
}

/***********************************************************************************************************************
-----函数功能    Disp显示任务初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
bool bDisp_TaskInit(void)
{
    v_disp_param_init();

	vDisp_OledIfaceInit(); // 初始化OLED硬件接口

    #if(boardUSE_OS)
    xTaskCreate((TaskFunction_t )vDisp_Task,            // 任务函数
                (const char* )"DispTask",             // 任务名称
                (u16 ) dispTASK_STK_SIZE,             // 任务堆栈大小
                (void* )NULL,                         // 传递给任务函数的参数
                (UBaseType_t ) dispTASK_PRIO,         // 任务优先级
                (TaskHandle_t*)&tDispTaskHandler);    // 任务句柄
    #endif  //boardUSE_OS

    return true;
}

/***********************************************************************************************************************
-----函数功能    tDisp显示任务
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_Task(void *pvParameters)
{
	static bool flag = false;
	static DevState_E next_state;
	
	#if(boardUSE_OS)
	for(;;)
	#endif  //boardUSE_OS
	{
		if(next_state != tSysInfo.eDevState)
		{
			flag = false;
			S_bDispPageDirty = true;
			next_state = tSysInfo.eDevState;
		}
		
		switch(tSysInfo.eDevState)
		{
			//--------------------------初始化----------------------------------
			case DS_INIT:
			{	
				if(flag ==false)
				{
					flag = true;
					v_disp_init();
				}
			}
			break;
			
			//---------------------------关闭中---------------------------------
			case DS_CLOSING:
			{
//				if(flag ==false)
				{
					flag = true;
					v_disp_closing();
				}
			}
			break;
			
			//---------------------------关闭----------------------------------
			case DS_SHUT_DOWN:
			{
				if(bKey_PowerIsPress() == false)
					v_disp_shut_down();
			}
			break;
			//----------------------------装载中---------------------------------
			case DS_BOOTING:
			{
				v_disp_booting();
			}
			break;
			
			//----------------------------工作中---------------------------------
			//---------------------------错误------------------------------------
			case DS_ERR: 
			case DS_WORK:
			{
				if(flag ==false)
				{
					flag = true;
					vDisp_OledReInit();
				}
				v_disp_work();
			}
			break;
			
			//----------------------------升级模式---------------------------------
			case DS_UPDATA_MODE:
			{
				#if(boardUPDATA)
				bDisp_Switch(ST_ON, true);         
				#endif  //boardUPDATA     
			}
			break;
			
			#if(boardENG_MODE_EN)
			//----------------------------工程模式---------------------------------
			case DS_ENG_MODE:
			{
				bDisp_Switch(ST_ON, true);
				
				vDisp_EnginModeDis();
				vTaskDelay(200);
			}
			break;
			#endif	
				
			default:
				vTaskDelay(boardDISP_REFRESH_TMIE);
				break;
		}
		
		#if(boardUSE_OS)
		vTaskDelay(boardDISP_REFRESH_TMIE);
		#endif  //boardUSE_OS
	}
}

/***********************************************************************************************************************
-----函数功能    初始化显示
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
__STATIC_INLINE void v_disp_init(void)
{
	bDisp_Switch(ST_OFF, false);
}


/***********************************************************************************************************************
-----函数功能    关闭中显示
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
__STATIC_INLINE void v_disp_closing(void)
{
	bDisp_Switch(ST_ON, false);
}

/***********************************************************************************************************************
-----函数功能    关闭显示
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
__STATIC_INLINE void v_disp_shut_down(void)
{
	bDisp_Switch(ST_OFF, false);
}

/***********************************************************************************************************************
-----函数功能    开启显示
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
__STATIC_INLINE void v_disp_booting(void)
{
	bDisp_Switch(ST_ON, false);
	if(S_bDispPageDirty)
	{
		vDisp_ShowHelloWorldTestPage();
		S_bDispPageDirty = false;
	}
}

/***********************************************************************************************************************
-----函数功能    LCD工作显示函数
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
__STATIC_INLINE void v_disp_work(void)
{
	//息屏
	if(tDisp.bLight == false) 
	{
		S_bDispPageDirty = true;
		return;
	}
	
	if(S_bDispPageDirty)
	{
		vDisp_ShowHelloWorldTestPage();
		S_bDispPageDirty = false;
	}
}


/***********************************************************************************************************************
-----函数功能    显示开关
-----说明(备注)  none
-----传入参数    type:类型   fore_en:强制打开
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
bool bDisp_Switch(SwitchType_E type, bool fore_en)
{
	switch(type)
	{
		case ST_ON:
			goto LoopOn;
		
		case ST_OFF:
			goto LoopOff;
		
		default:
		{
			if(tDisp.bLight == false)
			{
				LoopOn:
				if(tDisp.bLight == false)
				{
					vDisp_OledSetPower(true);
					S_bDispPageDirty = true;
				}
				tDisp.bLight = true;
				
				//关闭息屏
				if(fore_en == true)
					tDisp.usAutoOffTime = 0;
				
				//更新显示时间
				if(tDisp.usAutoOffTime)
					tDisp.usAutoOffCnt =  tDisp.usAutoOffTime;
			}
			else 
			{
				LoopOff:
				if(tDisp.bLight)
				{
					vDisp_OledClearBuffer();
					vDisp_OledRefresh();
					vDisp_OledSetPower(false);
				}
				v_disp_param_init();
				tDisp.bLight = false;
				S_bDispPageDirty = true;
			}
		}
		break;
	}
	
	return true;
}

/***********************************************************************************************************************
-----函数功能    背光自动关闭计时
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_TickTimer(void) 
{
	//非工作状态下退出
	if(tSysInfo.eDevState != DS_WORK) 
		return;
	
	//非亮屏幕状态
	if(tDisp.bLight == false)   
		return;
	
	//-----自动关闭背光--------------------------------------   
	if(tDisp.usAutoOffTime)
	{
		if(tDisp.usAutoOffCnt)
		{
			tDisp.usAutoOffCnt--;
			if(tDisp.usAutoOffCnt == 0)
			{
				v_disp_shut_down();
				if(uPrint.tFlag.bDispTask|| uPrint.tFlag.bImportant)
					sMyPrint("Lcd_Task:倒计时结束,进入息屏 时间 = %dS\r\n",tDisp.usAutoOffTime);
			}
		}
	}
}

/*****************************************************************************************************************
-----函数功能    初始化参数
-----说明(备注)  none
-----传入参数    p_disp_mem : disp记忆参数结构体
-----输出参数    none
-----返回值      true:设置成功  反之失败
*****************************************************************************************************************/
bool bDisp_MemParamInit(DispMemParam_T* p_disp_mem)
{
	p_disp_mem->ucHighLightValue = boardDISP_HIGH_LIGHT_VALUE;
	p_disp_mem->ucLowLightValue = boardDISP_LOW_LIGHT_VALUE;
	p_disp_mem->usAutoOffTime = boardDISP_OFF_TIME;
	return true;
}

#if(boardLOW_POWER)
/*****************************************************************************************************************
-----函数功能    检查系统的输入电源:外接电池
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
*****************************************************************************************************************/
void v_dis_power_select( void )
{
	if(tDisp.bLight)
	{
		/*************************************电源有输入*********************************************************/
		if( tAdcSamp.usBMS_Vin >= boardBMS_MIN_VOLT )   
		{
			Disp_EN_OFF();          //关闭显示屏的电池供电
		}
		/*************************************没有电源输入*******************************************************/
		else
		{
			Disp_EN_ON();          //打开显示屏的电池供电
		}	
	}
	else 
	{
		Disp_EN_OFF();          //关闭显示屏的电池供电
	}
}


/*****************************************************************************************************************
-----函数功能    进入低功耗
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
*****************************************************************************************************************/
void vLcd_EnterLowPower(void)
{
//	vLcd_IoEnterLowPower();
//	bAtti_EnterLowPower();
//	vExRTC_EnterLowPower();
	vTaskSuspend(tDispTaskHandler);
}

/*****************************************************************************************************************
-----函数功能    退出低功耗
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
*****************************************************************************************************************/
void vLcd_ExitLowPower(void)
{
//	vExRTC_ExitLowPower();
	vTaskResume(tDispTaskHandler);
}
#endif //boardLOW_POWER

#endif //boardDISPLAY_EN



