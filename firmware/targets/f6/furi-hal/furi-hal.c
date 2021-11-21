#include <furi-hal.h>

#include <comp.h>
#include <rtc.h>
#include <tim.h>
#include <gpio.h>

#include <stm32wbxx_ll_cortex.h>

#include <fatfs.h>

#define TAG "FuriHal"

void furi_hal_init() {
    furi_hal_clock_init();
    furi_hal_console_init();
    furi_hal_interrupt_init();
    furi_hal_delay_init();

    MX_GPIO_Init();
    FURI_LOG_I(TAG, "GPIO OK");

    MX_RTC_Init();
    FURI_LOG_I(TAG, "RTC OK");
    furi_hal_bootloader_init();
    furi_hal_version_init();

    furi_hal_spi_init();

    MX_TIM1_Init();
    FURI_LOG_I(TAG, "TIM1 OK");
    MX_TIM2_Init();
    FURI_LOG_I(TAG, "TIM2 OK");
    MX_TIM16_Init();
    FURI_LOG_I(TAG, "TIM16 OK");
    MX_COMP1_Init();
    FURI_LOG_I(TAG, "COMP1 OK");

    furi_hal_crypto_init();

    // VCP + USB
    furi_hal_usb_init();
    furi_hal_usb_set_config(&usb_cdc_single);
    furi_hal_vcp_init();
    FURI_LOG_I(TAG, "USB OK");

    furi_hal_i2c_init();

    // High Level
    furi_hal_power_init();
    furi_hal_light_init();
    furi_hal_vibro_init();
    furi_hal_subghz_init();
    furi_hal_nfc_init();
    furi_hal_rfid_init();
    furi_hal_bt_init();
    furi_hal_compress_icon_init();

    // FreeRTOS glue
    furi_hal_os_init();

    // FatFS driver initialization
    MX_FATFS_Init();
    FURI_LOG_I(TAG, "FATFS OK");

    // Partial null pointer dereference protection
    LL_MPU_Disable();
    LL_MPU_ConfigRegion(
        LL_MPU_REGION_NUMBER0, 0x00, 0x0,
        LL_MPU_REGION_SIZE_1MB
        | LL_MPU_REGION_PRIV_RO_URO
        | LL_MPU_ACCESS_BUFFERABLE
        | LL_MPU_ACCESS_CACHEABLE
        | LL_MPU_ACCESS_SHAREABLE
        | LL_MPU_TEX_LEVEL1
        | LL_MPU_INSTRUCTION_ACCESS_ENABLE
    );
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
}
