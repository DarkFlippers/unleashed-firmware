#pragma once

#include <stdbool.h>
#include <furi.h>
#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LocaleMeasurementUnitsMetric = 0, /**< Metric measurement units */
    LocaleMeasurementUnitsImperial = 1, /**< Imperial measurement units */
} LocaleMeasurementUnits;

typedef enum {
    LocaleTimeFormat24h = 0, /**< 24-hour format */
    LocaleTimeFormat12h = 1, /**< 12-hour format */
} LocaleTimeFormat;

typedef enum {
    LocaleDateFormatDMY = 0, /**< Day/Month/Year */
    LocaleDateFormatMDY = 1, /**< Month/Day/Year */
    LocaleDateFormatYMD = 2, /**< Year/Month/Day */
} LocaleDateFormat;

/** Get Locale measurement units
 *
 * @return     The locale measurement units.
 */
LocaleMeasurementUnits locale_get_measurement_unit(void);

/** Set locale measurement units
 *
 * @param[in]  format  The locale measurements units
 */
void locale_set_measurement_unit(LocaleMeasurementUnits format);

/** Convert Fahrenheit to Celsius
 *
 * @param[in]  temp_f  The Temperature in Fahrenheit
 *
 * @return     The Temperature in Celsius 
 */
float locale_fahrenheit_to_celsius(float temp_f);

/** Convert Celsius to Fahrenheit
 *
 * @param[in]  temp_c  The Temperature in Celsius 
 *
 * @return     The Temperature in Fahrenheit
 */
float locale_celsius_to_fahrenheit(float temp_c);

/** Get Locale time format
 *
 * @return     The locale time format.
 */
LocaleTimeFormat locale_get_time_format(void);

/** Set Locale Time Format
 *
 * @param[in]  format  The Locale Time Format
 */
void locale_set_time_format(LocaleTimeFormat format);

/** Format time to furi string
 *
 * @param[out] out_str       The FuriString to store formatted time
 * @param[in]  datetime      Pointer to the datetime
 * @param[in]  format        The Locale Time Format
 * @param[in]  show_seconds  The show seconds flag
 */
void locale_format_time(
    FuriString* out_str,
    const DateTime* datetime,
    const LocaleTimeFormat format,
    const bool show_seconds);

/** Get Locale DateFormat
 *
 * @return     The Locale DateFormat.
 */
LocaleDateFormat locale_get_date_format(void);

/** Set Locale DateFormat
 *
 * @param[in]  format  The Locale DateFormat
 */
void locale_set_date_format(LocaleDateFormat format);

/** Format date to furi string
 *
 * @param[out] out_str    The FuriString to store formatted date
 * @param[in]  datetime   Pointer to the datetime
 * @param[in]  format     The format
 * @param[in]  separator  The separator
 */
void locale_format_date(
    FuriString* out_str,
    const DateTime* datetime,
    const LocaleDateFormat format,
    const char* separator);

#ifdef __cplusplus
}
#endif
