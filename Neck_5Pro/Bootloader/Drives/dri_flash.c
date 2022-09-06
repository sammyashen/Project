#include "dri_flash.h"

//注意写的data类型为字类型，所以定义buff为uint8_t时，需要强制转换类型，datasize也要除以4
DATAFLASH_ERROR DataFlashWrite(u32 address, u32 *dataptr, u16 datasize)
{
	u32 startpageaddr;
	u32 endpageaddr;
	u8  nbrofpage;	
	u16 i;
	
	if (address > DATA_FLASH_END_ADDRESS) 
	{
		return ADDRESS_TOO_BIG; 
	}
	else if (address < DATA_FLASH_HEAD_ADDRESS)
	{
		return ADDRESS_TOO_SMALL;
	} 
	else if ((address % 4) != 0)
	{
		return WRITE_ODD_ADDRESS;
	}
	else if ((address + datasize * 4 - 1) > DATA_FLASH_END_ADDRESS)
	{
		return SIZE_TOO_BIG;
	}
	startpageaddr = (address - FLASH_HEAD_ADDRESS) / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE + FLASH_HEAD_ADDRESS;
	endpageaddr = (address + datasize * 4 - 1 - FLASH_HEAD_ADDRESS) / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE + FLASH_HEAD_ADDRESS;
	nbrofpage = (endpageaddr - startpageaddr) / FLASH_PAGE_SIZE + 1;
	
	FLASH_Unlock();	 
	FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	
	for (i = 0;i < nbrofpage;i++)
	{
		startpageaddr += i * FLASH_PAGE_SIZE;
		FLASH_EraseOnePage(startpageaddr);
	}
	
	for (i = 0;i < datasize;i++)
	{
//		FLASH_ProgramOBData(address, *dataptr++);
//		address += 2;
		FLASH_ProgramWord(address, *dataptr++);//N32G452仅支持字操作，不支持半字操作
		address += 4;
	}
	FLASH_Lock();
	return WRITE_SUCCESS;
}

DATAFLASH_ERROR DataFlashRead(u32 address, u8 *dataptr, u16 datasize)
{
	u16 i;
	
	if (address > DATA_FLASH_END_ADDRESS)
	{
		return ADDRESS_TOO_BIG;
	}
	else if (address < DATA_FLASH_HEAD_ADDRESS)
	{
		return ADDRESS_TOO_SMALL;
	}
	else if ((address + datasize - 1) > DATA_FLASH_END_ADDRESS)
	{
		return SIZE_TOO_BIG;
	}
	for (i = 0;i < datasize;i++)
	{
		*dataptr++ = *(u8 *)address;
		address++;
	}
	return READ_SUCCESS;
}

