#include "flipper.pb.h"
#include "rpc_i.h"
#include <gps/gps_i.h>

#define TAG "RpcGps"

typedef struct {
    RpcSession* session;
    Gps* gps;
} RpcGps;

static void rpc_gps_send_request(GpsRpcCommand command, uint8_t frequency, void* context) {
    RpcGps* rpc_gps = context;

    PB_Main* message = malloc(sizeof(PB_Main));
    memset(message, 0, sizeof(PB_Main));
    message->command_status = PB_CommandStatus_OK;

    switch(command) {
    case GpsRpcCommandStreamStart:
        message->which_content = PB_Main_gps_stream_start_request_tag;
        message->content.gps_stream_start_request.frequency = frequency;
        break;
    case GpsRpcCommandStreamStop:
        message->which_content = PB_Main_gps_stream_stop_request_tag;
        break;
    case GpsRpcCommandLocation:
        message->which_content = PB_Main_gps_location_request_tag;
        break;
    }

    rpc_send(rpc_gps->session, message);
    free(message);
}

static void rpc_gps_on_location(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_gps_location_tag);

    RpcGps* rpc_gps = context;

    GpsStatus status;
    switch(request->command_status) {
    case PB_CommandStatus_ERROR_GPS_NOT_SUPPORTED:
        status = GpsStatusNotSupported;
        break;
    case PB_CommandStatus_ERROR_GPS_NO_PERMISSION:
        status = GpsStatusNoPermission;
        break;
    default:
        status = GpsStatusOk;
        break;
    }

    if(status != GpsStatusOk) {
        gps_on_location(rpc_gps->gps, status, NULL);
        return;
    }

    const PB_Gps_Location* source = &request->content.gps_location;
    const GpsLocation location = {
        .latitude = source->latitude,
        .longitude = source->longitude,
        .heading = source->heading,
        .speed = source->speed,
        .altitude = source->altitude,
        .accuracy = source->accuracy,
        .satellites = source->satellites,
    };
    gps_on_location(rpc_gps->gps, status, &location);
}

void* rpc_gps_alloc(RpcSession* session) {
    furi_assert(session);

    RpcGps* rpc_gps = malloc(sizeof(RpcGps));
    rpc_gps->session = session;
    rpc_gps->gps = furi_record_open(RECORD_GPS);

    RpcHandler rpc_handler = {
        .message_handler = rpc_gps_on_location,
        .decode_submessage = NULL,
        .context = rpc_gps,
    };
    rpc_add_handler(session, PB_Main_gps_location_tag, &rpc_handler);

    gps_set_rpc_bridge(rpc_gps->gps, rpc_gps_send_request, rpc_gps);

    return rpc_gps;
}

void rpc_gps_free(void* context) {
    furi_assert(context);
    RpcGps* rpc_gps = context;

    gps_set_rpc_bridge(rpc_gps->gps, NULL, NULL);
    furi_record_close(RECORD_GPS);

    rpc_gps->session = NULL;
    free(rpc_gps);
}
