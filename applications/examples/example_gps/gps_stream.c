#include <furi.h>
#include <gui/gui.h>
#include <gps/gps.h>
#include <notification/notification_messages.h>

/*
 * Demonstration app for the firmware GPS service.
 *
 * The service is exposed through RECORD_GPS and receives location data from the
 * companion over RPC. The application intentionally keeps the flow explicit:
 *
 * 1. Open RECORD_GPS.
 * 2. Register a GpsLocationCallback.
 * 3. Request a location stream with gps_request_stream() until it succeeds,
 *    then keep the stream open without re-requesting it.
 * 4. Render the latest GpsLocation or service status; if no packets arrive
 *    for GPS_STREAM_TIMEOUT_MS, reset and wait for the connection again.
 * 5. Stop the stream and unregister the callback before exit.
 *
 * LED feedback: red blink on every failed stream open attempt, green blink on
 * every received location packet. OK toggles the display backlight.
 *
 * This is sample code for the GPS service API, not a full navigation app.
 */

#define GPS_STREAM_FREQUENCY  4
#define GPS_POLL_PERIOD_MS    1000
#define GPS_STREAM_TIMEOUT_MS 5000

typedef struct {
    FuriMutex* mutex;
    NotificationApp* notifications;
    bool connected;
    GpsStatus status;
    bool has_fix;
    GpsLocation location;
    uint32_t last_location_tick;
} GpsView;

/*
 * Reset only the demo app state. This does not talk to the GPS service.
 * Call gps_stop_stream() separately first when a live stream must be closed.
 */
static void gps_view_reset_locked(GpsView* gps_view) {
    gps_view->connected = false;
    gps_view->status = GpsStatusOk;
    gps_view->has_fix = false;
    gps_view->location = (GpsLocation){0};
    gps_view->last_location_tick = 0;
}

/*
 * GPS service callback.
 *
 * status describes the companion-side GPS result:
 * - GpsStatusOk with a non-NULL location means a valid fix/update arrived;
 * - GpsStatusNotSupported means the companion has no GPS provider;
 * - GpsStatusNoPermission means the companion denied location access;
 * - GpsStatusDisabled means the companion's location services are off;
 * - GpsStatusUnknown means the companion hit an undetermined location error.
 */
static void gps_location_callback(GpsStatus status, const GpsLocation* location, void* context) {
    GpsView* gps_view = context;
    furi_mutex_acquire(gps_view->mutex, FuriWaitForever);
    gps_view->connected = true;
    gps_view->status = status;
    gps_view->last_location_tick = furi_get_tick();
    bool got_fix = status == GpsStatusOk && location;
    if(got_fix) {
        gps_view->location = *location;
        gps_view->has_fix = true;
    } else {
        gps_view->has_fix = false;
        gps_view->location = (GpsLocation){0};
    }
    furi_mutex_release(gps_view->mutex);

    if(got_fix) {
        notification_message(gps_view->notifications, &sequence_blink_green_10);
    }
}

static void render_callback(Canvas* canvas, void* context) {
    GpsView* gps_view = context;
    furi_mutex_acquire(gps_view->mutex, FuriWaitForever);

    char buffer[64];

    if(!gps_view->connected) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "No USB/BLE connection");
    } else if(gps_view->status == GpsStatusNotSupported) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "GPS not available");
    } else if(gps_view->status == GpsStatusNoPermission) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "Permission denied");
    } else if(gps_view->status == GpsStatusDisabled) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "Location disabled");
    } else if(gps_view->status == GpsStatusUnknown) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "Location error");
    } else if(!gps_view->has_fix) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "Waiting for data...");
    } else {
        const GpsLocation* location = &gps_view->location;

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 32, 8, AlignCenter, AlignBottom, "Latitude");
        canvas_draw_str_aligned(canvas, 96, 8, AlignCenter, AlignBottom, "Longitude");
        canvas_draw_str_aligned(canvas, 21, 30, AlignCenter, AlignBottom, "Course");
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, "Speed");
        canvas_draw_str_aligned(canvas, 107, 30, AlignCenter, AlignBottom, "Altitude");
        canvas_draw_str_aligned(canvas, 32, 52, AlignCenter, AlignBottom, "Satellites");
        canvas_draw_str_aligned(canvas, 96, 52, AlignCenter, AlignBottom, "Accuracy");

        canvas_set_font(canvas, FontSecondary);

        // coordinate
        gps_location_format_coordinate(buffer, sizeof(buffer), location->latitude);
        canvas_draw_str_aligned(canvas, 32, 18, AlignCenter, AlignBottom, buffer);
        gps_location_format_coordinate(buffer, sizeof(buffer), location->longitude);
        canvas_draw_str_aligned(canvas, 96, 18, AlignCenter, AlignBottom, buffer);

        // heading
        gps_location_format_heading(buffer, sizeof(buffer), location->heading);
        canvas_draw_str_aligned(canvas, 21, 40, AlignCenter, AlignBottom, buffer);

        // speed
        gps_location_format_speed(buffer, sizeof(buffer), location->speed);
        strlcat(buffer, " m/s", sizeof(buffer));
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignBottom, buffer);

        // altitude
        gps_location_format_altitude(buffer, sizeof(buffer), location->altitude);
        strlcat(buffer, " m", sizeof(buffer));
        canvas_draw_str_aligned(canvas, 107, 40, AlignCenter, AlignBottom, buffer);

        // satellites
        snprintf(buffer, sizeof(buffer), "%lu", location->satellites);
        canvas_draw_str_aligned(canvas, 32, 62, AlignCenter, AlignBottom, buffer);

        // accuracy
        gps_location_format_accuracy(buffer, sizeof(buffer), location->accuracy);
        strlcat(buffer, " m", sizeof(buffer));
        canvas_draw_str_aligned(canvas, 96, 62, AlignCenter, AlignBottom, buffer);
    }

    furi_mutex_release(gps_view->mutex);
}

static void input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t gps_stream_app(void* p) {
    UNUSED(p);

    GpsView* gps_view = malloc(sizeof(GpsView));
    gps_view->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    gps_view_reset_locked(gps_view);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    /*
     * RECORD_GPS is the public firmware service handle. Applications do not
     * parse RPC directly; they use this service API and receive GpsLocation.
     */
    Gps* gps = furi_record_open(RECORD_GPS);
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    gps_view->notifications = notifications;
    gps_set_location_callback(gps, gps_location_callback, gps_view);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, gps_view);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    uint32_t next_poll = 0;
    bool stream_active = false;
    uint32_t stream_start_tick = 0;
    bool backlight_on = true;
    notification_message(notifications, &sequence_display_backlight_enforce_on);

    for(bool processing = true; processing;) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyBack) {
                processing = false;
            } else if(event.type == InputTypeShort && event.key == InputKeyOk) {
                backlight_on = !backlight_on;
                if(backlight_on) {
                    notification_message(notifications, &sequence_display_backlight_enforce_on);
                } else {
                    notification_message(notifications, &sequence_display_backlight_enforce_auto);
                    notification_message(notifications, &sequence_display_backlight_off);
                }
            }
        }

        if(furi_get_tick() >= next_poll) {
            uint32_t now = furi_get_tick();
            next_poll = now + furi_ms_to_ticks(GPS_POLL_PERIOD_MS);

            if(!stream_active) {
                /*
                 * gps_request_stream() asks the companion to stream location
                 * data. Its return value means the command was sent through a
                 * live GPS bridge, not that a fresh fix has already arrived.
                 * Once it succeeds the stream stays open; it is not
                 * re-requested until a timeout resets the state below.
                 */
                if(gps_request_stream(gps, GPS_STREAM_FREQUENCY)) {
                    stream_active = true;
                    stream_start_tick = now;
                } else {
                    notification_message(notifications, &sequence_blink_red_10);
                    furi_mutex_acquire(gps_view->mutex, FuriWaitForever);
                    gps_view_reset_locked(gps_view);
                    furi_mutex_release(gps_view->mutex);
                }
            } else {
                furi_mutex_acquire(gps_view->mutex, FuriWaitForever);
                uint32_t last_packet_tick = gps_view->last_location_tick ?
                                                gps_view->last_location_tick :
                                                stream_start_tick;
                furi_mutex_release(gps_view->mutex);

                /*
                 * No packets for GPS_STREAM_TIMEOUT_MS: close the stale
                 * stream, drop to the disconnected state and wait for the
                 * bridge again on the next poll cycles.
                 */
                if(now - last_packet_tick > furi_ms_to_ticks(GPS_STREAM_TIMEOUT_MS)) {
                    gps_stop_stream(gps);
                    stream_active = false;
                    furi_mutex_acquire(gps_view->mutex, FuriWaitForever);
                    gps_view_reset_locked(gps_view);
                    furi_mutex_release(gps_view->mutex);
                }
            }
        }

        view_port_update(view_port);
    }

    /*
     * Shut down in reverse order: stop the companion stream, detach the service
     * callback, then close RECORD_GPS.
     */
    gps_stop_stream(gps);
    gps_set_location_callback(gps, NULL, NULL);
    furi_record_close(RECORD_GPS);

    notification_message(notifications, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(gps_view->mutex);
    free(gps_view);

    return 0;
}
