#include "pid.h"

void pid_calc(pid_t *pid)
{
	float DelPv;
	float ti,ki;
	float td;
	float kd;
	float out;
	float beta;
 
    pid->Ek = pid->Pv - pid->Sv;   		//得到当前偏差值
    pid->Pout = pid->Kp * pid->Ek; 		//比例输出

	//积分分离
	if(fabs(pid->Ek) <= 50)
		beta = 1;
	else
		beta = 0;

 	pid->SEk += (beta * pid->Ek);        		//历史偏差总和
    ti = pid->T / pid->Ti;
    ki = ti * pid->Kp;
    pid->Iout = beta * ki * pid->SEk;  	//积分输出

	DelPv = pid->Pv - pid->Pv_1;
    td = pid->Td / pid->T;
    kd = pid->Kp * td;
    pid->Dout = kd * DelPv;    			//微分输出
 
    out = pid->Pout + pid->Iout - pid->Dout;
 
	//输出限幅
    if(out > pid->pwmcycle)
    {
        pid->OUT = pid->pwmcycle;
    }
    else if(out < 0)
    {
        pid->OUT = pid->OUT0; 
    }
    else 
    {
        pid->OUT = out;
    }

	pid->Pv_1 = pid->Pv;
}


