/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file n32g45x_it.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include "n32g45x_it.h"
#include "n32g45x.h"


/** @addtogroup N32G45X_StdPeriph_Template
 * @{
 */

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 */
void NMI_Handler(void)
{
    /* This interrupt is generated when HSE clock fails */

    if (RCC_GetIntStatus(RCC_INT_CLKSSIF) != RESET)
    {
        /* At this stage: HSE, PLL are disabled (but no change on PLL config) and HSI
            is selected as system clock source */

        /* Enable HSE */
        RCC_ConfigHse(RCC_HSE_ENABLE);

        /* Enable HSE Ready interrupt */
        RCC_ConfigInt(RCC_INT_HSERDIF, ENABLE);

#ifndef SYSCLK_HSE
        /* Enable PLL Ready interrupt */
        RCC_ConfigInt(RCC_INT_PLLRDIF, ENABLE);
#endif /* SYSCLK_HSE */

        /* Clear Clock Security System interrupt pending bit */
        RCC_ClrIntPendingBit(RCC_INT_CLKSSIF);

        /* Once HSE clock recover, the HSERDY interrupt is generated and in the RCC INTSTS
           routine the system clock will be reconfigured to its previous state (before
           HSE clock failure) */
    }
}

/**
 * @brief  This function handles Hard Fault exception.
 */
//__attribute__((__noreturn__)) void HardFault_Handler(void)
//{
//    /* Go to infinite loop when Hard Fault exception occurs */
//    while (1)
//    {
//    }
//}

/**
 * @brief  This function handles Memory Manage exception.
 */
__attribute__((__noreturn__)) void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 */
__attribute__((__noreturn__)) void BusFault_Handler(void) 
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 */
__attribute__((__noreturn__)) void UsageFault_Handler(void) 
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 */
void SVC_Handler(void) 
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 */
void DebugMon_Handler(void) 
{
}

/**
 * @brief  This function handles SysTick Handler.
 */
//extern __IO uint32_t g_ulDelayCnt;
//void SysTick_Handler(void)
//{
//	if(g_ulDelayCnt > 0)
//		g_ulDelayCnt--;
//}

/**
 * @brief  This function handles RCC interrupt request.
 */
void RCC_IRQHandler(void)
{
    if (RCC_GetIntStatus(RCC_INT_HSERDIF) != RESET)
    {
        /* Clear HSERDY interrupt pending bit */
        RCC_ClrIntPendingBit(RCC_INT_HSERDIF);

        /* Check if the HSE clock is still available */
        if (RCC_GetFlagStatus(RCC_FLAG_HSERD) != RESET)
        {
#ifdef SYSCLK_HSE
            /* Select HSE as system clock source */
            RCC_ConfigSysclk(RCC_SYSCLK_SRC_HSE);
#else
            /* Enable PLL: once the PLL is ready the PLLRDY interrupt is generated */
            RCC_EnablePll(ENABLE);
#endif /* SYSCLK_HSE */
        }
    }

    if (RCC_GetIntStatus(RCC_INT_PLLRDIF) != RESET)
    {
        /* Clear PLLRDY interrupt pending bit */
        RCC_ClrIntPendingBit(RCC_INT_PLLRDIF);

        /* Check if the PLL is still locked */
        if (RCC_GetFlagStatus(RCC_FLAG_PLLRD) != RESET)
        {
            /* Select PLL as system clock source */
            RCC_ConfigSysclk(RCC_SYSCLK_SRC_PLLCLK);
        }
    }
}

/******************************************************************************/
/*                 N32G45X Peripherals Interrupt Handlers                     */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_n32g45x.s).                                                 */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 */
/*void PPP_IRQHandler(void)
{
}*/

/**
 * @}
 */
/**
* @brief  FG1处理
*/
__IO static FlagStatus g_flagFG110usCal = RESET;
__IO static uint16_t	g_usFG110usCnt = 0;
__IO uint16_t	g_usFG1Freq = 0;

void EXTI9_5_IRQHandler(void)
{
	float _ftemp = 0.0f;

	if(EXTI_GetITStatus(EXTI_LINE6) != RESET)
	{
		EXTI_ClrITPendBit(EXTI_LINE6);
		if(g_flagFG110usCal == RESET)	//捕捉到第一个上升沿
		{
			g_flagFG110usCal = SET;
			TIM_Enable(TIM5, ENABLE);//启动计数
		}
		else						//捕捉到第二个上升沿
		{
			g_flagFG110usCal = RESET;
//			TIM_Enable(TIM5, DISABLE);//关闭计数
			_ftemp = (float)g_usFG110usCnt;
//			g_usFG1Freq = (uint16_t)(1.0f/(_ftemp/100000.0f));
			g_usFG1Freq = (uint16_t)(1.0f/(_ftemp*0.00001f));
			if(g_usFG1Freq > 1000)
				g_usFG1Freq = 0;
		}
		g_usFG110usCnt = 0;
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetIntStatus(TIM5, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(TIM5, TIM_INT_UPDATE);
		g_usFG110usCnt++;
		if(g_usFG110usCnt >= 5000)
		{
			g_usFG110usCnt = 0;
			g_usFG1Freq = 0;
			TIM_Enable(TIM5, DISABLE);
		}
	}
}

/**
* @brief  FG2处理
*/
__IO static FlagStatus g_flagFG210usCal = RESET;
__IO static uint16_t	g_usFG210usCnt = 0;
__IO uint16_t	g_usFG2Freq = 0;

void EXTI1_IRQHandler(void)
{
	float _ftemp = 0.0f;

	if(EXTI_GetITStatus(EXTI_LINE1) != RESET)
	{
		EXTI_ClrITPendBit(EXTI_LINE1);
		if(g_flagFG210usCal == RESET)	//捕捉到第一个上升沿
		{
			g_flagFG210usCal = SET;
			TIM_Enable(TIM6, ENABLE);//启动计数
		}
		else						//捕捉到第二个上升沿
		{
			g_flagFG210usCal = RESET;
//			TIM_Enable(TIM6, DISABLE);//关闭计数
			_ftemp = (float)g_usFG210usCnt;
//			g_usFG2Freq = (uint16_t)(1.0f/(_ftemp/100000.0f));
			g_usFG2Freq = (uint16_t)(1.0f/(_ftemp*0.00001f));
			if(g_usFG2Freq > 1000)
				g_usFG2Freq = 0;
		}
		g_usFG210usCnt = 0;
	}
}

void TIM6_IRQHandler(void)
{
	if(TIM_GetIntStatus(TIM6, TIM_INT_UPDATE) != RESET)
	{
		TIM_ClrIntPendingBit(TIM6, TIM_INT_UPDATE);
		g_usFG210usCnt++;
		if(g_usFG210usCnt >= 5000)
		{
			g_usFG210usCnt = 0;
			g_usFG2Freq = 0;
			TIM_Enable(TIM6, DISABLE);
		}
	}
}


 

