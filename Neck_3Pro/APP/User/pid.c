#include "pid.h"

void pid_calc(pid_t *pid)
{
	float DelPv;
	float ti,ki;
	float td;
	float kd;
	float out;
	float beta;
 
    pid->Ek = pid->Pv - pid->Sv;   		//�õ���ǰƫ��ֵ
    pid->Pout = pid->Kp * pid->Ek; 		//�������

	//���ַ���
	if(fabs(pid->Ek) <= 50)
		beta = 1;
	else
		beta = 0;

 	pid->SEk += (beta * pid->Ek);        		//��ʷƫ���ܺ�
    ti = pid->T / pid->Ti;
    ki = ti * pid->Kp;
    pid->Iout = beta * ki * pid->SEk;  	//�������

	DelPv = pid->Pv - pid->Pv_1;
    td = pid->Td / pid->T;
    kd = pid->Kp * td;
    pid->Dout = kd * DelPv;    			//΢�����
 
    out = pid->Pout + pid->Iout - pid->Dout;
 
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

	pid->Pv_1 = pid->Pv;
}


