#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void flash_erase(uint32_t addr, size_t size);
void flash_read(uint32_t addr, uint8_t *buf, size_t size);
int flash_write(uint32_t addr, const uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __HC32F460_GPIO_H__ */


