/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-19     MurphyZhao   the first version
 */

#ifndef __UTEST_LOG_H__
#define __UTEST_LOG_H__

#include <rtthread.h>

#define UTEST_DEBUG

#ifdef UTEST_DEBUG
#ifndef LOG_TAG
#define LOG_TAG				"utest"
#define LOG_LVL				LOG_LVL_DBG
#define ULOG_OUTPUT_LVL		LOG_LVL_DBG
#include <rtdbg.h>
#endif
#endif

#define UTEST_LOG_ALL    (1u)
#define UTEST_LOG_ASSERT (2u)

void utest_log_lv_set(rt_uint8_t lv);

#endif /* __UTEST_LOG_H__ */
