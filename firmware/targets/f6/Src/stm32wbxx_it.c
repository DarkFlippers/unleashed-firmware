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

void SysTick_Handler(void) {
    HAL_IncTick();
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

void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void TIM1_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
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
