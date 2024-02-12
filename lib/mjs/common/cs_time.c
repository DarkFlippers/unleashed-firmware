/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cs_time.h"

#if CS_ENABLE_STDIO

#ifndef _WIN32
#include <stddef.h>
/*
 * There is no sys/time.h on ARMCC.
 */
#if !(defined(__ARMCC_VERSION) || defined(__ICCARM__)) && !defined(__TI_COMPILER_VERSION__) && \
    (!defined(CS_PLATFORM) || CS_PLATFORM != CS_P_NXP_LPC)
#include <sys/time.h>
#endif
#else
#include <windows.h>
#endif

double cs_time(void) WEAK;
double cs_time(void) {
    double now;
#ifndef _WIN32
    struct timeval tv;
    if(gettimeofday(&tv, NULL /* tz */) != 0) return 0;
    now = (double)tv.tv_sec + (((double)tv.tv_usec) / (double)1000000.0);
#else
    SYSTEMTIME sysnow;
    FILETIME ftime;
    GetLocalTime(&sysnow);
    SystemTimeToFileTime(&sysnow, &ftime);
    /*
   * 1. VC 6.0 doesn't support conversion uint64 -> double, so, using int64
   * This should not cause a problems in this (21th) century
   * 2. Windows FILETIME is a number of 100-nanosecond intervals since January
   * 1, 1601 while time_t is a number of _seconds_ since January 1, 1970 UTC,
   * thus, we need to convert to seconds and adjust amount (subtract 11644473600
   * seconds)
   */
    now = (double)(((int64_t)ftime.dwLowDateTime + ((int64_t)ftime.dwHighDateTime << 32)) /
                   10000000.0) -
          11644473600;
#endif /* _WIN32 */
    return now;
}

double cs_timegm(const struct tm* tm) {
    /* Month-to-day offset for non-leap-years. */
    static const int month_day[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    /* Most of the calculation is easy; leap years are the main difficulty. */
    int month = tm->tm_mon % 12;
    int year = tm->tm_year + tm->tm_mon / 12;
    int year_for_leap;
    int64_t rt;

    if(month < 0) { /* Negative values % 12 are still negative. */
        month += 12;
        --year;
    }

    /* This is the number of Februaries since 1900. */
    year_for_leap = (month > 1) ? year + 1 : year;

    rt = tm->tm_sec /* Seconds */
         + 60 * (tm->tm_min /* Minute = 60 seconds */
                 + 60 * (tm->tm_hour /* Hour = 60 minutes */
                         + 24 * (month_day[month] + tm->tm_mday - 1 /* Day = 24 hours */
                                 + 365 * (year - 70) /* Year = 365 days */
                                 + (year_for_leap - 69) / 4 /* Every 4 years is leap... */
                                 - (year_for_leap - 1) / 100 /* Except centuries... */
                                 + (year_for_leap + 299) / 400))); /* Except 400s. */
    return rt < 0 ? -1 : (double)rt;
}

#endif