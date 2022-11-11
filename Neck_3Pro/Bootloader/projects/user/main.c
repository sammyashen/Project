#include "af_flash.h"
#include "iap.h"
#include "iap_addr_def.h"
#include "upgrade_format_def.h"
#include "dcd_user.h"

extern uint32_t IAPBUF[512];

u32 iapf0;
u32 head_flag=0,head_version=0,readfile_checksum=0;
u32 file_size=0;
u8 binpage=0;

static void delay_ms(u16 nms)
{
    u16 temp,i;
    for(i=0;i<nms;i++)
    {
        for(temp=0;temp<100;temp++);
    }
}


static void flash_init(void)
{
	if(FLASH_HSICLOCK_DISABLE == FLASH_ClockInit())
    {
//        printf("HSI oscillator not yet ready\r\n");
        while(1);
    }
}

int main(void)
{
    static const u32 goto_app_flag = UPGRADE_DONE;
    int err = 0;
	
	RCC_ClocksType RCC_Clocks;
	RCC_GetClocksFreqValue(&RCC_Clocks);
	flash_init();						//flash init
	
#if SPI_FLASH_ENABLE
	dcd_port_init();
#endif

	//RCC_Configuration();
    //Get and config device data
    uint32_t arr_buff[128] = {0};
    dcd_config_device(arr_buff);

#if SPI_FLASH_ENABLE
	{
		uint8_t _buff[4];
		dcd_port_read(UPGRADEaddr, (uint32_t *)_buff, 4);
		iapf0 = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
	}
#else	
    iapf0 = *(u32*)(UPGRADEaddr);               //读取固件更新标志位
#endif
    if(iapf0 == UPGRADE_START)                  //读出的是1A 1A 2B 2B 用的是新的版本号
    {
        do
        {
#if SPI_FLASH_ENABLE
			{
				uint8_t _buff[4];
				dcd_port_read(APPTEMPadress, (uint32_t *)_buff, 4);
				head_flag = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
				dcd_port_read(APPTEMPadress+4, (uint32_t *)_buff, 4);
				head_version = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
				dcd_port_read(APPTEMPadress+8, (uint32_t *)_buff, 4);
				file_size = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
				dcd_port_read(APPTEMPadress+12, (uint32_t *)_buff, 4);
				readfile_checksum = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
			}
#else
            head_flag=*(u32*)(APPTEMPadress);
            head_version=*(u32*)(APPTEMPadress+4);
            file_size=*(u32*)(APPTEMPadress+8);
            readfile_checksum=*(u32*)(APPTEMPadress+12);
#endif
            if(file_size%BR_PAGE_SIZE != 0)
                binpage = file_size/BR_PAGE_SIZE+1;
            else
                binpage = file_size/BR_PAGE_SIZE;

            if(binpage>BINPAGE_SIZE)
            {
                err = 1;
                break;
            }

            if (PACKET_HEAD_FLAG != head_flag)
            {
                err = 1;
                break;
            }
            if(PACKET_VERSION!=head_version)
            {
                err = 1;
                break;
            }
            unsigned long download_app_base = APPTEMPadress + UPGRADE_PACKET_HEAD_SIZE;
#if SPI_FLASH_ENABLE
			u32 file_checksum = af_spi_flash_check(download_app_base, file_size);
#else
            u32 file_checksum = af_flash_check(download_app_base, file_size);
#endif
            if (readfile_checksum!= file_checksum)
            {
                err = 1;
                break;
            }

            for(u8 i=0;i<binpage;i++)                                       //不超过XXK程序
            {
                u32 j=0,k=0;
                for(j=0;j<512;j++)
                {
#if SPI_FLASH_ENABLE
					{
						uint8_t _buff[4];
						dcd_port_read(download_app_base+k+i*BR_PAGE_SIZE, (uint32_t *)_buff, 4);
						IAPBUF[j] = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
					}
#else
                    IAPBUF[j] = *(u32*)(download_app_base+k+i*BR_PAGE_SIZE);    //将临时Flash里的数据读出来    每次读2K字节
#endif
                    k+=4;
                }
                k = 0;
                af_flash_unlock();
                af_flash_earse_one_page(APPUPadress+i*BR_PAGE_SIZE);            //先擦后写，每次擦除一页地址(2K);
                for(j=0;j<512;j++)
                {
                    af_flash_program_word((APPUPadress+j*4+i*BR_PAGE_SIZE),IAPBUF[j]);//flash  为一个字节存储，32位数据必须地址加4
                }
                af_flash_lock();
                j = 0;
                delay_ms(2);
            }

            delay_ms(30);

            file_checksum = af_flash_check(APPUPadress, file_size);
            if (readfile_checksum != file_checksum)
            {
                err = 2;
                break;
            }
#if SPI_FLASH_ENABLE
			dcd_port_erase(UPGRADEaddr, 2048);
			dcd_port_write(UPGRADEaddr, &goto_app_flag, 4);
#else
            af_flash_unlock();
            af_flash_earse_one_page(UPGRADEaddr);           //先擦后写，每次擦除一页地址(2K);
            af_flash_program_word(UPGRADEaddr,goto_app_flag);//flash  为一个字节存储，32位数据必须地址加4
            af_flash_lock();
#endif
            break;
        }
        while (0);

        if (err == 1)
        {
#if SPI_FLASH_ENABLE
			af_spi_erase_upgrade_data_partition();	
#else
            af_erase_upgrade_data_partition();
#endif
        }
        else if (err == 2)
        {
            SoftReset();                                        //软复位
        }
        else
        {
            ;
        }
    }

#if SPI_FLASH_ENABLE
	{
		uint8_t _buff[4];
		dcd_port_read(UPGRADEaddr, (uint32_t *)_buff, 4);
		iapf0 = (_buff[3]<<24) | (_buff[2]<<16) | (_buff[1]<<8) | _buff[0];
	}
#else
    iapf0 = *(u32*)(UPGRADEaddr);                               //读取固件更新标志位
#endif
    if(iapf0 == UPGRADE_DONE)                                   //读出的是3C 3C 4D 4D 跳转到更新的固件程序
    {
        uint32_t app_addr = ((*(vu32*)(APPUPadress+4))&0xFF000000);
        if(app_addr == 0x08000000)                              //判断是否为0X08XXXXXX.
        {
            iap_load_app(APPUPadress);                          //执行更新后的APP代码
        }
    }else if((iapf0 == UPGRADE_START)&&(APP1adress == APPUPadress)){    //读出的是1A 1A 2B 2B ,且更新区与出厂区一致
            SoftReset();                                                //软复位
    }

    {
        if(((*(vu32*)(APP1adress+4))&0xFF000000)==0x08000000)   //判断是否为0X08XXXXXX.
        {
            iap_load_app(APP1adress);                           //执行出厂时的APP代码
        }
    }
}
