#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "n32l40x.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define PIN_FAULT_ENABLE		GPIO_SetBits(GPIOA, GPIO_PIN_5)
#define PIN_FAULT_DISABLE		GPIO_ResetBits(GPIOA, GPIO_PIN_5)
#define PIN_DRV_ENABLE			GPIO_SetBits(GPIOA, GPIO_PIN_6)
#define PIN_DRV_DISABLE			GPIO_ResetBits(GPIOA, GPIO_PIN_6)
#define PIN_SLEEP_ENABLE		GPIO_SetBits(GPIOA, GPIO_PIN_7)
#define PIN_SLEEP_DISABLE		GPIO_ResetBits(GPIOA, GPIO_PIN_7)
#define PIN_DRV_PWR_ENABLE		GPIO_SetBits(GPIOB, GPIO_PIN_0)
#define PIN_DRV_PWR_DISABLE		GPIO_ResetBits(GPIOB, GPIO_PIN_0)

void pin_drv_motor_init(void);

#ifdef __cplusplus
}
#endif

#endif 



