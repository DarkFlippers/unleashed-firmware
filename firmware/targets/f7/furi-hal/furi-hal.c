#include <furi-hal.h>

#include <comp.h>
#include <rtc.h>
#include <tim.h>
#include <gpio.h>

void furi_hal_init() {
    furi_hal_clock_init();
    furi_hal_console_init();
    furi_hal_interrupt_init();
    furi_hal_delay_init();

    MX_GPIO_Init();
    FURI_LOG_I("HAL", "GPIO OK");

    MX_RTC_Init();
    FURI_LOG_I("HAL", "RTC OK");
    furi_hal_boot_init();
    furi_hal_version_init();

    furi_hal_spi_init();

    MX_TIM1_Init();
    FURI_LOG_I("HAL", "TIM1 OK");
    MX_TIM2_Init();
    FURI_LOG_I("HAL", "TIM2 OK");
    MX_TIM16_Init();
    FURI_LOG_I("HAL", "TIM16 OK");
    MX_COMP1_Init();
    FURI_LOG_I("HAL", "COMP1 OK");

    furi_hal_crypto_init();

    // VCP + USB
    furi_hal_usb_init();
    furi_hal_usb_set_config(UsbModeVcpSingle);
    furi_hal_vcp_init();
    FURI_LOG_I("HAL", "USB OK");

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
}
