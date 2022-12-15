#include "ladrc.h"

//sign函数
static float sign(float input)
{
	float output = 0;
	
	if(input > 0){
		output = 1;
	}
	else if(input == 0){
		output = 0;
	}
	else{
		output = -1;
	}
	
	return output;	
}

//最速综合函数
static float fhan(float x1, float x2, float r0, float h0)  
{
	float d = 0,a0 = 0,y = 0,a1 = 0,a2 = 0,sy = 0,a = 0,sa = 0;
	float ret = 0;  
	
	d = r0*h0*h0;
	a0 = h0*x2;
	y = x1 + a0;
	a1 = sqrt(d*(d + 8*fabs(y)));
	a2 = a0 + sign(y)*(a1 - d)*0.5;
	sy = (sign(y + d) - sign(y - d))*0.5;
	a = (a0 + y - a2)*sy + a2;
	sa = (sign(a + d) - sign(a - d))*0.5;
	ret = -r0*(a/d - sign(a))*sa - r0*sign(a);
	
	return ret;	
}

//跟踪微分，安排过渡过程
static void ladrc_TD(ladrc_t *ladrc_struct, float expect)
{
	float fh = 0;
	
	fh = fhan(ladrc_struct->v1_k - expect, ladrc_struct->v2_k, ladrc_struct->r, ladrc_struct->h);
	ladrc_struct->v1_k1 = ladrc_struct->v1_k + (ladrc_struct->v2_k*ladrc_struct->h);
	ladrc_struct->v2_k1 = ladrc_struct->v2_k + fh*ladrc_struct->h;
	ladrc_struct->v1_k  = ladrc_struct->v1_k1;
	ladrc_struct->v2_k  = ladrc_struct->v2_k1;
}

//扩张状态观测器
static void ladrc_ESO(ladrc_t *ladrc_struct, float feedback)
{
	float beta_01, beta_02, beta_03;
	float error;
	
	beta_01 = 3*ladrc_struct->w0;
	beta_02 = 3*ladrc_struct->w0*ladrc_struct->w0;
	beta_03 = ladrc_struct->w0*ladrc_struct->w0*ladrc_struct->w0;
	error = ladrc_struct->z1_k - feedback;
	ladrc_struct->z1_k1 = ladrc_struct->z1_k + ladrc_struct->h*(ladrc_struct->z2_k - beta_01*error);
  	ladrc_struct->z2_k1 = ladrc_struct->z2_k + ladrc_struct->h*(ladrc_struct->z3_k - beta_02*error + ladrc_struct->b0*ladrc_struct->u);
	ladrc_struct->z3_k1 = ladrc_struct->z3_k - beta_03*error*ladrc_struct->h;
	ladrc_struct->z1_k = ladrc_struct->z1_k1;
	ladrc_struct->z2_k = ladrc_struct->z2_k1;
	ladrc_struct->z3_k = ladrc_struct->z3_k1;
}

//线性控制律
static void ladrc_LSEF(ladrc_t *ladrc_struct)
{
	float Kp, Kd, e1, e2, u0;
	
	Kp = ladrc_struct->wc*ladrc_struct->wc;
	Kd = 2*ladrc_struct->wc;
	
	e1 = ladrc_struct->v1_k1 - ladrc_struct->z1_k1;
	e2 = ladrc_struct->v2_k1 - ladrc_struct->z2_k1;
	u0 = Kp*e1 + Kd*e2;
	ladrc_struct->u = (u0 - ladrc_struct->z3_k1)/ladrc_struct->b0;
}

void ladrc_ctrl(ladrc_t *ladrc_struct, float Sv, float Pv)  
{
	ladrc_TD(ladrc_struct, Sv);
	ladrc_ESO(ladrc_struct, Pv);
	ladrc_LSEF(ladrc_struct);
}




