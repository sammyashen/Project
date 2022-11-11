#include "debug.h"

static task_t debug_task;
static FlagStatus debug_on = RESET;

void debug_test(int argc, char **argv)
{
	if(atoi(argv[1]) == 0x00)	debug_on = RESET;
	else						debug_on = SET;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
debug_test, debug_test, debug test);

static void debug_task_cb(void *para, uint32_t evt)
{
	uint8_t send_data[30];
	uint8_t send_cnt = 0;
	uint16_t temp = 0;
	send_cnt = 0;
	uint8_t sum = 0;

	if(debug_on == SET){
		memset(send_data, 0, sizeof(send_data));
		send_data[send_cnt++] = 0xAA;
		send_data[send_cnt++] = 0x01;
		send_data[send_cnt++] = 0xAF;
		send_data[send_cnt++] = 0xF1;
		send_data[send_cnt++] = 0x12;
		temp = ADC_GetSample(VREF_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(NTC_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(BAT1VAL_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(BAT2VAL_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(BAT1TEMP_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(BAT2TEMP_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(M1VAL_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(M2VAL_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		temp = ADC_GetSample(M3VAL_SAMPLE);
		send_data[send_cnt++] = BYTE1(temp);
		send_data[send_cnt++] = BYTE0(temp);
		
		for(uint8_t i=0;i<send_cnt;i++)
			sum += send_data[i];
		send_data[send_cnt++] = sum;
		uart_write(DEV_UART1, send_data, send_cnt);
	}
}

static void debug_init(void)
{
	tiny_timer_create(&debug_task, debug_task_cb, NULL);
    tiny_timer_start(&debug_task, TIMER_FOREVER, 100);
}
app_initcall(debug_init);


