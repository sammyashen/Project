#ifndef _DRV_KEY_H
#define _DRV_KEY_H

#include "n32l40x.h"
#include "init.h"

/* 按键口对应的RCC时钟 */
#define RCC_ALL_KEY 	RCC_APB2_PERIPH_GPIOB

#define GPIO_PORT_K1    GPIOB				//S1
#define GPIO_PIN_K1	    GPIO_PIN_8

#define GPIO_PORT_K2    GPIOB				//MODE
#define GPIO_PIN_K2	    GPIO_PIN_4


#define KEY_COUNT    3		//按键个数	

/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
typedef enum
{
	KID_K1 = 0,
	KID_K2,
	KID_K1K2,
}KEY_ID_E;

#define KEY_FILTER_TIME   5				/* 滤波参数，按键消抖 */
#define KEY_LONG_TIME     100			/* 长按计数 */

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t  Count;			/* 滤波器计数器 */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t  RepeatSpeed;	/* 连续按键周期 */
	uint8_t  RepeatCount;	/* 连续按键计数器 */
}KEY_T;

/*
	定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件

	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2) 编译器可帮我们避免键值重复。
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 表示按键事件 */

	KEY_1_DOWN,				/* 1键按下 */
	KEY_1_UP,				/* 1键弹起 */
	KEY_1_LONG,				/* 1键长按 */
	KEY_1_LONG_UP,			/* 1键长按后弹起 */

	KEY_2_DOWN,				
	KEY_2_UP,				
	KEY_2_LONG,				
	KEY_2_LONG_UP,			

	KEY_3_DOWN,				/* 组合键 */	
	KEY_3_UP,				
	KEY_3_LONG,				
	KEY_3_LONG_UP,	
}KEY_ENUM;

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	3
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;

void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed);
void bsp_ClearKey(void);
void bsp_KeyScan(void);



#endif



