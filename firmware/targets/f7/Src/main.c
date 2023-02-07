#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <semphr.h>
#include <update_util/update_operation.h>

#define TAG "Main"

int32_t init_task(void* context) {
    UNUSED(context);

    // Flipper FURI HAL
    furi_hal_init();

    // Init flipper
    flipper_init();

    return 0;
}

int main() {
    // Initialize FURI layer
    furi_init();

    // Flipper critical FURI HAL
    furi_hal_init_early();

    FuriThread* main_thread = furi_thread_alloc_ex("Init", 4096, init_task, NULL);

#ifdef FURI_RAM_EXEC
    furi_thread_start(main_thread);
#else
    furi_hal_light_sequence("RGB");

    // Delay is for button sampling
    furi_delay_ms(100);

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
    } else if(!furi_hal_gpio_read(&gpio_button_up)) {
        furi_hal_light_sequence("rgb WR");
        flipper_boot_recovery_exec();
        furi_hal_power_reset();
    } else {
        furi_hal_light_sequence("rgb G");
        furi_thread_start(main_thread);
    }
#endif

    // Run Kernel
    furi_run();

    furi_crash("Kernel is Dead");
}

void Error_Handler(void) {
    furi_crash("ErrorHandler");
}

void abort() {
    furi_crash("AbortHandler");
}
