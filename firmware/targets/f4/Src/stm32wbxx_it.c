#include "main.h"
#include "stm32wbxx_it.h"
#include "FreeRTOS.h"
#include "task.h"

extern PCD_HandleTypeDef hpcd_USB_FS;
extern ADC_HandleTypeDef hadc1;
extern COMP_HandleTypeDef hcomp1;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

extern void HW_TS_RTC_Wakeup_Handler();
extern void HW_IPCC_Tx_Handler();
extern void HW_IPCC_Rx_Handler();

void NMI_Handler(void) {
    HAL_RCC_NMI_IRQHandler();
}

void HardFault_Handler(void) {
    if ((*(volatile uint32_t *)CoreDebug_BASE) & (1 << 0)) {
        __asm("bkpt 1");
    }
    while (1) {}
}

void MemManage_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void BusFault_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void UsageFault_Handler(void) {
    __asm("bkpt 1");
    while (1) {}
}

void DebugMon_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

void TAMP_STAMP_LSECSS_IRQHandler(void) {
    if (!LL_RCC_LSE_IsReady()) {
        // TODO: notify user about issue with LSE
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        NVIC_SystemReset();
    }
}

void RCC_IRQHandler(void) {
}

void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
}

void ADC1_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc1);
}

void USB_LP_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void COMP_IRQHandler(void) {
    HAL_COMP_IRQHandler(&hcomp1);
}

void TIM1_UP_TIM16_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
    HAL_TIM_IRQHandler(&htim16);
}

void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void TIM1_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void HSEM_IRQHandler(void) {
    HAL_HSEM_IRQHandler();
}

void RTC_WKUP_IRQHandler(void){
    HW_TS_RTC_Wakeup_Handler();
}

void IPCC_C1_TX_IRQHandler(void){
    HW_IPCC_Tx_Handler();
}

void IPCC_C1_RX_IRQHandler(void){
    HW_IPCC_Rx_Handler();
}
