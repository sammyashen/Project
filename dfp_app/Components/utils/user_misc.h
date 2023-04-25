#ifndef __USER_MISC_H__
#define __USER_MISC_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
	_UNKNOW = 0,
	_HEX,
	_WORD,
	_CHAR,
	_UCHAR,
	_INT,
	_UINT,
	_FLOAT,
}show_type_e;


#ifdef __cplusplus
}
#endif

#endif 



