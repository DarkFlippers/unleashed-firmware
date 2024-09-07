#include <furi_hal.h>
#include <furi_hal_mpu.h>
#include <furi_hal_memory.h>

#include <stm32wbxx_ll_cortex.h>

#define TAG "FuriHal"

void furi_hal_init_early(void) {
    furi_hal_cortex_init_early();
    furi_hal_clock_init_early();
    furi_hal_bus_init_early();
    furi_hal_dma_init_early();
    furi_hal_resources_init_early();
    furi_hal_os_init();
    furi_hal_spi_config_init_early();
    furi_hal_i2c_init_early();
    furi_hal_light_init();
    furi_hal_rtc_init_early();
    furi_hal_version_init();
}

void furi_hal_deinit_early(void) {
    furi_hal_rtc_deinit_early();
    furi_hal_i2c_deinit_early();
    furi_hal_spi_config_deinit_early();
    furi_hal_resources_deinit_early();
    furi_hal_dma_deinit_early();
    furi_hal_bus_deinit_early();
    furi_hal_clock_deinit_early();
}

void furi_hal_init(void) {
    furi_hal_mpu_init();
    furi_hal_adc_init();
    furi_hal_clock_init();
    furi_hal_random_init();
    furi_hal_serial_control_init();
    furi_hal_rtc_init();
    furi_hal_interrupt_init();
    furi_hal_flash_init();
    furi_hal_resources_init();
    furi_hal_region_init();
    furi_hal_spi_config_init();
    furi_hal_spi_dma_init();
    furi_hal_speaker_init();
    furi_hal_crypto_init();
    furi_hal_i2c_init();
    furi_hal_power_init();
    furi_hal_light_init();
    furi_hal_bt_init();
    furi_hal_memory_init();

#ifndef FURI_RAM_EXEC
    furi_hal_usb_init();
    furi_hal_vibro_init();
#endif
}

void furi_hal_switch(void* address) {
    __set_BASEPRI(0);
    asm volatile("ldr    r3, [%0]    \n"
                 "msr    msp, r3     \n"
                 "ldr    r3, [%1]    \n"
                 "mov    pc, r3      \n"
                 :
                 : "r"(address), "r"(address + 0x4)
                 : "r3");
}
