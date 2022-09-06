/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include "n32g45x.h"
#include "rtthread.h"
#include <fal.h>


#define PAGE_SIZE_ONCHIP	(2*1024)

//fal_partition_t fal_partition = NULL;

static int init(void)
{
    /* do nothing now */

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = n32g45x_onchip_flash.addr + offset;
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = n32g45x_onchip_flash.addr + offset;
    uint32_t read_data;
    uint32_t *buf_32 = (uint32_t *)buf;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
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

static int erase(long offset, size_t size)
{
	size_t i;
    FLASH_STS flash_status;
    size_t erase_pages;
    uint32_t addr = n32g45x_onchip_flash.addr + offset;

    /* calculate pages */
    erase_pages = size / PAGE_SIZE_ONCHIP;
    if (size % PAGE_SIZE_ONCHIP != 0) {
        erase_pages++;
    }

    /* start erase */
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BUSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    for (i = 0; i < erase_pages; i++) {
        flash_status = FLASH_EraseOnePage(addr + (PAGE_SIZE_ONCHIP * i));
        if (flash_status != FLASH_COMPL) {
            return -1;
        }
    }
    FLASH_Lock();

    return size;
}


const struct fal_flash_dev n32g45x_onchip_flash =
{
    .name       = "onchipflash",
    .addr       = 0x08000000,
    .len        = 256*1024,
    .blk_size   = 2*1024,
    .ops        = {init, read, write, erase},
    .write_gran = 32
};


