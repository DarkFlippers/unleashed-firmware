#include "gps_i.h"

#include <furi.h>

struct Gps {
    // Guards the RPC bridge, and is held while a request goes out through it
    FuriMutex* mutex;

    GpsRpcSend rpc_send;
    void* rpc_send_context;

    // Guards the location callback, and is held for as long as the callback runs. That is
    // what lets gps_set_location_callback() promise the old callback is not running any
    // more: an app frees the callback's context right after clearing it. Not the same mutex
    // as the bridge's, because a callback is free to call gps_request_location() and the
    // like, and an app that does so under a lock of its own must not find that lock's
    // holder waiting here. Recursive, so a callback can replace or clear itself
    FuriMutex* callback_mutex;

    GpsLocationCallback location_callback;
    void* location_context;
};

static Gps* gps_alloc(void) {
    Gps* gps = malloc(sizeof(Gps));
    gps->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    gps->callback_mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
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
    // Waits here for a location that is being delivered: it still has the old callback and
    // context in hand
    furi_mutex_acquire(gps->callback_mutex, FuriWaitForever);
    gps->location_callback = callback;
    gps->location_context = context;
    furi_mutex_release(gps->callback_mutex);
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
    // Held across the call, not just across reading the pointers: released before it, the
    // app could clear the callback and free the context while the call was still running
    furi_mutex_acquire(gps->callback_mutex, FuriWaitForever);
    if(gps->location_callback) {
        gps->location_callback(status, location, gps->location_context);
    }
    furi_mutex_release(gps->callback_mutex);
}

void gps_on_system_start(void* p) {
    UNUSED(p);
    furi_record_create(RECORD_GPS, gps_alloc());
}
