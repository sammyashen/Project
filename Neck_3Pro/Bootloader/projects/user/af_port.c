#include "af_flash.h"
#include "iap_addr_def.h"

#ifndef USING_RTTHREAD
#include "n32l40x.h"
#include "af_utils.h"

void af_flash_unlock(void)
{
	FLASH_Unlock();	
}

void af_flash_lock(void)
{
	FLASH_Lock();	
}

AfErrCode af_flash_earse_one_page(uint32_t Page_Address)
{
	AfErrCode result = AF_NO_ERR;
	FLASH_STS flash_sts = FLASH_EraseOnePage(Page_Address);
	if(flash_sts != FLASH_COMPL)
		result = AF_ERASE_ERR;
	return result;
}
	
AfErrCode af_flash_program_word(uint32_t Address, uint32_t Data)
{
	AfErrCode result = AF_NO_ERR;
	FLASH_STS flash_sts = FLASH_ProgramWord(Address,Data);
	if(flash_sts != FLASH_COMPL)
		result = AF_WRITE_ERR;
	return result;
}

AfErrCode af_port_read(uint32_t addr, uint32_t *buf, size_t size) {
	AfErrCode result = AF_NO_ERR;
    uint8_t *buf_8 = (uint8_t *)buf;
    size_t i;
	
	if(addr +  size >= BR_FLASH_END_ADDRESS)
	{
		result = AF_READ_ERR;
		return result;
	}
    /*copy from flash to ram */
    for (i = 0; i < size; i++, addr ++, buf_8++) {
        *buf_8 = *(uint8_t *) addr;
    }
	return result;
}

AfErrCode af_port_erase(uint32_t addr, size_t size)
{
	AfErrCode result = AF_NO_ERR;
    size_t erase_pages, i;

	if(addr +  size >= BR_FLASH_END_ADDRESS)
	{
		result = AF_ERASE_ERR;
		return result;
	}
    /* calculate pages */
    erase_pages = size / BR_PAGE_SIZE;
    if (size % BR_PAGE_SIZE != 0)
        erase_pages++;

    /* start erase */
    af_flash_unlock();
    for (i = 0; i < erase_pages; i++)
    {
        if(AF_NO_ERR != af_flash_earse_one_page(addr + (BR_PAGE_SIZE * i)))
        {
        	result = AF_ERASE_ERR;
			return result;
        }
    }
    af_flash_lock();
	return result;
}


AfErrCode af_port_write(uint32_t addr, const uint32_t *buf, size_t size)
{
	AfErrCode result = AF_NO_ERR;
    size_t i;
    uint32_t read_data;

	if(addr +  size >= BR_FLASH_END_ADDRESS)
	{
		result = AF_WRITE_ERR;
		return result;
	}
    af_flash_unlock();
    for (i = 0; i < size; i += 4, buf++, addr += 4)
    {
        /* write data */
        if(AF_NO_ERR != af_flash_program_word(addr, *buf))
        {
        	result = AF_WRITE_ERR;
			return result;
        }
        read_data = *(uint32_t *)addr;
        /* check data */
        if (read_data != *buf)
        {
        	result = AF_WRITE_ERR;
            return result;
        }
    }
    af_flash_lock();
	return result;
}

uint32_t	IAPBUF[512];
uint32_t af_flash_check(unsigned long addr, unsigned long size)
{
	unsigned int check_sum = 0x0;
	int j,read_size;
	for (j=0; j<size;)
	{
		read_size = ((size-j) > sizeof(IAPBUF)) ? sizeof(IAPBUF) : (size-j);
		af_port_read(addr+j, (u32 *)IAPBUF, read_size);
		check_sum = af_calc_crc32(check_sum, IAPBUF, read_size);
		j+=read_size;
	}
	return check_sum;
}

AfErrCode af_erase_upgrade_data_partition(void)
{
	return af_port_erase(UPGRADEaddr, UPGRADE_DATA_SIZE);
}

AfErrCode af_write_upgrade_data_patition(const uint32_t *data, uint32_t size)
{
	return af_port_write(UPGRADEaddr, (uint32_t *)data, size);
}

#else
#include "drv_flash.h"
/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
AfErrCode af_port_read(uint32_t addr, uint32_t *buf, size_t size)
{
    AfErrCode result = AF_NO_ERR;
    if(n32_flash_read(addr,(rt_uint8_t *)buf,size) < 0)
	{
		result = AF_ERASE_ERR;
	}
    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
AfErrCode af_port_erase(uint32_t addr, size_t size)
{
    AfErrCode result = AF_NO_ERR;
    if(n32_flash_erase(addr,size) < 0)
	{
		result = AF_ERASE_ERR;
	}
	return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
AfErrCode af_port_write(uint32_t addr, const uint32_t *buf, size_t size)
{
    AfErrCode result = AF_NO_ERR;
	if(n32_flash_write(addr,(rt_uint8_t *)buf,size) < 0)
	{
		result = AF_WRITE_ERR;
	}
    return result;
}
#endif





