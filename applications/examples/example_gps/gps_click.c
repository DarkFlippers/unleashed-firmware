#include <furi.h>
#include <gui/gui.h>
#include <gps/gps.h>
#include <notification/notification_messages.h>

/*
 * Demonstration app for the firmware GPS service: one-shot location request.
 *
 * The service is exposed through RECORD_GPS and receives location data from the
 * companion over RPC. Unlike gps_stream.c, this app never opens a stream; it
 * asks for a single fix each time the user presses OK:
 *
 * 1. Open RECORD_GPS.
 * 2. Register a GpsLocationCallback.
 * 3. On OK press, send one request with gps_request_location().
 * 4. Render the received GpsLocation or service status; if no reply arrives
 *    for GPS_REQUEST_TIMEOUT_MS, report it and let the user retry.
 * 5. Unregister the callback before exit.
 *
 * LED feedback: red blink when a request cannot be sent or times out, green
 * blink when a location fix arrives.
 *
 * This is sample code for the GPS service API, not a full navigation app.
 */

#define GPS_REQUEST_TIMEOUT_MS 10000

typedef enum {
    GpsClickStateIdle, /**< No request yet, waiting for OK */
    GpsClickStateWaiting, /**< Request sent, waiting for the companion reply */
    GpsClickStateSendFailed, /**< Request could not be sent (no GPS bridge) */
    GpsClickStateTimeout, /**< No reply within GPS_REQUEST_TIMEOUT_MS */
    GpsClickStateResult, /**< Reply received, status/location are valid */
} GpsClickState;

typedef struct {
    FuriMutex* mutex;
    NotificationApp* notifications;
    GpsClickState state;
    uint32_t request_tick;
    GpsStatus status;
    bool has_fix;
    GpsLocation location;
} GpsClickApp;

/*
 * GPS service callback.
 *
 * status describes the companion-side GPS result:
 * - GpsStatusOk with a non-NULL location means a valid fix arrived;
 * - GpsStatusNotSupported means the companion has no GPS provider;
 * - GpsStatusNoPermission means the companion denied location access;
 * - GpsStatusDisabled means the companion's location services are off;
 * - GpsStatusUnknown means the companion hit an undetermined location error.
 */
static void gps_location_callback(GpsStatus status, const GpsLocation* location, void* context) {
    GpsClickApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->state = GpsClickStateResult;
    app->status = status;
    bool got_fix = status == GpsStatusOk && location;
    if(got_fix) {
        app->location = *location;
        app->has_fix = true;
    } else {
        app->has_fix = false;
        app->location = (GpsLocation){0};
    }
    furi_mutex_release(app->mutex);

    if(got_fix) {
        notification_message(app->notifications, &sequence_blink_green_10);
    }
}

static void render_message(Canvas* canvas, const char* primary, const char* secondary) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, primary);
    if(secondary) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignBottom, secondary);
    }
}

static void render_callback(Canvas* canvas, void* context) {
    GpsClickApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    char buffer[64];

    if(app->state == GpsClickStateIdle) {
        render_message(canvas, "GPS one-shot request", "Press OK to get location");
    } else if(app->state == GpsClickStateWaiting) {
        render_message(canvas, "Requesting...", NULL);
    } else if(app->state == GpsClickStateSendFailed) {
        render_message(canvas, "No USB/BLE connection", "Press OK to retry");
    } else if(app->state == GpsClickStateTimeout) {
        render_message(canvas, "No response", "Press OK to retry");
    } else if(app->status == GpsStatusNotSupported) {
        render_message(canvas, "GPS not available", "Press OK to retry");
    } else if(app->status == GpsStatusNoPermission) {
        render_message(canvas, "Permission denied", "Press OK to retry");
    } else if(app->status == GpsStatusDisabled) {
        render_message(canvas, "Location disabled", "Press OK to retry");
    } else if(app->status == GpsStatusUnknown) {
        render_message(canvas, "Location error", "Press OK to retry");
    } else if(!app->has_fix) {
        render_message(canvas, "No location data", "Press OK to retry");
    } else {
        const GpsLocation* location = &app->location;

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

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t gps_click_app(void* p) {
    UNUSED(p);

    GpsClickApp* app = malloc(sizeof(GpsClickApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->state = GpsClickStateIdle;
    app->status = GpsStatusOk;
    app->has_fix = false;
    app->location = (GpsLocation){0};
    app->request_tick = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    /*
     * RECORD_GPS is the public firmware service handle. Applications do not
     * parse RPC directly; they use this service API and receive GpsLocation.
     */
    Gps* gps = furi_record_open(RECORD_GPS);
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    app->notifications = notifications;
    gps_set_location_callback(gps, gps_location_callback, app);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, app);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    notification_message(notifications, &sequence_display_backlight_enforce_on);

    InputEvent event;
    for(bool processing = true; processing;) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyBack) {
                processing = false;
            } else if(event.type == InputTypeShort && event.key == InputKeyOk) {
                furi_mutex_acquire(app->mutex, FuriWaitForever);
                bool waiting = app->state == GpsClickStateWaiting;
                furi_mutex_release(app->mutex);

                /*
                 * gps_request_location() asks the companion for a single fix.
                 * Its return value means the command was sent through a live
                 * GPS bridge, not that the fix has already arrived; the reply
                 * comes through the location callback. Ignore OK while a
                 * request is still in flight.
                 */
                if(!waiting) {
                    bool sent = gps_request_location(gps);
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    if(sent) {
                        app->state = GpsClickStateWaiting;
                        app->request_tick = furi_get_tick();
                    } else {
                        app->state = GpsClickStateSendFailed;
                    }
                    app->has_fix = false;
                    app->location = (GpsLocation){0};
                    furi_mutex_release(app->mutex);
                    if(!sent) {
                        notification_message(notifications, &sequence_blink_red_10);
                    }
                }
            }
        }

        /*
         * No reply for GPS_REQUEST_TIMEOUT_MS: report the timeout and let the
         * user retry with OK. A late reply after this point is still accepted
         * by the callback and simply replaces the timeout screen.
         */
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        bool timed_out = app->state == GpsClickStateWaiting &&
                         furi_get_tick() - app->request_tick >
                             furi_ms_to_ticks(GPS_REQUEST_TIMEOUT_MS);
        if(timed_out) {
            app->state = GpsClickStateTimeout;
        }
        furi_mutex_release(app->mutex);
        if(timed_out) {
            notification_message(notifications, &sequence_blink_red_10);
        }

        view_port_update(view_port);
    }

    /*
     * Shut down in reverse order: detach the service callback, then close
     * RECORD_GPS. There is no stream to stop for one-shot requests.
     */
    gps_set_location_callback(gps, NULL, NULL);
    furi_record_close(RECORD_GPS);

    notification_message(notifications, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
