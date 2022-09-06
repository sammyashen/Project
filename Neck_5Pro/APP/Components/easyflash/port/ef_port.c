/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for stm32f10x platform.
 * Created on: 2015-01-16
 */

#include "n32g45x.h"
#include "rtthread.h"
#include <fal.h>
#include <easyflash.h>

//extern fal_partition_t fal_partition;
static const struct fal_partition *part = NULL;

/* default environment variables set for user */
static const ef_env default_env_set[] = {
        {"user_mode_recipe", "0"},
        {"low_pwr_flag", "0"},
        {"test_env2", "0"},
};

static char log_buf[128];
static struct rt_semaphore env_cache_lock;

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;
          
    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

    rt_sem_init(&env_cache_lock, "env lock", 1, RT_IPC_FLAG_PRIO);

    part = fal_partition_find("easyflash");
    EF_ASSERT(part);

    return result;
}

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
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    fal_partition_read(part, (addr - EF_START_ADDR), (uint8_t *)buf, size);

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
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;
    
    /* start erase */
    if(fal_partition_erase(part, (addr - EF_START_ADDR), size) > 0)
		result = EF_NO_ERR;
	else
		result = EF_ERASE_ERR;
	
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
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    if(fal_partition_write(part, (addr - EF_START_ADDR), (uint8_t *)buf, size) > 0)
    	result = EF_NO_ERR;
	else
		result = EF_WRITE_ERR;

    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    rt_sem_take(&env_cache_lock, RT_WAITING_FOREVER);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    rt_sem_release(&env_cache_lock);
}


/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#ifdef PRINT_DEBUG

    va_list args;

	rt_memset(log_buf, 0, sizeof(log_buf));
    /* args point to the first variable parameter */
    va_start(args, format);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    rt_kprintf("(%s:%ld)%s", file, line, log_buf);
    va_end(args);

#endif

}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;

	rt_memset(log_buf, 0, sizeof(log_buf));
    /* args point to the first variable parameter */
    va_start(args, format);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    rt_kprintf("%s", log_buf);
    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;

	rt_memset(log_buf, 0, sizeof(log_buf));
    /* args point to the first variable parameter */
    va_start(args, format);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    rt_kprintf("%s", log_buf);
    va_end(args);
}
/**
 * This function is write log to flash.
 *
 * @param ... args
 */
#define GET_NUM(M, N)		((M - 1)/N + 1)
void ef_log_to_flash(const char *format, ...)
{
#ifdef EF_USING_LOG
	va_list args;

	rt_memset(log_buf, 0, sizeof(log_buf));
	/* args point to the first variable parameter */
    va_start(args, format);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    rt_kprintf("%s", log_buf);
    ef_log_write((uint32_t *)log_buf, (GET_NUM(rt_strlen((char *)log_buf), 4) << 2));//д�����ݱ���Ϊ4*n��bytes
    va_end(args);
#endif
}
/**
 * This function is read log flash flash.
 *
 * @param null
 */
void ef_read_all_log_from_flash(void)
{
	size_t used_size = 0;
	uint8_t log_disp[5] = {0};

	used_size = ef_log_get_used_size();
	if(used_size != 0)
	{
		rt_kprintf("[F/EASYFLASH] Log:\n");
		for(uint8_t i=0;i<(used_size>>2);i++)
		{
			rt_memset(log_disp, 0, 5);
			ef_log_read((i<<2), (uint32_t *)log_disp, 4);
			rt_kprintf("%s", log_disp);
		}
		rt_kprintf("\r\n");
	}
	else
	{
		rt_kprintf("[F/EASYFLASH] No log.\r\n");
	}	
}


