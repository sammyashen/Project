#ifndef __DRI_KEY_H
#define __DRI_KEY_H

#include "n32g45x.h"

/* 按键口对应的RCC时钟 */
#define RCC_ALL_KEY 	RCC_APB2_PERIPH_GPIOC

#define GPIO_PORT_K1    GPIOC
#define GPIO_PIN_K1	    GPIO_PIN_13


#define KEY_COUNT    1		//按键个数	

/* 根据应用程序的功能重命名按键宏 */
#define KEY_DOWN_K1		KEY_1_DOWN		/*S1*/
#define KEY_UP_K1		KEY_1_UP
#define KEY_LONG_K1		KEY_1_LONG

/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
typedef enum
{
	KID_K1 = 0
}KEY_ID_E;

#define KEY_FILTER_TIME   3				/* 滤波参数，按键消抖 */
#define KEY_LONG_TIME     50			/* 长按计数 */

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
}KEY_ENUM;

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	5
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;



typedef struct
{
    void (*Key_Init)(void);
    void (*PutKeyVal)(uint8_t _ucKeyVal);
    uint8_t (*GetKeyVal)(void);
    uint8_t (*GetKeyState)(KEY_ID_E _tKeyID);
    void (*SetKeyParam)(uint8_t _ucKeyID, uint16_t _usLongTime, uint8_t  _ucRepeatSpeed);
    void (*ClearKey)(void);
    void (*KeyScan)(void);
}T_KEY_Device;

typedef struct
{
    T_KEY_Device *Key_dev;
}T_KEY_Model;

//供外部调用的变量、函数接口
void Register_KEY_Model(T_KEY_Model *_ptKEY_Model);



#endif



