#include <target.h>
#include <stm32l4xx.h>
#include <stm32l4xx_ll_system.h>
#include <stm32l4xx_ll_bus.h>
#include <stm32l4xx_ll_utils.h>
#include <stm32l4xx_ll_rcc.h>
#include <stm32l4xx_ll_rtc.h>
#include <stm32l4xx_ll_pwr.h>
#include <stm32l4xx_ll_gpio.h>

// Boot request enum
#define BOOT_REQUEST_NONE 0x00000000
#define BOOT_REQUEST_DFU 0xDF00B000
// Boot to DFU pin
#define BOOT_DFU_PORT GPIOB
#define BOOT_DFU_PIN LL_GPIO_PIN_8
// LCD backlight
#define BOOT_LCD_BL_PORT GPIOB
#define BOOT_LCD_BL_PIN LL_GPIO_PIN_6
// LEDs
#define LED_RED_PORT GPIOA
#define LED_RED_PIN LL_GPIO_PIN_8
#define LED_GREEN_PORT GPIOB
#define LED_GREEN_PIN LL_GPIO_PIN_14
#define LED_BLUE_PORT GPIOB
#define LED_BLUE_PIN LL_GPIO_PIN_1
// USB pins
#define BOOT_USB_PORT GPIOA
#define BOOT_USB_DM_PIN LL_GPIO_PIN_11
#define BOOT_USB_DP_PIN LL_GPIO_PIN_12
#define BOOT_USB_PIN (BOOT_USB_DM_PIN | BOOT_USB_DP_PIN)

void clock_init() {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    LL_RCC_MSI_Enable();
    while(LL_RCC_MSI_IsReady() != 1) {
    }

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1) {
    }

    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    };

    /* Set APB1 & APB2 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Set systick to 1ms in using frequency set to 80MHz */
    /* This frequency can be calculated through LL RCC macro */
    /* ex: __LL_RCC_CALC_PLLCLK_FREQ(__LL_RCC_CALC_MSI_FREQ(LL_RCC_MSIRANGESEL_RUN, LL_RCC_MSIRANGE_6), 
                                    LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2)*/
    LL_Init1msTick(80000000);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(80000000);
}

void gpio_init() {
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
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
    LL_GPIO_SetPinPull(BOOT_DFU_PORT, BOOT_DFU_PIN, LL_GPIO_PULL_DOWN);
    // Display backlight
    LL_GPIO_SetPinMode(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_OUTPUT_PUSHPULL);
    // LEDs
    LL_GPIO_SetPinMode(LED_RED_PORT, LED_RED_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_RED_PORT, LED_RED_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetOutputPin(LED_RED_PORT, LED_RED_PIN);
    LL_GPIO_SetPinMode(LED_GREEN_PORT, LED_GREEN_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_GREEN_PORT, LED_GREEN_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetOutputPin(LED_GREEN_PORT, LED_GREEN_PIN);
    LL_GPIO_SetPinMode(LED_BLUE_PORT, LED_BLUE_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_BLUE_PORT, LED_BLUE_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetOutputPin(LED_BLUE_PORT, LED_BLUE_PIN);
}

void rtc_init() {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_EnableBkUpAccess();
    LL_RCC_EnableRTC();
}

void lcd_backlight_on() {
    LL_GPIO_SetOutputPin(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN);
}

void usb_wire_reset() {
    LL_GPIO_ResetOutputPin(BOOT_USB_PORT, BOOT_USB_PIN);
    LL_mDelay(10);
    LL_GPIO_SetOutputPin(BOOT_USB_PORT, BOOT_USB_PIN);
}

void target_init() {
    clock_init();
    rtc_init();
    gpio_init();

    usb_wire_reset();
}

int target_is_dfu_requested() {
    if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) == BOOT_REQUEST_DFU) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_NONE);
        return 1;
    }

    if(LL_GPIO_IsInputPinSet(BOOT_DFU_PORT, BOOT_DFU_PIN)) {
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
    LL_GPIO_ResetOutputPin(LED_BLUE_PORT, LED_BLUE_PIN);
    // Remap memory to system bootloader
    LL_SYSCFG_SetRemapMemory(LL_SYSCFG_REMAP_SYSTEMFLASH);
    target_switch(0x0);
}

void target_switch2os() {
    LL_GPIO_ResetOutputPin(LED_RED_PORT, LED_RED_PIN);
    SCB->VTOR = BOOT_ADDRESS + OS_OFFSET;
    target_switch((void*)(BOOT_ADDRESS + OS_OFFSET));
}