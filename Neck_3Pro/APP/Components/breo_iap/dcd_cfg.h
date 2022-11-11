#ifndef __DCD_CFG_H
#define __DCD_CFG_H

#include "iap_addr_def.h"

/* DCD USER AREA START */
#define DCD_BASE_ADDR 		DCD_DATA_ADDR	 /* @note you must define it for a value */
#define DCD_BASE_SIZE		DCD_DATA_SIZE	 /* @note you must define it for a value */

/* 写GPIO参数的参考值 */
typedef enum{
	DCD_GPIO_A	= 'A',
	DCD_GPIO_B ,
	DCD_GPIO_C ,
	DCD_GPIO_D ,
	DCD_GPIO_E ,
	DCD_GPIO_F 
}DCD_GPIO_PortType;

typedef enum{
    DCD_GPIO_PIN_0  = 0,
    DCD_GPIO_PIN_1,
    DCD_GPIO_PIN_2,
    DCD_GPIO_PIN_3,
    DCD_GPIO_PIN_4,
    DCD_GPIO_PIN_5,
    DCD_GPIO_PIN_6,
    DCD_GPIO_PIN_7,
    DCD_GPIO_PIN_8,
    DCD_GPIO_PIN_9,
    DCD_GPIO_PIN_10,
    DCD_GPIO_PIN_11,
    DCD_GPIO_PIN_12,
    DCD_GPIO_PIN_13,
    DCD_GPIO_PIN_14,
    DCD_GPIO_PIN_15
} DCD_GPIO_PinType;

typedef enum{
	DCD_GPIO_Mode_IN_FLOATING 	  = 0x00,	/*!< Input Floating Mode                   */
	DCD_GPIO_Mode_Out_PP    	  = 0x01, 	/*!< Output Push Pull Mode                 */
	DCD_GPIO_Mode_AF_PP     	  = 0x02,	/*!< Alternate Function Push Pull Mode     */
    DCD_GPIO_Mode_AIN       	  = 0x03, 	/*!< Analog Mode  */
    DCD_GPIO_Mode_Out_OD    	  = 0x11, 	/*!< Output Open Drain Mode                */
    DCD_GPIO_Mode_AF_OD     	  = 0x12,	/*!< Alternate Function Open Drain Mode    */
}DCD_GPIO_ModeType;

typedef enum{
	DCD_GPIO_Speed_HIGH 	= 0,
    DCD_GPIO_Speed_LOW 		= 1,
} DCD_GPIO_SpeedType;						

typedef enum{
    DCD_GPIO_LOW 	= 0,	
    DCD_GPIO_HIGH   = 1
} DCD_GPIO_LevelType;	

typedef enum
{
    DCD_GPIO_No_Pull 	= 0x00,   	/*!< No Pull-up or Pull-down activation  */
    DCD_GPIO_Pull_Up 	= 0x01,  	/*!< Pull-up activation                  */
    DCD_GPIO_Pull_Down 	= 0x02  	/*!< Pull-down activation                */
}DCD_GPIO_PuPdType;

/* 
--Flash wait state
    0: HCLK <= 32M   1: HCLK <= 64M
    2: HCLK <= 96M   3: HCLK <= 128M    4: HCLK <= 144M
    
*	RCC example :
	
	dev_rcc_struct dev_rcc;
	dev_rcc.sysclk  = 144000000;	     
	dev_rcc.pllsrc   = RCC_PLL_SRC_HSE_DIV1;
	dev_rcc.pllmul   = RCC_PLL_MUL_18;
	dev_rcc.pclk1div = RCC_HCLK_DIV4;
	dev_rcc.pclk2div = RCC_HCLK_DIV2;
*/



#endif




