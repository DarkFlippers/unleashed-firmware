#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <semphr.h>

#define TAG "Main"

#ifdef FURI_RAM_EXEC
int main() {
    // Initialize FURI layer
    furi_init();

    // Flipper critical FURI HAL
    furi_hal_init_early();

    // Flipper FURI HAL
    furi_hal_init();

    // Init flipper
    flipper_init();

    furi_run();

    while(1) {
    }
}
#else
int main() {
    // Initialize FURI layer
    furi_init();

    // Flipper critical FURI HAL
    furi_hal_init_early();
    furi_hal_light_sequence("RGB");

    // Delay is for button sampling
    furi_hal_delay_ms(100);

    FuriHalRtcBootMode boot_mode = furi_hal_rtc_get_boot_mode();
    if(boot_mode == FuriHalRtcBootModeDfu || !furi_hal_gpio_read(&gpio_button_left)) {
        furi_hal_light_sequence("rgb WB");
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
        flipper_boot_dfu_exec();
        furi_hal_power_reset();
    } else if(boot_mode == FuriHalRtcBootModeUpdate) {
        furi_hal_light_sequence("rgb BR");
        flipper_boot_update_exec();
        // if things go nice, we shouldn't reach this point.
        // But if we do, abandon to avoid bootloops
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
        furi_hal_power_reset();
    } else {
        furi_hal_light_sequence("rgb G");

        // Flipper FURI HAL
        furi_hal_init();

        // Init flipper
        flipper_init();

        furi_run();
    }

    while(1) {
    }
}
#endif

void Error_Handler(void) {
    furi_crash("ErrorHandler");
}

#ifdef USE_FULL_ASSERT
/**
    * @brief  Reports the name of the source file and the source line number
    *         where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
void assert_failed(uint8_t* file, uint32_t line) {
    furi_crash("HAL assert failed");
}
#endif /* USE_FULL_ASSERT */
