#ifndef __DRI_FLASH_H
#define __DRI_FLASH_H

#include "n32g45x.h"

#define    FLASH_PAGE_SIZE		0x800				//stm32中文参考手册“嵌入式闪存”章节描述，大容量每页2K字节
#define    FLASH_HEAD_ADDRESS   0x08000000			//Flash首地址
#define    DATA_FLASH_HEAD_ADDRESS   0x08000000		//自定义数据首地址，这里定义0x08000000后面地址存数据
#define    DATA_FLASH_END_ADDRESS    0x0803FFFF		//数据尾地址，RCT6的Flash大小256K


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

