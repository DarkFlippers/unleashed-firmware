#include <api-hal.h>

#include <adc.h>
#include <aes.h>
#include <comp.h>
#include <crc.h>
#include <pka.h>
#include <rf.h>
#include <rng.h>
#include <rtc.h>
#include <spi.h>
#include <tim.h>
#include <usb_device.h>
#include <gpio.h>

void api_hal_init() {
    api_hal_clock_init();
    api_hal_console_init();
    api_hal_interrupt_init();
    api_hal_delay_init();

    MX_GPIO_Init();
    FURI_LOG_I("HAL", "GPIO OK");

    MX_RTC_Init();
    FURI_LOG_I("HAL", "RTC OK");
    api_hal_boot_init();
    api_hal_version_init();
    
    MX_ADC1_Init();
    FURI_LOG_I("HAL", "ADC1 OK");

    MX_SPI1_Init();
    FURI_LOG_I("HAL", "SPI1 OK");
    MX_SPI2_Init();
    FURI_LOG_I("HAL", "SPI2 OK");
    api_hal_spi_init();

    MX_TIM1_Init();
    FURI_LOG_I("HAL", "TIM1 OK");
    MX_TIM2_Init();
    FURI_LOG_I("HAL", "TIM2 OK");
    MX_TIM16_Init();
    FURI_LOG_I("HAL", "TIM16 OK");
    MX_COMP1_Init();
    FURI_LOG_I("HAL", "COMP1 OK");
    MX_RF_Init();
    FURI_LOG_I("HAL", "RF OK");
    MX_PKA_Init();
    FURI_LOG_I("HAL", "PKA OK");
    MX_RNG_Init();
    FURI_LOG_I("HAL", "RNG OK");
    MX_AES1_Init();
    FURI_LOG_I("HAL", "AES1 OK");
    MX_AES2_Init();
    FURI_LOG_I("HAL", "AES2 OK");
    MX_CRC_Init();
    FURI_LOG_I("HAL", "CRC OK");

    // VCP + USB
    api_hal_vcp_init();
    MX_USB_Device_Init();
    FURI_LOG_I("HAL", "USB OK");

    api_hal_i2c_init();

    // High Level
    api_hal_power_init();
    api_hal_light_init();
    api_hal_vibro_init();
    api_hal_subghz_init();
    api_hal_nfc_init();

    // FreeRTOS glue
    api_hal_os_init();
}
