#include "dcd_user.h"
#include "dcd_cfg.h"
#include "n32l40x.h"
#include "af_flash.h"

DcdErrCode dcd_port_read(uint32_t addr, uint32_t *buf, size_t size)
{
	DcdErrCode result = DCD_NO_ERR;
	/* You can add your code under here. */
	af_port_read(addr,buf,size);
	
	return result;
}

DcdErrCode dcd_port_erase(uint32_t addr, size_t size)
{
	DcdErrCode result = DCD_NO_ERR;
	/* You can add your code under here. */
	af_port_erase(addr,size);

	return result;
}

DcdErrCode dcd_port_write(uint32_t addr, const uint32_t *buf, size_t size)
{
	DcdErrCode result = DCD_NO_ERR;
	/* You can add your code under here. */
	af_port_write(addr,buf,size);
	
	return result;
}



/* DCD example */
void dcd_write_example(void)
{
	/* DCD version,if you want to upgrade dcd,change the version */
	#define DCD_VERSION			0x01

	uint8_t dcd_version = DCD_VERSION;
	int ret = 0;

	/* rcc init data structure array */
	dev_rcc_struct dev_w_rcc[]={
		{	64000000,
			RCC_PLL_HSI_PRE_DIV2,
			RCC_PLL_MUL_8,
			RCC_HCLK_DIV4,
			RCC_HCLK_DIV2
		}
	};

	/* gpio device data structure array */
	/* 	{DCD_GPIO_B,DCD_GPIO_PIN_5,DCD_GPIO_Mode_Out_PP,DCD_GPIO_No_Pull,DCD_GPIO_Speed_HIGH,DCD_GPIO_HIGH},
		{DCD_GPIO_B,DCD_GPIO_PIN_4,DCD_GPIO_Mode_Out_PP,DCD_GPIO_No_Pull,DCD_GPIO_Speed_LOW,DCD_GPIO_LOW} 
	*/
	dev_gpio_struct dev_w_gpio[]={
		{DCD_GPIO_A,DCD_GPIO_PIN_8,DCD_GPIO_Mode_Out_PP,DCD_GPIO_No_Pull,DCD_GPIO_Speed_HIGH,DCD_GPIO_HIGH},
	};	
		
	/* message data structure array */
	const dev_message_struct dev_w_message[] = {
		{DCD_DEV_RCC,sizeof(dev_w_rcc)},
		{DCD_DEV_GPIO,sizeof(dev_w_gpio)}
	};
	int dcd_dev_size = sizeof(dev_w_message)/sizeof(dev_message_struct);

	/* install point all of device data */
	const uint32_t *p_data[]={(uint32_t *)&dev_w_rcc,(uint32_t *)&dev_w_gpio};
	
	ret = dcd_write_to_flash(dcd_version,dcd_dev_size,dev_w_message,p_data);
	if(ret == -1)
		printf("dcd device is the latest version = 0x%02X\r\n",DCD_VERSION);
	else if(ret == -2 )
		printf("dcd device return lenth error.\r\n");
	else
		printf("dcd device need to upgrade.\r\n");
}


void dcd_read_example(void){
	int ret = 0;
	uint32_t arr_buff[128] = {0};
	ret = dcd_config_device(arr_buff);
//	if(ret == -1)
//		printf("header tag error \r\n");
//	else if(ret == -2)
//		printf("header lenth error \r\n");
//	else if(ret == -3)
//		printf("header crc32 error \r\n");
//	else if(ret == -4)
//		printf("device type error \r\n");
//	else if(ret == -5)
//		printf("header crc32 error \r\n");
//	else if(ret == -6)
//		printf("device type error \r\n");
//	else		
//		printf("device read succuss!\r\n");
}


int GPIO_CFG_CpltCallback(dev_gpio_struct *dev_gpio)
{
	GPIO_ModeType mode = (GPIO_ModeType)dev_gpio->mode;
	GPIO_PuPdType pdmode = (GPIO_PuPdType)dev_gpio->pdmode;
	GPIO_SpeedType speed = (GPIO_SpeedType)dev_gpio->speed;
	uint8_t level = dev_gpio->level;
	uint8_t	port = dev_gpio->port;
	uint16_t pin = 1 << (dev_gpio->pin);
//	printf("dev_gpio = %d,%d,%d,%d,%d\r\n",mode,speed,level,port,pin);
	
	GPIO_InitType gpio_structure;
	GPIO_InitStruct(&gpio_structure);
	
	gpio_structure.Pin = pin;
	gpio_structure.GPIO_Slew_Rate = speed;
	gpio_structure.GPIO_Mode = mode;
	gpio_structure.GPIO_Pull = pdmode;
	GPIO_Module *gpiox = NULL;
	switch (port)
	{
		case 'A':
			gpiox = GPIOA;
			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA,ENABLE);
			//PA13,14,15需要重映射
//			if((pin == GPIO_PIN_13)||(pin == GPIO_PIN_14)||(pin == GPIO_PIN_15))
//			{
//				RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
//				gpio_structure.GPIO_Mode  = GPIO_Mode_AF_PP;
//				gpio_structure.GPIO_Alternate = GPIO_AF0_SW_JTAG;
//			}
			break;
		case 'B':
			gpiox = GPIOB;
			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB,ENABLE);
			//PB3,PB4需要重映射
//			if((pin == GPIO_PIN_3)||(pin == GPIO_PIN_4))
//			{
//				RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
//				gpio_structure.GPIO_Mode  = GPIO_Mode_AF_PP;
//				gpio_structure.GPIO_Alternate = GPIO_AF0_SW_JTAG;
//			}
			break;
		case 'C':
			gpiox = GPIOC;
			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC,ENABLE);
			break;
		case 'D':
			gpiox = GPIOD;
			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOD,ENABLE);
			break;
//		case 'E':
//			gpiox = GPIOE;
//			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOE,ENABLE);
//			break;
//		case 'F':
//			gpiox = GPIOF;
//			RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOF,ENABLE);
//			break;
		default:
			{
				return -1;
//				printf("gpio port error!\r\n");
			}
	}
	GPIO_InitPeripheral(gpiox, &gpio_structure);
	if(level)	
		GPIO_SetBits(gpiox, pin);
	else
		GPIO_ResetBits(gpiox, pin);
	return 0;
}

int RCC_CFG_CpltCallback(dev_rcc_struct *dev_rcc)
{
	uint32_t sysclk = dev_rcc->sysclk;
	uint32_t pllsrc = dev_rcc->pllsrc;
	uint32_t pllmul = dev_rcc->pllmul;
	uint32_t pclk1div = dev_rcc->pclk1div;
	uint32_t pclk2div = dev_rcc->pclk2div;

//	printf("rcc=0x%08x,0x%08x,0x%08x,0x%08x,0x%08x\r\n",sysclk,pllsrc,pllmul,pclk1div,pclk2div);
	
	ErrorStatus StartUpStatus = SUCCESS;
	RCC_DeInit();
	
	if(pllsrc == RCC_PLL_HSI_PRE_DIV2)
	{
		RCC_EnableHsi(ENABLE);
	}else if((pllsrc == RCC_PLL_SRC_HSE_DIV1) || (pllsrc == RCC_PLL_SRC_HSE_DIV2)){
		RCC_ConfigHse(RCC_HSE_ENABLE);
		StartUpStatus = RCC_WaitHseStable();
	}else{
		return -1;
	}

	if(StartUpStatus == SUCCESS)
	{
	    /* Flash wait state
        0: HCLK <= 32M
        1: HCLK <= 64M
		*/
	    FLASH->AC &= (uint32_t)((uint32_t)~FLASH_AC_LATENCY);
	    FLASH->AC |= (uint32_t)((sysclk - 1) / 32000000);
		RCC_ConfigHclk(RCC_SYSCLK_DIV1);
		RCC_ConfigPclk2(pclk2div);
		RCC_ConfigPclk1(pclk1div);
		RCC_ConfigPll(pllsrc, pllmul,RCC_PLLDIVCLK_DISABLE);
		RCC_EnablePll(ENABLE);
		while (RCC_GetFlagStatus(RCC_CTRL_FLAG_PLLRDF) == RESET);
		RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
		while (RCC_GetSysclkSrc() != 0x0C);

//		SysTick_Config(sysclk / 1000);
	}else{
		return -2;
	}
	return 0;
}


