#pragma once

#include <stm32wbxx_ll_tim.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Timer ISR */
typedef void (*ApiHalInterruptISR)();

/** Initialize interrupt subsystem */
void api_hal_interrupt_init();

/** Set DMA Channel ISR
 * We don't clear interrupt flags for you, do it by your self.
 * @param dma - DMA instance
 * @param channel - DMA channel
 * @param isr - your interrupt service routine or use NULL to clear
 */
void api_hal_interrupt_set_dma_channel_isr(DMA_TypeDef* dma, uint32_t channel, ApiHalInterruptISR isr);

/** Set Timer ISR
 * By default ISR is serviced by ST HAL. Use this function to override it.
 * We don't clear interrupt flags for you, do it by your self.
 * @param timer - timer instance
 * @param isr - your interrupt service routine or use NULL to clear
 */
void api_hal_interrupt_set_timer_isr(TIM_TypeDef *timer, ApiHalInterruptISR isr);


#ifdef __cplusplus
}
#endif
