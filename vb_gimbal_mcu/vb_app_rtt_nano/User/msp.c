#include "rtthread.h"
#include "msp.h"
#include <stdlib.h>

static void show_addr(int argc, char **argv)
{
	rt_uint32_t i = 0;
	rt_uint8_t addr_str_len = 0;
	char addr_buf_str[10] = {0};
	rt_uint8_t addr_buf[8] = {0};
	rt_uint32_t address = 0x0;
	rt_uint32_t length = 0;
	char str[8] = {0};
	/* 数据类型指针 */
	rt_uint8_t *ptr_hex = RT_NULL;
	rt_uint32_t *ptr_word = RT_NULL;
	rt_int8_t *ptr_char = RT_NULL;
	rt_uint8_t *ptr_uchar = RT_NULL;
	rt_int32_t *ptr_int = RT_NULL;
	float32_t *ptr_float = RT_NULL;

	if(argc == 1){
		rt_kprintf("please input [address] [length] [-type].\r\n");
	}else if(argc == 2){
		if(!rt_strcmp("-help", argv[1])){
			rt_kprintf("[address] : Please input address,eg:0xc000.\n");
			rt_kprintf("[length]  : Please input length,eg:100.\n");
			rt_kprintf("            [!note] : length must <=1024.\n");
			rt_kprintf("[-type]   : Please input type,eg:-h.\n");
			rt_kprintf("            [!note] : [-h]  --- show data and char as hex.\n");
			rt_kprintf("                      [-w]  --- show data as word.\n");
			rt_kprintf("                      [-c]  --- show data as char.\n");
			rt_kprintf("                      [-uc] --- show data as unsigned char.\n");
			rt_kprintf("                      [-i]  --- show data as int.\n");
			rt_kprintf("                      [-ui] --- show data as unsigned int.\n");
			rt_kprintf("                      [-f]  --- show data as float.\n");
		}else{
			rt_kprintf("please input [length] [-type].\r\n");
		}
	}else if(argc == 3){
		rt_kprintf("please input [-type].\r\n");
	}else if(argc == 4){
		show_type_e show_type = _UNKNOW;
		/* 判断显示类型 */
		if(!rt_strcmp("-h", argv[3]))			show_type = _HEX;
		else if(!rt_strcmp("-w", argv[3]))		show_type = _WORD;
		else if(!rt_strcmp("-c", argv[3]))		show_type = _CHAR;
		else if(!rt_strcmp("-uc", argv[3]))		show_type = _UCHAR;
		else if(!rt_strcmp("-i", argv[3]))		show_type = _INT;
		else if(!rt_strcmp("-ui", argv[3]))		show_type = _UINT;
		else if(!rt_strcmp("-f", argv[3]))		show_type = _FLOAT;
		/* 转换地址字符串 */
		addr_str_len = rt_strlen(argv[1]);
		if(addr_str_len > 10){
			rt_kprintf("wrong address.\r\n");
			return;
		}
		rt_memcpy(addr_buf_str, argv[1], (addr_str_len > 2? addr_str_len : 0));
		if(addr_buf_str[0] == '0' && (addr_buf_str[1] == 'x' || addr_buf_str[1] == 'X')){
			for(i = 0;i < (addr_str_len - 2);i++){
				if(addr_buf_str[addr_str_len - 1 - i] >= '0' && addr_buf_str[addr_str_len - 1 - i] <= '9'){
					addr_buf[7 - i] = 0x00 + addr_buf_str[addr_str_len - 1 - i] - '0';
				}else if(addr_buf_str[addr_str_len - 1 - i] >= 'a' && addr_buf_str[addr_str_len - 1 - i] <= 'f'){
					addr_buf[7 - i] = 0x0A + addr_buf_str[addr_str_len - 1 - i] - 'a';
				}else if(addr_buf_str[addr_str_len - 1 - i] >= 'A' && addr_buf_str[addr_str_len - 1 - i] <= 'F'){
					addr_buf[7 - i] = 0x0A + addr_buf_str[addr_str_len - 1 - i] - 'A';
				}else 
					break;
			}

			if(i < (addr_str_len - 2)){
				rt_kprintf("wrong address.\r\n");
				return;
			}
			
			address = (addr_buf[0]<<28)|(addr_buf[1]<<24)|(addr_buf[2]<<20)|(addr_buf[3]<<16)|
							(addr_buf[4]<<12)|(addr_buf[5]<<8)|(addr_buf[6]<<4)|(addr_buf[7]);
		}else{
			rt_kprintf("wrong address.\r\n");
			return;
		}
		/* 获取数据长度 */
		if(atoi(argv[2]) > 0 && atoi(argv[2]) <= 1024){		//最多读取1024个数据
			length = atoi(argv[2]);
		}else{
			rt_kprintf("invalid length.\r\n");
			return;
		}

#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
		switch(show_type)
		{
			case _HEX:
				/* 防止指针操作非法地址 */
				if(((address + length) > 0x80000 && (address + length) < 0x1FFF8000) || 	//非法地址区
					((address + length) > 0x20028000) || ((address & 0x80000000) == 0x80000000)										
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_hex = (rt_uint8_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_hex + (i<<3)), (ptr_hex + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							str[j] = __is_print((*(ptr_hex + (i<<3) + j))) ? (*(ptr_hex + (i<<3) + j)) : '.';
							rt_kprintf("[%02X] ", *(ptr_hex + (i<<3) + j));
						}else{
							str[j] = ' ';
							rt_kprintf("     ");
						}
					}
					rt_kprintf(" ----  [");
					for(rt_uint8_t k = 0;k < 8;k++){
						rt_kprintf("%c", str[k]);
					}
					rt_kprintf("] ");
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);	//由于是异步发送串口，所以交出cpu防止buff覆盖
				}
			break;

			case _WORD:
				if(((address + (length<<2)) > 0x80000 && (address + (length<<2)) < 0x1FFF8000) || 	
					((address + (length<<2)) > 0x20028000) || ((address & 0x80000000) == 0x80000000)									
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_word = (rt_uint32_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_word + (i<<3)), (ptr_word + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							rt_kprintf("[%08X] ", *(ptr_word + (i<<3) + j));
						}else{
							rt_kprintf("   ");
						}
					}
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);	
				}
			break;

			case _CHAR:
				if(((address + length) > 0x80000 && (address + length) < 0x1FFF8000) || 	
					((address + length) > 0x20028000) || ((address & 0x80000000) == 0x80000000)									
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_char = (rt_int8_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_char + (i<<3)), (ptr_char + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							rt_kprintf("[%4d] ", *(ptr_char + (i<<3) + j));
						}else{
							rt_kprintf("   ");
						}
					}
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);
				}
			break;

			case _UCHAR:
				if(((address + length) > 0x80000 && (address + length) < 0x1FFF8000) || 	
					((address + length) > 0x20028000) || ((address & 0x80000000) == 0x80000000)									
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_uchar = (rt_uint8_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_uchar + (i<<3)), (ptr_uchar + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							rt_kprintf("[%4u] ", *(ptr_uchar + (i<<3) + j));
						}else{
							rt_kprintf("   ");
						}
					}
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);
				}
			break;

			case _INT:
			case _UINT:
				if(((address + (length<<2)) > 0x80000 && (address + (length<<2)) < 0x1FFF8000) || 	
					((address + (length<<2)) > 0x20028000) || ((address & 0x80000000) == 0x80000000)									
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_int = (rt_int32_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_int + (i<<3)), (ptr_int + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							if(show_type == _INT){
								rt_kprintf("[%11d] ", *(ptr_int + (i<<3) + j));
							}else{
								rt_kprintf("[%11u] ", *(ptr_int + (i<<3) + j));
							}
						}else{
							rt_kprintf("   ");
						}
					}
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);	
				}
			break;

			case _FLOAT:
				if(((address + (length<<2)) > 0x80000 && (address + (length<<2)) < 0x1FFF8000) || 	
					((address + (length<<2)) > 0x20028000) || ((address & 0x80000000) == 0x80000000)									
				){
					rt_kprintf("operation illegal address.\r\n");
					return;
				}
				ptr_float = (float32_t *)address;
				for(i = 0;i < (((length - 1)>>3) + 1);i++){
					rt_kprintf("    [%08X-%08X]: ", (ptr_float + (i<<3)), (ptr_float + (i<<3) + 7));
					for(rt_uint8_t j = 0;j < 8;j++){
						if(((i<<3) + j) < length){
							rt_kprintf("[%14.4e] ", *(ptr_float + (i<<3) + j));
						}else{
							rt_kprintf("   ");
						}
					}
					rt_kprintf("\r\n");
					rt_thread_mdelay(2);	
				}
			break;
			
			default:
				rt_kprintf("unknow type.\r\n");
			break;
		}
	}else{
		rt_kprintf("paramater error.\r\n");
	}
}
MSH_CMD_EXPORT(show_addr, show_addr[addr or -help][len][-type]);



