#include "include.h"
#define DBG_TAG				"vb_gimbal"
#define DBG_LVL				DBG_LOG
#include <rtdbg.h>

const uint32_t Sine12bit[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,  4095,  4095,
                                0,  0,   0,    0,   0,  0,  0,  0,  0, 0};

static void imu_read(void)
{
	uint8_t id = 0;

	tiny_device_t imu_dev = tiny_device_find("icm42631");
	if(imu_dev != NULL){
		imu_dev->dops[0].read(&(icm42631_ops_data){0x00, WHO_AM_I, &id}, 1);
		rt_kprintf("i2c id = [0x%02X]\r\n", id);

		imu_dev->dops[1].read(&(icm42631_ops_data){0x00, WHO_AM_I, &id}, 1);
		rt_kprintf("spi id = [0x%02X]\r\n", id);
	}
}
MSH_CMD_EXPORT(imu_read, test);

static void pwm_drv_test(void)
{
	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 1000, 5000);
	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_9, GPIO_AF2_TIM1, TIM1, TIM_CH_2, 1000, 6600);
	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_10, GPIO_AF2_TIM1, TIM1, TIM_CH_3, 1000, 7550);
}
MSH_CMD_EXPORT(pwm_drv_test, pwm test);

static void dac_drv_test(void)
{
	dac_tx((uint32_t *)&Sine12bit[0], 32);
}
MSH_CMD_EXPORT(dac_drv_test, dac test);

const static uint8_t buff[2048] = {[0]=0xAA, [1 ... 2046]=0xBB, [2047]=0xCC};
static void flash_drv_test(void)
{
	uint32_t addr = 0x0801E000;
	uint8_t rx_buf[3] = {0};

	flash_erase(addr);
	flash_write(addr, buff, 2048);
	flash_read(addr, &rx_buf[0], 1);
	flash_read(addr+1024, &rx_buf[1], 1);
	flash_read(addr+2047, &rx_buf[2], 1);
	rt_kprintf("rx_buf[0]=[%02X], rx_buf[1]=[%02X], rx_buf[2]=[%02X]\r\n", rx_buf[0], rx_buf[1], rx_buf[2]);
}
MSH_CMD_EXPORT(flash_drv_test, flash test);

static void adc_drv_test(void)
{
	rt_kprintf("vref val=[%d]\r\n", adc_get_val(VREF_SAMPLE));
	rt_kprintf("out1 val=[%d]\r\n", adc_get_val(OUT1_SAMPLE));
	rt_kprintf("out2 val=[%d]\r\n", adc_get_val(OUT2_SAMPLE));
}
MSH_CMD_EXPORT(adc_drv_test, adc test);

static void pin_test(void)
{
	PIN_FAULT_ENABLE;
	PIN_DRV_ENABLE;
	PIN_SLEEP_ENABLE;
	PIN_DRV_PWR_ENABLE;
}
MSH_CMD_EXPORT(pin_test, pin test);

int32_t main(void)
{
	dac_init();
	pin_drv_motor_init();
	bsp_SetTIMOutPWM(GPIOA, GPIO_PIN_8, GPIO_AF2_TIM1, TIM1, TIM_CH_1, 2, 5000);
	while(1)
	{
		rt_uint8_t buf[30] = {0};
		rt_uint16_t size = 0;
		size = uart_read(DEV_UART2, buf, sizeof(buf));
		if(size != 0)
		{
			uart_write(DEV_UART2, buf, size);
		}

//		LOG_I("hello");
//		LOG_W("hello");
//		LOG_E("hello");
		rt_thread_mdelay(10);
	}
}

