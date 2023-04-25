#ifndef __DRV_ADC_H
#define __DRV_ADC_H

#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define    ADC1_DAT_Address	            ((u32)&ADC->DAT)
#define    ADC1_SampleChannel			0x0007		//ADC1_ch0~2
#define    ADC1_SampleTime				ADC_SAMP_TIME_239CYCLES5
#define    ADC1_SampleTimes				1
#define    ADC1_NbrOfChannel            3
#define    ADC1_SampleBufferSize        (ADC1_NbrOfChannel * ADC1_SampleTimes)

#define    OUT1_SAMPLE					0x00
#define    OUT2_SAMPLE					0x01
#define    VREF_SAMPLE					0x02

//外部调用
void ADC_RegConfig(void);
uint16_t ADC_GetSample(uint8_t type);

#ifdef __cplusplus
}
#endif

#endif


