#include "dispatch.h"

struct dispatch dispatch_struct;

static void _add(dispatch_t dispatch_struct, observer_t observer_struct)
{
	list_add_tail(&observer_struct->node, &dispatch_struct->list);  
}

static void _remove(dispatch_t dispatch_struct, observer_t observer_struct)
{
	list_del(&observer_struct->node);                       
}

static void _notify(dispatch_t dispatch_struct)
{
	observer_t iterator = NULL;
	
	list_for_each_entry(iterator, &dispatch_struct->list, node)  
	{
		iterator->notify(dispatch_struct->args);                
	}
}

static void dispatch_init(dispatch_t dispatch_struct)
{
	// 初始化数据
	dispatch_struct->args = NULL;

	// 初始化操作函数
	dispatch_struct->add = _add;
	dispatch_struct->remove = _remove;
	dispatch_struct->notify = _notify;

	// 初始化链表
	INIT_LIST_HEAD(&dispatch_struct->list);
}

static void dispatch_register(void)
{
	dispatch_init(&dispatch_struct);
}

app_initcall(dispatch_register);

