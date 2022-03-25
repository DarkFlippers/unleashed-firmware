#include "main.h"
#include "stm32wbxx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usbd_core.h"

extern usbd_device udev;

extern void HW_TS_RTC_Wakeup_Handler();
extern void HW_IPCC_Tx_Handler();
extern void HW_IPCC_Rx_Handler();

void SysTick_Handler(void) {
    HAL_IncTick();
}

void USB_LP_IRQHandler(void) {
    usbd_poll(&udev);
}

void HSEM_IRQHandler(void) {
    HAL_HSEM_IRQHandler();
}

void IPCC_C1_TX_IRQHandler(void) {
    HW_IPCC_Tx_Handler();
}

void IPCC_C1_RX_IRQHandler(void) {
    HW_IPCC_Rx_Handler();
}
