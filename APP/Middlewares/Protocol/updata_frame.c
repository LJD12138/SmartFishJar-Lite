/*******************************************************************************************************************************
 * Project : ProjectTeam
 * Module  : G:\1-Baiku_Projects\11-G24\1.software\G2404-3\APP\Middlewares\Protocol
 * File    : updata_frame.c
 * Date    : 2026-04-22
 * Author  : OpenAI Codex
 * Desc    : 升级协议通用帧构造与解析实现
 ******************************************************************************************************************************/

#include "updata_frame.h"

#if(boardUPDATA)

#include <string.h>

#include "check.h"

#define updataFRAME_LEN_H_OFFSET          5U       // LEN高字节偏移
#define updataFRAME_DATA_OFFSET           7U       // LEN字段结束后的起始偏移，即RESERVE2位置
#define updataFRAME_RESERVE1_OFFSET       3U       // RESERVE1高字节偏移
#define updataFRAME_RESERVE2_OFFSET       7U       // RESERVE2偏移
#define updataFRAME_CMD_OFFSET            8U       // CMD偏移
#define updataFRAME_RESERVE3_OFFSET       9U       // RESERVE3偏移
#define updataFRAME_MAX_FRAME_LEN         0xFFFFU  // 受u16长度约束的最大帧长
#define updataFRAME_MAX_PAYLOAD_LEN       (updataFRAME_MAX_FRAME_LEN - updataFRAME_MIN_FRAME_LEN) // 理论最大PAYLOAD长度

/*****************************************************************************************************************
-----函数功能    根据升级协议文档中的一、指令格式构造数据帧
-----说明(备注)  LEN字段表示RESERVE2到CRC16的总字节数，CRC16使用Modbus算法，低字节在前
-----传入参数    slave_addr: 从机地址
                ic_type: 芯片ID
                cmd: 指令码
                payload: 有效载荷首地址，payload_len为0时可为NULL
                payload_len: 有效载荷长度
                out_frame: 输出帧缓存
                frame_buff_len: 输出帧缓存大小
                out_frame_len: 实际输出帧长度
-----输出参数    none
-----返回值      小于0:操作失败   等于0:没操作    大于0:操作成功
******************************************************************************************************************/
s8 cUpdata_FrameCreate(u8 slave_addr, u8 ic_type, u8 cmd, const u8* payload, u16 payload_len,
                       u8* out_frame, u16 frame_buff_len, u16* out_frame_len)
{
    u16 us_proto_len = 0;
    u16 us_frame_len = 0;
    u16 us_crc16 = 0;

    if(out_frame == NULL || out_frame_len == NULL)
        return -1;

    *out_frame_len = 0;

    if(payload == NULL && payload_len != 0)
        return -2;

    if(payload_len > updataFRAME_MAX_PAYLOAD_LEN)
        return -3;

    // LEN字段从RESERVE2开始计数，到CRC16结束，共PAYLOAD + 5字节
    us_proto_len = payload_len + updataFRAME_LEN_OVERHEAD;
    us_frame_len = payload_len + updataFRAME_MIN_FRAME_LEN;

    if(frame_buff_len < us_frame_len)
        return -4;

    // 按协议固定字段顺序写入帧头和控制字段
    out_frame[0] = updataFRAME_HEAD_CODE;
    out_frame[1] = slave_addr;
    out_frame[2] = ic_type;
    out_frame[3] = updataFRAME_RESERVE_VALUE;
    out_frame[4] = updataFRAME_RESERVE_VALUE;
    out_frame[5] = (u8)(us_proto_len >> 8);
    out_frame[6] = (u8)(us_proto_len & 0x00FF);
    out_frame[7] = updataFRAME_RESERVE_VALUE;
    out_frame[8] = cmd;
    out_frame[9] = updataFRAME_RESERVE_VALUE;

    if(payload_len != 0)
        memcpy(&out_frame[updataFRAME_PAYLOAD_OFFSET], payload, payload_len);

    // CRC16覆盖前面所有字节，按协议示例以低字节在前的顺序存放
    us_crc16 = usCheck_GetModbusCrc16(out_frame, (u32)(us_frame_len - updataFRAME_CRC_LEN));
    out_frame[us_frame_len - 2] = (u8)(us_crc16 & 0x00FF);
    out_frame[us_frame_len - 1] = (u8)(us_crc16 >> 8);

    *out_frame_len = us_frame_len;

    return 1;
}

/*****************************************************************************************************************
-----函数功能    根据升级协议文档中的一、指令格式解析数据帧
-----说明(备注)  仅解析一帧，成功后tp_frame会指向当前帧中的各个字段
-----传入参数    tp_frame: 解析结果
                ucp_data: 待解析数据首地址
                len: 当前可用数据长度
-----输出参数    none
-----返回值      小于0:操作失败   等于0:没操作    大于0:操作成功
******************************************************************************************************************/
s8 cUpdata_FrameParse(UpdataFrame_t* tp_frame, const u8* ucp_data, u16 len)
{
    u16 us_proto_len = 0;
    u16 us_frame_len = 0;
    u16 us_crc16 = 0;
    u16 us_calc_crc16 = 0;

    if(tp_frame == NULL || ucp_data == NULL)
        return -1;

    memset(tp_frame, 0, sizeof(UpdataFrame_t));

    if(len < updataFRAME_MIN_FRAME_LEN)
        return -2;

    if(ucp_data[0] != updataFRAME_HEAD_CODE)
        return -3;

    // 协议文档中所有保留位均为固定值0x00
    if((ucp_data[updataFRAME_RESERVE1_OFFSET] != updataFRAME_RESERVE_VALUE) ||
       (ucp_data[updataFRAME_RESERVE1_OFFSET + 1] != updataFRAME_RESERVE_VALUE) ||
       (ucp_data[updataFRAME_RESERVE2_OFFSET] != updataFRAME_RESERVE_VALUE) ||
       (ucp_data[updataFRAME_RESERVE3_OFFSET] != updataFRAME_RESERVE_VALUE))
        return -3;

    us_proto_len = ((u16)ucp_data[updataFRAME_LEN_H_OFFSET] << 8) |
                   (u16)ucp_data[updataFRAME_LEN_H_OFFSET + 1];
    if(us_proto_len < updataFRAME_LEN_OVERHEAD)
        return -4;

    // 完整帧长度 = HEADER~LEN前固定7字节 + LEN字段所描述的剩余长度
    us_frame_len = updataFRAME_DATA_OFFSET + us_proto_len;
    if(len < us_frame_len)
        return -4;

    // 接收帧中的CRC16按低字节在前存放，这里还原为u16后再与计算值比较
    us_crc16 = ((u16)ucp_data[us_frame_len - 1] << 8) | (u16)ucp_data[us_frame_len - 2];
    us_calc_crc16 = usCheck_GetModbusCrc16((u8*)ucp_data, (u32)(us_frame_len - updataFRAME_CRC_LEN));
    if(us_crc16 != us_calc_crc16)
        return -5;

    tp_frame->ucpFrame = ucp_data;
    tp_frame->usFrameLen = us_frame_len;
    tp_frame->ucHead = ucp_data[0];
    tp_frame->ucSlaveAddr = ucp_data[1];
    tp_frame->ucIcType = ucp_data[2];
    tp_frame->usReserve1 = ((u16)ucp_data[3] << 8) | (u16)ucp_data[4];
    tp_frame->usProtoLen = us_proto_len;
    tp_frame->ucReserve2 = ucp_data[updataFRAME_RESERVE2_OFFSET];
    tp_frame->ucCmd = ucp_data[updataFRAME_CMD_OFFSET];
    tp_frame->ucReserve3 = ucp_data[updataFRAME_RESERVE3_OFFSET];
    tp_frame->usPayloadLen = us_proto_len - updataFRAME_LEN_OVERHEAD;
    if(tp_frame->usPayloadLen != 0)
        tp_frame->ucpPayload = &ucp_data[updataFRAME_PAYLOAD_OFFSET];
    tp_frame->usCrc16 = us_crc16;

    return 1;
}

#endif  //boardUPDATA