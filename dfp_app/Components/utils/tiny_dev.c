#include "tiny_dev.h"

static rt_list_t tiny_dev_list;
static tiny_device_t tiny_device_iterator = NULL;

//初始化链表
static int tiny_device_list_init(void)
{
	rt_list_init(&tiny_dev_list);
	return 0;
}
INIT_PREV_EXPORT(tiny_device_list_init);

//查找设备是否存在
static bool tiny_device_is_exists(tiny_device_t dev)
{
    rt_list_for_each_entry(tiny_device_iterator, &tiny_dev_list, node)
    {
    	if(rt_strcmp(tiny_device_iterator->name, dev->name) == 0)
    	{
            return true;
        }
    }
	
    return false;
}

//往设备链表中插入元素
static bool tiny_device_list_inster(tiny_device_t dev)
{
    rt_list_insert_after(&tiny_dev_list, &dev->node);

    return true;
}

//删除设备链表中的节点
static bool tiny_device_list_delete(tiny_device_t dev)
{
	rt_list_remove(&dev->node);

	return true;
}

//注册一个设备
bool tiny_device_register(tiny_device_t dev)
{
    if((NULL == dev) || (tiny_device_is_exists(dev)))
    {
        return false;
    }

    if((NULL == dev->name) ||  (NULL == dev->dops))
    {
        return false;
    }
    
    return tiny_device_list_inster(dev);
}

//卸载一个设备
bool tiny_device_unregister(tiny_device_t dev)
{
	if((NULL == dev) || (!tiny_device_is_exists(dev)))
    {
        return false;
    }

    if((NULL == dev->name) || (NULL == dev->dops))
    {
        return false;
    }

	tiny_device_list_delete(dev);
	dev->name = NULL;
	dev->dops = NULL;
	
    return true;
}

//查找一个名为name的设备
tiny_device_t tiny_device_find(const char *name)
{
	rt_list_for_each_entry(tiny_device_iterator, &tiny_dev_list, node)
	{
		if(rt_strcmp(tiny_device_iterator->name, name) == 0)
        {
            return tiny_device_iterator;
        }
	}
    
    return NULL;
}


