#include <esp_protocol.h>
#include "dev_uart.h"

uint8_t data_send_to_esp(uint8_t *dat, uint8_t dat_len)
{
	//USER TODO
	uint8_t len;

	len =  uart_write(DEV_UART4, dat, dat_len);

	return len;
}



