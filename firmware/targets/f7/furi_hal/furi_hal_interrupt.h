#pragma once

#include <stm32wbxx_ll_tim.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Timer ISR */
typedef void (*FuriHalInterruptISR)(void* context);

typedef enum {
    // TIM1, TIM16, TIM17
    FuriHalInterruptIdTim1TrgComTim17,
    FuriHalInterruptIdTim1Cc,
    FuriHalInterruptIdTim1UpTim16,

    // TIM2
    FuriHalInterruptIdTIM2,

    // DMA1
    FuriHalInterruptIdDma1Ch1,
    FuriHalInterruptIdDma1Ch2,
    FuriHalInterruptIdDma1Ch3,
    FuriHalInterruptIdDma1Ch4,
    FuriHalInterruptIdDma1Ch5,
    FuriHalInterruptIdDma1Ch6,
    FuriHalInterruptIdDma1Ch7,

    // DMA2
    FuriHalInterruptIdDma2Ch1,
    FuriHalInterruptIdDma2Ch2,
    FuriHalInterruptIdDma2Ch3,
    FuriHalInterruptIdDma2Ch4,
    FuriHalInterruptIdDma2Ch5,
    FuriHalInterruptIdDma2Ch6,
    FuriHalInterruptIdDma2Ch7,

    // RCC
    FuriHalInterruptIdRcc,

    // Comp
    FuriHalInterruptIdCOMP,

    // HSEM
    FuriHalInterruptIdHsem,

    // Service value
    FuriHalInterruptIdMax,
} FuriHalInterruptId;

/** Initialize interrupt subsystem */
void furi_hal_interrupt_init();

/** Set ISR and enable interrupt with default priority
 * We don't clear interrupt flags for you, do it by your self.
 * @param index - interrupt ID
 * @param isr - your interrupt service routine or use NULL to clear
 * @param context - isr context
 */
void furi_hal_interrupt_set_isr(FuriHalInterruptId index, FuriHalInterruptISR isr, void* context);

/** Set ISR and enable interrupt with custom priority
 * We don't clear interrupt flags for you, do it by your self.
 * @param index - interrupt ID
 * @param priority - 0 to 15, 0 highest
 * @param isr - your interrupt service routine or use NULL to clear
 * @param context - isr context
 */
void furi_hal_interrupt_set_isr_ex(
    FuriHalInterruptId index,
    uint16_t priority,
    FuriHalInterruptISR isr,
    void* context);

#ifdef __cplusplus
}
#endif
