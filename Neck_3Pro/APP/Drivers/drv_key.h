#ifndef _DRV_KEY_H
#define _DRV_KEY_H

#include "n32l40x.h"
#include "init.h"

/* �����ڶ�Ӧ��RCCʱ�� */
#define RCC_ALL_KEY 	RCC_APB2_PERIPH_GPIOB

#define GPIO_PORT_K1    GPIOB				//S1
#define GPIO_PIN_K1	    GPIO_PIN_8

#define GPIO_PORT_K2    GPIOB				//MODE
#define GPIO_PIN_K2	    GPIO_PIN_4


#define KEY_COUNT    3		//��������	

/* ����ID, ��Ҫ����bsp_KeyState()��������ڲ��� */
typedef enum
{
	KID_K1 = 0,
	KID_K2,
	KID_K1K2,
}KEY_ID_E;

#define KEY_FILTER_TIME   5				/* �˲��������������� */
#define KEY_LONG_TIME     100			/* �������� */

/*
	ÿ��������Ӧ1��ȫ�ֵĽṹ�������
*/
typedef struct
{
	/* ������һ������ָ�룬ָ���жϰ����ַ��µĺ��� */
	uint8_t (*IsKeyDownFunc)(void); /* �������µ��жϺ���,1��ʾ���� */

	uint8_t  Count;			/* �˲��������� */
	uint16_t LongCount;		/* ���������� */
	uint16_t LongTime;		/* �������³���ʱ��, 0��ʾ����ⳤ�� */
	uint8_t  State;			/* ������ǰ״̬�����»��ǵ��� */
	uint8_t  RepeatSpeed;	/* ������������ */
	uint8_t  RepeatCount;	/* �������������� */
}KEY_T;

/*
	�����ֵ����, ���밴���´���ʱÿ�����İ��¡�����ͳ����¼�

	�Ƽ�ʹ��enum, ����#define��ԭ��
	(1) ����������ֵ,�������˳��ʹ���뿴���������
	(2) �������ɰ����Ǳ����ֵ�ظ���
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 ��ʾ�����¼� */

	KEY_1_DOWN,				/* 1������ */
	KEY_1_UP,				/* 1������ */
	KEY_1_LONG,				/* 1������ */
	KEY_1_LONG_UP,			/* 1���������� */

	KEY_2_DOWN,				
	KEY_2_UP,				
	KEY_2_LONG,				
	KEY_2_LONG_UP,			

	KEY_3_DOWN,				/* ��ϼ� */	
	KEY_3_UP,				
	KEY_3_LONG,				
	KEY_3_LONG_UP,	
}KEY_ENUM;

/* ����FIFO�õ����� */
#define KEY_FIFO_SIZE	3
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* ��ֵ������ */
	uint8_t Read;					/* ��������ָ�� */
	uint8_t Write;					/* ������дָ�� */
}KEY_FIFO_T;

void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed);
void bsp_ClearKey(void);
void bsp_KeyScan(void);



#endif



