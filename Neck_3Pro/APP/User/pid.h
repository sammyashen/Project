#ifndef _PID_H_
#define _PID_H_
 
#include "n32l40x.h"
#include "tiny_os.h"
#include "init.h"
#include <math.h>

typedef struct
{
    float Sv;//�趨ֵ
    float Pv;//��ǰֵ
    //PID��Ŀ
    float Kp;
    float Ti;
    float Td; 
	//��������
	float T; 
	
    float Ek;  //����ƫ��
    float SEk; //��ʷƫ��
	float Pv_1;//��һ�β���ֵ
	
	float Iout;//���ֿ��������
	float Pout;//�������������
	float Dout;//΢�ֿ��������
	
 	float OUT0;//���ƫ��
	float OUT;//PID���
 	u16 pwmcycle;//PWM����
}pid_t;


void pid_calc(pid_t *pid);


#endif


