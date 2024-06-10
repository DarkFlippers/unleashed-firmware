#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int main(void);
extern void __libc_init_array(void);

void Default_Handler(void);

#define DEFAULT FURI_DEFAULT("Default_Handler")

/* 15 Unmask-able ISR */
DEFAULT void NMI_Handler(void);
DEFAULT void HardFault_Handler(void);
DEFAULT void MemManage_Handler(void);
DEFAULT void BusFault_Handler(void);
DEFAULT void UsageFault_Handler(void);
DEFAULT void SVC_Handler(void);
DEFAULT void DebugMon_Handler(void);
DEFAULT void PendSV_Handler(void);
DEFAULT void SysTick_Handler(void);

/* 63 Mask-able ISR */
DEFAULT void WWDG_IRQHandler(void);
DEFAULT void PVD_PVM_IRQHandler(void);
DEFAULT void TAMP_STAMP_LSECSS_IRQHandler(void);
DEFAULT void RTC_WKUP_IRQHandler(void);
DEFAULT void FLASH_IRQHandler(void);
DEFAULT void RCC_IRQHandler(void);
DEFAULT void EXTI0_IRQHandler(void);
DEFAULT void EXTI1_IRQHandler(void);
DEFAULT void EXTI2_IRQHandler(void);
DEFAULT void EXTI3_IRQHandler(void);
DEFAULT void EXTI4_IRQHandler(void);
DEFAULT void DMA1_Channel1_IRQHandler(void);
DEFAULT void DMA1_Channel2_IRQHandler(void);
DEFAULT void DMA1_Channel3_IRQHandler(void);
DEFAULT void DMA1_Channel4_IRQHandler(void);
DEFAULT void DMA1_Channel5_IRQHandler(void);
DEFAULT void DMA1_Channel6_IRQHandler(void);
DEFAULT void DMA1_Channel7_IRQHandler(void);
DEFAULT void ADC1_IRQHandler(void);
DEFAULT void USB_HP_IRQHandler(void);
DEFAULT void USB_LP_IRQHandler(void);
DEFAULT void C2SEV_PWR_C2H_IRQHandler(void);
DEFAULT void COMP_IRQHandler(void);
DEFAULT void EXTI9_5_IRQHandler(void);
DEFAULT void TIM1_BRK_IRQHandler(void);
DEFAULT void TIM1_UP_TIM16_IRQHandler(void);
DEFAULT void TIM1_TRG_COM_TIM17_IRQHandler(void);
DEFAULT void TIM1_CC_IRQHandler(void);
DEFAULT void TIM2_IRQHandler(void);
DEFAULT void PKA_IRQHandler(void);
DEFAULT void I2C1_EV_IRQHandler(void);
DEFAULT void I2C1_ER_IRQHandler(void);
DEFAULT void I2C3_EV_IRQHandler(void);
DEFAULT void I2C3_ER_IRQHandler(void);
DEFAULT void SPI1_IRQHandler(void);
DEFAULT void SPI2_IRQHandler(void);
DEFAULT void USART1_IRQHandler(void);
DEFAULT void LPUART1_IRQHandler(void);
DEFAULT void SAI1_IRQHandler(void);
DEFAULT void TSC_IRQHandler(void);
DEFAULT void EXTI15_10_IRQHandler(void);
DEFAULT void RTC_Alarm_IRQHandler(void);
DEFAULT void CRS_IRQHandler(void);
DEFAULT void PWR_SOTF_BLEACT_802ACT_RFPHASE_IRQHandler(void);
DEFAULT void IPCC_C1_RX_IRQHandler(void);
DEFAULT void IPCC_C1_TX_IRQHandler(void);
DEFAULT void HSEM_IRQHandler(void);
DEFAULT void LPTIM1_IRQHandler(void);
DEFAULT void LPTIM2_IRQHandler(void);
DEFAULT void LCD_IRQHandler(void);
DEFAULT void QUADSPI_IRQHandler(void);
DEFAULT void AES1_IRQHandler(void);
DEFAULT void AES2_IRQHandler(void);
DEFAULT void RNG_IRQHandler(void);
DEFAULT void FPU_IRQHandler(void);
DEFAULT void DMA2_Channel1_IRQHandler(void);
DEFAULT void DMA2_Channel2_IRQHandler(void);
DEFAULT void DMA2_Channel3_IRQHandler(void);
DEFAULT void DMA2_Channel4_IRQHandler(void);
DEFAULT void DMA2_Channel5_IRQHandler(void);
DEFAULT void DMA2_Channel6_IRQHandler(void);
DEFAULT void DMA2_Channel7_IRQHandler(void);
DEFAULT void DMAMUX1_OVR_IRQHandler(void);

#ifdef __cplusplus
}
#endif
