#ifndef _LADRC_H
#define _LADRC_H

#include "math.h"
#include "stdlib.h"

typedef struct
{
	float v1_k,v2_k;
	float v1_k1,v2_k1;
	float r;
	float h;
	float z1_k,z2_k,z3_k;
	float z1_k1,z2_k1,z3_k1;
	float w0,wc,b0,u;
}ladrc_t;

void ladrc_ctrl(ladrc_t *ladrc_struct, float Sv, float Pv);

#endif



