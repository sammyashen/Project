#ifndef __DRV_DAC_H__
#define __DRV_DAC_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void dac_init(void);
void dac_tx(uint32_t *mem_addr, uint32_t mem_size);

#ifdef __cplusplus
}
#endif

#endif 





