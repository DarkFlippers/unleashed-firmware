#include <furi_hal.h>

#include <stm32wbxx_ll_cortex.h>

#include <fatfs.h>

#define TAG "FuriHal"

void furi_hal_init_early() {
    furi_hal_cortex_init_early();

    furi_hal_clock_init_early();

    furi_hal_resources_init_early();

    furi_hal_os_init();

    furi_hal_spi_init_early();

    furi_hal_i2c_init_early();
    furi_hal_light_init();

    furi_hal_rtc_init_early();
}

void furi_hal_deinit_early() {
    furi_hal_rtc_deinit_early();

    furi_hal_i2c_deinit_early();
    furi_hal_spi_deinit_early();

    furi_hal_resources_deinit_early();

    furi_hal_clock_deinit_early();
}

void furi_hal_init() {
    furi_hal_clock_init();
    furi_hal_console_init();
    furi_hal_rtc_init();

    furi_hal_interrupt_init();

    furi_hal_flash_init();

    furi_hal_resources_init();
    FURI_LOG_I(TAG, "GPIO OK");

    furi_hal_version_init();

    furi_hal_spi_init();

    furi_hal_ibutton_init();
    FURI_LOG_I(TAG, "iButton OK");
    furi_hal_speaker_init();
    FURI_LOG_I(TAG, "Speaker OK");

    furi_hal_crypto_init();

    // USB
#ifndef FURI_RAM_EXEC
    furi_hal_usb_init();
    FURI_LOG_I(TAG, "USB OK");
#endif

    furi_hal_i2c_init();

    // High Level
    furi_hal_power_init();
    furi_hal_light_init();
#ifndef FURI_RAM_EXEC
    furi_hal_vibro_init();
    furi_hal_subghz_init();
    furi_hal_nfc_init();
    furi_hal_rfid_init();
#endif
    furi_hal_bt_init();
    furi_hal_compress_icon_init();

    // FatFS driver initialization
    MX_FATFS_Init();
    FURI_LOG_I(TAG, "FATFS OK");

    // Partial null pointer dereference protection
    LL_MPU_Disable();
    LL_MPU_ConfigRegion(
        LL_MPU_REGION_NUMBER0,
        0x00,
        0x0,
        LL_MPU_REGION_SIZE_1MB | LL_MPU_REGION_PRIV_RO_URO | LL_MPU_ACCESS_BUFFERABLE |
            LL_MPU_ACCESS_CACHEABLE | LL_MPU_ACCESS_SHAREABLE | LL_MPU_TEX_LEVEL1 |
            LL_MPU_INSTRUCTION_ACCESS_ENABLE);
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
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
