#include <furi-hal-rtc.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_rtc.h>

#include <furi.h>

#define TAG "FuriHalRtc"

#define FURI_HAL_RTC_BOOT_FLAGS_REG LL_RTC_BKP_DR0
#define FURI_HAL_RTC_BOOT_VERSION_REG LL_RTC_BKP_DR1
#define FURI_HAL_RTC_SYSTEM_REG LL_RTC_BKP_DR2

typedef struct {
    uint8_t log_level:4;
    uint8_t log_reserved:4;
    uint8_t flags;
    uint16_t reserved;
} DeveloperReg;

void furi_hal_rtc_init() {
    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    }

    LL_RCC_EnableRTC();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);

    LL_RTC_InitTypeDef RTC_InitStruct = {0};
    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = 127;
    RTC_InitStruct.SynchPrescaler = 255;
    LL_RTC_Init(RTC, &RTC_InitStruct);

    furi_log_set_level(furi_hal_rtc_get_log_level());

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_rtc_set_log_level(uint8_t level) {
    uint32_t data = LL_RTC_BAK_GetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG);
    ((DeveloperReg*)&data)->log_level = level;
    LL_RTC_BAK_SetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG, data);
    furi_log_set_level(level);
}

uint8_t furi_hal_rtc_get_log_level() {
    uint32_t data = LL_RTC_BAK_GetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG);
    return ((DeveloperReg*)&data)->log_level;
}

void furi_hal_rtc_set_flag(FuriHalRtcFlag flag) {
    uint32_t data = LL_RTC_BAK_GetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG);
    ((DeveloperReg*)&data)->flags |= flag;
    LL_RTC_BAK_SetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG, data);
}

void furi_hal_rtc_reset_flag(FuriHalRtcFlag flag) {
    uint32_t data = LL_RTC_BAK_GetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG);
    ((DeveloperReg*)&data)->flags &= ~flag;
    LL_RTC_BAK_SetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG, data);
}

bool furi_hal_rtc_is_flag_set(FuriHalRtcFlag flag) {
    uint32_t data = LL_RTC_BAK_GetRegister(RTC, FURI_HAL_RTC_SYSTEM_REG);
    return ((DeveloperReg*)&data)->flags & flag;
}

void furi_hal_rtc_set_datetime(FuriHalRtcDateTime* datetime) {
    furi_assert(datetime);

    /* Disable write protection */
    LL_RTC_DisableWriteProtection(RTC);

    /* Enter Initialization mode and wait for INIT flag to be set */
    LL_RTC_EnableInitMode(RTC);
    while(!LL_RTC_IsActiveFlag_INIT(RTC)) {}

    /* Set time */
    LL_RTC_TIME_Config(RTC,
        LL_RTC_TIME_FORMAT_AM_OR_24,
        __LL_RTC_CONVERT_BIN2BCD(datetime->hour),
        __LL_RTC_CONVERT_BIN2BCD(datetime->minute),
        __LL_RTC_CONVERT_BIN2BCD(datetime->second)
    );

    /* Set date */
    LL_RTC_DATE_Config(RTC,
        datetime->weekday,
        __LL_RTC_CONVERT_BIN2BCD(datetime->day),
        __LL_RTC_CONVERT_BIN2BCD(datetime->month),
        __LL_RTC_CONVERT_BIN2BCD(datetime->year - 2000)
    );

    /* Exit Initialization mode */
    LL_RTC_DisableInitMode(RTC);

    /* If RTC_CR_BYPSHAD bit = 0, wait for synchro else this check is not needed */
    if (!LL_RTC_IsShadowRegBypassEnabled(RTC)) {
        LL_RTC_ClearFlag_RS(RTC);
        while(!LL_RTC_IsActiveFlag_RS(RTC)) {};
    }

    /* Enable write protection */
    LL_RTC_EnableWriteProtection(RTC);
}

void furi_hal_rtc_get_datetime(FuriHalRtcDateTime* datetime) {
    furi_assert(datetime);

    uint32_t time = LL_RTC_TIME_Get(RTC); // 0x00HHMMSS
    uint32_t date = LL_RTC_DATE_Get(RTC); // 0xWWDDMMYY

    datetime->second = __LL_RTC_CONVERT_BCD2BIN((time>>0) & 0xFF);
    datetime->minute = __LL_RTC_CONVERT_BCD2BIN((time>>8) & 0xFF);
    datetime->hour = __LL_RTC_CONVERT_BCD2BIN((time>>16) & 0xFF);
    datetime->year = __LL_RTC_CONVERT_BCD2BIN((date >> 0) & 0xFF) + 2000;
    datetime->month = __LL_RTC_CONVERT_BCD2BIN((date >> 8) & 0xFF);
    datetime->day = __LL_RTC_CONVERT_BCD2BIN((date >> 16) & 0xFF);
    datetime->weekday = __LL_RTC_CONVERT_BCD2BIN((date >> 24) & 0xFF);
}
