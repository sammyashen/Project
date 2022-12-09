#ifndef _DEV_BLE_H
#define _DEV_BLE_H

#include "include.h"

#define BLE_EVT			0xBB

//frame base msg
#define FRAME_HEAD		0x4E
#define FRAME_TAIL		0x4E
#define VERSION			0x01
#define DEVICE			0x13

//frame err
#define NO_ERR			0x00
#define CMD_ERR			0x01
#define FRAME2SHORT		0x02
#define SUM_ERR			0x03
#define FLAG_ERR		0x06
#define FRAME2LONG		0x07
#define PROTOCOL_ERR	0x10

#endif





