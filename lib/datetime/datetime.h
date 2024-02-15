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
} DateTime;

/** Validate Date Time
 *
 * @param      datetime  The datetime to validate
 *
 * @return     { description_of_the_return_value }
 */
bool datetime_validate_datetime(DateTime* datetime);

/** Convert DateTime to UNIX timestamp
 * 
 * @warning    Mind timezone when perform conversion
 *
 * @param      datetime  The datetime (UTC)
 *
 * @return     UNIX Timestamp in seconds from UNIX epoch start
 */
uint32_t datetime_datetime_to_timestamp(DateTime* datetime);

/** Convert UNIX timestamp to DateTime
 *
 * @warning    Mind timezone when perform conversion
 *
 * @param[in]  timestamp  UNIX Timestamp in seconds from UNIX epoch start
 * @param[out] datetime   The datetime (UTC)
 */
void datetime_timestamp_to_datetime(uint32_t timestamp, DateTime* datetime);

/** Gets the number of days in the year according to the Gregorian calendar.
 *
 * @param year Input year.
 *
 * @return number of days in `year`.
 */
uint16_t datetime_get_days_per_year(uint16_t year);

/** Check if a year a leap year in the Gregorian calendar.
 *
 * @param year Input year.
 *
 * @return true if `year` is a leap year.
 */
bool datetime_is_leap_year(uint16_t year);

/** Get the number of days in the month.
 *
 * @param leap_year true to calculate based on leap years
 * @param month month to check, where 1 = January
 * @return the number of days in the month
 */
uint8_t datetime_get_days_per_month(bool leap_year, uint8_t month);

#ifdef __cplusplus
}
#endif
