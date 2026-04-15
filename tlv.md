<!--
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2025-03-03 14:00:39
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2025-03-03 14:54:34
 * 
 * Copyright (c) 2025 by fhchengz, All Rights Reserved. 
-->
# 基于TLV的XXX传输协议
||head|tag|length|value|crc|
|-|-|-|-|-|-|
|示例1|0x55AA|0x00|0x01|0x12|0xXX|
|示例2|0x55AA|0x01|0x02|0x1234|0xXX|

* head    帧头
* tag     帧类型，帧标识
* length  帧长度(仅指示value字段的长度)
* value   帧数据
* crc crc 校验数据


###### C语言表示
```c
#define FH_STREAM_HEAD      (0x55AA)

typedef uint16_t head_type;
typedef uint8_t tag_type;
typedef uint8_t length_type;
typedef uint8_t value_type;
typedef uint8_t crc_type;

typedef enum {
    FH_STREAM_TAG_DATA = 0x00,
    FH_STREAM_TAG_CMD  = 0x01,
    FH_STREAM_TAG_ACK  = 0x02,
}tag_type_e;

// 帧结构
typedef struct __attribute__((packed)) _fh_stream_frame {
    head_type head;            
    tag_type tag;              
    length_type length;        
    value_type value[];         // 指向帧数据的指针     
    //crc 单独处理
} fh_stream_frame_t;

```

tlv封包
```c
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
```