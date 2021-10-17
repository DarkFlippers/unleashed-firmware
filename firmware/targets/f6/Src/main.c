#include "main.h"

#include "fatfs/fatfs.h"

#include <furi.h>
#include <furi-hal.h>
#include <flipper.h>

int main(void) {
    // Initialize FURI layer
    furi_init();

    // Initialize ST HAL
    HAL_Init();

    // Flipper FURI HAL
    furi_hal_init();

    // 3rd party
    MX_FATFS_Init();
    FURI_LOG_I("HAL", "FATFS OK");

    // CMSIS initialization
    osKernelInitialize();
    FURI_LOG_I("HAL", "KERNEL OK");

    // Init flipper
    flipper_init();

    // Start kernel
    osKernelStart();

    while (1) {}
}

void Error_Handler(void) {
    asm("bkpt 1");
    while(1) {}
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
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
         tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
