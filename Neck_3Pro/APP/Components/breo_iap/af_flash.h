#ifndef AF_FLASH_H_
#define AF_FLASH_H_

#include <stdint.h>
#include <stddef.h>
#include "af_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EasyFlash error code */
typedef enum
{
    AF_NO_ERR,
    AF_ERASE_ERR,
    AF_READ_ERR,
    AF_WRITE_ERR
} AfErrCode;

AfErrCode af_port_read(uint32_t addr, uint32_t *buf, size_t size);
AfErrCode af_port_erase(uint32_t addr, size_t size);
AfErrCode af_port_write(uint32_t addr, const uint32_t *buf, size_t size);

#ifndef USING_RTTHREAD
void af_flash_unlock(void);
void af_flash_lock(void);
AfErrCode af_flash_earse_one_page(uint32_t Page_Address);
AfErrCode af_flash_program_word(uint32_t Address, uint32_t Data);
AfErrCode af_erase_upgrade_data_partition(void);
AfErrCode af_write_upgrade_data_patition(const uint32_t *data, uint32_t size);
uint32_t af_flash_check(unsigned long addr, unsigned long size);
#endif

#ifdef __cplusplus
}
#endif

#endif /* RY_FLASH_H_ */
