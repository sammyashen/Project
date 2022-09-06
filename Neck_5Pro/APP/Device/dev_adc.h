#ifndef _DEV_ADC_H
#define _DEV_ADC_H

#include "n32g45x.h"
#include "rtthread.h"
#include "init.h"
#include "drv_adc.h"

typedef struct{
	uint16_t ntc1_filter_adc;
	uint16_t ntc2_filter_adc;
	uint16_t bat1_filter_adc;
	uint16_t bat2_filter_adc;
	uint16_t bat1temp_filter_adc;
	uint16_t bat2temp_filter_adc;
}adc_filter_t;

void adc_filter_sftimer_init(void);
adc_filter_t *get_adc_filter_struct(void);


#endif

