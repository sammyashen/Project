#include "drv_flash.h"

void flash_erase(uint32_t addr)
{
    FLASH_Unlock();
    FLASH_EraseOnePage(addr);
    FLASH_Lock();
}

void flash_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    uint32_t i = 0;
	
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }
}

int flash_write(uint32_t addr, const uint8_t *buf, uint32_t size)
{
    uint32_t i = 0;
    uint32_t read_data = 0;
    uint32_t *buf_32 = (uint32_t *)buf;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
	while(FLASH_GetFlagSTS(FLASH_FLAG_BUSY));
    for (i = 0; i < size; i += 4, buf_32++, addr += 4) {
        /* write data */
		FLASH_ProgramWord(addr, *buf_32);
        read_data = *(uint32_t *)addr;
        /* check data */
        if (read_data != *buf_32) {
            return -1;
        }
    }
    FLASH_Lock();

    return size;
}

