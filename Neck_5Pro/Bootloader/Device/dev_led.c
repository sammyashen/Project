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
*@brief 	��ʼ��LED�����豸
*@retval	SUCCESS���ɹ�
*			ERROR��ʧ��	
*/
static ErrorStatus LED_DEV_Configure(void)
{
	//��ʼ��I2C
	bsp_InitI2C();
	
	//����������豸�Ƿ�����
	if(AW21024_CheckOk() == SUCCESS)
	{
		/* ��λ���� */
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_RESET);
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_RESET);
	
		/* ����LED_Dev_1 */
		AW21024_WriteByte(0x81, AW_DEV_ADDR, ADDR_GCR);	//�Զ����ܡ�16MHz��оƬʹ��

		//Ϩ������LED
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_GCFG0);//��������LEDΪͨ��ģʽ
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_GCR2);//����RGBMD
		for(uint8_t i = 0;i < 24;i++)
		{
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + i));//��������
			AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
			AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + i));//������ɫ��
		}
	
		/* ����LED_Dev_2 */
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCOLR);//����RED���ֵ
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_GCOLG);//����GREEN���ֵ
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCOLB);//����BLUE���ֵ
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_FADEH);//�����������ֵ
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_FADEL);//������С����ֵ
		AW21024_WriteByte(0x33, AW_DEV2_ADDR, ADDR_PATT0);//���ý���ʱ��->rise:0.38s,hold on:0.38s
		AW21024_WriteByte(0x30, AW_DEV2_ADDR, ADDR_PATT1);//���ý���ʱ��->fall:0.38s,hold off:0s
		AW21024_WriteByte(0x30, AW_DEV2_ADDR, ADDR_PATT2);//���ú�����ͣ��
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATT3);//���ú���ѭ������->forever

		AW21024_WriteByte(0x07, AW_DEV2_ADDR, ADDR_PATCFG);//����ʹ���Զ�����ģʽ
		AW21024_WriteByte(0xFF, AW_DEV2_ADDR, ADDR_GCFG0);//����GEn
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_GCFG1);//����LED����RGB��ɫ
		
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATGO);//�������Զ�����
		
		return SUCCESS;
	}
	else
		return ERROR;
}

/*
*@brief 	״̬����������
*@param[in]	_ucColIndex:��ɫ����
*			_ucBrightness:����ֵ
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
*@brief 	ģʽ�Ƶ���
*@param[in]	_ucColIndex:ģʽ����
*/
static void LED_MODE_ON(uint8_t _ucModeIndex)
{
	for(uint8_t i = 0;i < 16;i++)//ȫ��
	{
		AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_BR + i));
		AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
		AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + i));
	}
	
	switch(_ucModeIndex)
	{
		case 0x00:	//soft mode
			for(uint8_t i = 0;i < sizeof(gc_ucSoftLEDIndex);i++)//��Ӧ����
			{
				AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_BR + gc_ucSoftLEDIndex[i]));
				AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
				AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + gc_ucSoftLEDIndex[i]));
			}
		break;

		case 0x01:	//med mode
			for(uint8_t i = 0;i < sizeof(gc_ucMedLEDIndex);i++)//��Ӧ����
			{
				AW21024_WriteByte(0xFF, AW_DEV_ADDR, (ADDR_BR + gc_ucMedLEDIndex[i]));
				AW21024_WriteByte(0x00, AW_DEV_ADDR, ADDR_UPDATE);
				AW21024_WriteByte(0x00, AW_DEV_ADDR, (ADDR_COL + gc_ucMedLEDIndex[i]));
			}
		break;

		case 0x02:	//hard mode
			for(uint8_t i = 0;i < sizeof(gc_ucHardLEDIndex);i++)//��Ӧ����
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
		AW21024_WriteByte(0x01, AW_DEV2_ADDR, ADDR_PATGO);//������0��������1����������һ���Զ�����
	}
	else
	{
		AW21024_WriteByte(0x00, AW_DEV2_ADDR, ADDR_PATGO);
	}
}


