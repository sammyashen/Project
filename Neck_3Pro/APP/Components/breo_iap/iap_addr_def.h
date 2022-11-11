#ifndef __IAP_ADDR_DEF_H
#define __IAP_ADDR_DEF_H

//芯片flash大小
#define BR_PAGE_SIZE                      (0x800)       /* 2 Kbytes */
#define BR_FLASH_SIZE                     (0x20000)     /* 128 KBytes */
#define BR_FLASH_BASE                     (0x08000000)  /* flash起始地址 */
#define BR_FLASH_END_ADDRESS              (BR_FLASH_BASE+BR_FLASH_SIZE)

//external flash information
#define SPI_FLASH_SIZE					  (16 * 1024 * 1024)
#define SPI_FLASH_BASE					  (0x0)
#define SPI_FLASH_END_ADDRESS			  (SPI_FLASH_BASE+SPI_FLASH_SIZE)

//flash 修改这三个宏定义,或者自行修改参数
#define BR_ONEUPGRADE_AREA              1       //1:一个更新区域(出厂和更新APP在一个区域)，0:两个更新区域（出厂和更新APP在两个区域）
#define BR_FLASH_512K                   1       //1:512K的flash宏定义,0:flash为256K（暂设两个内存大小）

//extern flash macro definition
#define SPI_FLASH_ENABLE				0		//1:enable extern flash  0:disable extern flash

#if BR_ONEUPGRADE_AREA
#if BR_FLASH_512K                               //一个更新区域,512K

#if SPI_FLASH_ENABLE
#define BINPAGE_SIZE                    (480*1024/BR_PAGE_SIZE)     //升级文件所需页数

#define BOOTLOAD_FIRMWARE_SIZE          (30 * 1024)	
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (480 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (480 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (480 * 1024)
#define DOWNLOAD_AREA_ADDR              (SPI_FLASH_BASE)
#else
#define BINPAGE_SIZE                    (50*1024/BR_PAGE_SIZE)     //升级文件所需页数

#define BOOTLOAD_FIRMWARE_SIZE          (10 * 1024)	
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (50 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (50 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (50 * 1024)
#define DOWNLOAD_AREA_ADDR              (APPLICATION_FIRMWARE_ADDR + APPLICATION_FIRMWARE_SIZE)
#endif
#define UPGRADE_DATA_SIZE               (2 * 1024)
#define UPGRADE_DATA_ADDR               (DOWNLOAD_AREA_ADDR + DOWNLOAD_AREA_SIZE)
#define DCD_DATA_SIZE                   (2 * 1024)
#define DCD_DATA_ADDR                   (UPGRADE_DATA_ADDR + UPGRADE_DATA_SIZE)                 //电源引脚存储区
#define USER_DATA_SIZE                  (2 * 1024)
#define USER_DATA_ADDR                  (DCD_DATA_ADDR + DCD_DATA_SIZE)                     	//LOG区待用

#define BOOTadrress                     BOOTLOAD_FIRMWARE_ADDR          //0x08000000,10K,bootloader
#define APP1adress                      FACTORY_FIRMWARE_ADDR           //0x08002800,50K,第一个应用程序起始地址(存放在FLASH)
#define APPUPadress                     APPLICATION_FIRMWARE_ADDR       //0x08002800,50K,需要更新区
#define APPTEMPadress                   DOWNLOAD_AREA_ADDR              //0x0800F000,50K,更新数据暂存的地址
#define UPGRADEaddr                     UPGRADE_DATA_ADDR               //0x0801B800,2K,固件更新标识要写入的FLASH的首地址
#define DCDadress                       DCD_DATA_ADDR                   //0x0801C000,2K,电源引脚数据存储区
#define USERadress                      USER_DATA_SIZE                  //0x0801C800,2K,日志区

#else                                   //一个更新区域,256K

#if SPI_FLASH_ENABLE
#define BINPAGE_SIZE                    (224*1024/BR_PAGE_SIZE)

#define BOOTLOAD_FIRMWARE_SIZE          (30 * 1024)	
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (224 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (224 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (224 * 1024)
#define DOWNLOAD_AREA_ADDR              (SPI_FLASH_BASE)
#else
#define BINPAGE_SIZE                    (100*1024/BR_PAGE_SIZE)

#define BOOTLOAD_FIRMWARE_SIZE          (16 * 1024)	
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (100 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (100 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (100 * 1024)
#define DOWNLOAD_AREA_ADDR              (APPLICATION_FIRMWARE_ADDR + APPLICATION_FIRMWARE_SIZE)
#endif
#define UPGRADE_DATA_SIZE               (4 * 1024)
#define UPGRADE_DATA_ADDR               (DOWNLOAD_AREA_ADDR + DOWNLOAD_AREA_SIZE)
#define DCD_DATA_SIZE                   (4 * 1024)
#define DCD_DATA_ADDR                   (UPGRADE_DATA_ADDR + UPGRADE_DATA_SIZE)                 //电源引脚存储区
#define USER_DATA_SIZE                  (32 * 1024)
#define USER_DATA_ADDR                  (DCD_DATA_ADDR + DCD_DATA_SIZE)                     //LOG区待用

#define APP1adress                      APPLICATION_FIRMWARE_ADDR       //第一个应用程序起始地址(存放在FLASH)
#define APPUPadress                     APPLICATION_FIRMWARE_ADDR       //需要更新区
#define APPTEMPadress                   DOWNLOAD_AREA_ADDR              //更新数据暂存的地址
#define UPGRADEaddr                     UPGRADE_DATA_ADDR               //固件更新标识要写入的FLASH的首地址
#define DCDadress                       DCD_DATA_ADDR                   //电源引脚数据存储区
#define USERadress                      USER_DATA_ADDR                  //日志区

#endif

#else                                   //两个更新区域,512K
#if  BR_FLASH_512K

#if SPI_FLASH_ENABLE
#define BINPAGE_SIZE                    (240*1024/BR_PAGE_SIZE)

#define BOOTLOAD_FIRMWARE_SIZE          (30 * 1024)	//16K->30K
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (240 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (240 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (FACTORY_FIRMWARE_ADDR + FACTORY_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (240 * 1024)
#define DOWNLOAD_AREA_ADDR              (SPI_FLASH_BASE)
#else
#define BINPAGE_SIZE                    (152*1024/BR_PAGE_SIZE)

#define BOOTLOAD_FIRMWARE_SIZE          (16 * 1024)	
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (152 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (152 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (FACTORY_FIRMWARE_ADDR + FACTORY_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (152 * 1024)
#define DOWNLOAD_AREA_ADDR              (APPLICATION_FIRMWARE_ADDR + APPLICATION_FIRMWARE_SIZE)
#endif
#define UPGRADE_DATA_SIZE               (4 * 1024)
#define UPGRADE_DATA_ADDR               (DOWNLOAD_AREA_ADDR + DOWNLOAD_AREA_SIZE)
#define DCD_DATA_SIZE                   (4 * 1024)
#define DCD_DATA_ADDR                   (UPGRADE_DATA_ADDR + UPGRADE_DATA_SIZE)                 //电源引脚存储区
#define USER_DATA_SIZE                  (32 * 1024)
#define USER_DATA_ADDR                  (DCD_DATA_ADDR + DCD_DATA_SIZE)                     //LOG区待用

#define BOOTadrress                     BOOTLOAD_FIRMWARE_ADDR          //10K,bootloader
#define APP1adress                      FACTORY_FIRMWARE_ADDR           //240K,第一个应用程序起始地址(存放在FLASH)
#define APPUPadress                     APPLICATION_FIRMWARE_ADDR       //240K,需要更新区
#define APPTEMPadress                   DOWNLOAD_AREA_ADDR              //240K,更新数据暂存的地址
#define UPGRADEaddr                     UPGRADE_DATA_ADDR               //2K,固件更新标识要写入的FLASH的首地址
#define DCDadress                       DCD_DATA_ADDR                   //2K,电源引脚数据存储区
#define USERadress                      USER_DATA_ADDR                  //18K,日志区

#else                                   //两个更新区域,256K

#if SPI_FLASH_ENABLE
#define BINPAGE_SIZE                    (112*1024/BR_PAGE_SIZE)

#define BOOTLOAD_FIRMWARE_SIZE          (30 * 1024)	//16K->30K
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (112 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (112 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (FACTORY_FIRMWARE_ADDR + FACTORY_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (112 * 1024)
#define DOWNLOAD_AREA_ADDR              (SPI_FLASH_BASE)
#else
#define BINPAGE_SIZE                    (68*1024/BR_PAGE_SIZE)
#define BOOTLOAD_FIRMWARE_SIZE          (16 * 1024)	//16K
#define BOOTLOAD_FIRMWARE_ADDR          BR_FLASH_BASE
#define FACTORY_FIRMWARE_SIZE           (68 * 1024)
#define FACTORY_FIRMWARE_ADDR           (BOOTLOAD_FIRMWARE_ADDR + BOOTLOAD_FIRMWARE_SIZE)
#define APPLICATION_FIRMWARE_SIZE       (68 * 1024)
#define APPLICATION_FIRMWARE_ADDR       (FACTORY_FIRMWARE_ADDR + FACTORY_FIRMWARE_SIZE)
#define DOWNLOAD_AREA_SIZE              (68 * 1024)
#define DOWNLOAD_AREA_ADDR              (APPLICATION_FIRMWARE_ADDR + APPLICATION_FIRMWARE_SIZE)
#endif
#define UPGRADE_DATA_SIZE               (4 * 1024)
#define UPGRADE_DATA_ADDR               (DOWNLOAD_AREA_ADDR + DOWNLOAD_AREA_SIZE)
#define DCD_DATA_SIZE                   (4 * 1024)
#define DCD_DATA_ADDR                   (UPGRADE_DATA_ADDR + UPGRADE_DATA_SIZE)                 //电源引脚存储区
#define USER_DATA_SIZE                  (28 * 1024)
#define USER_DATA_ADDR                  (DCD_DATA_ADDR + DCD_DATA_SIZE)                     //LOG区待用

#define BOOTadrress                     BOOTLOAD_FIRMWARE_ADDR          //10K,bootloader
#define APP1adress                      FACTORY_FIRMWARE_ADDR           //第一个应用程序起始地址(存放在FLASH)
#define APPUPadress                     APPLICATION_FIRMWARE_ADDR       //需要更新区
#define APPTEMPadress                   DOWNLOAD_AREA_ADDR              //更新数据暂存的地址
#define UPGRADEaddr                     UPGRADE_DATA_ADDR               //固件更新标识要写入的FLASH的首地址
#define DCDadress                       DCD_DATA_ADDR                   //电源引脚数据存储区
#define USERadress                      USER_DATA_ADDR                  //日志区

#endif
#endif

/* NVIC偏移地址 */
#define VECT_TAB_ADDRESS                APPLICATION_FIRMWARE_ADDR

/* 下载地址 */
#define AppTempAddress                  DOWNLOAD_AREA_ADDR

#endif
