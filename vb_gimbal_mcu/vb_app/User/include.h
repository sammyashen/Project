#ifndef _INCLUDE_H
#define _INCLUDE_H

#include "hc32_ddl.h"
#include "tiny_os.h"
#include "init.h"
#include "state_machine.h"
#include "crc.h"
#include "message.h"

#include "drv_flash.h"
#include "drv_uart.h"

#include "dev_uart.h"

#define BOOT_EVT 	0xBB

typedef struct{
	uint8_t start;
	uint8_t cmd;
	uint8_t frame;
	uint8_t length;
	uint8_t data[256];
	uint16_t crc;
	uint8_t reserve[2];
}frame_single_t;

typedef struct{
	_fifo_t rx_fifo;
	frame_single_t frame_single_buf[5];
}__attribute__((packed, aligned(4))) frame_recv_t;
extern frame_recv_t frame_recv_struct;

#endif


