#ifndef _YMODEM_H_
#define _YMODEM_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rtthread.h"

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define TIMEOUT					(50)
#define MAX_PACK_SIZE			(1030)
#define FILENAME_LEN			(16)

typedef enum{
	FREE = 0,
	WAIT_START_PACK,
	RYM_DELAY_st,
	WAIT_DATA_PACK,
	ERROR_DATA,
	WAIT_EOT,
	RYM_DELAY_nd,
	WAIT_END_PACK,
	END_Ymodem,
}eYmodem_Work;

typedef struct
{
	eYmodem_Work ymodem_sta;
	char filename[FILENAME_LEN];
	rt_uint32_t filesize;
	rt_uint32_t timeout;
}ymodem_t;


#endif






