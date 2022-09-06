#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rtthread.h"

#define BYTE0(dwTemp)	(*((uint8_t *)(&dwTemp) + 0))
#define BYTE1(dwTemp)	(*((uint8_t *)(&dwTemp) + 1))
#define BYTE2(dwTemp)	(*((uint8_t *)(&dwTemp) + 2))
#define BYTE3(dwTemp)	(*((uint8_t *)(&dwTemp) + 3))


#endif







