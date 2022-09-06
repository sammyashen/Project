#ifndef _TOPIC_H_
#define _TOPIC_H_

#include "n32g45x.h"
#include "stdbool.h"
#include "stdlib.h"
#include "rtthread.h"

#define TOPIC_POOL_MAX              (30)            // ������Դ�����֧��������
#define TOPICCB_PER_ID_COUNT        (10)             // ÿ���������֧�ֶ���������
#define TOPICCB_POOL_MAX            (TOPIC_POOL_MAX * TOPICCB_PER_ID_COUNT)  // �����߳�       


typedef void (*TopicCb)(void* msg);

typedef struct _Topic{
    uint8_t id;                 // �����ʶ
    TopicCb *buff;              // ��������Ŀ�� 
    uint8_t size;               // Ŀ���������
    uint8_t count;              // Ŀ��ʹ����
}Topic_t;


bool Topic_Init(uint8_t topic, TopicCb* buff, uint8_t size);
bool Topic_Pushlish(uint8_t topic, void *msg);  
bool Topic_Subscrib(uint8_t topic, TopicCb cb);


#endif



