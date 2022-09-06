#include "drv_gpio.h"

static void pwr_ctrl_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_13;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	PWR_OFF;
}

static void esp32_pwr_ctrl_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);

	ESP32_PWR_OFF;
}

static void heat_ctrl_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

	//加热片使能脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_4;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

	HEAT1_OFF;
	HEAT2_OFF;
}

static void motor_ctrl_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_GPIOC, ENABLE);

	//升压使能脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_2;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	//电机转向控制脚
	GPIO_InitStructure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);

	//行程开关检测脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	BOOST_DISABLE;
	MOTOR_TRA_FORWARD;
	MOTOR_MASS_FORWARD;
}

static void usb_input_detect_gpio_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_15;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

static void dev_gpio_all_cfg(void)
{
	pwr_ctrl_gpio_init();
	esp32_pwr_ctrl_gpio_init();
	heat_ctrl_gpio_init();
	usb_input_detect_gpio_init();
	motor_ctrl_gpio_init();
}

device_initcall(dev_gpio_all_cfg);


