#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_GPS "gps"

typedef struct Gps Gps;

typedef enum {
    GpsStatusOk,
    GpsStatusNotSupported,
    GpsStatusNoPermission,
} GpsStatus;

typedef struct {
    double latitude;
    double longitude;
    float heading;
    float speed;
    float altitude;
    float accuracy;
    uint32_t satellites;
} GpsLocation;

/** Invoked for each location pushed by the companion device, or once with a
 * non-Ok status and NULL location when a request is rejected. */
typedef void (*GpsLocationCallback)(GpsStatus status, const GpsLocation* location, void* context);

/** Request a location stream from the companion device.
 * frequency is in Hz, valid range 1..10. Returns false on invalid frequency
 * or when no companion device is connected. */
bool gps_request_stream(Gps* gps, uint8_t frequency);

/** Stop a running location stream. Returns false when no companion device is
 * connected. */
bool gps_stop_stream(Gps* gps);

/** Request a single current location. Returns false when no companion device
 * is connected. */
bool gps_request_location(Gps* gps);

void gps_set_location_callback(Gps* gps, GpsLocationCallback callback, void* context);

#ifdef __cplusplus
}
#endif
