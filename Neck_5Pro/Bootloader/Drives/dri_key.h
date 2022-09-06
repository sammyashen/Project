#ifndef __DRI_KEY_H
#define __DRI_KEY_H

#include "n32g45x.h"

/* �����ڶ�Ӧ��RCCʱ�� */
#define RCC_ALL_KEY 	RCC_APB2_PERIPH_GPIOC

#define GPIO_PORT_K1    GPIOC
#define GPIO_PIN_K1	    GPIO_PIN_13


#define KEY_COUNT    1		//��������	

/* ����Ӧ�ó���Ĺ��������������� */
#define KEY_DOWN_K1		KEY_1_DOWN		/*S1*/
#define KEY_UP_K1		KEY_1_UP
#define KEY_LONG_K1		KEY_1_LONG

/* ����ID, ��Ҫ����bsp_KeyState()��������ڲ��� */
typedef enum
{
	KID_K1 = 0
}KEY_ID_E;

#define KEY_FILTER_TIME   3				/* �˲��������������� */
#define KEY_LONG_TIME     50			/* �������� */

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
}KEY_ENUM;

/* ����FIFO�õ����� */
#define KEY_FIFO_SIZE	5
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* ��ֵ������ */
	uint8_t Read;					/* ��������ָ�� */
	uint8_t Write;					/* ������дָ�� */
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

//���ⲿ���õı����������ӿ�
void Register_KEY_Model(T_KEY_Model *_ptKEY_Model);



#endif



