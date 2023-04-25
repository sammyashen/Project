#ifndef _TOPIC_H_
#define _TOPIC_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define TOPIC_POOL_MAX              (10)            // ������Դ�����֧��������
#define TOPICCB_PER_ID_COUNT        (5)             // ÿ���������֧�ֶ���������
#define TOPICCB_POOL_MAX            (TOPIC_POOL_MAX * TOPICCB_PER_ID_COUNT)  // �����߳�       


typedef void (*TopicCb)(void* msg);

typedef struct{
	TopicCb func;
	uint8_t is_async;
}Async_TopicCb_t;

typedef struct _Topic{
    uint8_t id;                 // �����ʶ
    Async_TopicCb_t *buff;      // ��������Ŀ�� 
    uint8_t size;               // Ŀ���������
    uint8_t count;              // Ŀ��ʹ����
}Topic_t;


bool Topic_Init(uint8_t topic);
bool Topic_Pushlish(uint8_t topic, void *msg);  
bool Topic_Subscrib_Sync(uint8_t topic, TopicCb cb);
bool Topic_Subscrib_Async(uint8_t topic, TopicCb cb);

#endif



