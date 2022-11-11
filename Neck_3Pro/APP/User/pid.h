#ifndef _PID_H_
#define _PID_H_
 
#include "n32l40x.h"
#include "tiny_os.h"
#include "init.h"
#include <math.h>

typedef struct
{
    float Sv;//设定值
    float Pv;//当前值
    //PID项目
    float Kp;
    float Ti;
    float Td; 
	//采样周期
	float T; 
	
    float Ek;  //本次偏差
    float SEk; //历史偏差
	float Pv_1;//上一次测量值
	
	float Iout;//积分控制器输出
	float Pout;//比例控制器输出
	float Dout;//微分控制器输出
	
 	float OUT0;//输出偏置
	float OUT;//PID输出
 	u16 pwmcycle;//PWM周期
}pid_t;


void pid_calc(pid_t *pid);


#endif


