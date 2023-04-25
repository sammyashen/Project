#ifndef __DEV_ADC_H__
#define __DEV_ADC_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"
#include "drv_adc.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	uint16_t avg;
	uint16_t avg_1;
	uint16_t buff[10];
	uint16_t sum;
	uint8_t  cnt;
	float    alpha;
}avgf_t;

uint16_t adc_get_val(uint8_t type);

#ifdef __cplusplus
}
#endif

#endif 



