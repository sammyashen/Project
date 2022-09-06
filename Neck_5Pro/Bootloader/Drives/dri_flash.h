#ifndef __DRI_FLASH_H
#define __DRI_FLASH_H

#include "n32g45x.h"

#define    FLASH_PAGE_SIZE		0x800				//stm32���Ĳο��ֲᡰǶ��ʽ���桱�½�������������ÿҳ2K�ֽ�
#define    FLASH_HEAD_ADDRESS   0x08000000			//Flash�׵�ַ
#define    DATA_FLASH_HEAD_ADDRESS   0x08000000		//�Զ��������׵�ַ�����ﶨ��0x08000000�����ַ������
#define    DATA_FLASH_END_ADDRESS    0x0803FFFF		//����β��ַ��RCT6��Flash��С256K


typedef enum
{
	ADDRESS_TOO_BIG = 1,
	ADDRESS_TOO_SMALL,
	SIZE_TOO_BIG,
	WRITE_ODD_ADDRESS,
	WRITE_SUCCESS,
	READ_SUCCESS,
}DATAFLASH_ERROR;

extern DATAFLASH_ERROR DataFlashWrite(u32 address, u32 *dataptr, u16 datasize);
extern DATAFLASH_ERROR DataFlashRead(u32 address, u8 *dataptr, u16 datasize);

#endif

