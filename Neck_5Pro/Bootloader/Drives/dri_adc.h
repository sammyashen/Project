#ifndef __DRI_adc_H
#define __DRI_adc_H

#include "n32g45x.h"

#define    ADC1_DAT_Address	            ((u32)0x4001244C)
#define    ADC1_SampleChannel			0x01D4		//ADC1_ch8、7、6、4、2
#define    ADC1_SampleTime				ADC_SAMP_TIME_55CYCLES5
#define    ADC1_SampleTimes				1
#define    ADC1_NbrOfChannel            5
#define    ADC1_SampleBufferSize        (ADC1_NbrOfChannel * ADC1_SampleTimes)

#define    PA1_SAMPLE               	0x01
#define    PA2_SAMPLE                	0x02
#define    PA3_SAMPLE                 	0x03
#define	   PA5_SAMPLE					0x04

//外部调用
void ADC1_Init(void);
void ADC2_Init(void);
uint16_t ADC_GetSample(uint8_t type);


#endif

