#include <target.h>
#include <stm32wbxx.h>
#include <stm32wbxx_ll_system.h>
#include <stm32wbxx_ll_bus.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_rtc.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_hal_flash.h>

#include <version.h>
#include <api-hal.h>

// Boot request enum
#define BOOT_REQUEST_TAINTED 0x00000000
#define BOOT_REQUEST_CLEAN 0xDADEDADE
#define BOOT_REQUEST_DFU 0xDF00B000
// Boot to DFU pin
#define BOOT_DFU_PORT GPIOB
#define BOOT_DFU_PIN LL_GPIO_PIN_11
// USB pins
#define BOOT_USB_PORT GPIOA
#define BOOT_USB_DM_PIN LL_GPIO_PIN_11
#define BOOT_USB_DP_PIN LL_GPIO_PIN_12
#define BOOT_USB_PIN (BOOT_USB_DM_PIN | BOOT_USB_DP_PIN)

#define RTC_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI1_IsReady())

void target_led_control(char* c) {
    api_hal_light_set(LightRed, 0x00);
    api_hal_light_set(LightGreen, 0x00);
    api_hal_light_set(LightBlue, 0x00);
    do {
        if(*c == 'R') {
            api_hal_light_set(LightRed, 0xFF);
        } else if(*c == 'G') {
            api_hal_light_set(LightGreen, 0xFF);
        } else if(*c == 'B') {
            api_hal_light_set(LightBlue, 0xFF);
        } else if(*c == '.') {
            LL_mDelay(125);
            api_hal_light_set(LightRed, 0x00);
            api_hal_light_set(LightGreen, 0x00);
            api_hal_light_set(LightBlue, 0x00);
            LL_mDelay(125);
        } else if(*c == '-') {
            LL_mDelay(250);
            api_hal_light_set(LightRed, 0x00);
            api_hal_light_set(LightGreen, 0x00);
            api_hal_light_set(LightBlue, 0x00);
            LL_mDelay(250);
        } else if(*c == '|') {
            api_hal_light_set(LightRed, 0x00);
            api_hal_light_set(LightGreen, 0x00);
            api_hal_light_set(LightBlue, 0x00);
        }
        c++;
    } while(*c != 0);
}

void clock_init() {
    LL_Init1msTick(4000000);
    LL_SetSystemCoreClock(4000000);
}

void gpio_init() {
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    // USB D+
    LL_GPIO_SetPinMode(BOOT_USB_PORT, BOOT_USB_DP_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(BOOT_USB_PORT, BOOT_USB_DP_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinOutputType(BOOT_USB_PORT, BOOT_USB_DP_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    // USB D-
    LL_GPIO_SetPinMode(BOOT_USB_PORT, BOOT_USB_DM_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(BOOT_USB_PORT, BOOT_USB_DM_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinOutputType(BOOT_USB_PORT, BOOT_USB_DM_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    // Button: back
    LL_GPIO_SetPinMode(BOOT_DFU_PORT, BOOT_DFU_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(BOOT_DFU_PORT, BOOT_DFU_PIN, LL_GPIO_PULL_UP);
}

void rtc_init() {
    // LSE and RTC
    LL_PWR_EnableBkUpAccess();
    if(!RTC_CLOCK_IS_READY()) {
        // Start LSI1 needed for CSS
        LL_RCC_LSI1_Enable();
        // Try to start LSE normal way
        LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_MEDIUMLOW);
        LL_RCC_LSE_Enable();
        uint32_t c = 0;
        while(!RTC_CLOCK_IS_READY() && c < 200) {
            LL_mDelay(10);
            c++;
        }
        // Plan B: reset backup domain
        if(!RTC_CLOCK_IS_READY()) {
            target_led_control("-R.R.R.");
            LL_RCC_ForceBackupDomainReset();
            LL_RCC_ReleaseBackupDomainReset();
            NVIC_SystemReset();
        }
        // Set RTC domain clock to LSE
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        // Enable LSE CSS
        LL_RCC_LSE_EnableCSS();
    }
    // Enable clocking
    LL_RCC_EnableRTC();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);
}

void version_save(void) {
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR1, (uint32_t)version_get());
}

void usb_wire_reset() {
    LL_GPIO_ResetOutputPin(BOOT_USB_PORT, BOOT_USB_PIN);
    LL_mDelay(10);
    LL_GPIO_SetOutputPin(BOOT_USB_PORT, BOOT_USB_PIN);
}

void target_init() {
    clock_init();
    gpio_init();
    api_hal_init();
    target_led_control("RGB");
    rtc_init();
    version_save();
    usb_wire_reset();

    // Errata 2.2.9, Flash OPTVERR flag is always set after system reset
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
}

int target_is_dfu_requested() {
    if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) == BOOT_REQUEST_TAINTED) {
        // Default system state is tainted
        // We must ensure that MCU is cleanly booted
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_CLEAN);
        NVIC_SystemReset();
    } else if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) == BOOT_REQUEST_DFU) {
        return 1;
    }
    LL_mDelay(100);
    if(!LL_GPIO_IsInputPinSet(BOOT_DFU_PORT, BOOT_DFU_PIN)) {
        return 1;
    }

    return 0;
}

void target_switch(void* offset) {
    asm volatile("ldr    r3, [%0]    \n"
                 "msr    msp, r3     \n"
                 "ldr    r3, [%1]    \n"
                 "mov    pc, r3      \n"
                 :
                 : "r"(offset), "r"(offset + 0x4)
                 : "r3");
}

void target_switch2dfu() {
    target_led_control("B");
    // Mark system as tainted, it will be soon
    LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_TAINTED);
    // Remap memory to system bootloader
    LL_SYSCFG_SetRemapMemory(LL_SYSCFG_REMAP_SYSTEMFLASH);
    target_switch(0x0);
}

void target_switch2os() {
    target_led_control("G");
    SCB->VTOR = OS_OFFSET;
    target_switch((void*)(BOOT_ADDRESS + OS_OFFSET));
}
