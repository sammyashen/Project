#ifndef __DRV_adc_H
#define __DRV_adc_H

#include "n32l40x.h"
#include "init.h"

#define    ADC1_DAT_Address	            ((u32)&ADC->DAT)
#define    ADC1_SampleChannel			0x067F		//ADC1_ch0~6、9、10
#define    ADC1_SampleTime				ADC_SAMP_TIME_239CYCLES5
#define    ADC1_SampleTimes				1
#define    ADC1_NbrOfChannel            9
#define    ADC1_SampleBufferSize        (ADC1_NbrOfChannel * ADC1_SampleTimes)

#define    NTC_SAMPLE               	0x00
#define    BAT1VAL_SAMPLE               0x01
#define	   BAT2VAL_SAMPLE				0x02
#define    BAT1TEMP_SAMPLE				0x03
#define    BAT2TEMP_SAMPLE				0x04
#define    M1VAL_SAMPLE					0x05
#define    M2VAL_SAMPLE					0x06
#define    M3VAL_SAMPLE					0x07
#define    VREF_SAMPLE					0x08

typedef struct
{
	uint16_t avg;
	uint16_t avg_1;
	uint16_t buff[10];
	uint16_t sum;
	uint8_t  cnt;
	float    alpha;
}avgf_t;

//外部调用
uint16_t ADC_GetSample(uint8_t type);


#endif


