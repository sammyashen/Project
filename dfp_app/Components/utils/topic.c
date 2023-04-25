#include "topic.h"
#include "rtthread.h"

static Topic_t s_Topic_Pool[TOPIC_POOL_MAX];    // 主题资源池
static uint8_t s_Topic_Count = 0;               // 资源池使用个数

static Async_TopicCb_t s_TopicCb_Pool[TOPICCB_POOL_MAX]; // 订阅者资源池
static uint8_t s_TopicCb_Count = 0;              // 资源池已占用个数

typedef struct{
	rt_uint8_t id;
	void *msg;
	rt_uint8_t index;
}__attribute__((packed, aligned(4))) mq_data_t;
static mq_data_t msgpool[TOPICCB_PER_ID_COUNT] = {0};
static struct rt_messagequeue topic_async_mq;

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

bool Topic_Init(uint8_t topic)
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

	pTopic->buff = &s_TopicCb_Pool[s_TopicCb_Count];
	pTopic->size = TOPICCB_PER_ID_COUNT;
	s_TopicCb_Count += TOPICCB_PER_ID_COUNT;
	pTopic->count = 0;

	rt_memset(pTopic->buff, 0x0, pTopic->size);
	return true;
}

bool Topic_Pushlish(uint8_t topic, void *msg)
{
	mq_data_t mq_data;

    if(s_Topic_Count == 0)
    {
        return false;
    }
        
    Topic_t* pTopic = Topic_Find(topic);
    if(pTopic == NULL)
    {
        return false;
    }
    
    for(int i = 0; i < pTopic->count; i ++)
    {
        if(pTopic->buff[i].func && pTopic->buff[i].is_async == 0)
        {
            pTopic->buff[i].func(msg);
        }else if(pTopic->buff[i].func && pTopic->buff[i].is_async == 1)
        {
        	mq_data.id = pTopic->id;
        	mq_data.msg = msg;
        	mq_data.index = i;
			rt_mq_send(&topic_async_mq, &mq_data, sizeof(mq_data_t));
        }
    }
    return true;
}

bool Topic_Subscrib_Sync(uint8_t topic, TopicCb cb)
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
    
    pTopic->buff[pTopic->count].func = cb;
    pTopic->buff[pTopic->count].is_async = 0;
    pTopic->count++;
    return true;
}

static void topic_thread_entry(void *parameter)
{
	while(1)
	{
		rt_err_t ret = RT_EOK;
		mq_data_t mq_data;
		ret = rt_mq_recv(&topic_async_mq, &mq_data, sizeof(mq_data_t), RT_WAITING_FOREVER);
		if(ret == RT_EOK){
			if(s_Topic_Count == 0)		continue;
			Topic_t* pTopic = Topic_Find(mq_data.id);
			if(pTopic == NULL)			continue;
			if(mq_data.index < pTopic->count){
				if(pTopic->buff[mq_data.index].func && pTopic->buff[mq_data.index].is_async == 1){
		        	pTopic->buff[mq_data.index].func(mq_data.msg);
		        }
			}
		}
	}
}

bool Topic_Subscrib_Async(uint8_t topic, TopicCb cb)
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
    
    pTopic->buff[pTopic->count].func = cb;
    pTopic->buff[pTopic->count].is_async = 1;
    pTopic->count++;
    return true;
}

static int topic_system_init(void)
{
	rt_thread_t tid = RT_NULL;
	rt_err_t ret = RT_EOK;

	ret = rt_mq_init(&topic_async_mq, "topic_async", msgpool, sizeof(mq_data_t), sizeof(msgpool), RT_IPC_FLAG_FIFO);
	tid = rt_thread_create("topic", topic_thread_entry, RT_NULL, 512, 16, 10);
	if (tid != RT_NULL && ret == RT_EOK)
        rt_thread_startup(tid);
        
    return 0;
}
INIT_APP_EXPORT(topic_system_init);

