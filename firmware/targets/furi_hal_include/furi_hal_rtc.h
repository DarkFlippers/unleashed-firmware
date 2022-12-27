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
    FuriHalRtcFlagFactoryReset = (1 << 1),
    FuriHalRtcFlagLock = (1 << 2),
    FuriHalRtcFlagC2Update = (1 << 3),
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
    FuriHalRtcLocaleUnitsMetric = 0, /**< Metric measurement units */
    FuriHalRtcLocaleUnitsImperial = 1, /**< Imperial measurement units */
} FuriHalRtcLocaleUnits;

typedef enum {
    FuriHalRtcLocaleTimeFormat24h = 0, /**< 24-hour format */
    FuriHalRtcLocaleTimeFormat12h = 1, /**< 12-hour format */
} FuriHalRtcLocaleTimeFormat;

typedef enum {
    FuriHalRtcLocaleDateFormatDMY = 0, /**< Day/Month/Year */
    FuriHalRtcLocaleDateFormatMDY = 1, /**< Month/Day/Year */
    FuriHalRtcLocaleDateFormatYMD = 2, /**< Year/Month/Day */
} FuriHalRtcLocaleDateFormat;

/** Early initialization */
void furi_hal_rtc_init_early();

/** Early deinitialization */
void furi_hal_rtc_deinit_early();

/** Initialize RTC subsystem */
void furi_hal_rtc_init();

uint32_t furi_hal_rtc_get_register(FuriHalRtcRegister reg);

void furi_hal_rtc_set_register(FuriHalRtcRegister reg, uint32_t value);

void furi_hal_rtc_set_log_level(uint8_t level);

uint8_t furi_hal_rtc_get_log_level();

void furi_hal_rtc_set_flag(FuriHalRtcFlag flag);

void furi_hal_rtc_reset_flag(FuriHalRtcFlag flag);

bool furi_hal_rtc_is_flag_set(FuriHalRtcFlag flag);

void furi_hal_rtc_set_boot_mode(FuriHalRtcBootMode mode);

FuriHalRtcBootMode furi_hal_rtc_get_boot_mode();

void furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackMode mode);

FuriHalRtcHeapTrackMode furi_hal_rtc_get_heap_track_mode();

/** Set locale units
 *
 * @param[in]  mode  The RTC Locale Units
 */
void furi_hal_rtc_set_locale_units(FuriHalRtcLocaleUnits value);

/** Get RTC Locale Units
 *
 * @return     The RTC Locale Units.
 */
FuriHalRtcLocaleUnits furi_hal_rtc_get_locale_units();

/** Set RTC Locale Time Format
 *
 * @param[in]  value  The RTC Locale Time Format
 */
void furi_hal_rtc_set_locale_timeformat(FuriHalRtcLocaleTimeFormat value);

/** Get RTC Locale Time Format
 *
 * @return     The RTC Locale Time Format.
 */
FuriHalRtcLocaleTimeFormat furi_hal_rtc_get_locale_timeformat();

/** Set RTC Locale Date Format
 *
 * @param[in]  value  The RTC Locale Date Format
 */
void furi_hal_rtc_set_locale_dateformat(FuriHalRtcLocaleDateFormat value);

/** Get RTC Locale Date Format
 *
 * @return     The RTC Locale Date Format
 */
FuriHalRtcLocaleDateFormat furi_hal_rtc_get_locale_dateformat();

/** Set RTC Date Time
 *
 * @param      datetime  The date time to set
 */

void furi_hal_rtc_set_datetime(FuriHalRtcDateTime* datetime);

void furi_hal_rtc_get_datetime(FuriHalRtcDateTime* datetime);

bool furi_hal_rtc_validate_datetime(FuriHalRtcDateTime* datetime);

void furi_hal_rtc_set_fault_data(uint32_t value);

uint32_t furi_hal_rtc_get_fault_data();

void furi_hal_rtc_set_pin_fails(uint32_t value);

uint32_t furi_hal_rtc_get_pin_fails();

uint32_t furi_hal_rtc_get_timestamp();

uint32_t furi_hal_rtc_datetime_to_timestamp(FuriHalRtcDateTime* datetime);

#ifdef __cplusplus
}
#endif
