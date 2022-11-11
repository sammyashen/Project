#ifndef _IAP_H_
#define _IAP_H_

#include "n32l40x.h"

typedef  void (*iapfun)(void);              //定义一个函数类型的参数.

void iap_load_app(u32 appxaddr);            //执行flash里面的app程序
void SoftReset(void);                       //软复位程序

#endif


