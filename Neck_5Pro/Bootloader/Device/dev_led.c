#include "dev_led.h"

static ErrorStatus LED_DEV_Configure(void);
static void LED_STA_SetBri(uint8_t _ucColIndex, uint8_t _ucBrightness);
static void LED_MODE_ON(uint8_t _ucModeIndex);
static void LED_NECK_AUTOBR(FlagStatus _flagRun);

T_LED_Device g_tLED_Dev = {
	._ucled_breath_index = 0,
	._ucled_blink_cnt = 6,			
	.LED_Config = LED_DEV_Configure,
	.LED_Sta_Run = LED_STA_SetBri,
	.LED_Mode_Run = LED_MODE_ON,
	.LED_Neck_BrRun = LED_NECK_AUTOBR
};

const uint8_t gc_ucSoftLEDIndex[6] = {2, 3, 6, 8, 12, 13};/* display "1" */
const uint8_t gc_ucMedLEDIndex[14] = {0, 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 15};/* display "2" */
const uint8_t gc_ucHardLEDIndex[14] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15};/* display "3" */

/*
*@brief 	初始化LED驱动设备
*@retval	SUCCESS：成功
*			ERROR：失败	
*/
static ErrorStatus LED_DEV_Configure(void)
{
	//初始化I2C
	bsp_InitI2C();
	
	//检测两个从设备是否正常
	if(AW21024_CheckOk() == SUCCESS)
	{
		/* 复位配置 */
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_RESET);
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_RESET);
	
		/* 配置LED_Dev_1 */
		AW21024_WriteByte(0x81, AW_DEV_ADDR, ADDR_GCR);	//自动节能、16MHz、芯片使能

		//熄灭所有LED
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_GCFG0);//设置所有LED为通用模式
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_GCR2);//设置RGBMD
		for(uint8_t i = 0;i < 24;i++)
		{
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + i));//设置亮度
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + i));//设置颜色？
		}
	
		/* 配置LED_Dev_2 */
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCOLR);//配置RED混合值
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_GCOLG);//配置GREEN混合值
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCOLB);//配置BLUE混合值
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_FADEH);//配置最大亮度值
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_FADEL);//配置最小亮度值
		AW21024_WriteByte(0x33, AW_DEV2_ADDR, ADDR_PATT0);//配置渐变时间->rise:0.38s,hold on:0.38s
		AW21024_WriteByte(0x30, AW_DEV2_ADDR, ADDR_PATT1);//配置渐变时间->fall:0.38s,hold off:0s
		AW21024_WriteByte(0x30, AW_DEV2_ADDR, ADDR_PATT2);//配置呼吸启停点
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATT3);//配置呼吸循环次数->forever

		AW21024_WriteByte(0x07, AW_DEV2_ADDR, ADDR_PATCFG);//配置使用自动呼吸模式
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_GCFG0);//配置GEn
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCFG1);//配置LED共享RGB颜色
		
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATGO);//不启动自动呼吸
		
		return SUCCESS;
	}
	else
		return ERROR;
}

/*
*@brief 	状态灯亮度设置
*@param[in]	_ucColIndex:颜色索引
*			_ucBrightness:亮度值
*/
static void LED_STA_SetBri(uint8_t _ucColIndex, uint8_t _ucBrightness)
{
	switch(_ucColIndex)
	{
		case RED_COLOR:
			//R
			AW21024_WriteByte(_ucBrightness, AW_DEV_ADDR, (ADDR_BR + 18));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_COL + 18));
			//G
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 17));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 17));
			//B
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 16));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 16));
		break;

		case GREEN_COLOR:
			//R
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 18));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 18));
			//G
			AW21024_WriteByte(_ucBrightness, AW_DEV_ADDR, (ADDR_BR + 17));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_COL + 17));
			//B
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 16));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 16));
		break;

		case BLUE_COLOR:
			//R
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 18));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 18));
			//G
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 17));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 17));
			//B
			AW21024_WriteByte(_ucBrightness, AW_DEV_ADDR, (ADDR_BR + 16));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_COL + 16));
		break;

		case PURPLE_COLOR:
			//R
			AW21024_WriteByte(_ucBrightness, AW_DEV_ADDR, (ADDR_BR + 18));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_COL + 18));
			//G
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + 17));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + 17));
			//B
			AW21024_WriteByte(_ucBrightness, AW_DEV_ADDR, (ADDR_BR + 16));
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_COL + 16));
		break;
	}
}

/*
*@brief 	模式灯点亮
*@param[in]	_ucColIndex:模式索引
*/
static void LED_MODE_ON(uint8_t _ucModeIndex)
{
	for(uint8_t i = 0;i < 16;i++)//全灭
	{
		AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + i));
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
		AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + i));
	}
	
	switch(_ucModeIndex)
	{
		case 0x00:	//soft mode
			for(uint8_t i = 0;i < sizeof(gc_ucSoftLEDIndex);i++)//对应点亮
			{
				AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_BR + gc_ucSoftLEDIndex[i]));
				AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
				AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + gc_ucSoftLEDIndex[i]));
			}
		break;

		case 0x01:	//med mode
			for(uint8_t i = 0;i < sizeof(gc_ucMedLEDIndex);i++)//对应点亮
			{
				AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_BR + gc_ucMedLEDIndex[i]));
				AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
				AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + gc_ucMedLEDIndex[i]));
			}
		break;

		case 0x02:	//hard mode
			for(uint8_t i = 0;i < sizeof(gc_ucHardLEDIndex);i++)//对应点亮
			{
				AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_BR + gc_ucHardLEDIndex[i]));
				AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
				AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + gc_ucHardLEDIndex[i]));
			}
		break;
	}
}

static void LED_NECK_AUTOBR(FlagStatus _flagRun)
{
	if(_flagRun == SET)
	{
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATGO);
		AW21024_WriteByte(0x01, AW_DEV2_ADDR, ADDR_PATGO);//先设置0，再设置1才能重启下一次自动呼吸
	}
	else
	{
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATGO);
	}
}


