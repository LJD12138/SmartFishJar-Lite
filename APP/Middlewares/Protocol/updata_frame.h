/*******************************************************************************************************************************
 * Project : ProjectTeam
 * Module  : G:\1-Baiku_Projects\11-G24\1.software\G2404-3\APP\Middlewares\Protocol
 * File    : updata_frame.h
 * Date    : 2026-04-22
 * Author  : OpenAI Codex
 * Desc    : 升级协议通用帧构造与解析接口
 ******************************************************************************************************************************/
#ifndef UPDATA_FRAME_H
#define UPDATA_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board_config.h"

#if(boardUPDATA)

/*
升级协议帧格式（参考《升级协议》一、指令格式）
---------------------------------------------------------------------------------------
| HEADER | SlaveAddr | ICType | RESERVE1 | LEN | RESERVE2 | CMD | RESERVE3 | PAYLOAD |
---------------------------------------------------------------------------------------
|   1    |     1     |   1    |    2     |  2  |    1     |  1  |    1     |   N     |
---------------------------------------------------------------------------------------
|                                       CRC16(Modbus)                                  |
---------------------------------------------------------------------------------------
|                                            2                                          |
---------------------------------------------------------------------------------------
*/

#define updataFRAME_HEAD_CODE             0xAAU    // 协议头固定值
#define updataFRAME_RESERVE_VALUE         0x00U    // 所有保留位固定值
#define updataFRAME_LEN_OVERHEAD          5U       // LEN字段覆盖的固定开销: RESERVE2 + CMD + RESERVE3 + CRC16(2)
#define updataFRAME_FIXED_HEAD_LEN        10U      // 从HEADER到RESERVE3的固定长度
#define updataFRAME_CRC_LEN               2U       // CRC16长度
#define updataFRAME_PAYLOAD_OFFSET        10U      // PAYLOAD起始偏移
#define updataFRAME_MIN_FRAME_LEN         (updataFRAME_FIXED_HEAD_LEN + updataFRAME_CRC_LEN) // 不带PAYLOAD时的最短帧长

typedef struct
{
    const u8*          ucpFrame;          // 指向原始完整帧首地址
    u16                usFrameLen;        // 当前完整帧总长度
    u8                 ucHead;            // 协议头
    u8                 ucSlaveAddr;       // 从机地址
    u8                 ucIcType;          // 芯片ID
    u16                usReserve1;        // 保留字段1，协议固定为0x0000
    u16                usProtoLen;        // LEN字段原始值，表示RESERVE2到CRC16的总字节数
    u8                 ucReserve2;        // 保留字段2，协议固定为0x00
    u8                 ucCmd;             // 指令码
    u8                 ucReserve3;        // 保留字段3，协议固定为0x00
    u16                usPayloadLen;      // 有效载荷长度
    const u8*          ucpPayload;        // 指向有效载荷首地址，长度为0时为NULL
    u16                usCrc16;           // 原始帧中的CRC16值
}UpdataFrame_t;

/*****************************************************************************************************************
-----函数功能    根据升级协议文档格式构造完整数据帧
-----说明(备注)  LEN字段采用文档定义，CRC16使用Modbus算法，发送顺序为低字节在前
-----传入参数    slave_addr: 从机地址
                ic_type: 芯片ID
                cmd: 指令码
                payload: 有效载荷数据首地址，payload_len为0时可为NULL
                payload_len: 有效载荷长度
                out_frame: 输出帧缓存
                frame_buff_len: 输出帧缓存大小
                out_frame_len: 实际输出帧长度
-----输出参数    none
-----返回值      小于0:操作失败   等于0:没操作    大于0:操作成功
******************************************************************************************************************/
s8 cUpdata_FrameCreate(u8 slave_addr, u8 ic_type, u8 cmd, const u8* payload, u16 payload_len,
                       u8* out_frame, u16 frame_buff_len, u16* out_frame_len);

/*****************************************************************************************************************
-----函数功能    根据升级协议文档格式解析并校验完整数据帧
-----说明(备注)  成功后可通过tp_frame->usFrameLen获取本帧长度，并通过ucpPayload访问有效载荷
-----传入参数    tp_frame: 解析结果结构体
                ucp_data: 待解析数据首地址
                len: 当前可用数据长度
-----输出参数    none
-----返回值      小于0:操作失败   等于0:没操作    大于0:操作成功
******************************************************************************************************************/
s8 cUpdata_FrameParse(UpdataFrame_t* tp_frame, const u8* ucp_data, u16 len);

#endif  //boardUPDATA

#ifdef __cplusplus
}
#endif

#endif  //UPDATA_FRAME_H