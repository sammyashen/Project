#include "dri_aw21024.h"

/*
*@brief 	检测AW21024设备是否正常
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus AW21024_CheckOk(void)
{
	if ((i2c_CheckDevice(AW_DEV_ADDR) == 0) && (i2c_CheckDevice(AW_DEV2_ADDR) == 0))
	{
		return SUCCESS;
	}
	else
	{
		/* 失败后，一定要发送总线停止信号 */
		i2c_Stop();
		return ERROR;
	}
}

/*
*@brief 	从AW21024读一个字节
*@param[in]	_pReadBuf:读缓冲区指针
*			_ucDevAddr:从设备地址
			_ucAddress:寄存器地址
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus AW21024_ReadByte(uint8_t *_pReadBuf, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	/* step 0:发送开始信号 */
	i2c_Start();
	/* step 1:发送设备地址 */
	i2c_SendByte(_ucDevAddr | I2C_WR);//这里是写指令
	/* step 2:等待应答 */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 3:发送寄存器地址，等待应答 */
	i2c_SendByte(_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:发送开始信号 */
	i2c_Start();
	/* step 5:发送设备地址 */
	i2c_SendByte(_ucDevAddr | I2C_RD);//这里是读指令
	/* step 6:等待应答 */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 7:读一个字节数据 */
	*_pReadBuf = i2c_ReadByte();
	/* step 8:发送非应答信号 */
	i2c_NAck();
	/* step 9:发送总线停止信号 */
	i2c_Stop();

	return SUCCESS;
	

cmd_fail: //指令执行失败后，发送总线停止命令
	i2c_Stop();
	
	return ERROR;
}

/*
*@brief 	写一个字节到AW21024
*@param[in]	_ucWriteByte:写缓冲区指针
*			_ucDevAddr:从设备地址
			_ucAddress:寄存器地址
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
ErrorStatus AW21024_WriteByte(uint8_t _ucWriteByte, uint8_t _ucDevAddr, uint8_t _ucAddress)
{
	/* step 0:发送开始信号 */
	i2c_Start();
	/* step 1:发送设备地址 */
	i2c_SendByte(_ucDevAddr | I2C_WR);//这里是写指令
	/* step 2:等待应答 */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	//设备无应答
	}
	/* step 3:发送寄存器地址，等待应答 */
	i2c_SendByte(_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	/* step 4:写一个字节数据 */
	i2c_SendByte(_ucWriteByte);
	/* step 5:发送非应答信号 */
	i2c_NAck();
	/* step 6:发送总线停止信号 */
	i2c_Stop();

cmd_fail: //指令执行失败后，发送总线停止命令
	i2c_Stop();
	
	return ERROR;
}



