/**
 * @file gps.h
 * GPS: companion location API
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GPS "gps"

typedef struct Gps Gps;

/** GPS request status */
typedef enum {
    GpsStatusOk, /**< Location data available */
    GpsStatusNotSupported, /**< Location is not supported by companion device */
    GpsStatusNoPermission, /**< Location permission is not granted */
    GpsStatusDisabled, /**< Location services are disabled on companion device */
    GpsStatusUnknown, /**< Unknown location error on companion device */
} GpsStatus;

/** GPS location data */
typedef struct {
    int32_t latitude; /**< Latitude in degrees * 1e7 */
    int32_t longitude; /**< Longitude in degrees * 1e7 */
    uint32_t heading; /**< Heading in degrees * 100, 0..36000 */
    uint32_t speed; /**< Speed in millimeters per second */
    int32_t altitude; /**< Altitude in centimeters */
    uint32_t accuracy; /**< Accuracy in millimeters */
    uint32_t satellites; /**< Satellites count */
} GpsLocation;

/** Get absolute value of signed 32-bit integer
 *
 * @param      value  Signed integer
 *
 * @return     Absolute value
 */
static inline uint32_t gps_location_abs_i32(int32_t value) {
    return (value < 0) ? (uint32_t)(-(value + 1)) + 1 : (uint32_t)value;
}

/** Format signed fixed-point value
 *
 * @param      buffer       Output buffer
 * @param      buffer_size  Output buffer size
 * @param      value        Fixed-point value
 * @param      scale        Fixed-point scale
 * @param      decimals     Decimal digits count
 */
static inline void gps_location_format_fixed_i32(
    char* buffer,
    size_t buffer_size,
    int32_t value,
    uint32_t scale,
    uint8_t decimals) {
    uint32_t abs_value = gps_location_abs_i32(value);
    snprintf(
        buffer,
        buffer_size,
        "%s%lu.%0*lu",
        value < 0 ? "-" : "",
        (unsigned long)(abs_value / scale),
        decimals,
        (unsigned long)(abs_value % scale));
}

/** Format unsigned fixed-point value
 *
 * @param      buffer       Output buffer
 * @param      buffer_size  Output buffer size
 * @param      value        Fixed-point value
 * @param      scale        Fixed-point scale
 * @param      decimals     Decimal digits count
 */
static inline void gps_location_format_fixed_u32(
    char* buffer,
    size_t buffer_size,
    uint32_t value,
    uint32_t scale,
    uint8_t decimals) {
    snprintf(
        buffer,
        buffer_size,
        "%lu.%0*lu",
        (unsigned long)(value / scale),
        decimals,
        (unsigned long)(value % scale));
}

/** Format coordinate
 *
 * @param      buffer       Output buffer
 * @param      buffer_size  Output buffer size
 * @param      degrees_e7   Coordinate in degrees * 1e7
 */
static inline void
    gps_location_format_coordinate(char* buffer, size_t buffer_size, int32_t degrees_e7) {
    gps_location_format_fixed_i32(buffer, buffer_size, degrees_e7, 10000000, 7);
}

/** Format heading
 *
 * @param      buffer         Output buffer
 * @param      buffer_size    Output buffer size
 * @param      degrees_centi  Heading in degrees * 100
 */
static inline void
    gps_location_format_heading(char* buffer, size_t buffer_size, uint32_t degrees_centi) {
    gps_location_format_fixed_u32(buffer, buffer_size, degrees_centi / 10, 10, 1);
}

/** Format speed
 *
 * @param      buffer                   Output buffer
 * @param      buffer_size              Output buffer size
 * @param      millimeters_per_second   Speed in millimeters per second
 */
static inline void gps_location_format_speed(
    char* buffer,
    size_t buffer_size,
    uint32_t millimeters_per_second) {
    gps_location_format_fixed_u32(buffer, buffer_size, millimeters_per_second / 10, 100, 2);
}

/** Format altitude
 *
 * @param      buffer       Output buffer
 * @param      buffer_size  Output buffer size
 * @param      centimeters  Altitude in centimeters
 */
static inline void
    gps_location_format_altitude(char* buffer, size_t buffer_size, int32_t centimeters) {
    gps_location_format_fixed_i32(buffer, buffer_size, centimeters / 10, 10, 1);
}

/** Format accuracy
 *
 * @param      buffer       Output buffer
 * @param      buffer_size  Output buffer size
 * @param      millimeters  Accuracy in millimeters
 */
static inline void
    gps_location_format_accuracy(char* buffer, size_t buffer_size, uint32_t millimeters) {
    gps_location_format_fixed_u32(buffer, buffer_size, millimeters / 100, 10, 1);
}

/** GPS location callback
 *
 * @param      status    GPS request status
 * @param      location  Location data, NULL on error
 * @param      context   Callback context
 */
typedef void (*GpsLocationCallback)(GpsStatus status, const GpsLocation* location, void* context);

/** Request location stream from companion device
 *
 * @param      gps        Gps instance
 * @param      frequency  Stream frequency in Hz, valid range 1..10
 *
 * @return     true on success
 */
bool gps_request_stream(Gps* gps, uint8_t frequency);

/** Stop location stream
 *
 * @param      gps   Gps instance
 *
 * @return     true on success
 */
bool gps_stop_stream(Gps* gps);

/** Request current location from companion device
 *
 * @param      gps   Gps instance
 *
 * @return     true on success
 */
bool gps_request_location(Gps* gps);

/** Report current location to companion device
 *
 * @param      gps       Gps instance
 * @param      location  Location data
 *
 * @return     true on success
 */
bool gps_report_location(Gps* gps, const GpsLocation* location);

/** Set location callback
 *
 * @param      gps       Gps instance
 * @param      callback  GpsLocationCallback
 * @param      context   GpsLocationCallback context
 */
void gps_set_location_callback(Gps* gps, GpsLocationCallback callback, void* context);

#ifdef __cplusplus
}
#endif
