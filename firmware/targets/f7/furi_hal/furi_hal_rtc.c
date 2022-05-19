#include <furi_hal_rtc.h>
#include <furi_hal_light.h>
#include <furi_hal_debug.h>

#include <stm32wbxx_ll_bus.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_rtc.h>
#include <stm32wbxx_ll_utils.h>

#include <furi.h>

#define TAG "FuriHalRtc"

#define RTC_CLOCK_IS_READY() (LL_RCC_LSE_IsReady() && LL_RCC_LSI1_IsReady())

#define FURI_HAL_RTC_HEADER_MAGIC 0x10F1
#define FURI_HAL_RTC_HEADER_VERSION 0

typedef struct {
    uint16_t magic;
    uint8_t version;
    uint8_t unused;
} FuriHalRtcHeader;

typedef struct {
    uint8_t log_level : 4;
    uint8_t log_reserved : 4;
    uint8_t flags;
    uint8_t boot_mode : 4;
    uint16_t reserved : 12;
} DeveloperReg;

_Static_assert(sizeof(DeveloperReg) == 4, "DeveloperReg size mismatch");

#define FURI_HAL_RTC_SECONDS_PER_MINUTE 60
#define FURI_HAL_RTC_SECONDS_PER_HOUR (FURI_HAL_RTC_SECONDS_PER_MINUTE * 60)
#define FURI_HAL_RTC_SECONDS_PER_DAY (FURI_HAL_RTC_SECONDS_PER_HOUR * 24)
#define FURI_HAL_RTC_MONTHS_COUNT 12
#define FURI_HAL_RTC_EPOCH_START_YEAR 1970
#define FURI_HAL_RTC_IS_LEAP_YEAR(year) \
    ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))

static const uint8_t furi_hal_rtc_days_per_month[][FURI_HAL_RTC_MONTHS_COUNT] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const uint16_t furi_hal_rtc_days_per_year[] = {365, 366};

void furi_hal_rtc_init_early() {
    // LSE and RTC
    LL_PWR_EnableBkUpAccess();
    if(!RTC_CLOCK_IS_READY()) {
        LL_RCC_LSI1_Enable();
        // Try to start LSE normal way
        LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
        LL_RCC_LSE_Enable();
        uint32_t c = 0;
        while(!RTC_CLOCK_IS_READY() && c < 200) {
            LL_mDelay(10);
            c++;
        }
        // Plan B: reset backup domain
        if(!RTC_CLOCK_IS_READY()) {
            furi_hal_light_sequence("rgb R.r.R.r.R");
            LL_RCC_ForceBackupDomainReset();
            LL_RCC_ReleaseBackupDomainReset();
            NVIC_SystemReset();
        }
        // Set RTC domain clock to LSE
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    }
    // Enable clocking
    LL_RCC_EnableRTC();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);

    // Verify header register
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterHeader);
    FuriHalRtcHeader* data = (FuriHalRtcHeader*)&data_reg;
    if(data->magic != FURI_HAL_RTC_HEADER_MAGIC || data->version != FURI_HAL_RTC_HEADER_VERSION) {
        // Reset all our registers to ensure consistency
        for(size_t i = 0; i < FuriHalRtcRegisterMAX; i++) {
            furi_hal_rtc_set_register(i, 0);
        }
        data->magic = FURI_HAL_RTC_HEADER_MAGIC;
        data->version = FURI_HAL_RTC_HEADER_VERSION;
        furi_hal_rtc_set_register(FuriHalRtcRegisterHeader, data_reg);
    }

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        furi_hal_debug_enable();
    } else {
        furi_hal_debug_disable();
    }
}

void furi_hal_rtc_deinit_early() {
}

void furi_hal_rtc_init() {
    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    }

    LL_RCC_EnableRTC();

    LL_RTC_InitTypeDef RTC_InitStruct = {0};
    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = 127;
    RTC_InitStruct.SynchPrescaler = 255;
    LL_RTC_Init(RTC, &RTC_InitStruct);

    furi_log_set_level(furi_hal_rtc_get_log_level());

    FURI_LOG_I(TAG, "Init OK");
}

uint32_t furi_hal_rtc_get_register(FuriHalRtcRegister reg) {
    return LL_RTC_BAK_GetRegister(RTC, reg);
}

void furi_hal_rtc_set_register(FuriHalRtcRegister reg, uint32_t value) {
    LL_RTC_BAK_SetRegister(RTC, reg, value);
}

void furi_hal_rtc_set_log_level(uint8_t level) {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    data->log_level = level;
    furi_hal_rtc_set_register(FuriHalRtcRegisterSystem, data_reg);
    furi_log_set_level(level);
}

uint8_t furi_hal_rtc_get_log_level() {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    return data->log_level;
}

void furi_hal_rtc_set_flag(FuriHalRtcFlag flag) {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    data->flags |= flag;
    furi_hal_rtc_set_register(FuriHalRtcRegisterSystem, data_reg);

    if(flag & FuriHalRtcFlagDebug) {
        furi_hal_debug_enable();
    }
}

void furi_hal_rtc_reset_flag(FuriHalRtcFlag flag) {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    data->flags &= ~flag;
    furi_hal_rtc_set_register(FuriHalRtcRegisterSystem, data_reg);

    if(flag & FuriHalRtcFlagDebug) {
        furi_hal_debug_disable();
    }
}

bool furi_hal_rtc_is_flag_set(FuriHalRtcFlag flag) {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    return data->flags & flag;
}

void furi_hal_rtc_set_boot_mode(FuriHalRtcBootMode mode) {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    data->boot_mode = mode;
    furi_hal_rtc_set_register(FuriHalRtcRegisterSystem, data_reg);
}

FuriHalRtcBootMode furi_hal_rtc_get_boot_mode() {
    uint32_t data_reg = furi_hal_rtc_get_register(FuriHalRtcRegisterSystem);
    DeveloperReg* data = (DeveloperReg*)&data_reg;
    return (FuriHalRtcBootMode)data->boot_mode;
}

void furi_hal_rtc_set_datetime(FuriHalRtcDateTime* datetime) {
    furi_assert(datetime);

    /* Disable write protection */
    LL_RTC_DisableWriteProtection(RTC);

    /* Enter Initialization mode and wait for INIT flag to be set */
    LL_RTC_EnableInitMode(RTC);
    while(!LL_RTC_IsActiveFlag_INIT(RTC)) {
    }

    /* Set time */
    LL_RTC_TIME_Config(
        RTC,
        LL_RTC_TIME_FORMAT_AM_OR_24,
        __LL_RTC_CONVERT_BIN2BCD(datetime->hour),
        __LL_RTC_CONVERT_BIN2BCD(datetime->minute),
        __LL_RTC_CONVERT_BIN2BCD(datetime->second));

    /* Set date */
    LL_RTC_DATE_Config(
        RTC,
        datetime->weekday,
        __LL_RTC_CONVERT_BIN2BCD(datetime->day),
        __LL_RTC_CONVERT_BIN2BCD(datetime->month),
        __LL_RTC_CONVERT_BIN2BCD(datetime->year - 2000));

    /* Exit Initialization mode */
    LL_RTC_DisableInitMode(RTC);

    /* If RTC_CR_BYPSHAD bit = 0, wait for synchro else this check is not needed */
    if(!LL_RTC_IsShadowRegBypassEnabled(RTC)) {
        LL_RTC_ClearFlag_RS(RTC);
        while(!LL_RTC_IsActiveFlag_RS(RTC)) {
        };
    }

    /* Enable write protection */
    LL_RTC_EnableWriteProtection(RTC);
}

void furi_hal_rtc_get_datetime(FuriHalRtcDateTime* datetime) {
    furi_assert(datetime);

    uint32_t time = LL_RTC_TIME_Get(RTC); // 0x00HHMMSS
    uint32_t date = LL_RTC_DATE_Get(RTC); // 0xWWDDMMYY

    datetime->second = __LL_RTC_CONVERT_BCD2BIN((time >> 0) & 0xFF);
    datetime->minute = __LL_RTC_CONVERT_BCD2BIN((time >> 8) & 0xFF);
    datetime->hour = __LL_RTC_CONVERT_BCD2BIN((time >> 16) & 0xFF);
    datetime->year = __LL_RTC_CONVERT_BCD2BIN((date >> 0) & 0xFF) + 2000;
    datetime->month = __LL_RTC_CONVERT_BCD2BIN((date >> 8) & 0xFF);
    datetime->day = __LL_RTC_CONVERT_BCD2BIN((date >> 16) & 0xFF);
    datetime->weekday = __LL_RTC_CONVERT_BCD2BIN((date >> 24) & 0xFF);
}

bool furi_hal_rtc_validate_datetime(FuriHalRtcDateTime* datetime) {
    bool invalid = false;

    invalid |= (datetime->second > 59);
    invalid |= (datetime->minute > 59);
    invalid |= (datetime->hour > 23);

    invalid |= (datetime->year < 2000);
    invalid |= (datetime->year > 2099);

    invalid |= (datetime->month == 0);
    invalid |= (datetime->month > 12);

    invalid |= (datetime->day == 0);
    invalid |= (datetime->day > 31);

    invalid |= (datetime->weekday == 0);
    invalid |= (datetime->weekday > 7);

    return !invalid;
}

void furi_hal_rtc_set_fault_data(uint32_t value) {
    furi_hal_rtc_set_register(FuriHalRtcRegisterFaultData, value);
}

uint32_t furi_hal_rtc_get_fault_data() {
    return furi_hal_rtc_get_register(FuriHalRtcRegisterFaultData);
}

void furi_hal_rtc_set_pin_fails(uint32_t value) {
    furi_hal_rtc_set_register(FuriHalRtcRegisterPinFails, value);
}

uint32_t furi_hal_rtc_get_pin_fails() {
    return furi_hal_rtc_get_register(FuriHalRtcRegisterPinFails);
}

uint32_t furi_hal_rtc_datetime_to_timestamp(FuriHalRtcDateTime* datetime) {
    uint32_t timestamp = 0;
    uint8_t years = 0;
    uint8_t leap_years = 0;

    for(uint16_t y = FURI_HAL_RTC_EPOCH_START_YEAR; y < datetime->year; y++) {
        if(FURI_HAL_RTC_IS_LEAP_YEAR(y)) {
            leap_years++;
        } else {
            years++;
        }
    }

    timestamp +=
        ((years * furi_hal_rtc_days_per_year[0]) + (leap_years * furi_hal_rtc_days_per_year[1])) *
        FURI_HAL_RTC_SECONDS_PER_DAY;

    uint8_t year_index = (FURI_HAL_RTC_IS_LEAP_YEAR(datetime->year)) ? 1 : 0;

    for(uint8_t m = 0; m < (datetime->month - 1); m++) {
        timestamp += furi_hal_rtc_days_per_month[year_index][m] * FURI_HAL_RTC_SECONDS_PER_DAY;
    }

    timestamp += (datetime->day - 1) * FURI_HAL_RTC_SECONDS_PER_DAY;
    timestamp += datetime->hour * FURI_HAL_RTC_SECONDS_PER_HOUR;
    timestamp += datetime->minute * FURI_HAL_RTC_SECONDS_PER_MINUTE;
    timestamp += datetime->second;

    return timestamp;
}
