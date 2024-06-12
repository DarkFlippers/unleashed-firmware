#include <furi_hal_interrupt.h>
#include <furi_hal_os.h>

#include <furi.h>
#include <FreeRTOS.h>

#include <stm32wbxx.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_cortex.h>

#define TAG "FuriHalInterrupt"

#define FURI_HAL_INTERRUPT_DEFAULT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 5)

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

    // UARTx
    [FuriHalInterruptIdUart1] = USART1_IRQn,

    // LPUARTx
    [FuriHalInterruptIdLpUart1] = LPUART1_IRQn,
};

FURI_ALWAYS_STATIC_INLINE void furi_hal_interrupt_call(FuriHalInterruptId index) {
    furi_check(furi_hal_interrupt_isr[index].isr);
    furi_hal_interrupt_isr[index].isr(furi_hal_interrupt_isr[index].context);
}

FURI_ALWAYS_STATIC_INLINE void
    furi_hal_interrupt_enable(FuriHalInterruptId index, uint16_t priority) {
    NVIC_SetPriority(
        furi_hal_interrupt_irqn[index],
        NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, 0));
    NVIC_EnableIRQ(furi_hal_interrupt_irqn[index]);
}

FURI_ALWAYS_STATIC_INLINE void furi_hal_interrupt_clear_pending(FuriHalInterruptId index) {
    NVIC_ClearPendingIRQ(furi_hal_interrupt_irqn[index]);
}

FURI_ALWAYS_STATIC_INLINE void furi_hal_interrupt_get_pending(FuriHalInterruptId index) {
    NVIC_GetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

FURI_ALWAYS_STATIC_INLINE void furi_hal_interrupt_set_pending(FuriHalInterruptId index) {
    NVIC_SetPendingIRQ(furi_hal_interrupt_irqn[index]);
}

FURI_ALWAYS_STATIC_INLINE void furi_hal_interrupt_disable(FuriHalInterruptId index) {
    NVIC_DisableIRQ(furi_hal_interrupt_irqn[index]);
}

void furi_hal_interrupt_init(void) {
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
    furi_hal_interrupt_set_isr_ex(index, FuriHalInterruptPriorityNormal, isr, context);
}

void furi_hal_interrupt_set_isr_ex(
    FuriHalInterruptId index,
    FuriHalInterruptPriority priority,
    FuriHalInterruptISR isr,
    void* context) {
    furi_check(index < FuriHalInterruptIdMax);
    furi_check(
        (priority >= FuriHalInterruptPriorityLowest &&
         priority <= FuriHalInterruptPriorityHighest) ||
        priority == FuriHalInterruptPriorityKamiSama);

    uint16_t real_priority = FURI_HAL_INTERRUPT_DEFAULT_PRIORITY - priority;

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
        furi_hal_interrupt_enable(index, real_priority);
    } else {
        // Post ISR clear
    }
}

/* Timer 2 */
void TIM2_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTIM2);
}

/* Timer 1 Update */
void TIM1_UP_TIM16_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1UpTim16);
}

void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1TrgComTim17);
}

void TIM1_CC_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdTim1Cc);
}

/* DMA 1 */
void DMA1_Channel1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch1);
}

void DMA1_Channel2_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch2);
}

void DMA1_Channel3_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch3);
}

void DMA1_Channel4_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch4);
}

void DMA1_Channel5_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch5);
}

void DMA1_Channel6_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch6);
}

void DMA1_Channel7_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma1Ch7);
}

/* DMA 2 */
void DMA2_Channel1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch1);
}

void DMA2_Channel2_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch2);
}

void DMA2_Channel3_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch3);
}

void DMA2_Channel4_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch4);
}

void DMA2_Channel5_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch5);
}

void DMA2_Channel6_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch6);
}

void DMA2_Channel7_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdDma2Ch7);
}

void HSEM_IRQHandler(void) {
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

void RCC_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdRcc);
}

void NMI_Handler(void) {
    if(LL_RCC_IsActiveFlag_HSECSS()) {
        LL_RCC_ClearFlag_HSECSS();
        FURI_LOG_E(TAG, "HSE CSS fired: resetting system");
        NVIC_SystemReset();
    }
}

void HardFault_Handler(void) {
    furi_crash("HardFault");
}

void MemManage_Handler(void) {
    if(FURI_BIT(SCB->CFSR, SCB_CFSR_MMARVALID_Pos)) {
        uint32_t memfault_address = SCB->MMFAR;
        if(memfault_address < (1024 * 1024)) {
            // from 0x00 to 1MB, see FuriHalMpuRegionNULL
            furi_crash("NULL pointer dereference");
        } else {
            // write or read of MPU region 1 (FuriHalMpuRegionThreadStack)
            furi_crash("MPU fault, possibly stack overflow");
        }
    } else if(FURI_BIT(SCB->CFSR, SCB_CFSR_MSTKERR_Pos)) {
        // push to stack on MPU region 1 (FuriHalMpuRegionThreadStack)
        furi_crash("MemManage fault, possibly stack overflow");
    }

    furi_crash("MemManage");
}

void BusFault_Handler(void) {
    furi_crash("BusFault");
}

void UsageFault_Handler(void) {
    furi_crash("UsageFault");
}

void DebugMon_Handler(void) {
}

#include "usbd_core.h"

extern usbd_device udev;

extern void HW_IPCC_Tx_Handler(void);
extern void HW_IPCC_Rx_Handler(void);

void SysTick_Handler(void) {
    furi_hal_os_tick();
}

void USB_LP_IRQHandler(void) {
#ifndef FURI_RAM_EXEC
    usbd_poll(&udev);
#endif
}

void USB_HP_IRQHandler(void) { //-V524
#ifndef FURI_RAM_EXEC
    usbd_poll(&udev);
#endif
}

void IPCC_C1_TX_IRQHandler(void) {
    HW_IPCC_Tx_Handler();
}

void IPCC_C1_RX_IRQHandler(void) {
    HW_IPCC_Rx_Handler();
}

void FPU_IRQHandler(void) {
    furi_crash("FpuFault");
}

void LPTIM1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLpTim1);
}

void LPTIM2_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLpTim2);
}

void USART1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdUart1);
}

void LPUART1_IRQHandler(void) {
    furi_hal_interrupt_call(FuriHalInterruptIdLpUart1);
}

const char* furi_hal_interrupt_get_name(uint8_t exception_number) {
    int32_t id = (int32_t)exception_number - 16;

    switch(id) {
    case -14:
        return "NMI";
    case -13:
        return "HardFault";
    case -12:
        return "MemMgmt";
    case -11:
        return "BusFault";
    case -10:
        return "UsageFault";
    case -5:
        return "SVC";
    case -4:
        return "DebugMon";
    case -2:
        return "PendSV";
    case -1:
        return "SysTick";
    case 0:
        return "WWDG";
    case 1:
        return "PVD_PVM";
    case 2:
        return "TAMP";
    case 3:
        return "RTC_WKUP";
    case 4:
        return "FLASH";
    case 5:
        return "RCC";
    case 6:
        return "EXTI0";
    case 7:
        return "EXTI1";
    case 8:
        return "EXTI2";
    case 9:
        return "EXTI3";
    case 10:
        return "EXTI4";
    case 11:
        return "DMA1_Ch1";
    case 12:
        return "DMA1_Ch2";
    case 13:
        return "DMA1_Ch3";
    case 14:
        return "DMA1_Ch4";
    case 15:
        return "DMA1_Ch5";
    case 16:
        return "DMA1_Ch6";
    case 17:
        return "DMA1_Ch7";
    case 18:
        return "ADC1";
    case 19:
        return "USB_HP";
    case 20:
        return "USB_LP";
    case 21:
        return "C2SEV_PWR_C2H";
    case 22:
        return "COMP";
    case 23:
        return "EXTI9_5";
    case 24:
        return "TIM1_BRK";
    case 25:
        return "TIM1_UP_TIM16";
    case 26:
        return "TIM1_TRG_COM_TIM17";
    case 27:
        return "TIM1_CC";
    case 28:
        return "TIM2";
    case 29:
        return "PKA";
    case 30:
        return "I2C1_EV";
    case 31:
        return "I2C1_ER";
    case 32:
        return "I2C3_EV";
    case 33:
        return "I2C3_ER";
    case 34:
        return "SPI1";
    case 35:
        return "SPI2";
    case 36:
        return "USART1";
    case 37:
        return "LPUART1";
    case 38:
        return "SAI1";
    case 39:
        return "TSC";
    case 40:
        return "EXTI15_10";
    case 41:
        return "RTC_Alarm";
    case 42:
        return "CRS";
    case 43:
        return "PWR_SOTF_BLE";
    case 44:
        return "IPCC_C1_RX";
    case 45:
        return "IPCC_C1_TX";
    case 46:
        return "HSEM";
    case 47:
        return "LPTIM1";
    case 48:
        return "LPTIM2";
    case 49:
        return "LCD";
    case 50:
        return "QUADSPI";
    case 51:
        return "AES1";
    case 52:
        return "AES2";
    case 53:
        return "RNG";
    case 54:
        return "FPU";
    case 55:
        return "DMA2_Ch1";
    case 56:
        return "DMA2_Ch2";
    case 57:
        return "DMA2_Ch3";
    case 58:
        return "DMA2_Ch4";
    case 59:
        return "DMA2_Ch5";
    case 60:
        return "DMA2_Ch6";
    case 61:
        return "DMA2_Ch7";
    case 62:
        return "DMAMUX1_OVR";
    default:
        return NULL;
    }
}
