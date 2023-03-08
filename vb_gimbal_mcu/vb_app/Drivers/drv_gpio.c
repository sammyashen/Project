#include "drv_gpio.h"

void led_ds1_init(void)
{
	stc_port_init_t stcPortInit;
	
	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
	PORT_Init(PortB, Pin14, &stcPortInit);

	PORT_SetBits(PortB, Pin14);
}

