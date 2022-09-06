#include "dri_mp2651.h"

/*
*@brief 	检测MP2651设备是否正常
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus MP2651_CheckOk(void)
{
	if (i2c_CheckDevice_MP(MP_DEV_ADDR) == 0)
	{
		return SUCCESS;
	}
	else
	{
		/* 失败后，一定要发送总线停止信号 */
		i2c_Stop_MP();
		return ERROR;
	}
}

/*
*@brief 	从MP2651读一个字数据
*@param[in]	_pReadBuf:读缓冲区指针
*			_ucDevAddr:从设备地址
			_ucAddress:寄存器地址
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus MP2651_ReadByte(uint16_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	/* step 0:发送开始信号 */
	i2c_Start_MP();
	/* step 1:发送设备地址 */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_WR);//这里是写指令
	/* step 2:等待应答 */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 3:发送寄存器地址，等待应答 */
	i2c_SendByte_MP(_ucAddress);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:发送开始信号 */
	i2c_Start_MP();
	/* step 5:发送设备地址 */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_RD);//这里是读指令
	/* step 6:等待应答 */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 7:读低位字节数据 */
	_ucTmp = i2c_ReadByte_MP();
	/* step 8:发送应答信号 */
	i2c_Ack_MP();
	/* step 9:读高位字节数据，组合数据 */
	*_pReadBuf = (uint16_t)((i2c_ReadByte_MP() << 8) | _ucTmp);
	/* step 10:发送非应答信号 */
	i2c_NAck_MP();
	/* step 11:发送总线停止信号 */
	i2c_Stop_MP();

	return SUCCESS;
	

cmd_fail: //指令执行失败后，发送总线停止命令
	i2c_Stop_MP();
	
	return ERROR;
}

/*
*@brief 	写一个字到MP2651
*@param[in]	_usWriteByte:写数据
*			_ucDevAddr:从设备地址
			_ucAddress:寄存器地址
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus MP2651_WriteByte(uint16_t _usWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	uint8_t _ucTmp = 0;
	
	/* step 0:发送开始信号 */
	i2c_Start_MP();
	/* step 1:发送设备地址 */
	i2c_SendByte_MP(_ucDevAddr | I2C_MP_WR);//这里是写指令
	/* step 2:等待应答 */
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 3:发送寄存器地址，等待应答 */
	i2c_SendByte_MP(_ucAddress);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:写低位字节数据，等待应答 */
	_ucTmp = (uint8_t)(_usWriteByte & 0x00FF);
	i2c_SendByte_MP(_ucTmp);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 5:写高位字节数据，等待应答 */
	_ucTmp = (uint8_t)(_usWriteByte & 0xFF00);
	i2c_SendByte_MP(_ucTmp);
	if (i2c_WaitAck_MP() != 0)
	{
		goto cmd_fail;	
	}
	/* step 6:发送总线停止信号 */
	i2c_Stop_MP();

cmd_fail: //指令执行失败后，发送总线停止命令
	i2c_Stop_MP();
	
	return ERROR;
}




