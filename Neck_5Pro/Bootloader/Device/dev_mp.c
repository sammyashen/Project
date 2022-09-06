#include "dev_mp.h"

/*
*@brief 	��ʼ����Դ�����豸
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus MP_DEV_Configure(void)
{
	//��ʼ��I2C
	bsp_InitI2C_MP();

	//�����豸�Ƿ�����
	if(MP2651_CheckOk() == SUCCESS)
	{
		//��λ����
		MP2651_WriteByte(0x3C77, MP_DEV_ADDR, ADDR_REG12);
#if 0
		/*
		*@brief ����REG0E
		*		ʹ��ADC����ת��
		*		����Ƶ������Ϊ600KHz
		*		������ģʽ��ֵ2����Ϊ102%����ֵ1����Ϊ101%
		*/
		MP2651_WriteByte(0x0190, MP_DEV_ADDR, ADDR_REG0E);
		/*
		*@brief ����REG0F
		*		ʹ���ȵ��ڻ�·�����ȵ�����ֵ��Ϊ120%
		*		���������100mA������������1A�����������200mA
		*/
		MP2651_WriteByte(0xF2A4, MP_DEV_ADDR, ADDR_REG0F);
		/*
		*@brief ����REG10
		*		ʹ��ACGATE����ʹ��ACGATE����
		*		�¶ȼ��
		*		�����ֵ100mV/�ڣ�2�ڵ��
		*		���봫�е���10m������ص���������10m��
		*/
		MP2651_WriteByte(0x0240, MP_DEV_ADDR, ADDR_REG10);
		/*
		*@brief ����REG11
		*		��ѹ��ֵ����Ϊ110%����ѹ��ֵ����Ϊ75%
		*		OVPʱ��Ϊ100ns��OVPʱ��ز��ŵ�
		*		����Ƿѹ3.2V�������ѹ22.4V��������ѹ����
		*		��ʹ�ܽ�������������
		*/
		MP2651_WriteByte(0x60C8, MP_DEV_ADDR, ADDR_REG11);
		/*
		*@brief ����REG12
		*		ʹ�ܳ�簲ȫ��ʱ������䶨ʱ������Ϊ20Сʱ��������䶨ʱ����չ��ʹ�ܳ����ֹ
		*		��ʹ�ܿ��Ź�
		*		ʹ��DC-DC
		*		ʹ��Դģʽ����
		*		ʹ�ܵ���������
		*		ʹ�ܳ������
		*/
		MP2651_WriteByte(0x3C7B, MP_DEV_ADDR, ADDR_REG12);
		/*
		*@brief ����REG14
		*		����������Ϊ2A
		*/
		MP2651_WriteByte(0x0A00, MP_DEV_ADDR, ADDR_REG14);
		/*
		*@brief ����REG15
		*		���(����)�����ѹ����Ϊ8.4V
		*/
		MP2651_WriteByte(0x3480, MP_DEV_ADDR, ADDR_REG15);
#else
		//�ο����Ʒɵ���
		MP2651_WriteByte(0x0208, MP_DEV_ADDR, ADDR_REG05);
		MP2651_WriteByte(0x0039, MP_DEV_ADDR, ADDR_REG06);
		MP2651_WriteByte(0x0030, MP_DEV_ADDR, ADDR_REG07);
		MP2651_WriteByte(0x0050, MP_DEV_ADDR, ADDR_REG08);
		MP2651_WriteByte(0x00F9, MP_DEV_ADDR, ADDR_REG09);
		MP2651_WriteByte(0x0028, MP_DEV_ADDR, ADDR_REG0A);
		MP2651_WriteByte(0x3080, MP_DEV_ADDR, ADDR_REG0B);
		MP2651_WriteByte(0x3410, MP_DEV_ADDR, ADDR_REG0C);
		MP2651_WriteByte(0xB199, MP_DEV_ADDR, ADDR_REG0D);
		MP2651_WriteByte(0x0190, MP_DEV_ADDR, ADDR_REG0E);//ʹ��ADC����ת��
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



