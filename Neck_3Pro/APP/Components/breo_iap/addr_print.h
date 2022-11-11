#ifndef __ADDR_PRINT_H
#define __ADDR_PRINT_H

#include "stdio.h"
#include "iap_addr_def.h"

//打印flash区的地址，用于调试
static inline int dump_print(){
    printf("\nFlash address and size:\n");
    printf("BOOTLOAD_FIRMWARE_ADDR      =   0x%08x\n",BOOTLOAD_FIRMWARE_ADDR);
    printf("BOOTLOAD_FIRMWARE_SIZE      =   %dK\n",BOOTLOAD_FIRMWARE_SIZE/1024);
    printf("FACTORY_FIRMWARE_ADDR       =   0x%08x\n",FACTORY_FIRMWARE_ADDR);
    printf("FACTORY_FIRMWARE_SIZE       =   %dK\n",FACTORY_FIRMWARE_SIZE/1024);
    printf("APPLICATION_FIRMWARE_ADDR   =   0x%08x\n",APPLICATION_FIRMWARE_ADDR );
    printf("APPLICATION_FIRMWARE_SIZE   =   %dK\n",APPLICATION_FIRMWARE_SIZE/1024);
    printf("DOWNLOAD_AREA_ADDR      =   0x%08x\n",DOWNLOAD_AREA_ADDR);
    printf("DOWNLOAD_AREA_SIZE      =   %dK\n",DOWNLOAD_AREA_SIZE/1024);
    printf("UPGRADE_DATA_ADDR       =   0x%08x\n",UPGRADE_DATA_ADDR);
    printf("UPGRADE_DATA_SIZE       =   %dK\n",UPGRADE_DATA_SIZE/1024);
    printf("DCD_DATA_ADDR           =   0x%08x\n", DCD_DATA_ADDR);
    printf("DCD_DATA_SIZE           =   %dK\n", DCD_DATA_SIZE/1024);
    printf("USER_DATA_ADDR          =   0x%08x\n", USER_DATA_ADDR);
    printf("USER_DATA_SIZE          =   %dK\n", USER_DATA_SIZE/1024);
    printf("\n");
};

#endif
