#include "dri_mp2651.h"

/*
*@brief 	���MP2651�豸�Ƿ�����
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus MP2651_CheckOk(void)
{
	if (i2c_CheckDevice_MP(MP_DEV_ADDR) == 0)
	{
		return SUCCESS;
	}
	else
	{
		/* ʧ�ܺ�һ��Ҫ��������ֹͣ�ź� */
		i2c_Stop_MP();
		return ERROR;
	}
}

/*
*@brief 	��MP2651��һ��������
*@param[in]	_pReadBuf:��������ָ��
*			_ucDevAddr:���豸��ַ
			_ucAddress:�Ĵ�����ַ
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus MP2651_ReadByte(uint16_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	/* step 0:���Ϳ�ʼ�ź� */
	i2c_Start_MP();
	/* step 1:�����豸��ַ */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_WR);//������дָ��
	/* step 2:�ȴ�Ӧ�� */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 3:���ͼĴ�����ַ���ȴ�Ӧ�� */
	i2c_SendByte_MP(_ucAddress);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:���Ϳ�ʼ�ź� */
	i2c_Start_MP();
	/* step 5:�����豸��ַ */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_RD);//�����Ƕ�ָ��
	/* step 6:�ȴ�Ӧ�� */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 7:����λ�ֽ����� */
	_ucTmp = i2c_ReadByte_MP();
	/* step 8:����Ӧ���ź� */
	i2c_Ack_MP();
	/* step 9:����λ�ֽ����ݣ�������� */
	*_pReadBuf = (uint16_t)((i2c_ReadByte_MP() << 8) | _ucTmp);
	/* step 10:���ͷ�Ӧ���ź� */
	i2c_NAck_MP();
	/* step 11:��������ֹͣ�ź� */
	i2c_Stop_MP();

	return SUCCESS;
	

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	i2c_Stop_MP();
	
	return ERROR;
}

/*
*@brief 	дһ���ֵ�MP2651
*@param[in]	_usWriteByte:д����
*			_ucDevAddr:���豸��ַ
			_ucAddress:�Ĵ�����ַ
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus MP2651_WriteByte(uint16_t _usWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	/* step 0:���Ϳ�ʼ�ź� */
	i2c_Start_MP();
	/* step 1:�����豸��ַ */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_WR);//������дָ��
	/* step 2:�ȴ�Ӧ�� */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 3:���ͼĴ�����ַ���ȴ�Ӧ�� */
	i2c_SendByte_MP(_ucAddress);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:д��λ�ֽ����ݣ��ȴ�Ӧ�� */
	_ucTmp = (uint8_t)(_usWriteByte & 0x00FF);
	i2c_SendByte_MP(_ucTmp);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 5:д��λ�ֽ����ݣ��ȴ�Ӧ�� */
	_ucTmp = (uint8_t)(_usWriteByte & 0xFF00);
	i2c_SendByte_MP(_ucTmp);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 6:��������ֹͣ�ź� */
	i2c_Stop_MP();

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	i2c_Stop_MP();
	
	return ERROR;
}




