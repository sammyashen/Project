#include "drv_motor_fg.h"

static void Motor_FGIO_Config(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.Pin = GPIO_PIN_1;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
}

static void PA6_EXTI_Configure(void)
{
	EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_ConfigEXTILine(GPIOA_PORT_SOURCE, GPIO_PIN_SOURCE6);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE6;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void PB1_EXTI_Configure(void)
{
	EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    GPIO_ConfigEXTILine(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE1);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE1;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void motor_fg_configure(void)
{
	Motor_FGIO_Config();
	PA6_EXTI_Configure();
	PB1_EXTI_Configure();
	/* TIM Configer */
	bsp_SetTIMforInt(TIM5, 100000, 1, 1);
	TIM_Enable(TIM5, DISABLE);
	bsp_SetTIMforInt(TIM6, 100000, 1, 1);
	TIM_Enable(TIM6, DISABLE);
}
device_initcall(motor_fg_configure);


