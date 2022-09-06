#ifndef __DRV_adc_H
#define __DRV_adc_H

#include "n32g45x.h"
#include "init.h"

#define    ADC1_DAT_Address	            ((u32)0x4001244C)
#define    ADC1_SampleChannel			0x01D4		//ADC1_ch8、7、6、4、2
#define    ADC1_SampleTime				ADC_SAMP_TIME_55CYCLES5
#define    ADC1_SampleTimes				1
#define    ADC1_NbrOfChannel            5
#define    ADC1_SampleBufferSize        (ADC1_NbrOfChannel * ADC1_SampleTimes)

#define    NTC1_SAMPLE               	0x01
#define    NTC2_SAMPLE                	0x02
#define    BAT2VAL_SAMPLE               0x03
#define	   BAT1VAL_SAMPLE				0x04
#define    BAT2TEMP_SAMPLE				0x05
#define    BAT1TEMP_SAMPLE				0x06
#define    VREF_SAMPLE					0x07

//外部调用
uint16_t ADC_GetSample(uint8_t type);


#endif


