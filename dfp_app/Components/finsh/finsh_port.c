/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rthw.h>
#include <rtconfig.h>
#include "dev_uart.h"

void rt_hw_console_output(const char *str)
{
	rt_size_t i = 0, size = 0;
    char a = '\r';
    size = rt_strlen(str);

    for (i = 0; i < size; i++)
    {
        if (*(str + i) == '\n')
        {
			uart_write(DEV_UART3, (uint8_t *)&a, 1);
        }
		uart_write(DEV_UART3, (uint8_t *)(str + i), 1);
    }
}

char rt_hw_console_getchar(void)
{
    /* Note: the initial value of ch must < 0 */
    int ch = -1;
	int len = 0;
	char rd_tmp;

	len = uart_read(DEV_UART3, (uint8_t *)&rd_tmp, 1);
	if (len != 0)
    {
        ch = rd_tmp;
    }
    else
    {
        rt_thread_mdelay(10);
    }

    return ch;
}


