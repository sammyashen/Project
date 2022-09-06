#include "drv_sf_i2c_led.h"

#define RCC_LED_I2C_PORT 	RCC_APB2_PERIPH_GPIOA		

#define PORT_LED_I2C_SCL	GPIOA			
#define PIN_LED_I2C_SCL		GPIO_PIN_11		

#define PORT_LED_I2C_SDA	GPIOA			
#define PIN_LED_I2C_SDA		GPIO_PIN_12		

#define LED_I2C_SCL_PIN		GPIO_PIN_11			
#define LED_I2C_SDA_PIN		GPIO_PIN_12			

#define LED_I2C_SCL_1()  PORT_LED_I2C_SCL->PBSC = LED_I2C_SCL_PIN				
#define LED_I2C_SCL_0()  PORT_LED_I2C_SCL->PBC = LED_I2C_SCL_PIN				

#define LED_I2C_SDA_1()  PORT_LED_I2C_SDA->PBSC = LED_I2C_SDA_PIN				
#define LED_I2C_SDA_0()  PORT_LED_I2C_SDA->PBC = LED_I2C_SDA_PIN				

#define LED_I2C_SDA_READ()  ((PORT_LED_I2C_SDA->PID & LED_I2C_SDA_PIN) != 0)	
#define LED_I2C_SCL_READ()  ((PORT_LED_I2C_SCL->PID & LED_I2C_SCL_PIN) != 0)	

void sf_i2c_led_init(void)
{
	GPIO_InitType GPIO_InitStructure;

	RCC_EnableAPB2PeriphClk(RCC_LED_I2C_PORT, ENABLE);	

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	
	
	GPIO_InitStructure.Pin = PIN_LED_I2C_SCL;
	GPIO_InitPeripheral(PORT_LED_I2C_SCL, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PIN_LED_I2C_SDA;
	GPIO_InitPeripheral(PORT_LED_I2C_SDA, &GPIO_InitStructure);

	sf_i2c_led_stop();
}

static void sf_i2c_led_delay(void)
{
	uint8_t i;

	for (i = 0; i < 50; i++);
}

void sf_i2c_led_start(void)
{
	LED_I2C_SDA_1();
	LED_I2C_SCL_1();
	sf_i2c_led_delay();
	LED_I2C_SDA_0();
	sf_i2c_led_delay();
	
	LED_I2C_SCL_0();
	sf_i2c_led_delay();
}

void sf_i2c_led_stop(void)
{
	LED_I2C_SDA_0();
	LED_I2C_SCL_1();
	sf_i2c_led_delay();
	LED_I2C_SDA_1();
	sf_i2c_led_delay();
}

void sf_i2c_led_sendbyte(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			LED_I2C_SDA_1();
		}
		else
		{
			LED_I2C_SDA_0();
		}
		sf_i2c_led_delay();
		LED_I2C_SCL_1();
		sf_i2c_led_delay();
		LED_I2C_SCL_0();
		if (i == 7)
		{
			 LED_I2C_SDA_1(); // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		sf_i2c_led_delay();
	}
}

uint8_t sf_i2c_led_readbyte(void)
{
	uint8_t i;
	uint8_t val;

	/* ������1��bitΪ���ݵ�bit7 */
	val = 0;
	for (i = 0; i < 8; i++)
	{
		val <<= 1;
		LED_I2C_SCL_1();
		sf_i2c_led_delay();
		if (LED_I2C_SDA_READ())
		{
			val++;
		}
		LED_I2C_SCL_0();
		sf_i2c_led_delay();
	}
	return val;
}

uint8_t sf_i2c_led_waitack(void)
{
	uint8_t re;

	LED_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
	sf_i2c_led_delay();
	LED_I2C_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	sf_i2c_led_delay();
	if (LED_I2C_SDA_READ())	/* CPU��ȡSDA����״̬ */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	LED_I2C_SCL_0();
	sf_i2c_led_delay();
	return re;
}

void sf_i2c_led_ack(void)
{
	LED_I2C_SDA_0();	/* CPU����SDA = 0 */
	sf_i2c_led_delay();
	LED_I2C_SCL_1();	/* CPU����1��ʱ�� */
	sf_i2c_led_delay();
	LED_I2C_SCL_0();
	sf_i2c_led_delay();
	LED_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
}

void sf_i2c_led_nack(void)
{
	LED_I2C_SDA_1();	/* CPU����SDA = 1 */
	sf_i2c_led_delay();
	LED_I2C_SCL_1();	/* CPU����1��ʱ�� */
	sf_i2c_led_delay();
	LED_I2C_SCL_0();
	sf_i2c_led_delay();
}

uint8_t sf_i2c_led_check_device(uint8_t _Address)
{
	uint8_t ucAck;

	if (LED_I2C_SDA_READ() && LED_I2C_SCL_READ())
	{
		sf_i2c_led_start();		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
		sf_i2c_led_sendbyte(_Address | 0x00);
		ucAck = sf_i2c_led_waitack();	/* ����豸��ACKӦ�� */

		sf_i2c_led_stop();			/* ����ֹͣ�ź� */

		return ucAck;
	}
	return 1;	/* I2C�����쳣 */
}





