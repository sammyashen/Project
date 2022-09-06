/*
*@brief bootloader
*/
#include "include.h"


/* 宏定义 */
#define UPDATE_FLAG_ADDR		(0x08037000 - 4)
#define APP_SIZE				(0x19000)
#define FLASH_APP_RUN_ADDR		(0x08002800)
#define FLASH_APP_BK_ADDR		(0x0801E000)

/* 函数声明 */
typedef  void (*iapfun)(void);		//定义一个函数类型的参数
void iap_load_app(u32 appxaddr);
static void DelayMS(uint16_t _usMs);


/* 变量声明 */
iapfun jump2app;
__IO uint32_t g_ulDelayCnt = 0;



/*
*@brief 栈顶设置
*/
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/*
*@brief 转跳应用代码
*/
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		
		MSR_MSP(*(vu32*)appxaddr);					
		jump2app();								
	}
}


/*
*@brief 毫秒级延迟函数
*/
static void DelayMS(uint16_t _usMs)
{
	g_ulDelayCnt = _usMs;
	while(g_ulDelayCnt);
}

/*
*@brief USB升级处理函数
*/
static void USB_Update_Pro(void)
{
	static uint8_t s_ucState = Code_Version_Check;
	static uint32_t s_ucUpdateFlag = 0;
	uint32_t i;
	
	while(1)
	{
		switch(s_ucState)
		{
			case Code_Version_Check:
				DataFlashRead(UPDATE_FLAG_ADDR, (uint8_t *)&s_ucUpdateFlag, sizeof(uint32_t));
			
				if(s_ucUpdateFlag != 0xAAAAAAAA)
				{
					s_ucState = App_Run;
				}
				else
				{
					s_ucUpdateFlag = 0xFFFFFFFF;
					DataFlashWrite(UPDATE_FLAG_ADDR, &s_ucUpdateFlag, 1);
					s_ucState = Code_Copy;
				}
			break;

			case Code_Copy:
				for(i = 0;i < (APP_SIZE/FLASH_PAGE_SIZE);i++)
				{
					DataFlashWrite((FLASH_APP_RUN_ADDR + i*FLASH_PAGE_SIZE), 		//整页写入
						(u32 *)(FLASH_APP_BK_ADDR + i*FLASH_PAGE_SIZE), 
						(FLASH_PAGE_SIZE>>2));
				}

				DataFlashWrite((FLASH_APP_RUN_ADDR + i*FLASH_PAGE_SIZE), 		//写入最后一页数据
					(u32 *)(FLASH_APP_BK_ADDR + i*FLASH_PAGE_SIZE), 
					((APP_SIZE%FLASH_PAGE_SIZE)>>2) + 1);
				
				s_ucState = App_Run;
			break;

			case App_Run:
				iap_load_app(FLASH_APP_RUN_ADDR);
			break;
		}

		DelayMS(10);
	}
}

/*
*@brief main函数
*/
int main(void)
{
	/* SysClk Configer */
	g_tSysClk_Dev.SysClkConfig(USE_HSE_FOR_CLKSRC, 72000000);
	SysTick_Config(72000000 / 1000);//1ms tick

	while(1)
	{
		DelayMS(100);
		USB_Update_Pro();
	}
}

