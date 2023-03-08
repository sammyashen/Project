#include "rtthread.h"
#include "utest.h"

static void vb_api_printf(void)
{
	rt_kprintf("i am vb_api_printf!\r\n");
	uassert_true(1);
}

static void vb_api_printf2(void)
{
	rt_kprintf("i am vb_api_printf2!\r\n");
	uassert_true(0);
}

static rt_err_t utest_tc_init(void)
{
	return RT_EOK;
}

static rt_err_t utest_tc_clean(void)
{
	return RT_EOK;
}

static void testcase(void)
{
	UTEST_UNIT_RUN(vb_api_printf);
	UTEST_UNIT_RUN(vb_api_printf2);
}
UTEST_TC_EXPORT(testcase, "vb_api_tc", utest_tc_init, utest_tc_clean, 10);

