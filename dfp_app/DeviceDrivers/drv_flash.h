#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void flash_erase(uint32_t addr);
void flash_read(uint32_t addr, uint8_t *buf, uint32_t size);
int flash_write(uint32_t addr, const uint8_t *buf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif 


