#include "drv_systick.h"
#include "rtthread.h"

static void sys_tick_init(void)
{
    en_clk_sys_source_t     enSysClkSrc;
    stc_clk_sysclk_cfg_t    stcSysClkCfg;
    stc_clk_xtal_cfg_t      stcXtalCfg;
    stc_clk_mpll_cfg_t      stcMpllCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv = ClkSysclkDiv1;   // Max 200MHz
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;  // Max 100MHz
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;  // Max 200MHz
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;  // Max 100MHz
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;  // Max 50MHz
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;  // Max 50MHz
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;  // Max 100MHz
    CLK_SysClkConfig(&stcSysClkCfg);

    stcXtalCfg.enMode = ClkXtalModeOsc;       
    stcXtalCfg.enDrv = ClkXtalLowDrv;         
    stcXtalCfg.enFastStartup = Enable;        
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);                      

    /* MPLL config. */
    /*system clk = 200M*/
    stcMpllCfg.pllmDiv = 1u;                  
    stcMpllCfg.plln = 50u;                    
    stcMpllCfg.PllpDiv = 2u;                 
    stcMpllCfg.PllqDiv = 2u;
    stcMpllCfg.PllrDiv = 2u; 
    CLK_SetPllSource(ClkPllSrcXTAL);          
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
	EFM_InstructionCacheCmd(Enable);
    EFM_SetLatency(EFM_LATENCY_5);             
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);

    stc_sram_config_t   stcSramCfg;
    MEM_ZERO_STRUCT(stcSramCfg);
    stcSramCfg.u8SramIdx = Sram12Idx | Sram3Idx | SramRetIdx;
    stcSramCfg.enSramRC = SramCycle2;
    stcSramCfg.enSramWC = SramCycle2;
    SRAM_Init(&stcSramCfg);
}

/**
 * @brief  	  : sys tick device register func.
 * @param[1]  : none.
 * @return    : none.
 */
static int sys_tick_device_register(void)
{
	extern uint32_t SystemCoreClock;
	sys_tick_init();	//200MHz
	SysTick_Config(200000000ul / RT_TICK_PER_SECOND);//1ms
	SystemCoreClock = 200000000ul;
	
	return 0;
}
INIT_BOARD_EXPORT(sys_tick_device_register);



