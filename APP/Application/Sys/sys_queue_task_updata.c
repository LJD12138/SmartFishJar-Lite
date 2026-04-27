/*****************************************************************************************************************
*                                                                                                                *
 *                                         系统的队列函数                                                  		*
*                                                                                                                *
******************************************************************************************************************/
#include "Sys/sys_queue_task.h"

#if(boardUPDATA)
#include "Sys/sys_task.h"
#include "Print/print_task.h"
#include "Sys/sys_queue_task_updata.h"

#include "gpio_init.h"
#include "app_info.h"


#define     	sysTASK_UPDATA_CYCLE_TIME				sysTASK_CYCLE_TIME //任务时间
#define       	updataREC_LOST_OVERTIME                	((360 * 1000) / boardREPET_TIMER_CYCLE_TMIE) 	//ms


//****************************************************参数初始化**************************************************//
Updata_T tUpdata;


/***********************************************************************************************************************
-----函数功能    升级
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/ 
void v_sys_queue_task_updata(Task_T *tp_task)
{
    switch (tp_task->ucStep)
    {
		case 0:
		{
			bSys_SetDevState(DS_UPDATA_MODE, true);
			cQueue_GotoStep(tp_task, STEP_NEXT);  //下一步
		}
		break;
		
		//启动升级对象
		case 1:
		{
			tSysSetParam t_sys_set_param = {0};

			#if(boardBMS_EN)
			if(tUpdata.eObj == UO_BMS)
			{
				if(tpBmsTask->tReplyBuff.buff == NULL)
				{
					cQueue_GotoStep( tp_task, STEP_END );  //结束
					return;
				}

				t_sys_set_param.obj = UO_BMS;
				t_sys_set_param.cmd = mainUPDATA_FLAG;
				lwrb_reset(&tpBmsTask->tReplyBuff);
				lwrb_write(&tpBmsTask->tReplyBuff, &t_sys_set_param, sizeof(t_sys_set_param));

				if(cQueue_AddQueueTask(tpBmsTask, BTI_REQ_SET_CMD, 0, true) <= 0)
				{
					cQueue_GotoStep( tp_task, STEP_END );  //结束
					return;
				}
			}
			else
			#endif  //boardBMS_EN
				break;
			
			cQueue_GotoStep(tp_task, STEP_NEXT);
		}
		
		//等待对象进入升级模式
		case 2:
		{
			//超时重新发送
			tp_task->usStepWaitCnt++;
			if(tp_task->usStepWaitCnt >= (3000 / sysTASK_UPDATA_CYCLE_TIME))
			{
				cQueue_GotoStep(tp_task, STEP_FORWARD);
				break;
			}

			#if(boardBMS_EN)
			if(tUpdata.eObj == UO_BMS)
			{
				if(tBms.eDevState == DS_UPDATA_MODE)
					cQueue_GotoStep(tp_task, STEP_NEXT);
				else
					break;
			}
			else
			#endif  //boardBMS_EN
				break;
		}
		
		//开启升级通道
		case 3:
		{
			if(tUpdata.eChType == CT_PRINT)
			{
				if(cQueue_AddQueueTask(tpPrintTask, PTI_UPDATA, tUpdata.eObj, false) > 0)
					cQueue_GotoStep(tp_task, STEP_NEXT);  //下一步
				else
					break;
			}
			else
				break;
		}
		
		//等待进入透传模式
		case 4:
		{
			//超时重新发送
			tp_task->usStepWaitCnt++;
			if(tp_task->usStepWaitCnt >= (1500 / sysTASK_UPDATA_CYCLE_TIME))
			{
				cQueue_GotoStep(tp_task, STEP_FORWARD);
				break;
			}

			
			if(tUpdata.eChType == CT_PRINT)
			{
				if(tpPrintTask->ucID == PTI_UPDATA)
					cQueue_GotoStep(tp_task, STEP_NEXT);  //下一步
				else
					break;
			}
			else
				break;
		}
		
		//等待升级完成
		case 5:
		{
			//升级超时
			if(tUpdata.usLostOverTimeCnt == 0)
			{
				bUpdata_Init();
				cSys_Switch(SO_KEY, ST_OFF, false);
				cQueue_GotoStep(tp_task, STEP_END);  //结束
			}
		}
		break;
		
        default:
				cQueue_GotoStep(tp_task, STEP_END);  //结束
			break;
    }
		
	#if(boardUSE_OS)
	vTaskDelay(sysTASK_UPDATA_CYCLE_TIME);
	#endif  //boardUSE_OS
}

/*****************************************************************************************************************
-----函数功能    参数初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
bool bUpdata_Init(void)
{
	memset(&tUpdata, 0, sizeof(tUpdata));
	tUpdata.usLostOverTimeCnt = updataREC_LOST_OVERTIME;
	return true;
}


/***********************************************************************************************************************
-----函数功能	升级通道选择
-----传入参数   e_obj
-----传入参数   ch_type
-----返回值     s8
-----作者       LJD
-----日期       2026-03-16
************************************************************************************************************************/
s8 cUpdata_ChSelect(UpdataObj_E e_obj, ChannelType_E ch_type)
{
	if(e_obj >= UO_INVAILD || ch_type >= CT_INVAILD)
		return -1;
	
	if(ch_type == tUpdata.eChType && 
		tSysInfo.eDevState == DS_UPDATA_MODE)
	{
		tUpdata.usLostOverTimeCnt = updataREC_LOST_OVERTIME;
		return 0;
	}
	
	switch(e_obj)
	{
		case UO_DEFAULT:
		case UO_CONSOLE:
		{
			
		}
		break;
		
		case UO_BMS:
		{
			
		}
		break;
		
		default:
			return -3;
	}
	
	tUpdata.eObj = e_obj;
	tUpdata.eChType = ch_type;
	tUpdata.usLostOverTimeCnt = updataREC_LOST_OVERTIME;
	

	if(cQueue_AddQueueTask(tpSysTask, STI_UPDATA, 0, false) < 0)
		return -2;

	return 1;
}

/***********************************************************************************************************************
-----函数功能 	协议选择
-----传入参数   e_obj
-----传入参数   proto_type
-----返回值     s8
-----作者       LJD
-----日期       2026-03-16
************************************************************************************************************************/
s8 cUpdata_ProtoSelect(UpdataObj_E e_obj, ProtoType_E proto_type)
{
	if(e_obj >= UO_INVAILD || proto_type >= PT_INVAILD)
		return -1;
	
	if(proto_type == tUpdata.eProtoType && 
		tSysInfo.eDevState == DS_UPDATA_MODE)
	{
		tUpdata.usRecOverTimeCnt = updataREC_LOST_OVERTIME;
		return 0;
	}
	tUpdata.eProtoType = proto_type;
	return 1;
}

/***********************************************************************************************************************
-----函数功能    升级任务Tick计时
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vUpdata_TickTimer(void)
{
	if(tUpdata.usRecOverTimeCnt > 0)
	{
		tUpdata.usRecOverTimeCnt--;
		if(tUpdata.usRecOverTimeCnt == 0)
		{
			
		}
	}
	
	if(tUpdata.usLostOverTimeCnt > 0)
	{
		tUpdata.usLostOverTimeCnt--;
		if(tUpdata.usLostOverTimeCnt == 0)
		{
			//bSys_SetDevState(DS_SHUT_DOWN, true);
		}
	}
}
#endif  //boardUPDATA
