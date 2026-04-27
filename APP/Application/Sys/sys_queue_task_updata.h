/***********************************************************************************************************************
 * Project : ProjectTeam
 * Module  : G:\1-Baiku_Projects\15-M50\1.software\M5004-3\APP\Application\Sys
 * File    : sys_queue_task_updata.h
 * Date    : 2026-03-13 17:00:57
 * Author  : LJD(291483914@qq.com)
 * Desc    : description
 * -------------------------------------------------------
 * todo    :
 * 1.
 * -------------------------------------------------------
 * Copyright (c) 2026 -inc
************************************************************************************************************************/
#ifndef SYS_QUEUE_TASK_UPDATA_H
#define SYS_QUEUE_TASK_UPDATA_H


#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================includes====================================*/
#include "board_config.h"

#if(boardUPDATA)
/* ==========================================macros======================================*/

/* ==========================================types=======================================*/
//通道选择
typedef enum
{
	CT_NULL = 0,		//未选择
    CT_CONSOLE,			//Xmodem协议
    CT_PRINT,			//上位机
	CT_INVAILD,			//超范围
}ChannelType_E;

//协议类型
typedef enum
{
	PT_NULL = 0,		//未选择
    PT_XMODEM,			//Xmodem协议
    PT_BAIKU,			//百酷协议
	PT_INVAILD,			//超范围
}ProtoType_E;

//升级对象
typedef enum
{
    UO_DEFAULT = 0,		//当前连接设备
    UO_CONSOLE,			//主控
	UO_BMS,				//电池
	UO_MPPT,			//光伏
	UO_DCAC,			//逆变
	UO_INVAILD,			//超范围
}UpdataObj_E;

/* ==========================================globals=====================================*/
typedef struct
{
	vu16				usRecFrameCnt;		//记录当前接收的帧数
	vu16 				usTotalFrmValue; 	//总帧数
	vu16            	usRecOverTimeCnt;	//帧超时计数
	vu16            	usLostOverTimeCnt;	//接受超时计数
	UpdataObj_E			eObj;				//升级对象
	ChannelType_E		eChType;			//通道类型
	ProtoType_E 		eProtoType;			//协议类型
}Updata_T;
extern Updata_T  tUpdata;

/* ==========================================extern======================================*/
bool bUpdata_Init(void);
s8 cUpdata_ChSelect(UpdataObj_E e_obj, ChannelType_E ch_type);
s8 cUpdata_ProtoSelect(UpdataObj_E e_obj, ProtoType_E proto_type);
void vUpdata_TickTimer(void);
#endif  //boardUPDATA

#ifdef __cplusplus
}
#endif

#endif  //SYS_QUEUE_TASK_UPDATA_H
