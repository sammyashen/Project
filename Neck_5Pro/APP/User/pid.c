#include "pid.h"

void pid_calc(pid_t *pid)
{
	float DelEk;
	float ti,ki;
	float td;
	float kd;
	float out;
 
    pid->Ek = pid->Sv - pid->Pv;   		//�õ���ǰƫ��ֵ
    pid->Pout = pid->Kp * pid->Ek; 		//�������

	//�����ֱ���
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
// 	pid->SEk += pid->Ek;        		//��ʷƫ���ܺ�
    DelEk = pid->Ek - pid->Ek_1;  		//�������ƫ��֮��
 
    ti = pid->T / pid->Ti;
    ki = ti * pid->Kp;
    pid->Iout = ki * pid->SEk;  	//�������

    td = pid->Td / pid->T;
    kd = pid->Kp * td;
    pid->Dout = kd * DelEk;    			//΢�����
 
    out = pid->Pout + pid->Iout + pid->Dout;
 
	//����޷�
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

    pid->Ek_1 = pid->Ek;  				//����ƫ��
}


