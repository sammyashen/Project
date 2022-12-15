#include "include.h"
#include <stdlib.h>
#include <math.h>
#include "n32g45x.h"

/*
*PID�ŵ㣺������ϵͳģ�ͣ������������
*PIDȱ�㣺1������ȡֵ��ʹ��de/dt���ܺܺõ�׷��΢���ź�					    		--->TD
		  2����Ȩ��(����)����δ�������										--->NLSEF
		  3�����ַ����ĸ�����(��������ʱ����ǿ�������𳬵�)							--->ESO
*/

//TD
float r = 200000;	//���ٸ�������
					//rԽ�󣬿�����Խ�ã������������𳬵��ͷ�ɢ����100~500�����û�����ͬ��
float h = 0.003;	//�˲����ӡ���������(3ms)
					//hԽ�󣬾�̬���ԽС���տ�ʼ�����ġ�������ԽС����ʼ�����ԽС������Ӱ������ԡ�
//ESO
float b = 2;		//ϵͳϵ����һ������1��2
					//bԽС����ʱ��Խ�̣�����̫С�ᵼ����
float delta = 0.02; //deltaΪfal(e, alpha, delta)���������������ȣ��̶���������5h <= delta <= 10h
					//�۲�������omega = 10
float beta01 = 30;	//�̶���ʽ��beta01 = 3*omega
float beta02 = 300;	//�̶���ʽ��beta02 = 3*(omega^2)
float beta03 = 1000;//�̶���ʽ��beta03 = omega^3
//NLSEF
float alpha0 = 0.4;
float alpha1 = 0.8;	//0<alpha1<1
float alpha2 = 1.2;	//1<alpha2
float beta0 = 0;
float beta1 = -1.8;	//���������ź����棬�൱��Kp
float beta2 = -0.02;//��������΢���ź����棬�൱��Kd

int sign(float x)
{
	if(x > 1e-6)		return 1;
	else if(x < -1e-6)	return -1;
	else 			return 0;
}

//fhan�����ۺϷ���:x1(��������)��x2(���������΢���ź�)
#define FIRST_FHAN
float fhan(float x1, float x2, float r, float h)
{
#ifdef FIRST_FHAN		//��һ�׷���
/*
	TD���̣�
	fh = fhan(x1(k) - v(k), x2(k), r, h)
	x1(k+1) = x1(k) + hx2(k)
	x2(k+1) = x2(k) + hfh

	ESO���̣�
	fsg(x,d) = (sign(x+d)-sign(x-d))/2
	d = r*h^2
	a0 = h*x2
	y = x1+a0
	a1 = (d*(d+8|y|))������
	a2 = a0+sign(y)*(a1-d)/2
	a = (a0+y)*fsg(y,d)+a2*(1-fsg(y,d))
	fhan = -r*(a/d)*fsg(y,d) - r*sign(a)*(1-fsg(a,d))
	e = z1 - y
	fe = fal(e, 0.5, delta)
	fe1 = fal(e, 0.25, delta)
	z1 = z1 + h*(z2 - beta01*e)
	z2 = z2 + h*(z3 - beta02*fe + b*u)
	z3 = z3 + h*(-beta03*fe1)

	sign(x) = 1(x>0),-1(x<0),0(x=0)

	NLSEF���̣�
	��һ�֣�u0 = beta1*e1 + beta2*e2
	�ڶ��֣�u0 = beta1*fal(e1, alpha1, delta) + beta2*fal(e2, alpha2, delta), 0<a1<1<a2
	�����֣�u0 = -fhan(e1, e2, r, h)
	�����֣�u0 = -fhan(e1, ce2, r, h)
*/
	float d = 0;
	float a = 0;
	float a0 = 0;
	float a1 = 0;
	float a2 = 0;
	float y = 0;
	float fhan = 0;

	d = r * h * h;
	a0 = h * x2;
	y = x1 + a0;
	a1 = sqrtf(d * (d + 8 * fabsf(y)));
	a2 = a0 + sign(y) * (a1 - d) / 2.0;
	a = (a0 + y) * ((sign(y + d) - sign(y - d)) / 2.0) + a2 * (1 - ((sign(y + d) - sign(y - d)) / 2.0));
	fhan = -r * (a / d) * ((sign(y + d) - sign(y - d)) / 2.0) - r * sign(a) * (1 - ((sign(a + d) - sign(a - d)) / 2.0));
#else 					//�ڶ��׷���
	float delta_a = 0;
	float delta_a0 = 0;
	float y = 0;
	float a0 = 0;
	float a = 0;
	float fhan = 0;

	delta_a = r * h;
	delta_a0 = delta_a * h;
	y = x1 + x2 * h;
	a0 = sqrtf(delta_a * delta_a + 8 * r * fabsf(y));
	if(fabsf(y) <= delta_a0)
		a = x2 + y / h;
	else
		a = x2 + 0.5 * (a0 - delta_a) * sign(y);
	if(fabsf(a) <= delta_a)
		fhan = -r * a / delta_a;
	else
		fhan = -r * sign(a);
#endif

	return fhan;
}

float fal(float e, float alpha, float delta)
{
	float ret = 0;

	if(delta >= fabsf(e))
		ret = e / powf(delta, (1.0 - alpha));
	else
		ret = powf(fabsf(e), alpha) * sign(e);

	return ret;
}

//�м������
//TD
float x1 = 0;	//��������
float x2 = 0;	//���������΢��
//ESO
float e = 0;	//���
float z1 = 0;	//���ٷ���ֵ
float z2 = 0;	//���ٷ���ֵ��΢��
float z3 = 0; 	//ϵͳ���Ŷ�
//NLSEF
float u = 0;
float adrc_calc(float Sv, float Pv)
{
	float u0 = 0;
	float e0 = 0;
	float e1 = 0;
	float e2 = 0;

//TD
	x1 += h * x2;
	x2 += h * fhan(x1 - Sv, x2, r, h);
//ESO
	e = z1 - Pv;
	z1 += h * (z2 - beta01 * e);
	z2 += h * (z3 - beta02 * fal(e, 0.5, delta) + b * u);
	z3 += h * (-beta03 * fal(e, 0.25, delta));
//�޷�
	if(z1 >= 30000)			z1 = 30000;
	else if(z1 <= -30000)	z1 = -30000;
	if(z2 >= 30000)			z2 = 30000;
	else if(z2 <= -30000)	z2 = -30000;
	if(z3 >= 30000)			z3 = 30000;
	else if(z3 <= -30000)	z3 = -30000;
//NLSEF
	e0 += e1;
	e1 = x1 - z1;
	e2 = x2 - z2;

	u0 = beta0*fal(e0, alpha0, delta) + beta1*fal(e1, alpha1, delta) + beta2*fal(e2, alpha2, delta);

	u = u0 - z3 / b;

	return u0;
}

