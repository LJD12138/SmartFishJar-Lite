/*****************************************************************************************************************
*                                                                                                                *
 *                                         Disp显示任务                                                          *
*                                                                                                                *
 ******************************************************************************************************************/
#include "MD_Display/md_display_task.h"

#if(boardDISPLAY_EN)
#include "MD_Display/md_display_api.h"
#include "MD_Display/md_display_iface.h"
#include "MD_Display/md_display_queue_task.h"
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

#if(boardBUZ_EN)
#include "Buz/buz_task.h"
#endif  //boardBUZ_EN


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
TaskHandle_t tDispTaskHandler = NULL; 
void vDisp_Task(void *pvParameters);
#endif  //boardUSE_OS

//****************************************************参数初始化**************************************************//
Disp_T tDisp; 
u8g2_t u8g2;  // 显示器初始化结构体
bool g_bDispPageDirty = true;   //页面脏标志

//****************************************************局部函数定义************************************************//
static void v_disp_param_init(void);

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
	
	tDisp.eDevState = DS_INIT;
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

	vDisp_IfaceInit(); // 初始化OLED硬件接口
	
	if(bDisp_QueueInit() == false)
		return false;

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
 -----函数功能    设置显示设备运行状态
 -----说明(备注)  none
 -----传入参数    state: 设备状态
 -----输出参数    none
 -----返回值      true:操作成功   false:操作失败
 ************************************************************************************************************************/
bool bDisp_SetDevState(DevState_E state)
{
	if(tDisp.eDevState != state)
	{
		tDisp.eDevState = state;
		if(tDisp.eDevState == DS_INIT)  //初始化
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为初始化\r\n");
		}
		else if(tDisp.eDevState == DS_CLOSING)  //关闭中
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为关闭中\r\n");
		}
		else if(tDisp.eDevState == DS_SHUT_DOWN)  //关闭
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为关闭\r\n");
		}
		else if(tDisp.eDevState == DS_ERR)  //错误
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为错误\r\n");
		}
		else if(tDisp.eDevState == DS_BOOTING)    //启动中
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为启动中\r\n");
		}
		else if(tDisp.eDevState == DS_WORK)    //工作
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为工作\r\n");
		}
		#if(boardENG_MODE_EN)
		else if(tDisp.eDevState == DS_ENG_MODE)  //工程模式
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为工程模式\r\n");
		}
		#endif //boardENG_MODE_EN
		else if(tDisp.eDevState == DS_UPDATA_MODE)    //升级模式
		{
			if(uPrint.tFlag.bDispTask || uPrint.tFlag.bImportant)
				sMyPrint("bDispTask:显示任务状态为升级模式\r\n");
		}
	}
	
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
	static Task_T *tp_task = NULL;
	
	#if(boardUSE_OS)
	for(;;)
	#endif  //boardUSE_OS
	{
		if(tp_task == NULL)
		{
			if(tpDispTask != NULL)
				tp_task = tpDispTask;
			
			#if(boardUSE_OS)
			vTaskDelay(100);
			continue;
			#else
			return;
			#endif  //boardUSE_OS
		}
		
		if(tp_task->vp_func != NULL && tp_task->bNowRun == false)
			tp_task->vp_func(tp_task);
		else if(tp_task->vp_func == NULL || tp_task->bNowRun == true)
		{
			#if(boardUSE_OS)
			if(lwrb_get_full(&tp_task->tQueueBuff) == 0)
				ulTaskNotifyTake(pdFALSE, boardDISP_REFRESH_TMIE);
			#endif  //boardUSE_OS
			
			if(tp_task->bp_task_manage_func != NULL)
				tp_task->bp_task_manage_func(tp_task);
		}
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
					vDisp_SetPower(true);
					g_bDispPageDirty = true;
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
					vDisp_ClearBuffer();
					vDisp_Refresh();
					vDisp_SetPower(false);
				}
				v_disp_param_init();
				tDisp.bLight = false;
				g_bDispPageDirty = true;
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
				bDisp_Switch(ST_OFF, false);
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
	vTaskResume(tDispTaskHandler);
}
#endif //boardLOW_POWER

#endif //boardDISPLAY_EN
