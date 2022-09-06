#include "dev_adc.h"
#include "include.h"

rt_timer_t adc_filter_sftimer = RT_NULL;
static adc_filter_t adc_filter_struct ={
	.ntc1_filter_adc = 2048,
	.ntc2_filter_adc = 2048,
	.bat1_filter_adc = 4095,
	.bat2_filter_adc = 4095,
	.bat1temp_filter_adc = 2048,
	.bat2temp_filter_adc = 2048,
};

static void adc_filter_sftimer_cb(void *para)
{
	static uint8_t avg_filter_cnt = 0;
	static uint8_t pid_filter_cnt = 0;
	static uint16_t avg_filter_sum[4] = {0};
	static uint16_t pid_filter_sum[2] = {0};
	float _fRatio = 0.0f;

	if(++pid_filter_cnt > 2)
	{
		pid_filter_cnt = 0;
		adc_filter_struct.ntc1_filter_adc = pid_filter_sum[0] >> 1;
		adc_filter_struct.ntc2_filter_adc = pid_filter_sum[1] >> 1;
		rt_memset(pid_filter_sum, 0, sizeof(pid_filter_sum));
	}
	else
	{
		//先处理纹波
		_fRatio = ((float)ADC_GetSample(VREF_SAMPLE))/1489.0f;
		
		pid_filter_sum[0] += (uint16_t)((float)ADC_GetSample(NTC1_SAMPLE)/_fRatio);
		pid_filter_sum[1] += (uint16_t)((float)ADC_GetSample(NTC2_SAMPLE)/_fRatio);
	}

	if(++avg_filter_cnt > 8)
	{
		avg_filter_cnt = 0;
		adc_filter_struct.bat1_filter_adc = avg_filter_sum[0] >> 3;
		adc_filter_struct.bat2_filter_adc = avg_filter_sum[1] >> 3;
		adc_filter_struct.bat1temp_filter_adc = avg_filter_sum[2] >> 3;
		adc_filter_struct.bat2temp_filter_adc = avg_filter_sum[3] >> 3;
		rt_memset(avg_filter_sum, 0, sizeof(avg_filter_sum));
	}
	else
	{
		//先处理纹波
		_fRatio = ((float)ADC_GetSample(VREF_SAMPLE))/1489.0f;

		avg_filter_sum[0] += (uint16_t)((float)ADC_GetSample(BAT1VAL_SAMPLE)/_fRatio);
		avg_filter_sum[1] += (uint16_t)((float)ADC_GetSample(BAT2VAL_SAMPLE)/_fRatio);
		avg_filter_sum[2] += (uint16_t)((float)ADC_GetSample(BAT1TEMP_SAMPLE)/_fRatio);
		avg_filter_sum[3] += (uint16_t)((float)ADC_GetSample(BAT2TEMP_SAMPLE)/_fRatio);
	}
}

adc_filter_t *get_adc_filter_struct(void)
{
	adc_filter_t *p_adc_filter_struct = &adc_filter_struct;

	return p_adc_filter_struct;
}

void adc_filter_sftimer_init(void)
{
	adc_filter_sftimer = rt_timer_create("adc_filter_sftimer", adc_filter_sftimer_cb, RT_NULL, 100, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
	if(adc_filter_sftimer != RT_NULL)
		rt_timer_start(adc_filter_sftimer);
}

void get_adc(void)
{
	adc_filter_t *adc_filter_struct = get_adc_filter_struct();
	
	LOG_I("ntc1:%d", adc_filter_struct->ntc1_filter_adc);
	LOG_I("ntc2:%d", adc_filter_struct->ntc2_filter_adc);
	LOG_I("bat1:%d", adc_filter_struct->bat1_filter_adc);
	LOG_I("bat2:%d", adc_filter_struct->bat2_filter_adc);
	LOG_I("bat1temp:%d", adc_filter_struct->bat1temp_filter_adc);
	LOG_I("bat2temp:%d", adc_filter_struct->bat2temp_filter_adc);
}
MSH_CMD_EXPORT(get_adc, get adc);



