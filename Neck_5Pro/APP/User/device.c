#include "device.h"

static struct list_head mini_dev_list;//创建链表
static mini_device_t mini_device_iterator = NULL;//创建迭代器

//初始化链表
static void mini_device_list_init(void)
{
	INIT_LIST_HEAD(&mini_dev_list);
}

//查找字符设备是否存在
static bool mini_device_is_exists(mini_device_t dev)
{
    list_for_each_entry(mini_device_iterator, &mini_dev_list, node)
    {
    	if(rt_strcmp(mini_device_iterator->name,dev->name)==0)
        {
            return true;
        }
    }
	
    return false;
}

//往字符设备链表中插入元素
static bool mini_device_list_inster(mini_device_t dev)
{
    list_add_tail(&dev->node, &mini_dev_list);

    return true;
}

//注册一个字符设备
bool mini_device_register(mini_device_t dev)
{
    if((NULL == dev) || (mini_device_is_exists(dev)))
    {
        return false;
    }

    if((NULL == dev->name) ||  (NULL == dev->dops))
    {
        return false;
    }
    return mini_device_list_inster(dev);
}

//查找一个名为name的字符设备
mini_device_t mini_device_find(const char *name)
{
	list_for_each_entry(mini_device_iterator, &mini_dev_list, node)
	{
		if(rt_strcmp(mini_device_iterator->name,name)==0)
        {
            return mini_device_iterator;
        }
	}
    
    return NULL;
}

sys_initcall(mini_device_list_init);

