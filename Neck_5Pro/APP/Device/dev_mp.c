#include "dev_mp.h"
#include "include.h"
#include <stdlib.h>

static void mp2651_readword(uint16_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x00);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	sf_i2c_mp_sendbyte(_ucAddress);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x01);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	_ucTmp = sf_i2c_mp_readbyte();
	sf_i2c_mp_ack();
	*_pReadBuf = (uint16_t)((sf_i2c_mp_readbyte() << 8) | _ucTmp);
	sf_i2c_mp_nack();
	sf_i2c_mp_stop();

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	sf_i2c_mp_stop();
}

static void mp2651_writeword(uint16_t _usWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x00);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	sf_i2c_mp_sendbyte(_ucAddress);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	_ucTmp = (uint8_t)_usWriteByte;
	sf_i2c_mp_sendbyte(_ucTmp);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	_ucTmp = (uint8_t)(_usWriteByte>>8);
	sf_i2c_mp_sendbyte(_ucTmp);
	if (sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	sf_i2c_mp_stop();

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	sf_i2c_mp_stop();
}

static void sgm41513_readbyte(uint8_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x00);
	if(sf_i2c_mp_waitack() != 0)		goto cmd_fail;	
	sf_i2c_mp_sendbyte(_ucAddress);
	if(sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x01);
	if(sf_i2c_mp_waitack() != 0)		goto cmd_fail;
	*_pReadBuf = sf_i2c_mp_readbyte();
	sf_i2c_mp_nack();
	sf_i2c_mp_stop();

cmd_fail: 
	sf_i2c_mp_stop();
}

static void sgm41513_writebyte(uint8_t _ucWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	sf_i2c_mp_start();
	sf_i2c_mp_sendbyte(_ucDevAddr | 0x00);
	if(sf_i2c_mp_waitack() != 0)		goto cmd_fail;	
	sf_i2c_mp_sendbyte(_ucAddress);
	if(sf_i2c_mp_waitack() != 0)		goto cmd_fail;	
	sf_i2c_mp_sendbyte(_ucWriteByte);
	sf_i2c_mp_nack();
	sf_i2c_mp_stop();

cmd_fail: 
	sf_i2c_mp_stop();
}

uint8_t get_mp2651_chargesta(void)
{
	uint16_t _usTmp = 0; 
	
	mp2651_readword(&_usTmp, MP2651_ADDR, MP2651_REG16);
	_usTmp = ((_usTmp >> 6) & 0x0007);

	return (uint8_t)_usTmp;
}

uint16_t get_mp2651_chargecurr(void)
{
	uint16_t _usTmp = 0;
	uint16_t _usChargeCurr = 0;

	mp2651_readword(&_usTmp, MP2651_ADDR, MP2651_REG27);
	if(((_usTmp >> 9) & 0x0001) == 0x0001)
		_usChargeCurr += 6400;
	if(((_usTmp >> 8) & 0x0001) == 0x0001)
		_usChargeCurr += 3200;
	if(((_usTmp >> 7) & 0x0001) == 0x0001)
		_usChargeCurr += 1600;
	if(((_usTmp >> 6) & 0x0001) == 0x0001)
		_usChargeCurr += 800;
	if(((_usTmp >> 5) & 0x0001) == 0x0001)
		_usChargeCurr += 400;
	if(((_usTmp >> 4) & 0x0001) == 0x0001)
		_usChargeCurr += 200;
	if(((_usTmp >> 3) & 0x0001) == 0x0001)
		_usChargeCurr += 100;
	if(((_usTmp >> 2) & 0x0001) == 0x0001)
		_usChargeCurr += 50;
	if(((_usTmp >> 1) & 0x0001) == 0x0001)
		_usChargeCurr += 25;
	if(((_usTmp >> 0) & 0x0001) == 0x0001)
		_usChargeCurr += 13;

	return _usChargeCurr;
}

uint8_t get_sgm41513_chargesta(void)
{
	uint8_t _ucTmp = 0;

	sgm41513_readbyte(&_ucTmp, SGM41513_ADDR, SGM41513_REG08);
	_ucTmp =((_ucTmp >> 3) & 0x03);

	return _ucTmp;
}


static void mp_init(void)
{
	sf_i2c_mp_init();
	if(sf_i2c_mp_check_device(MP2651_ADDR) == 0x00 && sf_i2c_mp_check_device(SGM41513_ADDR) == 0x00)
	{
		/* ʹ�ܿ��Ź� */
		mp2651_writeword(0x0209, MP2651_ADDR, MP2651_REG05);
		/* ��С�����ѹ����4.56V */
		mp2651_writeword(0x0039, MP2651_ADDR, MP2651_REG06);
		/* ��ʹ�ܳ�����ģʽ */
		mp2651_writeword(0x0030, MP2651_ADDR, MP2651_REG07);
		/* ��������������3000mA */
		mp2651_writeword(0x003C, MP2651_ADDR, MP2651_REG08);
		/* Դģʽ�����ѹ4.98V */
		mp2651_writeword(0x00F9, MP2651_ADDR, MP2651_REG09);
		/* Դģʽ�����������2A */
		mp2651_writeword(0x0028, MP2651_ADDR, MP2651_REG0A);
		/* ��ص�ѹ����ʹ�ܣ����Ԥ�����ֵ3.0V/Cell */
		mp2651_writeword(0x3080, MP2651_ADDR, MP2651_REG0B);
		/* JEITA�������� */
		mp2651_writeword(0x3410, MP2651_ADDR, MP2651_REG0C);
		/* �¶ȱ������� */
		mp2651_writeword(0xF399, MP2651_ADDR, MP2651_REG0D);
		/* ʹ��ADC����ת�� */
		mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG0E);
		/* ʹ���Ȼ�·���ڣ������¶�120�㣬������100mA���������400mA���������200mA */
		mp2651_writeword(0xF244, MP2651_ADDR, MP2651_REG0F);
		/* ���üĴ���2���� */
		mp2651_writeword(0x0C7E, MP2651_ADDR, MP2651_REG10);
		/* ���üĴ���3���� */
		mp2651_writeword(0x6008, MP2651_ADDR, MP2651_REG11);
		/* ���üĴ���4���� */
		mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG12);
		/* ��������ֵ400mA */
		mp2651_writeword(0x0200, MP2651_ADDR, MP2651_REG14);
		/* ��������ѹ12.4V */
		mp2651_writeword(0x4D80, MP2651_ADDR, MP2651_REG15);
		/* �ж��������� */
		mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG18);
		mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG19);


		sgm41513_writebyte(0x17, SGM41513_ADDR, SGM41513_REG00);//IIN_LIM:2400mA
		sgm41513_writebyte(0x0A, SGM41513_ADDR, SGM41513_REG01);
		sgm41513_writebyte(0xA7, SGM41513_ADDR, SGM41513_REG02);//960mA
		sgm41513_writebyte(0xAA, SGM41513_ADDR, SGM41513_REG03);
		sgm41513_writebyte(0x58, SGM41513_ADDR, SGM41513_REG04);//4.208V
		sgm41513_writebyte(0x87, SGM41513_ADDR, SGM41513_REG05);
		sgm41513_writebyte(0x66, SGM41513_ADDR, SGM41513_REG06);//OVP:6.5V
		sgm41513_writebyte(0x00, SGM41513_ADDR, SGM41513_REG07);
		sgm41513_writebyte(0x00, SGM41513_ADDR, SGM41513_REG0A);
		sgm41513_writebyte(0x00, SGM41513_ADDR, SGM41513_REG0B);
		sgm41513_writebyte(0x75, SGM41513_ADDR, SGM41513_REG0C);
		sgm41513_writebyte(0x01, SGM41513_ADDR, SGM41513_REG0D);
		sgm41513_writebyte(0x00, SGM41513_ADDR, SGM41513_REG0F);
	}
	else
	{
		sf_i2c_mp_stop();
	}
}

void mpdevice_charge_init(void)
{
	mp2651_writeword(0x0101, MP2651_ADDR, MP2651_REG0E);
	mp2651_writeword(0x3C53, MP2651_ADDR, MP2651_REG12);
	mp2651_writeword(0x4D80, MP2651_ADDR, MP2651_REG15);
	
	sgm41513_writebyte(0x1A, SGM41513_ADDR, SGM41513_REG01);
	sgm41513_writebyte(0xA7, SGM41513_ADDR, SGM41513_REG02);
	sgm41513_writebyte(0x58, SGM41513_ADDR, SGM41513_REG04);
	sgm41513_writebyte(0xBF, SGM41513_ADDR, SGM41513_REG05);
	sgm41513_writebyte(0x4C, SGM41513_ADDR, SGM41513_REG07);
}
MSH_CMD_EXPORT(mpdevice_charge_init, configure mp charge);

void mpdevice_off_init(void)
{
	mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG0E);
	mp2651_writeword(0x0000, MP2651_ADDR, MP2651_REG12);
	mp2651_writeword(0x4D80, MP2651_ADDR, MP2651_REG15);
	
	sgm41513_writebyte(0x0A, SGM41513_ADDR, SGM41513_REG01);
	sgm41513_writebyte(0xA7, SGM41513_ADDR, SGM41513_REG02);
	sgm41513_writebyte(0x58, SGM41513_ADDR, SGM41513_REG04);
	sgm41513_writebyte(0x87, SGM41513_ADDR, SGM41513_REG05);
	sgm41513_writebyte(0x00, SGM41513_ADDR, SGM41513_REG07);
}
MSH_CMD_EXPORT(mpdevice_off_init, configure mp off);



void get_mp2651_reg(void)
{
	uint16_t _usTmp = 0;

	for(uint8_t i=0;i<17;i++)
	{
		mp2651_readword(&_usTmp, MP2651_ADDR, MP2651_REG05+i);
		LOG_I("mp2651 reg0x%02X:0x%04X", MP2651_REG05+i, _usTmp);
		rt_thread_mdelay(10);
	}
}
MSH_CMD_EXPORT(get_mp2651_reg, get mp2651 reg);

void get_sgm41513_reg(void)
{
	uint8_t _usTmp = 0;

	for(uint8_t i=0;i<16;i++)
	{
		sgm41513_readbyte(&_usTmp, SGM41513_ADDR, SGM41513_REG00+i);
		LOG_I("sgm41513 reg0x%02X:0x%02X", SGM41513_REG00+i, _usTmp);
		rt_thread_mdelay(10);
	}
}
MSH_CMD_EXPORT(get_sgm41513_reg, get sgm41513 reg);




device_initcall(mp_init);



