/*
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2026-04-15 16:48:11
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2026-04-16 18:15:54
 * 
 * Copyright (c) 2026 by fhchengz, All Rights Reserved. 
 */
#include "fh_stream.h"
#include <string.h>

int fh_stream_pack(uint8_t *buffer, tag_type tag, value_type *data, length_type data_len) 
{
    fh_stream_frame_t *frame = (fh_stream_frame_t *)buffer;
    frame->head = FH_STREAM_HEAD;
    frame->tag = tag;
    frame->length = data_len;
    memcpy(frame->value, data, data_len);
    // crc 计算和添加
    *(crc_type *)&frame->value[data_len] = 0xEE; // 占位，实际crc计算后覆盖
    return sizeof(fh_stream_frame_t) + data_len + sizeof(crc_type);
}

typedef struct {
    rx_state_t state;
    length_type idx;    // 用于接收value数据的索引
    crc_type crc; 
    fh_stream_frame_t frame;
} fh_stream_rx_context_t;

typedef fh_event_t (*fh_state_handler_t)(uint8_t byte);

static uint8_t fream_buff[sizeof(fh_stream_rx_context_t) + (1 << sizeof(length_type) * 8)]; // 用于接收数据的缓冲区，大小足够存放一个完整的帧和上下文
static fh_stream_rx_context_t *rx_context = (fh_stream_rx_context_t *)fream_buff;

static crc_type fh_stream_crc(uint8_t *data, length_type len) 
{
    (void *)data;
    (void *)len;
    return 0xEE; // 占位，实际crc计算逻辑根据需要实现
}

// 等待帧头状态
static fh_event_t fh_state_idle(uint8_t byte)
{
    if (byte == FH_STREAM_HEAD) {
        rx_context->idx = 0; // 重置索引
        rx_context->frame.head = byte;
        rx_context->state = FH_STREAM_RX_STATE_TAG;
        return FH_STREAM_EVENT_NULL;
    } else {
        return FH_STREAM_EVENT_NULL;
    }
}

// 接收tag状态
static fh_event_t fh_state_tag(uint8_t byte)
{
    rx_context->frame.tag = byte;
    rx_context->state = FH_STREAM_RX_STATE_LENGTH;
    return FH_STREAM_EVENT_NULL;
}

// 接收length状态
static fh_event_t fh_state_length(uint8_t byte)
{
    rx_context->frame.length = byte;
    rx_context->state = FH_STREAM_RX_STATE_VALUE;
    return FH_STREAM_EVENT_NULL;
}

// 接收value状态
static fh_event_t fh_state_value(uint8_t byte)
{
    rx_context->frame.value[rx_context->idx++] = byte;
    if (rx_context->idx >= rx_context->frame.length) {
        rx_context->state = FH_STREAM_RX_STATE_CRC;
    } 
    return FH_STREAM_EVENT_NULL;
}

static fh_event_t fh_state_crc(uint8_t byte)
{
    rx_context->crc = byte;
    if (fh_stream_crc((uint8_t *)&rx_context->frame.length, sizeof(fh_stream_frame_t) - sizeof(head_type) + rx_context->frame.length) != byte) { // 占位crc校验失败
        rx_context->state = FH_STREAM_RX_STATE_IDLE; // 重置状态机
        return FH_STREAM_EVENT_ERROR_CRC; // crc错误事件
    }
    rx_context->state = FH_STREAM_RX_STATE_IDLE; // 重置状态机
    return FH_STREAM_EVENT_FRAME_RECEIVED; // 成功接收完整帧
}

static const fh_state_handler_t state_handlers[] = {
    [FH_STREAM_RX_STATE_IDLE] = fh_state_idle,
    [FH_STREAM_RX_STATE_TAG] = fh_state_tag,
    [FH_STREAM_RX_STATE_LENGTH] = fh_state_length,
    [FH_STREAM_RX_STATE_VALUE] = fh_state_value,
    [FH_STREAM_RX_STATE_CRC] = fh_state_crc,
};

fh_event_t fh_stream_unpack(uint8_t buffer, fh_stream_frame_t *freame) 
{
    if(state_handlers[rx_context->state](buffer) == FH_STREAM_EVENT_FRAME_RECEIVED) {
        memcpy(freame, &rx_context->frame, sizeof(fh_stream_frame_t) + rx_context->frame.length);
        return FH_STREAM_EVENT_FRAME_RECEIVED; // 成功解包
    }
    return FH_STREAM_EVENT_NULL; // 解包失败
}