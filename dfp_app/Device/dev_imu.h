#ifndef __DEV_IMU_H__
#define __DEV_IMU_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_hw_i2c.h"
#include "drv_hw_spi.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
	uint16_t acc_x;
	uint16_t acc_y;
	uint16_t acc_z;

	uint16_t gyro_x;
	uint16_t gyro_y;
	uint16_t gyro_z;

	uint64_t tmst;
}__attribute__((packed, aligned(4))) imu_pack_data, *imu_pack_data_t;

typedef struct{
	uint8_t bank;
	uint8_t reg;
	uint8_t *buf;
}icm42631_ops_data, *icm42631_ops_data_t;

#define REG_WR					0x00
#define REG_RD					0x80

#define IMU_DEV_ADDR			0xD0	//7bits_AD|RW
#define ICM42631_ID				0x5C

//bank0
#define DEVICE_CONFIG			0x11
#define DRIVE_CONFIG			0x13
#define INT_CONFIG				0x14
#define FIFO_CONFIG				0x16
#define TEMP_DATA1_UI			0x1D
#define TEMP_DATA0_UI			0x1E
#define ACCEL_DATA_X1_UI		0x1F
#define ACCEL_DATA_X0_UI		0x20
#define ACCEL_DATA_Y1_UI		0x21
#define ACCEL_DATA_Y0_UI		0x22
#define ACCEL_DATA_Z1_UI		0x23
#define ACCEL_DATA_Z0_UI		0x24
#define GYRO_DATA_X1_UI			0x25
#define GYRO_DATA_X0_UI			0x26
#define GYRO_DATA_Y1_UI			0x27
#define GYRO_DATA_Y0_UI			0x28
#define GYRO_DATA_Z1_UI			0x29
#define GYRO_DATA_Z0_UI			0x2A
#define TMST_FSYNCH				0x2B
#define TMST_FSYNCL				0x2C
#define INT_STATUS				0x2D
#define FIFO_COUNTH				0x2E
#define FIFO_COUNTL				0x2F
#define FIFO_DATA				0x30
#define APEX_DATA0				0x31
#define APEX_DATA1				0x32
#define APEX_DATA2				0x33
#define APEX_DATA3				0x34
#define APEX_DATA4				0x35
#define APEX_DATA5				0x36
#define INT_STATUS2				0x37
#define INT_STATUS3				0x38
#define SIGNAL_PATH_RESET		0x4B
#define INTF_CONFIG0			0x4C
#define INTF_CONFIG1			0x4D
#define PWR_MGMT0				0x4E
#define GYRO_CONFIG0			0x4F
#define ACCEL_CONFIG0			0x50
#define GYRO_CONFIG1			0x51
#define GYRO_ACCEL_CONFIG0		0x52
#define ACCEL_CONFIG1			0x53
#define TMST_CONFIG				0x54
#define APEX_CONFIG0			0x56
#define SMD_CONFIG				0x57
#define FIFO_CONFIG1			0x5F
#define FIFO_CONFIG2			0x60
#define FIFO_CONFIG3			0x61
#define FSYNC_CONFIG			0x62
#define INT_CONFIG0				0x63
#define INT_CONFIG1				0x64
#define INT_SOURCE0				0x65
#define INT_SOURCE1				0x66
#define INT_SOURCE2				0x67
#define INT_SOURCE3				0x68
#define INT_SOURCE4				0x69
#define INT_SOURCE5				0x6A
#define FIFO_LOST_PKT0			0x6C
#define FIFO_LOST_PKT1			0x6D
#define SELF_TEST_CONFIG		0x70
#define WHO_AM_I				0x75
#define REG_BANK_SEL			0x76
//bank1
#define SENSOR_CONFIG0			0x03
#define GYRO_CONFIG_STATIC2		0x0B
#define GYRO_CONFIG_STATIC3		0x0C
#define GYRO_CONFIG_STATIC4		0x0D
#define GYRO_CONFIG_STATIC5		0x0E
#define GYRO_CONFIG_STATIC6		0x0F
#define GYRO_CONFIG_STATIC7		0x10
#define GYRO_CONFIG_STATIC8		0x11
#define GYRO_CONFIG_STATIC9		0x12
#define GYRO_CONFIG_STATIC10	0x13
#define XG_ST_DATA				0x5F
#define YG_ST_DATA				0x60
#define ZG_ST_DATA				0x61
#define TMSTVAL0				0x62
#define TMSTVAL1				0x63
#define TMSTVAL2				0x64
#define INTF_CONFIG4			0x7A
#define INTF_CONFIG5			0x7B
#define INTF_CONFIG6			0x7C
//bank2
#define ACCEL_CONFIG_STATIC2	0x03
#define ACCEL_CONFIG_STATIC3	0x04
#define ACCEL_CONFIG_STATIC4	0x05
#define XA_ST_DATA				0x3B
#define YA_ST_DATA				0x3C
#define ZA_ST_DATA				0x3D
#define AUX1_CONFIG1			0x44
#define AUX1_CONFIG2			0x45
#define AUX1_CONFIG3			0x46
#define TEMP_DATA1_AUX1			0x47
#define TEMP_DATA0_AUX1			0x48
#define ACCEL_DATA_X1_AUX1		0x49
#define ACCEL_DATA_X0_AUX1		0x4A
#define ACCEL_DATA_Y1_AUX1		0x4B
#define ACCEL_DATA_Y0_AUX1		0x4C
#define ACCEL_DATA_Z1_AUX1		0x4D
#define ACCEL_DATA_Z0_AUX1		0x4E
#define GYRO_DATA_X1_AUX1		0x4F
#define GYRO_DATA_X0_AUX1		0x50
#define GYRO_DATA_Y1_AUX1		0x51
#define GYRO_DATA_Y0_AUX1		0x52
#define GYRO_DATA_Z1_AUX1		0x53
#define GYRO_DATA_Z0_AUX1		0x54
#define TMSTVAL0_AUX1			0x55
#define TMSTVAL1_AUX1			0x56
#define INT_STATUS_AUX1			0x57
#define AUX2_CONFIG1			0x59
#define AUX2_CONFIG2			0x5A
#define AUX2_CONFIG3			0x5B
#define TEMP_DATA1_AUX2			0x5C
#define TEMP_DATA0_AUX2			0x5D
#define ACCEL_DATA_X1_AUX2		0x5E
#define ACCEL_DATA_X0_AUX2		0x5F
#define ACCEL_DATA_Y1_AUX2		0x60
#define ACCEL_DATA_Y0_AUX2		0x61
#define ACCEL_DATA_Z1_AUX2		0x62
#define ACCEL_DATA_Z0_AUX2		0x63
#define GYRO_DATA_X1_AUX2		0x64
#define GYRO_DATA_X0_AUX2		0x65
#define GYRO_DATA_Y1_AUX2		0x66
#define GYRO_DATA_Y0_AUX2		0x67
#define GYRO_DATA_Z1_AUX2		0x68
#define GYRO_DATA_Z0_AUX2		0x69
#define TMSTVAL0_AUX2			0x6A
#define TMSTVAL1_AUX2			0x6B
#define INT_STATUS_AUX2			0x6C
//bank3
#define PU_PD_CONFIG1			0x06
#define PU_PD_CONFIG2			0x0E
//bank4
#define FDR_CONFIG				0x09
#define APEX_CONFIG1			0x40
#define APEX_CONFIG2			0x41
#define APEX_CONFIG3			0x42
#define APEX_CONFIG4			0x43
#define APEX_CONFIG5			0x44
#define APEX_CONFIG6			0x45
#define APEX_CONFIG7			0x46
#define APEX_CONFIG8			0x47
#define APEX_CONFIG9			0x48
#define APEX_CONFIG10			0x49
#define ACCEL_WOM_X_THR			0x4A
#define ACCEL_WOM_Y_THR			0x4B
#define ACCEL_WOM_Z_THR			0x4C
#define INT_SOURCE6				0x4D
#define INT_SOURCE7				0x4E
#define INT_SOURCE8				0x4F
#define INT_SOURCE9				0x50
#define INT_SOURCE10			0x51
#define OFFSET_USER0			0x77
#define OFFSET_USER1			0x78
#define OFFSET_USER2			0x79
#define OFFSET_USER3			0x7A
#define OFFSET_USER4			0x7B
#define OFFSET_USER5			0x7C
#define OFFSET_USER6			0x7D
#define OFFSET_USER7			0x7E
#define OFFSET_USER8			0x7F


#ifdef __cplusplus
}
#endif

#endif 





