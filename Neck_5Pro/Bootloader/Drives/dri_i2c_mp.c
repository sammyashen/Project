/*
*@brief MP2651 I2C ����
*/
#include "dri_i2c_mp.h"

#define RCC_I2C_MP_PORT 	RCC_APB2_PERIPH_GPIOC		

#define PORT_I2C_MP_SCL		GPIOC			
#define PIN_I2C_MP_SCL		GPIO_PIN_6		

#define PORT_I2C_MP_SDA		GPIOC			
#define PIN_I2C_MP_SDA		GPIO_PIN_7		

#define I2C_MP_SCL_PIN		GPIO_PIN_6			
#define I2C_MP_SDA_PIN		GPIO_PIN_7			

/* �����дSCL��SDA�ĺ� */
#define I2C_MP_SCL_1()  PORT_I2C_MP_SCL->PBSC = I2C_MP_SCL_PIN				/* SCL = 1 */
#define I2C_MP_SCL_0()  PORT_I2C_MP_SCL->PBC = I2C_MP_SCL_PIN				/* SCL = 0 */

#define I2C_MP_SDA_1()  PORT_I2C_MP_SDA->PBSC = I2C_MP_SDA_PIN				/* SDA = 1 */
#define I2C_MP_SDA_0()  PORT_I2C_MP_SDA->PBC = I2C_MP_SDA_PIN				/* SDA = 0 */

#define I2C_MP_SDA_READ()  ((PORT_I2C_MP_SDA->PID & I2C_MP_SDA_PIN) != 0)	/* ��SDA����״̬ */
#define I2C_MP_SCL_READ()  ((PORT_I2C_MP_SCL->PID & I2C_MP_SCL_PIN) != 0)	/* ��SCL����״̬ */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitI2C_MP
*	����˵��: ����I2C���ߵ�GPIO������ģ��IO�ķ�ʽʵ��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitI2C_MP(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_I2C_MP_PORT, ENABLE);	/* ��GPIOʱ�� */

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	/* ��©���ģʽ */
	
	GPIO_InitStructure.Pin = PIN_I2C_MP_SCL;
	GPIO_InitPeripheral(PORT_I2C_MP_SCL, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PIN_I2C_MP_SDA;
	GPIO_InitPeripheral(PORT_I2C_MP_SDA, &GPIO_InitStructure);

	/* ��һ��ֹͣ�ź�, ��λI2C�����ϵ������豸������ģʽ */
	i2c_Stop_MP();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Delay_MP
*	����˵��: I2C����λ�ӳ٣����400KHz
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void i2c_Delay_MP(void)
{
	uint8_t i;

	/*��
		CPU��Ƶ168MHzʱ�����ڲ�Flash����, MDK���̲��Ż�����̨ʽʾ�����۲Ⲩ�Ρ�
		ѭ������Ϊ5ʱ��SCLƵ�� = 1.78MHz (����ʱ: 92ms, ��д������������ʾ����̽ͷ���ϾͶ�дʧ�ܡ�ʱ��ӽ��ٽ�)
		ѭ������Ϊ10ʱ��SCLƵ�� = 1.1MHz (����ʱ: 138ms, ���ٶ�: 118724B/s)
		ѭ������Ϊ30ʱ��SCLƵ�� = 440KHz�� SCL�ߵ�ƽʱ��1.0us��SCL�͵�ƽʱ��1.2us

		��������ѡ��2.2Kŷʱ��SCL������ʱ��Լ0.5us�����ѡ4.7Kŷ����������Լ1us

		ʵ��Ӧ��ѡ��400KHz���ҵ����ʼ���
	*/
	for (i = 0; i < 30; i++);
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Start_MP
*	����˵��: CPU����I2C���������ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Start_MP(void)
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź� */
	I2C_MP_SDA_1();
	I2C_MP_SCL_1();
	i2c_Delay_MP();
	I2C_MP_SDA_0();
	i2c_Delay_MP();
	
	I2C_MP_SCL_0();
	i2c_Delay_MP();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Stop_MP
*	����˵��: CPU����I2C����ֹͣ�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Stop_MP(void)
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź� */
	I2C_MP_SDA_0();
	I2C_MP_SCL_1();
	i2c_Delay_MP();
	I2C_MP_SDA_1();
	i2c_Delay_MP();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_SendByte_MP
*	����˵��: CPU��I2C�����豸����8bit����
*	��    ��:  _ucByte �� �ȴ����͵��ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_SendByte_MP(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			I2C_MP_SDA_1();
		}
		else
		{
			I2C_MP_SDA_0();
		}
		i2c_Delay_MP();
		I2C_MP_SCL_1();
		i2c_Delay_MP();
		I2C_MP_SCL_0();
		if (i == 7)
		{
			 I2C_MP_SDA_1(); // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		i2c_Delay_MP();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_ReadByte_MP
*	����˵��: CPU��I2C�����豸��ȡ8bit����
*	��    ��:  ��
*	�� �� ֵ: ����������
*********************************************************************************************************
*/
uint8_t i2c_ReadByte_MP(void)
{
	uint8_t i;
	uint8_t value;

	/* ������1��bitΪ���ݵ�bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		I2C_MP_SCL_1();
		i2c_Delay_MP();
		if (I2C_MP_SDA_READ())
		{
			value++;
		}
		I2C_MP_SCL_0();
		i2c_Delay_MP();
	}
	return value;
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_WaitAck_MP
*	����˵��: CPU����һ��ʱ�ӣ�����ȡ������ACKӦ���ź�
*	��    ��:  ��
*	�� �� ֵ: ����0��ʾ��ȷӦ��1��ʾ��������Ӧ
*********************************************************************************************************
*/
uint8_t i2c_WaitAck_MP(void)
{
	uint8_t re;

	I2C_MP_SDA_1();	/* CPU�ͷ�SDA���� */
	i2c_Delay_MP();
	I2C_MP_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	i2c_Delay_MP();
	if (I2C_MP_SDA_READ())	/* CPU��ȡSDA����״̬ */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_MP_SCL_0();
	i2c_Delay_MP();
	return re;
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Ack_MP
*	����˵��: CPU����һ��ACK�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_Ack_MP(void)
{
	I2C_MP_SDA_0();	/* CPU����SDA = 0 */
	i2c_Delay_MP();
	I2C_MP_SCL_1();	/* CPU����1��ʱ�� */
	i2c_Delay_MP();
	I2C_MP_SCL_0();
	i2c_Delay_MP();
	I2C_MP_SDA_1();	/* CPU�ͷ�SDA���� */
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_NAck_MP
*	����˵��: CPU����1��NACK�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void i2c_NAck_MP(void)
{
	I2C_MP_SDA_1();	/* CPU����SDA = 1 */
	i2c_Delay_MP();
	I2C_MP_SCL_1();	/* CPU����1��ʱ�� */
	i2c_Delay_MP();
	I2C_MP_SCL_0();
	i2c_Delay_MP();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_CheckDevice_MP
*	����˵��: ���I2C�����豸��CPU�����豸��ַ��Ȼ���ȡ�豸Ӧ�����жϸ��豸�Ƿ����
*	��    ��:  _Address���豸��I2C���ߵ�ַ
*	�� �� ֵ: ����ֵ 0 ��ʾ��ȷ�� ����1��ʾδ̽�⵽
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice_MP(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_MP_SDA_READ() && I2C_MP_SCL_READ())
	{
		i2c_Start_MP();		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
		i2c_SendByte_MP(_Address | I2C_MP_WR);
		ucAck = i2c_WaitAck_MP();	/* ����豸��ACKӦ�� */

		i2c_Stop_MP();			/* ����ֹͣ�ź� */

		return ucAck;
	}
	return 1;	/* I2C�����쳣 */
}





