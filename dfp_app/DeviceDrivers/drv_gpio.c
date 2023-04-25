#include "drv_gpio.h"

void pin_drv_motor_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructures);

	GPIO_InitStructures.Pin = GPIO_PIN_0;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);

	PIN_FAULT_DISABLE;
	PIN_DRV_DISABLE;
	PIN_SLEEP_DISABLE;
	PIN_DRV_PWR_DISABLE;
}

