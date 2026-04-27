/*****************************************************************************************************************
*                                                                                                                *
 *                                         系统的队列函数                                                  		*
*                                                                                                                *
******************************************************************************************************************/
#include "Sys/sys_queue_task.h"
#include "Sys/sys_task.h"
#include "Print/print_task.h"

#if(boardBMS_EN)
#include "MD_Bms/md_bms_task.h"
#endif

#if(boardDCAC_EN)
#include "MD_Dcac/md_dcac_task.h"
#endif

#if(boardADC_EN)
#include "Adc/adc_task.h"
#endif

#if(boardWATER_PUMP_EN)
#include "Pump/pump_task.h"
#endif

#if(boardO2PUMP_EN)
#include "O2Pump/o2pump_task.h"
#endif

#include "app_info.h"
#include "gpio_init.h"
														
#define     	sysTASK_WORK_CYCLE_TIME					10 //任务时间


//****************************************************参数初始化**************************************************//
SysPerm_U uPerm;

//****************************************************函数声明****************************************************//
static void v_chg_pwr_manage(void);
static void v_pump_manage(void);

/***********************************************************************************************************************
-----函数功能    工作
************************************************************************************************************************/ 
void v_sys_queue_task_work(Task_T *tp_task)
{
	if(tSysInfo.eDevState != DS_WORK)
		bSys_SetDevState(DS_WORK, false);
	
	//检查系统活跃状态
	if(bSys_CheckActState() == true)
		bSys_SetAutoOffTime(tAppMemParam.tSYS.usAutoOffTime);
	
	//队列里面有任务
	if(lwrb_get_full(&tp_task->tQueueBuff))  
	{
		cQueue_GotoStep( tp_task, STEP_END );  //结束
		return;
	}
	
    switch (tp_task->ucStep)
    {
		case 0:
		{
			/* 进入工作状态：启动水泵和O2泵默认挡位 */
			#if(boardWATER_PUMP_EN)
			bPump_SetMode(PUMP_MID);
			#endif
			#if(boardO2PUMP_EN)
			bO2Pump_SetMode(O2PUMP_MID);
			#endif
			cQueue_GotoStep( tp_task, STEP_NEXT );
		}
		break;
		
		case 1:
		{
			v_chg_pwr_manage();
			v_pump_manage();
		}
		break;
		
        default:
			cQueue_GotoStep(tp_task, STEP_END);  //结束
			break;
    }
	
	#if(boardUSE_OS)
	vTaskDelay(sysTASK_WORK_CYCLE_TIME);
	#endif  //boardUSE_OS
}

__STATIC_INLINE void v_chg_pwr_manage(void)
{
	#if(boardBMS_EN)
	
	//1:充满  0:刚插上电可以充电  -1:一直插着电可以充电
	static s8  c_chg_full_flag = 0;
	
	memset(&tSysInfo.tSetChgPwr, 0, sizeof(tSysInfo.tSetChgPwr));
	
	//不许可充电
	if(tSysInfo.uPerm.tPerm.bChgPerm == false ||
		tSysInfo.uPerm.tPerm.bForceClose == true)
		return;
	
	//充满后,需要降低到90才再次开始充电
	if(ucBms_GetSoc() == 100)
		c_chg_full_flag = 1;
	else if(ucBms_GetSoc() <= 90)
		c_chg_full_flag = -1;
	
	//移除充电,可以再次充电
	if(
		#if(boardMPPT_EN)
		tMppt.eDevState == DS_SHUT_DOWN
		#else
		true
		#endif  //boardMPPT_EN
		&&
		#if(boardDCAC_EN)
		tDcac.eChgState == IOS_SHUT_DOWN
		#else
		true
		#endif  //boardDCAC_EN
		)
		c_chg_full_flag = 0;
	
	//充满未释放
	if(c_chg_full_flag > 0)
		return;

	//设置MPPT充电功率
	#if(boardMPPT_EN)
	if(tMppt.bChgPerm == true && 
		tMppt.eDevState >= DS_BOOTING)
	{
		if(ucBms_GetSoc() >= 98)
			tSysInfo.tSetChgPwr.usMPPT = sysCHG_PWR_LEVEL1;
		else if(ucBms_GetSoc() <=2 || ucBms_GetSoc() >= 90)
			tSysInfo.tSetChgPwr.usMPPT = sysCHG_PWR_LEVEL2;
		else
			tSysInfo.tSetChgPwr.usMPPT = tAppMemParam.tMPPT.usInPwrRating / 10;
	}
	#endif  //boardMPPT_EN
	
	//设置DCAC充电功率
	#if(boardDCAC_EN)
	if(tDcac.uPerm.tPerm.bChgPerm == true && 
		tDcac.eChgState >= IOS_STARTING)
	{
		if(ucBms_GetSoc() >= 98)
			tSysInfo.tSetChgPwr.usDCAC = sysCHG_PWR_LEVEL1;
		else if(ucBms_GetSoc() <=2 || ucBms_GetSoc() >= 90)
			tSysInfo.tSetChgPwr.usDCAC = sysCHG_PWR_LEVEL2;
		else
			tSysInfo.tSetChgPwr.usDCAC = tAppMemParam.tDCAC.usInPwrRating;
	}
	#endif  //boardDCAC_EN

	#endif  //boardBMS_EN
}

/***********************************************************************************************************************
-----函数功能    水泵/O2泵过流检测与状态管理（每次工作任务周期调用）
************************************************************************************************************************/
static void v_pump_manage(void)
{
    #if(boardWATER_PUMP_EN && boardADC_EN)
    /* 水泵过流检测：电流超过3A时关闭，避免损坏 */
    if(tAdcSamp.fPumpCurr > 3.0f)
        bPump_SetMode(PUMP_OFF);
    #endif

    #if(boardO2PUMP_EN && boardADC_EN)
    /* O2泵过流检测：电流超过2A时关闭 */
    if(tAdcSamp.fO2Curr > 2.0f)
        bO2Pump_SetMode(O2PUMP_OFF);
    #endif
}

