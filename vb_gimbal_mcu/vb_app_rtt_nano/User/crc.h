#ifndef _CRC_H
#define _CRC_H

#include "hc32_ddl.h"
#include "init.h"

void crc16_calculate(uint8_t *p_buffer, uint16_t length, uint8_t crc_extra, uint16_t *ret_crc);
void crc32(uint8_t *p_data, uint32_t length, uint32_t *crc_ret);

#endif



