#include "main.h"

#include <furi.h>
#include <furi-hal.h>
#include <flipper.h>

#define TAG "Main"

int main(void) {
    // Initialize FURI layer
    furi_init();

    // Initialize ST HAL
    HAL_Init();

    // Flipper FURI HAL
    furi_hal_init();

    // CMSIS initialization
    osKernelInitialize();
    FURI_LOG_I(TAG, "KERNEL OK");

    // Init flipper
    flipper_init();

    // Start kernel
    osKernelStart();

    while (1) {}
}

void Error_Handler(void) {
    furi_crash("ErrorHandler");
}

#ifdef  USE_FULL_ASSERT
/**
    * @brief  Reports the name of the source file and the source line number
    *         where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
void assert_failed(uint8_t *file, uint32_t line) {
    furi_crash("HAL assert failed");
}
#endif /* USE_FULL_ASSERT */
