#include "dcd_user.h"

dcd_header_struct  dcd_w_header = {DCD_TAG,0,0,0,0};
								
/* DCD write to flash */
static uint16_t caldcd_data_len(uint32_t dev_size,const dev_message_struct *mess)
{
    int lenth = 0;
	int n = dev_size;
	for(int i = 0;i < n;i++)
		lenth += mess[i].lenth + DEV_MESSAGE_STRUCT_SIZE;
	return lenth;
}

static uint32_t caldcd_write_crc32(uint32_t dev_size,const dev_message_struct *mess,const uint32_t **data)
{
	uint32_t crc32 = 0;
	const uint8_t **p_data = (const uint8_t **)data;
	crc32 = af_calc_crc32(crc32,mess,DEV_MESSAGE_STRUCT_SIZE*dev_size);
	
	for(int i=0;i<dev_size;i++)
	{
		crc32 = af_calc_crc32(crc32,p_data[i],mess[i].lenth);		
	}
//	printf("crc32 = 0x%08X\r\n",crc32);
	return crc32;
}

/**
 * func：			设备配置数据写到flash中（包含是否升级判断和长度校验）
 * @param version：	版本信息	 
 * @param dev_size：设备种类数	
 * @param mess：	设备信息
 * @param p_data：	设备指针，指向所有需要写入设备的地址
 * @return 			-1 不需要升级
 *					-2 长度错误
 *					0  升级成功
 */
int dcd_write_to_flash(uint8_t version,uint32_t dev_size,const dev_message_struct *mess,const uint32_t **p_data)
{
	uint32_t header_word = 0;
	dcd_port_read(DCD_BASE_ADDR,&header_word,sizeof(uint32_t));
	uint8_t header_tag = (uint8_t)header_word;
	uint8_t header_version = (uint8_t)(header_word >> 8);	
	
	if((DCD_TAG == header_tag)&&(version == header_version))
		return -1;
	
	/* write header test */
//	printf("flash header_tag = 0x%02X,header_version = 0x%02X\r\n",header_tag,header_version);
	
	uint32_t feed_address = DCD_BASE_ADDR;
	uint32_t dcd_lenth = DCD_HEADER_STRUCT_SIZE;
	for(int i=0;i<dev_size;i++)
	{
		dcd_lenth += mess[i].lenth + DEV_MESSAGE_STRUCT_SIZE;
	}
	if(dcd_lenth >= DCD_BASE_SIZE)
		return -2;

	dcd_w_header.version = version;
	dcd_w_header.tab_size = dev_size;
	dcd_w_header.lenth = caldcd_data_len(dev_size,mess);
	dcd_w_header.crc32 = caldcd_write_crc32(dev_size,mess,p_data);
	
	dcd_port_erase(DCD_BASE_ADDR,DCD_BASE_SIZE);
	dcd_port_write(feed_address,(uint32_t *)&dcd_w_header,DCD_HEADER_STRUCT_SIZE);
	feed_address += DCD_HEADER_STRUCT_SIZE;
	dcd_port_write(feed_address,(uint32_t *)mess,DEV_MESSAGE_STRUCT_SIZE*dev_size);
	feed_address += DEV_MESSAGE_STRUCT_SIZE*dev_size;
	for(int i=0;i<dev_size;i++)
	{
		dcd_port_write(feed_address,(uint32_t *)p_data[i],mess[i].lenth);
		feed_address += mess[i].lenth;
	}
	return 0;
}


/* DCD read from flash */
static uint32_t caldcd_read_crc32(uint32_t *buff,uint16_t len)
{
	uint32_t crc32 = 0;
	crc32 = af_calc_crc32(crc32,buff,len);
	return crc32;
}

static int dcd_read_check_header(uint32_t *buff,uint16_t *lenth,uint32_t *size)
{
	uint32_t *p_buff = buff;
	dcd_header_struct dcd_r_header = {0};
	uint16_t dcd_headsz = sizeof(dcd_r_header);
	dcd_port_read(DCD_BASE_ADDR,(uint32_t *)p_buff,dcd_headsz);
	memcpy((dcd_header_struct *)&dcd_r_header,p_buff,dcd_headsz);
				
	if(dcd_r_header.tag != DCD_TAG)
		return -1;
	if(dcd_r_header.lenth >= DCD_BASE_SIZE-DCD_HEADER_STRUCT_SIZE)
		return -2;

//	printf("dcd_r_header = 0x%02X,0x%02X,0x%04X,0x%X,0x%02X\r\n",dcd_r_header.tag,dcd_r_header.version,\
		dcd_r_header.lenth,dcd_r_header.crc32,dcd_r_header.tab_size);
	
	p_buff += dcd_headsz/sizeof(uint32_t);
	dcd_port_read(DCD_BASE_ADDR+DCD_HEADER_STRUCT_SIZE,(uint32_t *)p_buff,dcd_r_header.lenth);
	
	if(dcd_r_header.crc32 != caldcd_read_crc32(p_buff,dcd_r_header.lenth))
		return -3;
	
	*lenth = dcd_r_header.lenth;
	*size = dcd_r_header.tab_size;
	return 0;
}

static int dcd_read_check_flash(uint32_t *buff)
{
	uint32_t *p_buff = buff;
	uint16_t data_lenth = 0;
	uint32_t tab_size = 0;
	int head_ret = dcd_read_check_header(p_buff,&data_lenth,&tab_size);
	if(head_ret != 0)
	{
//		dcd_port_erase(DCD_BASE_ADDR,DCD_BASE_SIZE);	
//		printf("header error = %d\r\n",head_ret);
		return -1;
	}
	p_buff += DCD_HEADER_STRUCT_SIZE/sizeof(uint32_t);
	uint16_t sum_len = 0;
	for(int i=0;i<tab_size;i++)
	{
		uint16_t dev_len = (uint16_t )((*p_buff) >> 16);
//		printf("dev_len = %d\r\n",dev_len);
		p_buff += DEV_MESSAGE_STRUCT_SIZE/sizeof(uint32_t);
		sum_len += dev_len+DEV_MESSAGE_STRUCT_SIZE;
	}
	if(data_lenth != sum_len)
	{
//		printf("header lenth error \r\n");
		return -2;
	}
	return 0;
}

/**
 * func：			读取设备配置（包含校验）
 * @param buff：	读取数据的缓存区	
 * @return 			-1 标签tag错误
 *					-2 长度不合法
 *					-3 crc32检验失败
 *					-4 设备类型不存在
 *					0	读取成功
 */				
int dcd_config_device(uint32_t *buff)
{
	uint32_t *p_iap =  buff+(DCD_HEADER_STRUCT_SIZE/sizeof(uint32_t)-1);
	uint32_t tab_size = 0;

	int ret = dcd_read_check_flash((uint32_t *)buff);
	if(ret < 0)
	{
		return ret;
	}
	tab_size = *p_iap;	
//	printf("tab_size = %d\r\n",tab_size);
	p_iap += 1;
	uint32_t *p_dev_cfg = p_iap;
	p_dev_cfg += tab_size*DEV_MESSAGE_STRUCT_SIZE/sizeof(uint32_t);
	for(int i=0;i<tab_size;i++)
	{
		uint16_t dev_type = (uint16_t)(*p_iap);
		uint16_t dev_len = (uint16_t)((*p_iap)>>16);
		p_iap += DEV_MESSAGE_STRUCT_SIZE/sizeof(uint32_t);
//		printf("dev_type = 0x%08x\r\n",dev_type);
		switch (dev_type)
		{
			case DCD_DEV_GPIO:
			{
				int dev_num = dev_len/DEV_GPIO_STRUCT_SIZE;
				dev_gpio_struct *dev_conf = (dev_gpio_struct *)malloc(dev_len);
				memcpy(dev_conf, p_dev_cfg, dev_len);
				dev_gpio_struct *p_dev_conf = dev_conf;
//				printf("* p_gpio_conf = 0x%08X\r\n",*p_dev_conf);
				while(dev_num--)
				{
					/* gpio configure */
					{
						if(GPIO_CFG_CpltCallback(p_dev_conf) < 0)
							return -5;
					}
//					printf("p_gpio_conf = 0x%08X\r\n",p_dev_conf);
//					printf("*p_gpio_conf = 0x%08X\r\n",*p_dev_conf);
					p_dev_conf++;
				}
				p_dev_cfg += dev_len/sizeof(uint32_t);
				free(dev_conf);
				break;
			}
			case DCD_DEV_RCC:
			{
				int dev_num = dev_len/DEV_RCC_STRUCT_SIZE;
				dev_rcc_struct *dev_conf = (dev_rcc_struct *)malloc(dev_len);
				memcpy(dev_conf, p_dev_cfg, dev_len);
				dev_rcc_struct *p_dev_conf = dev_conf;
//				printf("* p_rcc_conf = 0x%08X\r\n",*p_dev_conf);
				while(dev_num--)
				{
					/* rcc configure */
					{
						if(RCC_CFG_CpltCallback(p_dev_conf)<0)
							return -6;
					}
//					printf("p_rcc_conf = 0x%08X\r\n",p_dev_conf);
//					printf("*p_rcc_conf = 0x%08X\r\n",*p_dev_conf);
					p_dev_conf++;
				}
				p_dev_cfg += dev_len/sizeof(uint32_t);
				free(dev_conf);
				break;
			}
			default:
			{
				return -4;
			}
		}
	}
	return 0;
}


/* 单个GPIO设备的配置参数，数据结构为 dev_gpio_struct */
int GPIO_CFG_CpltCallback(dev_gpio_struct *dev_gpio) __attribute__((weak));
/* 单个RCC设备的配置参数，数据结构为 dev_rcc_struct */
int RCC_CFG_CpltCallback(dev_rcc_struct *dev_rcc) __attribute__((weak));

int GPIO_CFG_CpltCallback(dev_gpio_struct *dev_gpio)
{
//	printf("GPIO_CFG_CpltCallback have not used.\r\n");
}
int RCC_CFG_CpltCallback(dev_rcc_struct *dev_rcc)	
{
//	printf("RCC_CFG_CpltCallback have not used.\r\n");	
}

