#include <furi_hal_interrupt.h>
#include <furi_hal_os.h>

#include <furi.h>

#include <stm32wbxx.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_cortex.h>

#define TAG "FuriHalInterrupt"

#define FURI_HAL_INTERRUPT_DEFAULT_PRIORITY 5

typedef struct {
    FuriHalInterruptISR isr;
    void* context;
} FuriHalInterruptISRPair;

FuriHalInterruptISRPair furi_hal_interrupt_isr[FuriHalInterruptIdMax] = {0};

const IRQn_Type furi_hal_interrupt_irqn[FuriHalInterruptIdMax] = {
    // TIM1, TIM16, TIM17
    [FuriHalInterruptIdTim1TrgComTim17] = TIM1_TRG_COM_TIM17_IRQn,
    [FuriHalInterruptIdTim1Cc] = TIM1_CC_IRQn,
    [FuriHalInterruptIdTim1UpTim16] = TIM1_UP_TIM16_IRQn,

    // TIM2
    [FuriHalInterruptIdTIM2] = TIM2_IRQn,

    // DMA1
    [FuriHalInterruptIdDma1Ch1] = DMA1_Channel1_IRQn,
    [FuriHalInterruptIdDma1Ch2] = DMA1_Channel2_IRQn,
    [FuriHalInterruptIdDma1Ch3] = DMA1_Channel3_IRQn,
    [FuriHalInterruptIdDma1Ch4] = DMA1_Channel4_IRQn,
    [FuriHalInterruptIdDma1Ch5] = DMA1_Channel5_IRQn,
    [FuriHalInterruptIdDma1Ch6] = DMA1_Channel6_IRQn,
    [FuriHalInterruptIdDma1Ch7] = DMA1_Channel7_IRQn,

    // DMA2
    [FuriHalInterruptIdDma2Ch1] = DMA2_Channel1_IRQn,
    [FuriHalInterruptIdDma2Ch2] = DMA2_Channel2_IRQn,
    [FuriHalInterruptIdDma2Ch3] = DMA2_Channel3_IRQn,
    [FuriHalInterruptIdDma2Ch4] = DMA2_Channel4_IRQn,
    [FuriHalInterruptIdDma2Ch5] = DMA2_Channel5_IRQn,
    [FuriHalInterruptIdDma2Ch6] = DMA2_Channel6_IRQn,
    [FuriHalInterruptIdDma2Ch7] = DMA2_Channel7_IRQn,

    // RCC
    [FuriHalInterruptIdRcc] = RCC_IRQn,

    // COMP
    [FuriHalInterruptIdCOMP] = COMP_IRQn,

    // HSEM
    [FuriHalInterruptIdHsem] = HSEM_IRQn,

    // LPTIMx
    [FuriHalInterruptIdLpTim1] = LPTIM1_IRQn,
    [FuriHalInterruptIdLpTim2] = LPTIM2_IRQn,
};

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_call(FuriHalInterruptId index) {
    furi_check(furi_hal_interrupt_isr[index].isr);
    furi_hal_interrupt_isr[index].isr(furi_hal_interrupt_isr[index].context);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_enable(FuriHalInterruptId index, uint16_t priority) {
    NVIC_SetPriority(
        furi_hal_interrupt_irqn[index],
        NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, 0));
    NVIC_EnableIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_clear_pending(FuriHalInterruptId index) {
    NVIC_ClearPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_get_pending(FuriHalInterruptId index) {
    NVIC_GetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_set_pending(FuriHalInterruptId index) {
    NVIC_SetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

__attribute__((always_inline)) static inline void
    furi_hal_interrupt_disable(FuriHalInterruptId index) {
    NVIC_DisableIRQ(furi_hal_interrupt_irqn[index]);
}

void furi_hal_interrupt_init() {
    NVIC_SetPriority(
        TAMP_STAMP_LSECSS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TAMP_STAMP_LSECSS_IRQn);

    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    NVIC_SetPriority(FPU_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(FPU_IRQn);

    LL_SYSCFG_DisableIT_FPU_IOC();
    LL_SYSCFG_DisableIT_FPU_DZC();
    LL_SYSCFG_DisableIT_FPU_UFC();
    LL_SYSCFG_DisableIT_FPU_OFC();
    LL_SYSCFG_DisableIT_FPU_IDC();
    LL_SYSCFG_DisableIT_FPU_IXC();

    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_USG);
    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_BUS);
    LL_HANDLER_EnableFault(LL_HANDLER_FAULT_MEM);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_interrupt_set_isr(FuriHalInterruptId index, FuriHalInterruptISR isr, void* context) {
    furi_hal_interrupt_set_isr_ex(index, FURI_HAL_INTERRUPT_DEFAULT_PRIORITY, isr, context);
}

void furi_hal_interrupt_set_isr_ex(
    FuriHalInterruptId index,
    uint16_t priority,
    FuriHalInterruptISR isr,
    void* context) {
    furi_check(index < FuriHalInterruptIdMax);
    furi_check(priority <= 15);

    if(isr) {
        // Pre ISR set
        furi_check(furi_hal_interrupt_isr[index].isr == NULL);
    } else {
        // Pre ISR clear
        furi_hal_interrupt_disable(index);
        furi_hal_interrupt_clear_pending(index);
    }

    furi_hal_interrupt_isr[index].isr = isr;
    furi_hal_interrupt_isr[index].context = context;
    __DMB();

    if(isr) {
        // Post ISR set
        furi_hal_interrupt_clear_pending(index);
        furi_hal_interrupt_enable(index, priority);
    } else {
        // Post ISR clear
    }
}

/* Timer 2 */
void TIM2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdTIM2);
}

/* Timer 1 Update */
void TIM1_UP_TIM16_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1UpTim16);
}

void TIM1_TRG_COM_TIM17_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1TrgComTim17);
}

void TIM1_CC_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1Cc);
}

/* DMA 1 */
void DMA1_Channel1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch1);
}

void DMA1_Channel2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch2);
}

void DMA1_Channel3_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch3);
}

void DMA1_Channel4_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch4);
}

void DMA1_Channel5_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch5);
}

void DMA1_Channel6_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch6);
}

void DMA1_Channel7_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch7);
}

/* DMA 2 */
void DMA2_Channel1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch1);
}

void DMA2_Channel2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch2);
}

void DMA2_Channel3_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch3);
}

void DMA2_Channel4_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch4);
}

void DMA2_Channel5_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch5);
}

void DMA2_Channel6_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch6);
}

void DMA2_Channel7_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch7);
}

void HSEM_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdHsem);
}

void TAMP_STAMP_LSECSS_IRQHandler(void) {
    if(LL_RCC_IsActiveFlag_LSECSS()) {
        LL_RCC_ClearFlag_LSECSS();
        if(!LL_RCC_LSE_IsReady()) {
            FURI_LOG_E(TAG, "LSE CSS fired: resetting system");
            NVIC_SystemReset();
        } else {
            FURI_LOG_E(TAG, "LSE CSS fired: but LSE is alive");
        }
    }
}

void RCC_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdRcc);
}

void NMI_Handler() {
    if(LL_RCC_IsActiveFlag_HSECSS()) {
        LL_RCC_ClearFlag_HSECSS();
        FURI_LOG_E(TAG, "HSE CSS fired: resetting system");
        NVIC_SystemReset();
    }
}

void HardFault_Handler() {
    furi_crash("HardFault");
}

void MemManage_Handler() {
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MMARVALID_Pos)) {
        uint32_t memfault_address = SCB->MMFAR;
        if(memfault_address < (1024 * 1024)) {
            // from 0x00 to 1MB, see FuriHalMpuRegionNULL
            furi_crash("NULL pointer dereference");
        } else {
            // write or read of MPU region 1 (FuriHalMpuRegionStack)
            furi_crash("MPU fault, possibly stack overflow");
        }
    } else if(FURI_BIT(SCB->CFSR, SCB_CFSR_MSTKERR_Pos)) {
        // push to stack on MPU region 1 (FuriHalMpuRegionStack)
        furi_crash("MemManage fault, possibly stack overflow");
    }

    furi_crash("MemManage");
}

void BusFault_Handler() {
    furi_crash("BusFault");
}

void UsageFault_Handler() {
    furi_crash("UsageFault");
}

void DebugMon_Handler() {
}

#include "usbd_core.h"

extern usbd_device udev;

extern void HW_IPCC_Tx_Handler();
extern void HW_IPCC_Rx_Handler();

void SysTick_Handler() {
    furi_hal_os_tick();
}

void USB_LP_IRQHandler() {
#ifndef FURI_RAM_EXEC
    usbd_poll(&udev);
#endif
}

void USB_HP_IRQHandler() {
}

void IPCC_C1_TX_IRQHandler() {
    HW_IPCC_Tx_Handler();
}

void IPCC_C1_RX_IRQHandler() {
    HW_IPCC_Rx_Handler();
}

void FPU_IRQHandler() {
    furi_crash("FpuFault");
}

void LPTIM1_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLpTim1);
}

void LPTIM2_IRQHandler() {
    furi_hal_interrupt_call(FuriHalInterruptIdLpTim2);
}
