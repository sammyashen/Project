#include "device.h"

static struct list_head mini_dev_list;//��������
static mini_device_t mini_device_iterator = NULL;//����������

//��ʼ������
static void mini_device_list_init(void)
{
	INIT_LIST_HEAD(&mini_dev_list);
}

//�����ַ��豸�Ƿ����
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

//���ַ��豸�����в���Ԫ��
static bool mini_device_list_inster(mini_device_t dev)
{
    list_add_tail(&dev->node, &mini_dev_list);

    return true;
}

//ע��һ���ַ��豸
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

//����һ����Ϊname���ַ��豸
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

