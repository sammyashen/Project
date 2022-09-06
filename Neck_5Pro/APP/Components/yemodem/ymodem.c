#include "ymodem.h"
#include "dev_uart.h"
#include "include.h"
#include <fal.h>

#define APP_VERSION		"V1.0.1"

static struct rt_semaphore rysem;
static ymodem_t ymodem_struct;
static rt_uint8_t buf[MAX_PACK_SIZE];
static rt_uint32_t copy_setoff = 0;

static const rt_uint16_t ccitt_table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
static rt_uint16_t Calc_CRC16(unsigned char *q, int len)
{
    rt_uint16_t crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}

rt_uint8_t _rym_putchar(rt_uint8_t code)
{
	uart_write(DEV_UART1, &code, 1);
	return 1;
}

rt_err_t _rym_do_recv()
{
	rt_uint16_t recv_crc, cal_crc;
	rt_uint16_t _usRecvSize = 0;
	rt_uint8_t filename_len = 0;
	rt_err_t err = RT_EOK;
	static const struct fal_partition *partition = RT_NULL;

	partition = fal_partition_find("download");

	do
	{
		switch(ymodem_struct.ymodem_sta)
		{
			case FREE:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
				{
					ymodem_struct.timeout = 0;
					if(((buf[0] == SOH) || (buf[0] == STX)) && (buf[1] == 0x00) && (buf[2] == 0xFF)
						&& (buf[3] != 0x00))
					{
						recv_crc = (rt_uint16_t)(buf[_usRecvSize-2] << 8) | buf[_usRecvSize-1];
						cal_crc = Calc_CRC16((rt_uint8_t *)&buf[3], (_usRecvSize-5));
    					if(recv_crc == cal_crc)
    					{
							strcpy(ymodem_struct.filename, (char *)&buf[3]);
							filename_len = rt_strlen(ymodem_struct.filename);
							for(uint8_t i=0;i<10;i++)
							{
								if(buf[3+filename_len+1+i] == ' ')
								{
									buf[3+filename_len+1+i] = 0x00;
									break;
								}
							}
							ymodem_struct.filesize = atoi((char *)&buf[3+filename_len+1]);
							ymodem_struct.ymodem_sta = RYM_DELAY_st;
						
							_rym_putchar(ACK);
						}
						else
							_rym_putchar(NAK);
					}
				}
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
						_rym_putchar(CRC16);
					}
				}
			break;

			case RYM_DELAY_st:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
					ymodem_struct.timeout = 0;
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
						ymodem_struct.ymodem_sta = WAIT_DATA_PACK;
						_rym_putchar(CRC16);
					}
				}
			break;

			case WAIT_DATA_PACK:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
				{
					ymodem_struct.timeout = 0;
					if((_usRecvSize > 1) && (buf[0] == STX) && ((buf[1]+buf[2]) == 0xFF))
					{
						recv_crc = (rt_uint16_t)(buf[_usRecvSize-2] << 8) | buf[_usRecvSize-1];
						cal_crc = Calc_CRC16((rt_uint8_t *)&buf[3], (_usRecvSize-5));
    					if(recv_crc == cal_crc)
    					{
							rt_enter_critical();
							if(partition != RT_NULL)
								fal_partition_write(partition, copy_setoff, &buf[3], 0x400);
							rt_exit_critical();
							copy_setoff += 0x400;
							_rym_putchar(ACK);
						}
						else
							ymodem_struct.ymodem_sta = ERROR_DATA;
					}
					else if((_usRecvSize > 1) && (buf[0] == SOH) && ((buf[1]+buf[2]) == 0xFF))
					{
						recv_crc = (rt_uint16_t)(buf[_usRecvSize-2] << 8) | buf[_usRecvSize-1];
						cal_crc = Calc_CRC16((rt_uint8_t *)&buf[3], (_usRecvSize-5));
    					if(recv_crc == cal_crc)
    					{
							if((copy_setoff+0x80) > ymodem_struct.filesize)
							{
								rt_enter_critical();
								if(partition != RT_NULL)
									fal_partition_write(partition, copy_setoff, &buf[3], (ymodem_struct.filesize - copy_setoff));
								rt_exit_critical();
								copy_setoff = ymodem_struct.filesize;
							}
							else
							{
								rt_enter_critical();
								if(partition != RT_NULL)
									fal_partition_write(partition, copy_setoff, &buf[3], 0x80);
								rt_exit_critical();
								copy_setoff += 0x80;
							}
							_rym_putchar(ACK);
						}
						else
							ymodem_struct.ymodem_sta = ERROR_DATA;
					}
					else if((_usRecvSize == 1) && (buf[0] == EOT))
					{
						ymodem_struct.ymodem_sta = WAIT_EOT;
						_rym_putchar(NAK);
					}
					else
					{
						ymodem_struct.ymodem_sta = ERROR_DATA;
					}
				}
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
						_rym_putchar(CRC16);
					}
				}
			break;

			case ERROR_DATA:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize == 0)
				{
					if(++ymodem_struct.timeout > 10)
					{
						ymodem_struct.timeout = 0;
						ymodem_struct.ymodem_sta = WAIT_DATA_PACK;
						_rym_putchar(NAK);
					}
				}
			break;

			case WAIT_EOT:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
				{
					ymodem_struct.timeout = 0;
					if(buf[0] == EOT)
					{
						ymodem_struct.ymodem_sta = RYM_DELAY_nd;
						_rym_putchar(ACK);
					}
				}
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
						_rym_putchar(NAK);
					}
				}
			break;

			case RYM_DELAY_nd:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
				{	
					ymodem_struct.timeout = 0;
				}
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
						ymodem_struct.ymodem_sta = WAIT_END_PACK;
						_rym_putchar(CRC16);
					}
				}
			break;

			case WAIT_END_PACK:
				_usRecvSize = uart_read(DEV_UART1, buf, MAX_PACK_SIZE);
				if(_usRecvSize != 0)
				{
					ymodem_struct.timeout = 0;
					if(((buf[0] == SOH) || (buf[0] == STX)) && (buf[1] == 0x00) && (buf[2] == 0xFF)
						&& (buf[3] == 0x00))
					{
						_rym_putchar(ACK);
						ymodem_struct.ymodem_sta = END_Ymodem;
					}
				}
				else
				{
					if(++ymodem_struct.timeout > TIMEOUT)
					{
						ymodem_struct.timeout = 0;
					}
				}
			break;

			case END_Ymodem:
				rt_sem_release(&rysem);
			break;
		}

//		uart_poll_dma_tx(DEV_UART1);
	}while(rt_sem_take(&rysem, 10) != RT_EOK);

	return err;
}

void ymodem(void)
{
	rt_err_t res;
	const struct fal_partition *partition = RT_NULL;
	rt_uint32_t update_flag = 0x0;

	rt_sem_init(&rysem, "rymsem", 0, RT_IPC_FLAG_FIFO);

	partition = fal_partition_find("download");
	if(partition != RT_NULL)
	{
		if(fal_partition_erase(partition, 0, 0x19000) >= 0)
		{
			res = _rym_do_recv();
			if(res == RT_EOK)
			{
				rt_kprintf("ymodem recv successed.\r\n");
				update_flag = 0xAAAAAAAA;
				if(fal_partition_write(partition, (0x19000 - 4), (uint8_t *)&update_flag, sizeof(rt_uint32_t)) >= 0)
					rt_kprintf("update app update_flag successed.\r\n");
				else
					rt_kprintf("update app update_flag failed.\r\n");
			}
			else
				rt_kprintf("ymodem recv failed.\r\n");
		}
		else
			rt_kprintf("download area is not earse.\r\n");
	}
	else
		rt_kprintf("download area is not exist.\r\n");

	partition = fal_partition_find("easyflash");
	if(partition != RT_NULL)
	{
		if(fal_partition_erase(partition, 0, 0x2800) >= 0)
			rt_kprintf("easyflash area is earse ok.\r\n");
		else
			rt_kprintf("easyflash area is not earse.\r\n");
	}
	else
		rt_kprintf("easyflash area is not exist.\r\n");
}
MSH_CMD_EXPORT(ymodem, ymodem recv);

void check_app(void)
{
	rt_uint8_t debug[8];
	const struct fal_partition *partition = RT_NULL;

	partition = fal_partition_find("download");
	fal_partition_read(partition, 0, debug, 8);
	LOG_I("app first:0x%02X%02X%02X%02X", debug[0], debug[1], debug[2], debug[3]);
	fal_partition_read(partition, (0x19000 - 4), debug, 4);
	LOG_I("update flag:0x%02X%02x%02X%02X", debug[0], debug[1], debug[2], debug[3]);
}
MSH_CMD_EXPORT(check_app, check app);

void re_boot(void)
{
	__disable_irq();
	NVIC_SystemReset();
}
MSH_CMD_EXPORT(re_boot, re-boot app);

void show_version(void)
{
	rt_kprintf("%s\r\n", APP_VERSION);
}
MSH_CMD_EXPORT(show_version, show app version);



