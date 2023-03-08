#include "drv_flash.h"

#define BR_PAGE_SIZE	(8*1024)	//8K

void flash_erase(uint32_t addr, size_t size)
{
    size_t erase_pages, i;

    /* calculate pages */
    erase_pages = size / BR_PAGE_SIZE;
    if (size % BR_PAGE_SIZE != 0)
        erase_pages++;

    /* start erase */
    EFM_Unlock();
    EFM_FlashCmd(Enable);
	EFM_ClearFlag(EFM_FLAG_EOP | EFM_FLAG_WRPERR | EFM_FLAG_PGSZERR);
	while(!EFM_GetFlagStatus(EFM_FLAG_RDY));
    for (i = 0; i < erase_pages; i++)
    {
        EFM_SectorErase(addr + (BR_PAGE_SIZE * i));
    }
    EFM_Lock();
}

void flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    uint32_t i = 0;
	
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }
}

int flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    uint32_t i = 0;
    uint32_t read_data = 0;
    uint32_t *buf_32 = (uint32_t *)buf;

	EFM_Unlock();
	EFM_FlashCmd(Enable);
	EFM_ClearFlag(EFM_FLAG_EOP | EFM_FLAG_WRPERR | EFM_FLAG_PGSZERR);
	while(!EFM_GetFlagStatus(EFM_FLAG_RDY));
    for (i = 0; i < size; i += 4, buf_32++, addr += 4) {
        /* write data */
		EFM_SingleProgram(addr, *buf_32);
        read_data = *(uint32_t *)addr;
        /* check data */
        if (read_data != *buf_32) {
            return -1;
        }
    }
    EFM_Lock();

    return size;
}


