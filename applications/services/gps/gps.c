#include "gps_i.h"

#include <furi.h>

struct Gps {
    FuriMutex* mutex;

    GpsRpcSend rpc_send;
    void* rpc_send_context;

    GpsLocationCallback location_callback;
    void* location_context;
};

static Gps* gps_alloc(void) {
    Gps* gps = malloc(sizeof(Gps));
    gps->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    gps->rpc_send = NULL;
    gps->rpc_send_context = NULL;
    gps->location_callback = NULL;
    gps->location_context = NULL;
    return gps;
}

static bool
    gps_send(Gps* gps, GpsRpcCommand command, uint8_t frequency, const GpsLocation* location) {
    furi_mutex_acquire(gps->mutex, FuriWaitForever);
    bool sent = false;
    if(gps->rpc_send) {
        gps->rpc_send(command, frequency, location, gps->rpc_send_context);
        sent = true;
    }
    furi_mutex_release(gps->mutex);
    return sent;
}

bool gps_request_stream(Gps* gps, uint8_t frequency) {
    furi_check(gps);
    if(frequency < 1 || frequency > 10) return false;
    return gps_send(gps, GpsRpcCommandStreamStart, frequency, NULL);
}

bool gps_stop_stream(Gps* gps) {
    furi_check(gps);
    return gps_send(gps, GpsRpcCommandStreamStop, 0, NULL);
}

bool gps_request_location(Gps* gps) {
    furi_check(gps);
    return gps_send(gps, GpsRpcCommandLocation, 0, NULL);
}

bool gps_report_location(Gps* gps, const GpsLocation* location) {
    furi_check(gps);
    furi_check(location);
    return gps_send(gps, GpsRpcCommandSendLocation, 0, location);
}

void gps_set_location_callback(Gps* gps, GpsLocationCallback callback, void* context) {
    furi_check(gps);
    furi_mutex_acquire(gps->mutex, FuriWaitForever);
    gps->location_callback = callback;
    gps->location_context = context;
    furi_mutex_release(gps->mutex);
}

void gps_set_rpc_bridge(Gps* gps, GpsRpcSend send, void* context) {
    furi_check(gps);
    furi_mutex_acquire(gps->mutex, FuriWaitForever);
    gps->rpc_send = send;
    gps->rpc_send_context = context;
    furi_mutex_release(gps->mutex);
}

void gps_on_location(Gps* gps, GpsStatus status, const GpsLocation* location) {
    furi_check(gps);
    furi_mutex_acquire(gps->mutex, FuriWaitForever);
    GpsLocationCallback callback = gps->location_callback;
    void* context = gps->location_context;
    furi_mutex_release(gps->mutex);
    if(callback) {
        callback(status, location, context);
    }
}

void gps_on_system_start(void* p) {
    UNUSED(p);
    furi_record_create(RECORD_GPS, gps_alloc());
}
