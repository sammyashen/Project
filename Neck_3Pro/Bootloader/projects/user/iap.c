#include "iap.h"

iapfun jump2app;

extern void iap_load_app(u32 appxaddr);

//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr)
{
    MSR MSP, r0             //set Main Stack value
    BX r14
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
    if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000) //检查栈顶地址是否合法.
    {
        jump2app=(iapfun)*(vu32*)(appxaddr+4);      //用户代码区第二个字为程序开始地址(复位地址)
        MSR_MSP(*(vu32*)appxaddr);                  //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
        jump2app();                                 //跳转到APP.
    }
}

void SoftReset(void)                                    //软复位程序
{
    __set_FAULTMASK(1);                                 //关闭所有中断
    NVIC_SystemReset();                                 //复位    以上两个函数位于 core_cm3.h 文件中
}

