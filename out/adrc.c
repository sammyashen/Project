#include "include.h"
#include <stdlib.h>
#include <math.h>
#include "n32g45x.h"

/*
*PID优点：不依赖系统模型，靠误差控制误差
*PID缺点：1、误差的取值，使用de/dt不能很好的追踪微分信号					    		--->TD
		  2、加权和(线性)策略未必是最好										--->NLSEF
		  3、积分反馈的副作用(积分作用时滞性强，会引起超调)							--->ESO
*/

//TD
float r = 200000;	//快速跟踪因子
					//r越大，快速性越好，但是容易引起超调和发散；在100~500内作用基本相同。
float h = 0.003;	//滤波因子、采样周期(3ms)
					//h越大，静态误差越小，刚开始带来的“超调”越小，初始的误差越小；但会影响快速性。
//ESO
float b = 2;		//系统系数，一般设置1或2
					//b越小调节时间越短，但是太小会导致震荡
float delta = 0.02; //delta为fal(e, alpha, delta)函数的线性区间宽度（固定参数）：5h <= delta <= 10h
					//观测器带宽omega = 10
float beta01 = 30;	//固定公式：beta01 = 3*omega
float beta02 = 300;	//固定公式：beta02 = 3*(omega^2)
float beta03 = 1000;//固定公式：beta03 = omega^3
//NLSEF
float alpha0 = 0.4;
float alpha1 = 0.8;	//0<alpha1<1
float alpha2 = 1.2;	//1<alpha2
float beta0 = 0;
float beta1 = -1.8;	//跟踪输入信号增益，相当于Kp
float beta2 = -0.02;//跟踪输入微分信号增益，相当于Kd

int sign(float x)
{
	if(x > 1e-6)		return 1;
	else if(x < -1e-6)	return -1;
	else 			return 0;
}

//fhan最速综合方程:x1(跟踪输入)、x2(跟踪输入的微分信号)
#define FIRST_FHAN
float fhan(float x1, float x2, float r, float h)
{
#ifdef FIRST_FHAN		//第一套方程
/*
	TD方程：
	fh = fhan(x1(k) - v(k), x2(k), r, h)
	x1(k+1) = x1(k) + hx2(k)
	x2(k+1) = x2(k) + hfh

	ESO方程：
	fsg(x,d) = (sign(x+d)-sign(x-d))/2
	d = r*h^2
	a0 = h*x2
	y = x1+a0
	a1 = (d*(d+8|y|))开根号
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

	NLSEF方程：
	第一种：u0 = beta1*e1 + beta2*e2
	第二种：u0 = beta1*fal(e1, alpha1, delta) + beta2*fal(e2, alpha2, delta), 0<a1<1<a2
	第三种：u0 = -fhan(e1, e2, r, h)
	第四种：u0 = -fhan(e1, ce2, r, h)
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
#else 					//第二套方程
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

//中间变量区
//TD
float x1 = 0;	//跟踪输入
float x2 = 0;	//跟踪输入的微分
//ESO
float e = 0;	//误差
float z1 = 0;	//跟踪反馈值
float z2 = 0;	//跟踪反馈值的微分
float z3 = 0; 	//系统总扰动
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
//限幅
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

