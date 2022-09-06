#include "dev_mp.h"

/*
*@brief 	初始化电源管理设备
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus MP_DEV_Configure(void)
{
	//初始化I2C
	bsp_InitI2C_MP();

	//检测从设备是否正常
	if(MP2651_CheckOk() == SUCCESS)
	{
		//复位配置
		MP2651_WriteByte(0x3C77, MP_DEV_ADDR, ADDR_REG12);
#if 0
		/*
		*@brief 配置REG0E
		*		使能ADC连续转换
		*		开关频率设置为600KHz
		*		超声波模式阈值2设置为102%，阈值1设置为101%
		*/
		MP2651_WriteByte(0x0190, MP_DEV_ADDR, ADDR_REG0E);
		/*
		*@brief 配置REG0F
		*		使能热调节回路，结热调节阈值设为120%
		*		涓流充电电流100mA、恒流充电电流1A、浮充充电电流200mA
		*/
		MP2651_WriteByte(0xF2A4, MP_DEV_ADDR, ADDR_REG0F);
		/*
		*@brief 配置REG10
		*		使能ACGATE，不使能ACGATE控制
		*		温度监控
		*		充电阈值100mV/节，2节电池
		*		输入传感电阻10mΩ，电池电流检测电阻10mΩ
		*/
		MP2651_WriteByte(0x0240, MP_DEV_ADDR, ADDR_REG10);
		/*
		*@brief 配置REG11
		*		过压阈值设置为110%，低压阈值设置为75%
		*		OVP时间为100ns，OVP时电池不放电
		*		输入欠压3.2V，输入过压22.4V，开启过压保护
		*		不使能接收器电流配置
		*/
		MP2651_WriteByte(0x60C8, MP_DEV_ADDR, ADDR_REG11);
		/*
		*@brief 配置REG12
		*		使能充电安全定时器，快充定时器设置为20小时，开启快充定时器扩展，使能充电终止
		*		不使能看门狗
		*		使能DC-DC
		*		使能源模式配置
		*		使能电流环配置
		*		使能充电配置
		*/
		MP2651_WriteByte(0x3C7B, MP_DEV_ADDR, ADDR_REG12);
		/*
		*@brief 配置REG14
		*		充电电流设置为2A
		*/
		MP2651_WriteByte(0x0A00, MP_DEV_ADDR, ADDR_REG14);
		/*
		*@brief 配置REG15
		*		电池(两节)满电电压设置为8.4V
		*/
		MP2651_WriteByte(0x3480, MP_DEV_ADDR, ADDR_REG15);
#else
		//参考胡云飞调参
		MP2651_WriteByte(0x0208, MP_DEV_ADDR, ADDR_REG05);
		MP2651_WriteByte(0x0039, MP_DEV_ADDR, ADDR_REG06);
		MP2651_WriteByte(0x0030, MP_DEV_ADDR, ADDR_REG07);
		MP2651_WriteByte(0x0050, MP_DEV_ADDR, ADDR_REG08);
		MP2651_WriteByte(0x00F9, MP_DEV_ADDR, ADDR_REG09);
		MP2651_WriteByte(0x0028, MP_DEV_ADDR, ADDR_REG0A);
		MP2651_WriteByte(0x3080, MP_DEV_ADDR, ADDR_REG0B);
		MP2651_WriteByte(0x3410, MP_DEV_ADDR, ADDR_REG0C);
		MP2651_WriteByte(0xB199, MP_DEV_ADDR, ADDR_REG0D);
		MP2651_WriteByte(0x0190, MP_DEV_ADDR, ADDR_REG0E);//使能ADC连续转换
		MP2651_WriteByte(0xF244, MP_DEV_ADDR, ADDR_REG0F);
		MP2651_WriteByte(0x0C40, MP_DEV_ADDR, ADDR_REG10);
		MP2651_WriteByte(0x60E8, MP_DEV_ADDR, ADDR_REG11);
		MP2651_WriteByte(0x3C53, MP_DEV_ADDR, ADDR_REG12);
		MP2651_WriteByte(0x0780, MP_DEV_ADDR, ADDR_REG14);
		MP2651_WriteByte(0x4EC0, MP_DEV_ADDR, ADDR_REG15);
		MP2651_WriteByte(0x7B00, MP_DEV_ADDR, ADDR_REG16);
		MP2651_WriteByte(0x0000, MP_DEV_ADDR, ADDR_REG17);
		MP2651_WriteByte(0x0000, MP_DEV_ADDR, ADDR_REG18);
		MP2651_WriteByte(0x0000, MP_DEV_ADDR, ADDR_REG19);
		MP2651_WriteByte(0x0050, MP_DEV_ADDR, ADDR_REG22);
		MP2651_WriteByte(0x01B8, MP_DEV_ADDR, ADDR_REG23);
		MP2651_WriteByte(0x00FB, MP_DEV_ADDR, ADDR_REG24);
		MP2651_WriteByte(0x0348, MP_DEV_ADDR, ADDR_REG25);
		MP2651_WriteByte(0x0272, MP_DEV_ADDR, ADDR_REG26);
		MP2651_WriteByte(0x004E, MP_DEV_ADDR, ADDR_REG27);
		MP2651_WriteByte(0x01F5, MP_DEV_ADDR, ADDR_REG28);
		MP2651_WriteByte(0x01F1, MP_DEV_ADDR, ADDR_REG29);
		MP2651_WriteByte(0x01D7, MP_DEV_ADDR, ADDR_REG2A);
		MP2651_WriteByte(0x000D, MP_DEV_ADDR, ADDR_REG2B);
		MP2651_WriteByte(0x0000, MP_DEV_ADDR, ADDR_REG2C);
		MP2651_WriteByte(0x0000, MP_DEV_ADDR, ADDR_REG2D);
#endif 
		return SUCCESS;
	}
	else
		return ERROR;
}



