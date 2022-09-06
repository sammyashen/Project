#include "drv_sf_i2c_mp.h"

#define RCC_MP_I2C_PORT 	RCC_APB2_PERIPH_GPIOC		

#define PORT_MP_I2C_SCL		GPIOC			
#define PIN_MP_I2C_SCL		GPIO_PIN_6		

#define PORT_MP_I2C_SDA		GPIOC			
#define PIN_MP_I2C_SDA		GPIO_PIN_7		

#define MP_I2C_SCL_PIN		GPIO_PIN_6			
#define MP_I2C_SDA_PIN		GPIO_PIN_7			

#define MP_I2C_SCL_1()  PORT_MP_I2C_SCL->PBSC = MP_I2C_SCL_PIN				
#define MP_I2C_SCL_0()  PORT_MP_I2C_SCL->PBC = MP_I2C_SCL_PIN				

#define MP_I2C_SDA_1()  PORT_MP_I2C_SDA->PBSC = MP_I2C_SDA_PIN				
#define MP_I2C_SDA_0()  PORT_MP_I2C_SDA->PBC = MP_I2C_SDA_PIN				

#define MP_I2C_SDA_READ()  ((PORT_MP_I2C_SDA->PID & MP_I2C_SDA_PIN) != 0)	
#define MP_I2C_SCL_READ()  ((PORT_MP_I2C_SCL->PID & MP_I2C_SCL_PIN) != 0)	

void sf_i2c_mp_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_MP_I2C_PORT, ENABLE);	

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	
	
	GPIO_InitStructure.Pin = PIN_MP_I2C_SCL;
	GPIO_InitPeripheral(PORT_MP_I2C_SCL, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PIN_MP_I2C_SDA;
	GPIO_InitPeripheral(PORT_MP_I2C_SDA, &GPIO_InitStructure);

	sf_i2c_mp_stop();
}

static void sf_i2c_mp_delay(void)
{
	uint8_t i;

	for (i = 0; i < 50; i++);
}

void sf_i2c_mp_start(void)
{
	MP_I2C_SDA_1();
	MP_I2C_SCL_1();
	sf_i2c_mp_delay();
	MP_I2C_SDA_0();
	sf_i2c_mp_delay();
	
	MP_I2C_SCL_0();
	sf_i2c_mp_delay();
}

void sf_i2c_mp_stop(void)
{
	MP_I2C_SDA_0();
	MP_I2C_SCL_1();
	sf_i2c_mp_delay();
	MP_I2C_SDA_1();
	sf_i2c_mp_delay();
}

void sf_i2c_mp_sendbyte(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			MP_I2C_SDA_1();
		}
		else
		{
			MP_I2C_SDA_0();
		}
		sf_i2c_mp_delay();
		MP_I2C_SCL_1();
		sf_i2c_mp_delay();
		MP_I2C_SCL_0();
		if (i == 7)
		{
			 MP_I2C_SDA_1(); // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		sf_i2c_mp_delay();
	}
}

uint8_t sf_i2c_mp_readbyte(void)
{
	uint8_t i;
	uint8_t val;

	/* ������1��bitΪ���ݵ�bit7 */
	val = 0;
	for (i = 0; i < 8; i++)
	{
		val <<= 1;
		MP_I2C_SCL_1();
		sf_i2c_mp_delay();
		if (MP_I2C_SDA_READ())
		{
			val++;
		}
		MP_I2C_SCL_0();
		sf_i2c_mp_delay();
	}
	return val;
}

uint8_t sf_i2c_mp_waitack(void)
{
	uint8_t re;

	MP_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
	sf_i2c_mp_delay();
	MP_I2C_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	sf_i2c_mp_delay();
	if (MP_I2C_SDA_READ())	/* CPU��ȡSDA����״̬ */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	MP_I2C_SCL_0();
	sf_i2c_mp_delay();
	return re;
}

void sf_i2c_mp_ack(void)
{
	MP_I2C_SDA_0();	/* CPU����SDA = 0 */
	sf_i2c_mp_delay();
	MP_I2C_SCL_1();	/* CPU����1��ʱ�� */
	sf_i2c_mp_delay();
	MP_I2C_SCL_0();
	sf_i2c_mp_delay();
	MP_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
}

void sf_i2c_mp_nack(void)
{
	MP_I2C_SDA_1();	/* CPU����SDA = 1 */
	sf_i2c_mp_delay();
	MP_I2C_SCL_1();	/* CPU����1��ʱ�� */
	sf_i2c_mp_delay();
	MP_I2C_SCL_0();
	sf_i2c_mp_delay();
}

uint8_t sf_i2c_mp_check_device(uint8_t _Address)
{
	uint8_t ucAck;

	if (MP_I2C_SDA_READ() && MP_I2C_SCL_READ())
	{
		sf_i2c_mp_start();		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
		sf_i2c_mp_sendbyte(_Address | 0x00);
		ucAck = sf_i2c_mp_waitack();	/* ����豸��ACKӦ�� */

		sf_i2c_mp_stop();			/* ����ֹͣ�ź� */

		return ucAck;
	}
	return 1;	/* I2C�����쳣 */
}






