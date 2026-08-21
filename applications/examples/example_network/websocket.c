#include <furi.h>
#include <furi_hal_version.h>
#include <gui/gui.h>
#include <network/network.h>
#include <string.h>

/* WebSocket round-trip: open wss://, send one text frame, wait for the echo,
 * close.
 *
 * Network API used: network_websocket_open(), network_websocket_send(),
 * network_close(), network_set_event_callback(). Received frames arrive as
 * NetworkEventReceived with the binary flag set for binary frames.
 */

#define WS_BASE_ID    21
#define WS_URL        "wss://echo.websocket.org"
#define WS_TIMEOUT_MS 15000

typedef enum {
    WsInit,
    WsNoBridge,
    WsConnecting,
    WsWaiting,
    WsDone,
    WsError,
} WsState;

typedef struct {
    FuriMutex* mutex;
    WsState state;
    NetworkError error;
    uint32_t conn_id;
    char name[64];
    size_t tx_bytes;
    size_t rx_bytes;
    uint32_t start_tick;
    uint32_t elapsed_ticks;
    char received[128];
    bool got_echo;
    bool send_requested;
    bool close_requested;
    bool restart_requested;
} WsTestApp;

static void ws_test_reset(WsTestApp* app) {
    app->error = NetworkErrorNone;
    app->tx_bytes = 0;
    app->rx_bytes = 0;
    app->start_tick = 0;
    app->elapsed_ticks = 0;
    app->received[0] = '\0';
    app->got_echo = false;
    app->send_requested = false;
    app->close_requested = false;

    const char* name = furi_hal_version_get_name_ptr();
    strlcpy(app->name, (name && name[0]) ? name : "Flipper", sizeof(app->name));
}

static void ws_test_start(WsTestApp* app, Network* network) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    ws_test_reset(app);
    app->state = WsConnecting;
    uint32_t id = app->conn_id;
    furi_mutex_release(app->mutex);

    /* DNS+TCP+TLS+upgrade all happen on the companion; the handshake result
     * arrives as NetworkEventConnected. NULL = no extra handshake headers.
     * false = no live companion session. */
    bool opened = network_websocket_open(network, id, WS_URL, NULL, WS_TIMEOUT_MS);
    if(!opened) {
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->state = WsNoBridge;
        furi_mutex_release(app->mutex);
    }
}

/* Called from the RPC session thread; demuxed by connection_id so events from
 * a previous (restarted) connection are ignored. */
static void ws_test_event_callback(const NetworkEvent* event, void* context) {
    WsTestApp* app = context;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    if(event->connection_id != app->conn_id) {
        furi_mutex_release(app->mutex);
        return;
    }

    switch(event->type) {
    /* Handshake answer for network_websocket_open(). */
    case NetworkEventConnected:
        if(event->error == NetworkErrorNone && event->state == NetworkStateConnected) {
            app->send_requested = true;
        } else {
            app->state = WsError;
            app->error = event->error;
        }
        break;
    /* One event per inbound frame. event->data is valid only during the
     * callback - copy it out. event->binary marks binary frames (unused: the
     * echo comes back as text). */
    case NetworkEventReceived:
        if(event->data && event->size) {
            app->rx_bytes += event->size;
            size_t copy = event->size < sizeof(app->received) - 1 ? event->size :
                                                                    sizeof(app->received) - 1;
            memcpy(app->received, event->data, copy);
            app->received[copy] = '\0';

            if(!app->got_echo && event->size == app->tx_bytes &&
               memcmp(event->data, app->name, app->tx_bytes) == 0) {
                app->got_echo = true;
                app->elapsed_ticks = furi_get_tick() - app->start_tick;
                app->state = WsDone;
                app->close_requested = true;
            }
        }
        break;
    /* Ack for network_websocket_send(); error set if the frame was not sent. */
    case NetworkEventSent:
        if(event->error != NetworkErrorNone) {
            app->state = WsError;
            app->error = event->error;
        }
        break;
    /* Connection dropped by the peer/companion, or our close was confirmed. */
    case NetworkEventStateChanged:
    case NetworkEventClosed:
        if(app->state != WsDone && app->state != WsError) {
            app->state = app->got_echo ? WsDone : WsError;
            if(!app->got_echo && app->error == NetworkErrorNone) {
                app->error = event->error == NetworkErrorNone ? NetworkErrorNotConnected :
                                                                event->error;
            }
        }
        break;
    case NetworkEventHttpResponse:
        break;
    }
    furi_mutex_release(app->mutex);
}

static void ws_test_draw_callback(Canvas* canvas, void* context) {
    WsTestApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    char line[64];

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignBottom, "WS echo test");

    canvas_set_font(canvas, FontSecondary);
    const char* status = "Starting...";
    switch(app->state) {
    case WsNoBridge:
        status = "No USB/BLE connection";
        break;
    case WsConnecting:
        status = "Connecting...";
        break;
    case WsWaiting:
        status = "Waiting for echo...";
        break;
    case WsDone:
        status = "Echo OK, closed";
        break;
    case WsError:
        status = network_error_to_string(app->error);
        break;
    default:
        break;
    }
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignBottom, status);

    snprintf(line, sizeof(line), "TX: %lu bytes", (unsigned long)app->tx_bytes);
    canvas_draw_str_aligned(canvas, 0, 33, AlignLeft, AlignBottom, line);

    snprintf(
        line,
        sizeof(line),
        "RX: %lu bytes  %lu ms",
        (unsigned long)app->rx_bytes,
        (unsigned long)app->elapsed_ticks);
    canvas_draw_str_aligned(canvas, 0, 44, AlignLeft, AlignBottom, line);

    if(app->received[0]) {
        snprintf(line, sizeof(line), "< %.60s", app->received);
        canvas_draw_str_aligned(canvas, 0, 55, AlignLeft, AlignBottom, line);
    }

    canvas_draw_str_aligned(canvas, 0, 64, AlignLeft, AlignBottom, "OK: retry  Back: exit");

    furi_mutex_release(app->mutex);
}

static void ws_test_input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t network_websocket_app(void* p) {
    UNUSED(p);

    WsTestApp* app = malloc(sizeof(WsTestApp));
    memset(app, 0, sizeof(WsTestApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->state = WsInit;
    app->conn_id = WS_BASE_ID;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    Network* network = furi_record_open(RECORD_NETWORK);
    network_set_event_callback(network, ws_test_event_callback, app);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, ws_test_draw_callback, app);
    view_port_input_callback_set(view_port, ws_test_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    ws_test_start(app, network);

    InputEvent event;
    for(bool processing = true; processing;) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                if(event.key == InputKeyBack) {
                    processing = false;
                } else if(event.key == InputKeyOk) {
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    app->restart_requested = true;
                    furi_mutex_release(app->mutex);
                }
            }
        }

        furi_mutex_acquire(app->mutex, FuriWaitForever);
        bool do_send = app->send_requested;
        app->send_requested = false;
        bool do_close = app->close_requested;
        app->close_requested = false;
        bool do_restart = app->restart_requested;
        app->restart_requested = false;
        uint32_t id = app->conn_id;
        if(do_send) {
            app->start_tick = furi_get_tick();
            app->tx_bytes = strlen(app->name);
            app->state = WsWaiting;
        }
        furi_mutex_release(app->mutex);

        if(do_send) {
            /* Text frame (binary = false), payload <= NETWORK_MAX_DATA_SIZE.
             * Delivery is confirmed by NetworkEventSent. */
            network_websocket_send(
                network, id, (const uint8_t*)app->name, strlen(app->name), false);
        }

        if(do_close) {
            network_close(network, id);
        }

        if(do_restart) {
            network_close(network, id);
            /* Never reuse a closed connection id: bump it so events from the
             * old connection fail the conn_id check in the callback. */
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->conn_id++;
            furi_mutex_release(app->mutex);
            ws_test_start(app, network);
        }

        view_port_update(view_port);
    }

    /* Close the (possibly already closed) connection, then unsubscribe before
     * releasing the record so late events cannot reach a freed context. */
    network_close(network, app->conn_id);
    network_set_event_callback(network, NULL, NULL);
    furi_record_close(RECORD_NETWORK);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
