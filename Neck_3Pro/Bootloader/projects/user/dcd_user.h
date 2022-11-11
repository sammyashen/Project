#ifndef __DCD_USER_H
#define __DCD_USER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "af_utils.h"
#include "dcd_cfg.h"

/* DCD version,if you want to upgrade dcd,change the version */
#define DCD_TAG				0x4F

/* Add user device */
#define	DCD_DEV_RCC			0x01		//设备类型为RCC
#define	DCD_DEV_GPIO 		0x02		//设备类型为GPIO

#define DCD_HEADER_STRUCT_SIZE		sizeof(dcd_header_struct)
#define DEV_MESSAGE_STRUCT_SIZE		sizeof(dev_message_struct)
#define DEV_GPIO_STRUCT_SIZE		sizeof(dev_gpio_struct)
#define DEV_RCC_STRUCT_SIZE 		sizeof(dev_rcc_struct)

typedef struct{
	uint8_t tag;			//固定标签
	uint8_t version;		//版本
	uint16_t lenth;			//数据长度（不包括头）
	uint32_t crc32;
	uint32_t tab_size;		//设备种类数量
}dcd_header_struct;			//头结构

typedef struct{
	uint16_t dev_type;			//设备类型
	uint16_t lenth;				//设备长度
}dev_message_struct;			//设备信息

typedef struct{
	uint32_t sysclk		;		//系统时钟频率
    uint32_t pllsrc   	;		//pll源
    uint32_t pllmul  	;		//pll倍频
    uint32_t pclk1div 	;		//pclk1分频
    uint32_t pclk2div 	;		//pclk2分频
}dev_rcc_struct;				//RCC结构体

typedef struct{
	DCD_GPIO_PortType port:8;		//字符A,B,C,D,E,F,G...
	DCD_GPIO_PinType pin:8;			//pin 0~15
	DCD_GPIO_ModeType mode:8;		//GPIO_ModeType
	DCD_GPIO_PuPdType pdmode:2;		//GPIO_PuPdType
	DCD_GPIO_SpeedType speed:4;		//GPIO_SpeedType
	DCD_GPIO_LevelType level:2;		//0:low;1:high
}dev_gpio_struct;

typedef enum
{
    DCD_NO_ERR,
    DCD_ERASE_ERR,
    DCD_READ_ERR,
    DCD_WRITE_ERR
} DcdErrCode;
	
/* dcd flash function */ 
DcdErrCode dcd_port_read(uint32_t addr, uint32_t *buf, size_t size);
DcdErrCode dcd_port_erase(uint32_t addr, size_t size);
DcdErrCode dcd_port_write(uint32_t addr, const uint32_t *buf, size_t size);

/* dcd write/read flash function */
int dcd_write_to_flash(uint8_t version,uint32_t dev_size,const dev_message_struct *mess,const uint32_t **p_data);			//写设备函数
int dcd_config_device(uint32_t *buff);															//读设备函数
int GPIO_CFG_CpltCallback(dev_gpio_struct *dev_gpio);											//gpio设备回调，获取一个gpio设备信息
int RCC_CFG_CpltCallback(dev_rcc_struct *dev_rcc);												//gpio设备回调，获取一个gpio设备信息

void dcd_write_example(void);																	//dcd写设备例程
void dcd_read_example(void);																	//dcd读设备例程

#endif




