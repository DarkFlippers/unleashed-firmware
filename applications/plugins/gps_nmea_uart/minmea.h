/*
 * Copyright © 2014 Kosma Moczek <kosma@cloudyourcar.com>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See the COPYING file for more details.
 */

#ifndef MINMEA_H
#define MINMEA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#ifdef MINMEA_INCLUDE_COMPAT
#include <minmea_compat.h>
#endif

#ifndef MINMEA_MAX_SENTENCE_LENGTH
#define MINMEA_MAX_SENTENCE_LENGTH 80
#endif

enum minmea_sentence_id {
    MINMEA_INVALID = -1,
    MINMEA_UNKNOWN = 0,
    MINMEA_SENTENCE_GBS,
    MINMEA_SENTENCE_GGA,
    MINMEA_SENTENCE_GLL,
    MINMEA_SENTENCE_GSA,
    MINMEA_SENTENCE_GST,
    MINMEA_SENTENCE_GSV,
    MINMEA_SENTENCE_RMC,
    MINMEA_SENTENCE_VTG,
    MINMEA_SENTENCE_ZDA,
};

struct minmea_float {
    int_least32_t value;
    int_least32_t scale;
};

struct minmea_date {
    int day;
    int month;
    int year;
};

struct minmea_time {
    int hours;
    int minutes;
    int seconds;
    int microseconds;
};

struct minmea_sentence_gbs {
    struct minmea_time time;
    struct minmea_float err_latitude;
    struct minmea_float err_longitude;
    struct minmea_float err_altitude;
    int svid;
    struct minmea_float prob;
    struct minmea_float bias;
    struct minmea_float stddev;
};

struct minmea_sentence_rmc {
    struct minmea_time time;
    bool valid;
    struct minmea_float latitude;
    struct minmea_float longitude;
    struct minmea_float speed;
    struct minmea_float course;
    struct minmea_date date;
    struct minmea_float variation;
};

struct minmea_sentence_gga {
    struct minmea_time time;
    struct minmea_float latitude;
    struct minmea_float longitude;
    int fix_quality;
    int satellites_tracked;
    struct minmea_float hdop;
    struct minmea_float altitude;
    char altitude_units;
    struct minmea_float height;
    char height_units;
    struct minmea_float dgps_age;
};

enum minmea_gll_status {
    MINMEA_GLL_STATUS_DATA_VALID = 'A',
    MINMEA_GLL_STATUS_DATA_NOT_VALID = 'V',
};

// FAA mode added to some fields in NMEA 2.3.
enum minmea_faa_mode {
    MINMEA_FAA_MODE_AUTONOMOUS = 'A',
    MINMEA_FAA_MODE_DIFFERENTIAL = 'D',
    MINMEA_FAA_MODE_ESTIMATED = 'E',
    MINMEA_FAA_MODE_MANUAL = 'M',
    MINMEA_FAA_MODE_SIMULATED = 'S',
    MINMEA_FAA_MODE_NOT_VALID = 'N',
    MINMEA_FAA_MODE_PRECISE = 'P',
};

struct minmea_sentence_gll {
    struct minmea_float latitude;
    struct minmea_float longitude;
    struct minmea_time time;
    char status;
    char mode;
};

struct minmea_sentence_gst {
    struct minmea_time time;
    struct minmea_float rms_deviation;
    struct minmea_float semi_major_deviation;
    struct minmea_float semi_minor_deviation;
    struct minmea_float semi_major_orientation;
    struct minmea_float latitude_error_deviation;
    struct minmea_float longitude_error_deviation;
    struct minmea_float altitude_error_deviation;
};

enum minmea_gsa_mode {
    MINMEA_GPGSA_MODE_AUTO = 'A',
    MINMEA_GPGSA_MODE_FORCED = 'M',
};

enum minmea_gsa_fix_type {
    MINMEA_GPGSA_FIX_NONE = 1,
    MINMEA_GPGSA_FIX_2D = 2,
    MINMEA_GPGSA_FIX_3D = 3,
};

struct minmea_sentence_gsa {
    char mode;
    int fix_type;
    int sats[12];
    struct minmea_float pdop;
    struct minmea_float hdop;
    struct minmea_float vdop;
};

struct minmea_sat_info {
    int nr;
    int elevation;
    int azimuth;
    int snr;
};

struct minmea_sentence_gsv {
    int total_msgs;
    int msg_nr;
    int total_sats;
    struct minmea_sat_info sats[4];
};

struct minmea_sentence_vtg {
    struct minmea_float true_track_degrees;
    struct minmea_float magnetic_track_degrees;
    struct minmea_float speed_knots;
    struct minmea_float speed_kph;
    enum minmea_faa_mode faa_mode;
};

struct minmea_sentence_zda {
    struct minmea_time time;
    struct minmea_date date;
    int hour_offset;
    int minute_offset;
};

/**
 * Calculate raw sentence checksum. Does not check sentence integrity.
 */
uint8_t minmea_checksum(const char* sentence);

/**
 * Check sentence validity and checksum. Returns true for valid sentences.
 */
bool minmea_check(const char* sentence, bool strict);

/**
 * Determine talker identifier.
 */
bool minmea_talker_id(char talker[3], const char* sentence);

/**
 * Determine sentence identifier.
 */
enum minmea_sentence_id minmea_sentence_id(const char* sentence, bool strict);

/**
 * Scanf-like processor for NMEA sentences. Supports the following formats:
 * c - single character (char *)
 * d - direction, returned as 1/-1, default 0 (int *)
 * f - fractional, returned as value + scale (struct minmea_float *)
 * i - decimal, default zero (int *)
 * s - string (char *)
 * t - talker identifier and type (char *)
 * D - date (struct minmea_date *)
 * T - time stamp (struct minmea_time *)
 * _ - ignore this field
 * ; - following fields are optional
 * Returns true on success. See library source code for details.
 */
bool minmea_scan(const char* sentence, const char* format, ...);

/*
 * Parse a specific type of sentence. Return true on success.
 */
bool minmea_parse_gbs(struct minmea_sentence_gbs* frame, const char* sentence);
bool minmea_parse_rmc(struct minmea_sentence_rmc* frame, const char* sentence);
bool minmea_parse_gga(struct minmea_sentence_gga* frame, const char* sentence);
bool minmea_parse_gsa(struct minmea_sentence_gsa* frame, const char* sentence);
bool minmea_parse_gll(struct minmea_sentence_gll* frame, const char* sentence);
bool minmea_parse_gst(struct minmea_sentence_gst* frame, const char* sentence);
bool minmea_parse_gsv(struct minmea_sentence_gsv* frame, const char* sentence);
bool minmea_parse_vtg(struct minmea_sentence_vtg* frame, const char* sentence);
bool minmea_parse_zda(struct minmea_sentence_zda* frame, const char* sentence);

/**
 * Convert GPS UTC date/time representation to a UNIX calendar time.
 */
int minmea_getdatetime(
    struct tm* tm,
    const struct minmea_date* date,
    const struct minmea_time* time_);

/**
 * Convert GPS UTC date/time representation to a UNIX timestamp.
 */
int minmea_gettime(
    struct timespec* ts,
    const struct minmea_date* date,
    const struct minmea_time* time_);

/**
 * Rescale a fixed-point value to a different scale. Rounds towards zero.
 */
static inline int_least32_t minmea_rescale(const struct minmea_float* f, int_least32_t new_scale) {
    if(f->scale == 0) return 0;
    if(f->scale == new_scale) return f->value;
    if(f->scale > new_scale)
        return (f->value + ((f->value > 0) - (f->value < 0)) * f->scale / new_scale / 2) /
               (f->scale / new_scale);
    else
        return f->value * (new_scale / f->scale);
}

/**
 * Convert a fixed-point value to a floating-point value.
 * Returns NaN for "unknown" values.
 */
static inline float minmea_tofloat(const struct minmea_float* f) {
    if(f->scale == 0) return NAN;
    return (float)f->value / (float)f->scale;
}

/**
 * Convert a raw coordinate to a floating point DD.DDD... value.
 * Returns NaN for "unknown" values.
 */
static inline float minmea_tocoord(const struct minmea_float* f) {
    if(f->scale == 0) return NAN;
    if(f->scale > (INT_LEAST32_MAX / 100)) return NAN;
    if(f->scale < (INT_LEAST32_MIN / 100)) return NAN;
    int_least32_t degrees = f->value / (f->scale * 100);
    int_least32_t minutes = f->value % (f->scale * 100);
    return (float)degrees + (float)minutes / (60 * f->scale);
}

/**
 * Check whether a character belongs to the set of characters allowed in a
 * sentence data field.
 */
static inline bool minmea_isfield(char c) {
    return isprint((unsigned char)c) && c != ',' && c != '*';
}

#ifdef __cplusplus
}
#endif

#endif /* MINMEA_H */

/* vim: set ts=4 sw=4 et: */
