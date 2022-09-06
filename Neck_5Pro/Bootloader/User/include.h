#ifndef _INCLUDE_H
#define _INCLUDE_H

//System head files
#include "n32g45x.h"

//RTOS head files

//Drives head files
#include "dri_sysclk.h"
//#include "dri_uart.h"
#include "dri_flash.h"
//#include "dri_i2c.h"
//#include "dri_aw21024.h"


//Device head files
//#include "dev_uart.h"
//#include "dev_led.h"

//C/C++ Lib head files
#include <stdio.h>
#include <string.h>

//User head files
//#include "fifo.h"

//Moduel head files


//宏定义

typedef enum
{
	Code_Version_Check = 1,
	Code_Copy,
	App_Run
}STATE;



//外部调用




#endif

