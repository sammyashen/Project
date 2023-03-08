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

//应答标志
#define ACK_OK					0x80
#define ACK_NO_NEED_UPDATE		0x81
#define ACK_NEED_UPDATE			0x82
#define ACK_SHAKE_HAND			0x83
#define ACK_ERR_SAME_POINT		0xB0
#define ACK_ERR_NOT_SUPPORT		0xB1
#define ACK_ERR_IMU				0xB2
#define ACK_ERR_LIMIT			0xB3
#define ACK_ERR_NOT_READY		0xB4
#define ACK_ERR_LENGTH			0xE0
#define ACK_ERR_CRC16			0xE1
#define ACK_ERR_INVALID_APP		0xE2
#define ACK_ERR_INVALID_FRAME	0xE3
#define ACK_ERR_DP_LENGTH		0xE4
#define ACK_ERR_SIZE			0xE5
#define ACK_ERR_CRC32			0xE6
#define ACK_ERR_CHECKSUM		0xE7
#define ACK_ERR_CRC32_CP		0xE8
#define ACK_ERR_CHECKSUM_CP		0xE9
#define ACK_ERR_UPDATE_STATUS	0xEA
#define ACK_ERR_FRAME_FLAG		0xEB
#define ACK_ERR_INVALID_DATA	0xEC
#define ACK_ERR_STATUS			0xED
#define ACK_ERR_INVALID_CMD		0xEE
#define ACK_FINISHED			0xFF
//CMD标志
#define CMD_UPDATE_RESET		0xF0
#define CMD_UPDATE_SHAKE_HAND	0xF1
#define CMD_UPDATE_INFO			0xF2
#define CMD_UPDATING			0xF3
#define CMD_UPDATED				0xF4
#define CMD_UPDATE_REBOOT		0xF5

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


