/*
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2026-04-15 16:48:11
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2026-04-15 17:48:52
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