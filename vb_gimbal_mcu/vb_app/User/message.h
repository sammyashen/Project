#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define GET_START_BIT		(0x00)
#define GET_CMD_BIT			(0x01)
#define GET_FRAME_BIT		(0x02)
#define GET_LENGTH_BIT		(0x03)
#define GET_DATA			(0x04)
#define GET_CRC_BIT			(0x05)
#define GET_FRAME_FINISH	(0x06)

#ifdef __cplusplus
}
#endif

#endif 



