#include "dri_aw21024.h"

/*
*@brief 	���AW21024�豸�Ƿ�����
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus AW21024_CheckOk(void)
{
	if ((i2c_CheckDevice(AW_DEV_ADDR) == 0) && (i2c_CheckDevice(AW_DEV2_ADDR) == 0))
	{
		return SUCCESS;
	}
	else
	{
		/* ʧ�ܺ�һ��Ҫ��������ֹͣ�ź� */
		i2c_Stop();
		return ERROR;
	}
}

/*
*@brief 	��AW21024��һ���ֽ�
*@param[in]	_pReadBuf:��������ָ��
*			_ucDevAddr:���豸��ַ
			_ucAddress:�Ĵ�����ַ
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus AW21024_ReadByte(uint8_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	/* step 0:���Ϳ�ʼ�ź� */
	i2c_Start();
	/* step 1:�����豸��ַ */
	i2c_SendByte(_ucDevAddr | I2C_WR);//������дָ��
	/* step 2:�ȴ�Ӧ�� */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 3:���ͼĴ�����ַ���ȴ�Ӧ�� */
	i2c_SendByte(_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:���Ϳ�ʼ�ź� */
	i2c_Start();
	/* step 5:�����豸��ַ */
	i2c_SendByte(_ucDevAddr | I2C_RD);//�����Ƕ�ָ��
	/* step 6:�ȴ�Ӧ�� */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 7:��һ���ֽ����� */
	*_pReadBuf = i2c_ReadByte();
	/* step 8:���ͷ�Ӧ���ź� */
	i2c_NAck();
	/* step 9:��������ֹͣ�ź� */
	i2c_Stop();

	return SUCCESS;
	

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	i2c_Stop();
	
	return ERROR;
}

/*
*@brief 	дһ���ֽڵ�AW21024
*@param[in]	_ucWriteByte:д������ָ��
*			_ucDevAddr:���豸��ַ
			_ucAddress:�Ĵ�����ַ
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
ErrorStatus AW21024_WriteByte(uint8_t _ucWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	/* step 0:���Ϳ�ʼ�ź� */
	i2c_Start();
	/* step 1:�����豸��ַ */
	i2c_SendByte(_ucDevAddr | I2C_WR);//������дָ��
	/* step 2:�ȴ�Ӧ�� */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//�豸��Ӧ��
	}
	/* step 3:���ͼĴ�����ַ���ȴ�Ӧ�� */
	i2c_SendByte(_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:дһ���ֽ����� */
	i2c_SendByte(_ucWriteByte);
	/* step 5:���ͷ�Ӧ���ź� */
	i2c_NAck();
	/* step 6:��������ֹͣ�ź� */
	i2c_Stop();

cmd_fail: //ָ��ִ��ʧ�ܺ󣬷�������ֹͣ����
	i2c_Stop();
	
	return ERROR;
}



