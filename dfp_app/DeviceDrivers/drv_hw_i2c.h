#ifndef __DRV_HW_I2C_H__
#define __DRV_HW_I2C_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

void i2c_hw_init(void);
void i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data, uint32_t timeout);
void i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif 





