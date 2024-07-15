#include "datetime.h"
#include <furi.h>

#define TAG "DateTime"

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR   (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY    (SECONDS_PER_HOUR * 24)
#define MONTHS_COUNT       12
#define EPOCH_START_YEAR   1970

static const uint8_t datetime_days_per_month[2][MONTHS_COUNT] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const uint16_t datetime_days_per_year[] = {365, 366};

bool datetime_validate_datetime(DateTime* datetime) {
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

uint32_t datetime_datetime_to_timestamp(DateTime* datetime) {
    furi_check(datetime);

    uint32_t timestamp = 0;
    uint8_t years = 0;
    uint8_t leap_years = 0;

    for(uint16_t y = EPOCH_START_YEAR; y < datetime->year; y++) {
        if(datetime_is_leap_year(y)) {
            leap_years++;
        } else {
            years++;
        }
    }

    timestamp += ((years * datetime_days_per_year[0]) + (leap_years * datetime_days_per_year[1])) *
                 SECONDS_PER_DAY;

    bool leap_year = datetime_is_leap_year(datetime->year);

    for(uint8_t m = 1; m < datetime->month; m++) {
        timestamp += datetime_get_days_per_month(leap_year, m) * SECONDS_PER_DAY;
    }

    timestamp += (datetime->day - 1) * SECONDS_PER_DAY;
    timestamp += datetime->hour * SECONDS_PER_HOUR;
    timestamp += datetime->minute * SECONDS_PER_MINUTE;
    timestamp += datetime->second;

    return timestamp;
}

void datetime_timestamp_to_datetime(uint32_t timestamp, DateTime* datetime) {
    furi_check(datetime);
    uint32_t days = timestamp / SECONDS_PER_DAY;
    uint32_t seconds_in_day = timestamp % SECONDS_PER_DAY;

    datetime->year = EPOCH_START_YEAR;
    datetime->weekday = ((days + 3) % 7) + 1;

    while(days >= datetime_get_days_per_year(datetime->year)) {
        days -= datetime_get_days_per_year(datetime->year);
        (datetime->year)++;
    }

    datetime->month = 1;
    while(days >=
          datetime_get_days_per_month(datetime_is_leap_year(datetime->year), datetime->month)) {
        days -=
            datetime_get_days_per_month(datetime_is_leap_year(datetime->year), datetime->month);
        (datetime->month)++;
    }

    datetime->day = days + 1;
    datetime->hour = seconds_in_day / SECONDS_PER_HOUR;
    datetime->minute = (seconds_in_day % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
    datetime->second = seconds_in_day % SECONDS_PER_MINUTE;
}

uint16_t datetime_get_days_per_year(uint16_t year) {
    return datetime_days_per_year[datetime_is_leap_year(year) ? 1 : 0];
}

bool datetime_is_leap_year(uint16_t year) {
    return (((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0);
}

uint8_t datetime_get_days_per_month(bool leap_year, uint8_t month) {
    return datetime_days_per_month[leap_year ? 1 : 0][month - 1];
}
