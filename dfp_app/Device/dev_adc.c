#include "dev_adc.h"
#include "rtthread.h"

volatile static avgf_t avg_filter_para[3] = 
{
	{2048, 2048, {0}, 0, 0, 0.8},				//out1 val
	{2048, 2048, {0}, 0, 0, 0.8},				//out2 val
	{1489, 1489, {0}, 0, 0, 0.5},				//vref
};

static void adc_thread_entry(void *parameter)
{
	uint16_t temp;
	
	while(1)
	{
		for(uint8_t i=0;i<3;i++){
			if(avg_filter_para[i].cnt >= 10){
				avg_filter_para[i].cnt = 0;
				//排序
				for(uint8_t m=0;m<9;m++){
					for(uint8_t n=0;n<(9-m);n++){
						if(avg_filter_para[i].buff[n] > avg_filter_para[i].buff[n+1]){
							temp = avg_filter_para[i].buff[n];
							avg_filter_para[i].buff[n] = avg_filter_para[i].buff[n+1];
							avg_filter_para[i].buff[n+1] = temp;
						}
					}
				}
				//算数平均
				avg_filter_para[i].sum = 0;
				for(uint8_t j=1;j<9;j++){
					avg_filter_para[i].sum += avg_filter_para[i].buff[j];
				}
				avg_filter_para[i].avg = avg_filter_para[i].sum >> 3;
				//一阶低通
				avg_filter_para[i].avg = (uint16_t)(avg_filter_para[i].alpha * (float)avg_filter_para[i].avg + 
							(1 - avg_filter_para[i].alpha) * (float)avg_filter_para[i].avg_1);
				avg_filter_para[i].avg_1 = avg_filter_para[i].avg;
			}else{
				if(i == 0)			avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC_GetSample(OUT1_SAMPLE);
				else if(i == 1)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC_GetSample(OUT2_SAMPLE);
				else if(i == 2)		avg_filter_para[i].buff[avg_filter_para[i].cnt++] = ADC_GetSample(VREF_SAMPLE);
			}
		}
	
		rt_thread_mdelay(10);
	}
}

uint16_t adc_get_val(uint8_t type)
{
	uint16_t temp = 0;
    switch (type)
    {
		case OUT1_SAMPLE:
			temp = avg_filter_para[0].avg;
			break;

		case OUT2_SAMPLE:
			temp = avg_filter_para[1].avg;
			break;

		case VREF_SAMPLE:
			temp = avg_filter_para[2].avg;
			break;
			
        default:
            break;
    }
    
    return temp;
}

static int adc_dev_init(void)
{
	ADC_RegConfig();
	return 0;
}
INIT_DEVICE_EXPORT(adc_dev_init);

static int adc_filter_init(void)
{
	rt_thread_t tid = RT_NULL;

	tid = rt_thread_create("adc", adc_thread_entry, RT_NULL, 1024, 20, 10);
	if (tid != RT_NULL)
        rt_thread_startup(tid);
        
    return 0;
}
INIT_APP_EXPORT(adc_filter_init);


