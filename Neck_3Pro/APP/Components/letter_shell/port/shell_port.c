/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#include "tiny_os.h"
#include "shell.h"
#include "n32l40x.h"
#include "dev_uart.h"
#include "init.h"

Shell shell;
char shellBuffer[512];

/**
 * @brief 用户shell�? * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长�? */
short userShellWrite(char *data, unsigned short len)
{
    uart_write(DEV_UART1, (uint8_t *)data, len);
    return len;
}


/**
 * @brief 用户shell�? * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取�? */
short userShellRead(char *data, unsigned short len)
{
	return uart_read(DEV_UART1, (uint8_t *)data, len);
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
    return 0;
}

/**
 * @brief 用户shell解锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{
    return 0;
}

/**
 * @brief 用户shell初始�? * 
 */
static int userShellInit(void)
{
    shell.write = userShellWrite;
    shell.read = userShellRead;
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);

    return 0;
}

static void shell_register(void)
{
	userShellInit();
}
app_initcall(shell_register);

void user_shell_task(void)
{
	shellTask(&shell);
}

