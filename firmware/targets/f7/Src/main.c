#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>

#define TAG "Main"

int main(void) {
    // Flipper critical FURI HAL
    furi_hal_init_critical();

    // Initialize FURI layer
    furi_init();

    // Flipper FURI HAL
    furi_hal_init();

    // CMSIS initialization
    osKernelInitialize();
    FURI_LOG_I(TAG, "KERNEL OK");

    // Init flipper
    flipper_init();

    // Start kernel
    osKernelStart();

    while(1) {
    }
}
