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

    // LPTIMx
    FuriHalInterruptIdLpTim1,
    FuriHalInterruptIdLpTim2,

    //UARTx
    FuriHalInterruptIdUart1,

    //LPUARTx
    FuriHalInterruptIdLpUart1,

    // Service value
    FuriHalInterruptIdMax,
} FuriHalInterruptId;

typedef enum {
    FuriHalInterruptPriorityLowest =
        -3, /**< Lowest priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityLower =
        -2, /**< Lower priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityLow =
        -1, /**< Low priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityNormal =
        0, /**< Normal(default) priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityHigh =
        1, /**< High priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityHigher =
        2, /**< Higher priority level, you can use ISR-safe OS primitives */
    FuriHalInterruptPriorityHighest =
        3, /**< Highest priority level, you can use ISR-safe OS primitives */

    /* Special group, read docs first(ALL OF THEM: especially FreeRTOS configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) */
    FuriHalInterruptPriorityKamiSama =
        6, /**< Forget about thread safety, you are god now. No one can prevent you from messing with OS critical section. You are not allowed to use any OS primitives, but who can stop you? Use this priority only for direct hardware interaction with LL HAL. */
} FuriHalInterruptPriority;

/** Initialize interrupt subsystem */
void furi_hal_interrupt_init();

/** Set ISR and enable interrupt with default priority
 *
 * @warning    Interrupt flags are not cleared automatically. You may want to
 *             ensure that your peripheral status flags are cleared.
 *
 * @param      index    - interrupt ID
 * @param      isr      - your interrupt service routine or use NULL to clear
 * @param      context  - isr context
 */
void furi_hal_interrupt_set_isr(FuriHalInterruptId index, FuriHalInterruptISR isr, void* context);

/** Set ISR and enable interrupt with custom priority
 *
 * @warning    Interrupt flags are not cleared automatically. You may want to
 *             ensure that your peripheral status flags are cleared.
 *
 * @param      index     - interrupt ID
 * @param      priority  - One of FuriHalInterruptPriority
 * @param      isr       - your interrupt service routine or use NULL to clear
 * @param      context   - isr context
 */
void furi_hal_interrupt_set_isr_ex(
    FuriHalInterruptId index,
    FuriHalInterruptPriority priority,
    FuriHalInterruptISR isr,
    void* context);

#ifdef __cplusplus
}
#endif
