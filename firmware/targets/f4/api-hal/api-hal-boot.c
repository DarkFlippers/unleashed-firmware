#include <api-hal-boot.h>
#include <stm32wbxx_ll_rtc.h>

#define BOOT_REQUEST_NONE 0x00000000
#define BOOT_REQUEST_DFU 0xDF00B000

void api_hal_boot_set_mode(ApiHalBootMode mode) {
    if (mode == ApiHalBootModeNormal) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_NONE);
    } else if (mode == ApiHalBootModeDFU) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, BOOT_REQUEST_DFU);
    }
}