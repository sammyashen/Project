#ifndef _DRI_MP2651_H
#define _DRI_MP5651_H

#include "n32g45x.h"
#include "dri_i2c_mp.h"

#define MP2651

#ifdef MP2651
	#define MP_DEV_NAME			"MP2651"
	#define MP_DEV_ADDR			0x10	//设备1地址
#endif

//外部接口
ErrorStatus MP2651_CheckOk(void);
ErrorStatus MP2651_ReadByte(uint16_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress);
ErrorStatus MP2651_WriteByte(uint16_t _usWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress);




#endif


