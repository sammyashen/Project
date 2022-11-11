#include "drv_gpio.h"
#include "init.h"

/**
 * @brief  	  : charge io configure func.
 * @param[1]  : none.
 * @return    : none.
 */
static void charge_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_GPIOC, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructures.GPIO_Pull = GPIO_Pull_Down;
	GPIO_InitStructures.Pin = GPIO_PIN_14 | GPIO_PIN_15;//charging-->set,discharge-->clr
	GPIO_InitPeripheral(GPIOC, &GPIO_InitStructures);

	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_2;//enable charge-->clr,disable charge-->set
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);
	CHARGE_DISABLE;
}

/**
 * @brief  	  : enable charge io func.
 * @param[1]  : none.
 * @return    : none.
 */
void enable_charge(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructures.Pin = GPIO_PIN_2;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);
}

/**
 * @brief  	  : disable charge io func.
 * @param[1]  : none.
 * @return    : none.
 */
void disable_charge(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_2;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);
	CHARGE_DISABLE;
}


/**
 * @brief  	  : mcu power io configure func.
 * @param[1]  : none.
 * @return    : none.
 */
static void mcu_pwr_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_9;//enable mcu pwr-->set,disable mcu pwr-->clr
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);
	MCU_PWR_DISABLE;
}

static void ble_pwr_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_13;//enable ble pwr-->set,disable ble pwr-->clr
	GPIO_InitPeripheral(GPIOC, &GPIO_InitStructures);
	BLE_PWR_DISABLE;
}

/**
 * @brief  	  : motor control io configure func.
 * @param[1]  : none.
 * @return    : none.
 */
static void motor_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_10 | GPIO_PIN_11;//enable m11,m12-->set,disable m11,m12-->clr
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);

	GPIO_InitStructures.Pin = GPIO_PIN_12 | GPIO_PIN_13;//enable m21,m22-->set,disable m21,m22-->clr
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);

	GPIO_InitStructures.Pin = GPIO_PIN_11 | GPIO_PIN_12;//enable m31,m32-->set,disable m31,m32-->clr
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructures);

	MOTOR_1_FORWARD;
	MOTOR_2_FORWARD;
	MOTOR_3_FORWARD;
}

/**
 * @brief  	  : psw io configure func.
 * @param[1]  : none.
 * @return    : none.
 */
static void psw_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOD, ENABLE);
	RCC_ConfigHse(RCC_HSE_DISABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Input;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.GPIO_Pull = GPIO_Pull_Up;
	GPIO_InitStructures.Pin = GPIO_PIN_14 | GPIO_PIN_15;//psw1,psw2 arrivaled-->clr,psw1,psw2 not arrivaled-->set
	GPIO_InitPeripheral(GPIOD, &GPIO_InitStructures);
}

/**
 * @brief  	  : heat io configure func.
 * @param[1]  : none.
 * @return    : none.
 */
//static void heat_io_init(void)
//{
//	GPIO_InitType GPIO_InitStructures;
//
//	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
//	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
//	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
//	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
//	GPIO_InitStructures.Pin = GPIO_PIN_6;//enable heat-->set,disable heat-->clr
//	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructures);
//	HEAT_OFF;
//}

static void led_io_init(void)
{
	GPIO_InitType GPIO_InitStructures;

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);
	GPIO_InitStructures.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructures.GPIO_Current = GPIO_DC_4mA;
	GPIO_InitStructures.GPIO_Pull = GPIO_No_Pull;
	GPIO_InitStructures.GPIO_Slew_Rate = GPIO_Slew_Rate_Low;
	GPIO_InitStructures.Pin = GPIO_PIN_15;
	GPIO_InitPeripheral(GPIOA, &GPIO_InitStructures);
	GPIO_InitStructures.Pin = GPIO_PIN_3 | GPIO_PIN_5;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);
	BT_LED_OFF;
	RED_LED_OFF;
	GREEN_LED_OFF;
}

/**
 * @brief  	  : gpio device register func.
 * @param[1]  : none.
 * @return    : none.
 */
static void gpio_device_register(void)
{
	charge_io_init();
	mcu_pwr_io_init();
	ble_pwr_io_init();
	motor_io_init();
	psw_io_init();
//	heat_io_init();
	led_io_init();
}

debug_initcall(gpio_device_register);


