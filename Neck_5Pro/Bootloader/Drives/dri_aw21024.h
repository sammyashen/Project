#ifndef _DRI_AW21024_H
#define _DRI_AW21024_H

#include "n32g45x.h"
#include "dri_i2c.h"

#define AW21024

#ifdef AW21024
	#define AW_DEV_NAME			"AW21024"
	#define AW_DEV_ADDR			0x30	//�豸1��ַ
	#define AW_DEV2_ADDR		0x34	//�豸2��ַ
#endif

//�ⲿ�ӿ�
ErrorStatus AW21024_CheckOk(void);
ErrorStatus AW21024_ReadByte(uint8_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress);
ErrorStatus AW21024_WriteByte(uint8_t _ucWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress);




#endif

