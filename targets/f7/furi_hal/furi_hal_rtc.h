/**
 * @file furi_hal_rtc.h
 * Furi Hal RTC API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // Time
    uint8_t hour; /**< Hour in 24H format: 0-23 */
    uint8_t minute; /**< Minute: 0-59 */
    uint8_t second; /**< Second: 0-59 */
    // Date
    uint8_t day; /**< Current day: 1-31 */
    uint8_t month; /**< Current month: 1-12 */
    uint16_t year; /**< Current year: 2000-2099 */
    uint8_t weekday; /**< Current weekday: 1-7 */
} FuriHalRtcDateTime;

typedef enum {
    FuriHalRtcFlagDebug = (1 << 0),
    FuriHalRtcFlagStorageFormatInternal = (1 << 1),
    FuriHalRtcFlagLock = (1 << 2),
    FuriHalRtcFlagC2Update = (1 << 3),
    FuriHalRtcFlagHandOrient = (1 << 4),
    FuriHalRtcFlagLegacySleep = (1 << 5),
    FuriHalRtcFlagStealthMode = (1 << 6),
    FuriHalRtcFlagDetailedFilename = (1 << 7),
} FuriHalRtcFlag;

typedef enum {
    FuriHalRtcBootModeNormal = 0, /**< Normal boot mode, default value */
    FuriHalRtcBootModeDfu, /**< Boot to DFU (MCU bootloader by ST) */
    FuriHalRtcBootModePreUpdate, /**< Boot to Update, pre update */
    FuriHalRtcBootModeUpdate, /**< Boot to Update, main */
    FuriHalRtcBootModePostUpdate, /**< Boot to Update, post update */
} FuriHalRtcBootMode;

typedef enum {
    FuriHalRtcHeapTrackModeNone = 0, /**< Disable allocation tracking */
    FuriHalRtcHeapTrackModeMain, /**< Enable allocation tracking for main application thread */
    FuriHalRtcHeapTrackModeTree, /**< Enable allocation tracking for main and children application threads */
    FuriHalRtcHeapTrackModeAll, /**< Enable allocation tracking for all threads */
} FuriHalRtcHeapTrackMode;

typedef enum {
    FuriHalRtcRegisterHeader, /**< RTC structure header */
    FuriHalRtcRegisterSystem, /**< Various system bits */
    FuriHalRtcRegisterVersion, /**< Pointer to Version */
    FuriHalRtcRegisterLfsFingerprint, /**< LFS geometry fingerprint */
    FuriHalRtcRegisterFaultData, /**< Pointer to last fault message */
    FuriHalRtcRegisterPinFails, /**< Failed pins count */
    /* Index of FS directory entry corresponding to FW update to be applied */
    FuriHalRtcRegisterUpdateFolderFSIndex,

    FuriHalRtcRegisterMAX, /**< Service value, do not use */
} FuriHalRtcRegister;

typedef enum {
    FuriHalRtcLocaleUnitsMetric = 0x0, /**< Metric measurement units */
    FuriHalRtcLocaleUnitsImperial = 0x1, /**< Imperial measurement units */
} FuriHalRtcLocaleUnits;

typedef enum {
    FuriHalRtcLocaleTimeFormat24h = 0x0, /**< 24-hour format */
    FuriHalRtcLocaleTimeFormat12h = 0x1, /**< 12-hour format */
} FuriHalRtcLocaleTimeFormat;

typedef enum {
    FuriHalRtcLocaleDateFormatDMY = 0x0, /**< Day/Month/Year */
    FuriHalRtcLocaleDateFormatMDY = 0x1, /**< Month/Day/Year */
    FuriHalRtcLocaleDateFormatYMD = 0x2, /**< Year/Month/Day */
} FuriHalRtcLocaleDateFormat;

typedef enum {
    FuriHalRtcLogDeviceUsart = 0x0, /**< Default: USART */
    FuriHalRtcLogDeviceLpuart = 0x1, /**< Default: LPUART */
    FuriHalRtcLogDeviceReserved = 0x2, /**< Reserved for future use */
    FuriHalRtcLogDeviceNone = 0x3, /**< None, disable serial logging */
} FuriHalRtcLogDevice;

typedef enum {
    FuriHalRtcLogBaudRate230400 = 0x0, /**< 230400 baud */
    FuriHalRtcLogBaudRate9600 = 0x1, /**< 9600 baud */
    FuriHalRtcLogBaudRate38400 = 0x2, /**< 38400 baud */
    FuriHalRtcLogBaudRate57600 = 0x3, /**< 57600 baud */
    FuriHalRtcLogBaudRate115200 = 0x4, /**< 115200 baud */
    FuriHalRtcLogBaudRate460800 = 0x5, /**< 460800 baud */
    FuriHalRtcLogBaudRate921600 = 0x6, /**< 921600 baud */
    FuriHalRtcLogBaudRate1843200 = 0x7, /**< 1843200 baud */
} FuriHalRtcLogBaudRate;

/** Early initialization */
void furi_hal_rtc_init_early(void);

/** Early de-initialization */
void furi_hal_rtc_deinit_early(void);

/** Initialize RTC subsystem */
void furi_hal_rtc_init(void);

/** Force sync shadow registers */
void furi_hal_rtc_sync_shadow(void);

/** Reset ALL RTC registers content */
void furi_hal_rtc_reset_registers();

/** Get RTC register content
 *
 * @param[in]  reg   The register identifier
 *
 * @return     content of the register
 */
uint32_t furi_hal_rtc_get_register(FuriHalRtcRegister reg);

/** Set register content
 *
 * @param[in]  reg    The register identifier
 * @param[in]  value  The value to store into register
 */
void furi_hal_rtc_set_register(FuriHalRtcRegister reg, uint32_t value);

/** Set Log Level value
 *
 * @param[in]  level  The level to store
 */
void furi_hal_rtc_set_log_level(uint8_t level);

/** Get Log Level value
 *
 * @return     The Log Level value
 */
uint8_t furi_hal_rtc_get_log_level(void);

/** Set logging device
 *
 * @param[in]  device  The device
 */
void furi_hal_rtc_set_log_device(FuriHalRtcLogDevice device);

/** Get logging device
 *
 * @return     The furi hal rtc log device.
 */
FuriHalRtcLogDevice furi_hal_rtc_get_log_device(void);

/** Set logging baud rate
 *
 * @param[in]  baud_rate  The baud rate
 */
void furi_hal_rtc_set_log_baud_rate(FuriHalRtcLogBaudRate baud_rate);

/** Get logging baud rate
 *
 * @return     The furi hal rtc log baud rate.
 */
FuriHalRtcLogBaudRate furi_hal_rtc_get_log_baud_rate(void);

/** Set RTC Flag
 *
 * @param[in]  flag  The flag to set
 */
void furi_hal_rtc_set_flag(FuriHalRtcFlag flag);

/** Reset RTC Flag
 *
 * @param[in]  flag  The flag to reset
 */
void furi_hal_rtc_reset_flag(FuriHalRtcFlag flag);

/** Check if RTC Flag is set
 *
 * @param[in]  flag  The flag to check
 *
 * @return     true if set
 */
bool furi_hal_rtc_is_flag_set(FuriHalRtcFlag flag);

/** Set RTC boot mode
 *
 * @param[in]  mode  The mode to set
 */
void furi_hal_rtc_set_boot_mode(FuriHalRtcBootMode mode);

/** Get RTC boot mode
 *
 * @return     The RTC boot mode.
 */
FuriHalRtcBootMode furi_hal_rtc_get_boot_mode(void);

/** Set Heap Track mode
 *
 * @param[in]  mode  The mode to set
 */
void furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackMode mode);

/** Get RTC Heap Track mode
 *
 * @return     The RTC heap track mode.
 */
FuriHalRtcHeapTrackMode furi_hal_rtc_get_heap_track_mode(void);

/** Set locale units
 *
 * @param[in]  mode  The RTC Locale Units
 */
void furi_hal_rtc_set_locale_units(FuriHalRtcLocaleUnits value);

/** Get RTC Locale Units
 *
 * @return     The RTC Locale Units.
 */
FuriHalRtcLocaleUnits furi_hal_rtc_get_locale_units(void);

/** Set RTC Locale Time Format
 *
 * @param[in]  value  The RTC Locale Time Format
 */
void furi_hal_rtc_set_locale_timeformat(FuriHalRtcLocaleTimeFormat value);

/** Get RTC Locale Time Format
 *
 * @return     The RTC Locale Time Format.
 */
FuriHalRtcLocaleTimeFormat furi_hal_rtc_get_locale_timeformat(void);

/** Set RTC Locale Date Format
 *
 * @param[in]  value  The RTC Locale Date Format
 */
void furi_hal_rtc_set_locale_dateformat(FuriHalRtcLocaleDateFormat value);

/** Get RTC Locale Date Format
 *
 * @return     The RTC Locale Date Format
 */
FuriHalRtcLocaleDateFormat furi_hal_rtc_get_locale_dateformat(void);

/** Set RTC Date Time
 *
 * @param      datetime  The date time to set
 */
void furi_hal_rtc_set_datetime(FuriHalRtcDateTime* datetime);

/** Get RTC Date Time
 *
 * @param      datetime  The datetime
 */
void furi_hal_rtc_get_datetime(FuriHalRtcDateTime* datetime);

/** Validate Date Time
 *
 * @param      datetime  The datetime to validate
 *
 * @return     { description_of_the_return_value }
 */
bool furi_hal_rtc_validate_datetime(FuriHalRtcDateTime* datetime);

/** Set RTC Fault Data
 *
 * @param[in]  value  The value
 */
void furi_hal_rtc_set_fault_data(uint32_t value);

/** Get RTC Fault Data
 *
 * @return     RTC Fault Data value
 */
uint32_t furi_hal_rtc_get_fault_data(void);

/** Set Pin Fails count
 *
 * @param[in]  value  The Pin Fails count
 */
void furi_hal_rtc_set_pin_fails(uint32_t value);

/** Get Pin Fails count
 *
 * @return     Pin Fails Count
 */
uint32_t furi_hal_rtc_get_pin_fails(void);

/** Get UNIX Timestamp
 *
 * @return     Unix Timestamp in seconds from UNIX epoch start
 */
uint32_t furi_hal_rtc_get_timestamp(void);

/** Convert DateTime to UNIX timestamp
 * 
 * @warning    Mind timezone when perform conversion
 *
 * @param      datetime  The datetime (UTC)
 *
 * @return     UNIX Timestamp in seconds from UNIX epoch start
 */
uint32_t furi_hal_rtc_datetime_to_timestamp(FuriHalRtcDateTime* datetime);

/** Convert UNIX timestamp to DateTime
 *
 * @warning    Mind timezone when perform conversion
 *
 * @param[in]  timestamp  UNIX Timestamp in seconds from UNIX epoch start
 * @param[out] datetime   The datetime (UTC)
 */
void furi_hal_rtc_timestamp_to_datetime(uint32_t timestamp, FuriHalRtcDateTime* datetime);

/** Gets the number of days in the year according to the Gregorian calendar.
 *
 * @param year Input year.
 *
 * @return number of days in `year`.
 */
uint16_t furi_hal_rtc_get_days_per_year(uint16_t year);

/** Check if a year a leap year in the Gregorian calendar.
 *
 * @param year Input year.
 *
 * @return true if `year` is a leap year.
 */
bool furi_hal_rtc_is_leap_year(uint16_t year);

/** Get the number of days in the month.
 *
 * @param leap_year true to calculate based on leap years
 * @param month month to check, where 1 = January
 * @return the number of days in the month
 */
uint8_t furi_hal_rtc_get_days_per_month(bool leap_year, uint8_t month);

#ifdef __cplusplus
}
#endif
