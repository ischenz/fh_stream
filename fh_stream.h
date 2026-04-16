/*
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2026-04-15 16:48:04
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2026-04-16 17:53:22
 * 
 * Copyright (c) 2026 by fhchengz, All Rights Reserved. 
 */

/* |head|tag|length|value|crc| */
#ifndef __FH_STREAM_H
#define __FH_STREAM_H

#include <stdio.h>

#define FH_STREAM_HEAD      (0x55)

typedef uint8_t head_type;
typedef uint8_t tag_type;
typedef uint8_t length_type;
typedef uint8_t value_type;
typedef uint8_t crc_type;

typedef enum {
    FH_STREAM_EVENT_NULL = 0,
    FH_STREAM_EVENT_FRAME_RECEIVED,
    FH_STREAM_EVENT_ERROR_CRC,
}fh_event_t;
typedef enum {
    FH_STREAM_TAG_DATA = 0x00,
    FH_STREAM_TAG_CMD  = 0x01,
    FH_STREAM_TAG_ACK  = 0x02,
}tag_type_e;

//状态机状态定义
typedef enum {
    FH_STREAM_RX_STATE_IDLE = 0, // 等待帧头
    FH_STREAM_RX_STATE_TAG,
    FH_STREAM_RX_STATE_LENGTH,
    FH_STREAM_RX_STATE_VALUE,
    FH_STREAM_RX_STATE_CRC,
} rx_state_t;

// 帧结构
typedef struct __attribute__((packed)) _fh_stream_frame {
    head_type head;            
    tag_type tag;              
    length_type length;        
    value_type value[];         // 指向帧数据的指针     
    //crc 单独处理
} fh_stream_frame_t;

int fh_stream_pack(uint8_t *buffer, tag_type tag, value_type *data, length_type data_len);
fh_event_t fh_stream_unpack(uint8_t buffer, fh_stream_frame_t *freame);

#endif