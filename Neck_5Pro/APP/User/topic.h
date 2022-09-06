#ifndef _TOPIC_H_
#define _TOPIC_H_

#include "n32g45x.h"
#include "stdbool.h"
#include "stdlib.h"
#include "rtthread.h"

#define TOPIC_POOL_MAX              (30)            // 主题资源池最大支持主题数
#define TOPICCB_PER_ID_COUNT        (10)             // 每个主题最大支持订阅者数量
#define TOPICCB_POOL_MAX            (TOPIC_POOL_MAX * TOPICCB_PER_ID_COUNT)  // 订阅者池       


typedef void (*TopicCb)(void* msg);

typedef struct _Topic{
    uint8_t id;                 // 主题标识
    TopicCb *buff;              // 订阅主题目标 
    uint8_t size;               // 目标最大容量
    uint8_t count;              // 目标使用数
}Topic_t;


bool Topic_Init(uint8_t topic, TopicCb* buff, uint8_t size);
bool Topic_Pushlish(uint8_t topic, void *msg);  
bool Topic_Subscrib(uint8_t topic, TopicCb cb);


#endif



