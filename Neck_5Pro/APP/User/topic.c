#include "topic.h"

static Topic_t s_Topic_Pool[TOPIC_POOL_MAX];    // ������Դ��
static uint8_t s_Topic_Count = 0;               // ��Դ��ʹ�ø���

static TopicCb s_TopicCb_Pool[TOPICCB_POOL_MAX]; // ��������Դ��
static uint8_t s_TopicCb_Count = 0;              // ��Դ����ռ�ø���

Topic_t* Topic_Find(uint8_t topic)
{ 
    int i = 0;
    for(i = 0;i < s_Topic_Count; i++)
    {
        if(topic == s_Topic_Pool[i].id)
        {
            return &s_Topic_Pool[i];
        }
    }
    return NULL;
}

/*****************************************************************************
 * Function      : Topic_Init
 * Description   : �����ʼ��
 * Input         :  uint8_t topic  �����ʶ��
         			TopicCb* buff  �����߻�����
         			uint8_t size   ���֧�ֶ���������
 * Output        : bool
  *****************************************************************************/
  bool Topic_Init(uint8_t topic, TopicCb* buff, uint8_t size)
  {
   if(s_Topic_Count >= TOPIC_POOL_MAX)
   {
       return false;
   }

   Topic_t* pTopic = Topic_Find(topic);
   if(pTopic)
   {
       return true;
   }

   pTopic = &s_Topic_Pool[s_Topic_Count];
   s_Topic_Count ++;

   pTopic->id = topic;

   if(pTopic->buff)
   {
       pTopic->buff = buff;
       pTopic->size = size;
   }
   else
   {
       pTopic->buff = &s_TopicCb_Pool[s_TopicCb_Count];
       pTopic->size = TOPICCB_PER_ID_COUNT;
       s_TopicCb_Count += TOPICCB_PER_ID_COUNT;
   }
   pTopic->count = 0;

   rt_memset(pTopic->buff, 0x0, pTopic->size);
   return true;
  }

bool Topic_Pushlish(uint8_t topic, void *msg)
{
    if(s_Topic_Count == 0)
    {
        return false;
    }
        
    Topic_t* pTopic = Topic_Find(topic);
    if(pTopic == NULL)
    {
        return false;
    }
    
    int i = 0;
    for(i = 0; i < pTopic->count; i ++)
    {
        if(pTopic->buff[i])
        {
            pTopic->buff[i](msg);
        }
    }
    return true;
}

bool Topic_Subscrib(uint8_t topic, TopicCb cb)
{
    Topic_t* pTopic = Topic_Find(topic);
    if(pTopic == NULL)
    {
        return false;
    }
    
    if(pTopic->count >= pTopic->size)
    {
        return false;
    }
    
    pTopic->buff[pTopic->count] = cb;
    pTopic->count++;
    return true;
}


