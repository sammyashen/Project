#include "pid.h"

void pid_calc(pid_t *pid)
{
	float DelEk;
	float ti,ki;
	float td;
	float kd;
	float out;
 
    pid->Ek = pid->Sv - pid->Pv;   		//得到当前偏差值
    pid->Pout = pid->Kp * pid->Ek; 		//比例输出

	//抗积分饱和
 	if(pid->OUT >= pid->pwmcycle)
 	{
 		if(pid->Ek < 0)
 			pid->SEk += pid->Ek;
 	}
 	else if(pid->OUT <= pid->OUT0)
 	{
 		if(pid->Ek > 0)
 			pid->SEk += pid->Ek;
 	}
	else
		pid->SEk += pid->Ek;
// 	pid->SEk += pid->Ek;        		//历史偏差总和
    DelEk = pid->Ek - pid->Ek_1;  		//最近两次偏差之差
 
    ti = pid->T / pid->Ti;
    ki = ti * pid->Kp;
    pid->Iout = ki * pid->SEk;  	//积分输出

    td = pid->Td / pid->T;
    kd = pid->Kp * td;
    pid->Dout = kd * DelEk;    			//微分输出
 
    out = pid->Pout + pid->Iout + pid->Dout;
 
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

    pid->Ek_1 = pid->Ek;  				//更新偏差
}


