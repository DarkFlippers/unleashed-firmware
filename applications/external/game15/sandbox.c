#include <furi.h>
#include <gui/gui.h>
#include "sandbox.h"

FuriMessageQueue* sandbox_event_queue;
FuriMutex** sandbox_mutex;
ViewPort* sandbox_view_port;
Gui* sandbox_gui;
FuriTimer* sandbox_timer;
bool sandbox_loop_processing;
SandboxRenderCallback sandbox_user_render_callback;
SandboxEventHandler sandbox_user_event_handler;

static void sandbox_render_callback(Canvas* const canvas, void* context) {
    UNUSED(context);
    if(furi_mutex_acquire(sandbox_mutex, 25) != FuriStatusOk) return;

    if(sandbox_user_render_callback) sandbox_user_render_callback(canvas);

    furi_mutex_release(sandbox_mutex);
}

static void sandbox_input_callback(InputEvent* input_event, void* context) {
    UNUSED(context);
    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(sandbox_event_queue, &event, FuriWaitForever);
}

static void sandbox_timer_callback(void* context) {
    UNUSED(context);
    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(sandbox_event_queue, &event, 0);
}

void sandbox_loop() {
    sandbox_loop_processing = true;
    while(sandbox_loop_processing) {
        GameEvent event;
        FuriStatus event_status = furi_message_queue_get(sandbox_event_queue, &event, 100);
        if(event_status != FuriStatusOk) {
            // timeout
            continue;
        }

        furi_mutex_acquire(sandbox_mutex, FuriWaitForever);

        if(sandbox_user_event_handler) sandbox_user_event_handler(event);

        view_port_update(sandbox_view_port);
        furi_mutex_release(sandbox_mutex);
    }
}

void sandbox_loop_exit() {
    sandbox_loop_processing = false;
}

void sandbox_init(
    uint8_t fps,
    SandboxRenderCallback u_render_callback,
    SandboxEventHandler u_event_handler) {
    sandbox_user_render_callback = u_render_callback;
    sandbox_user_event_handler = u_event_handler;

    sandbox_event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));
    sandbox_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    sandbox_view_port = view_port_alloc();
    view_port_draw_callback_set(sandbox_view_port, sandbox_render_callback, NULL);
    view_port_input_callback_set(sandbox_view_port, sandbox_input_callback, NULL);

    sandbox_gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(sandbox_gui, sandbox_view_port, GuiLayerFullscreen);

    if(fps > 0) {
        sandbox_timer = furi_timer_alloc(sandbox_timer_callback, FuriTimerTypePeriodic, NULL);
        furi_timer_start(sandbox_timer, furi_kernel_get_tick_frequency() / fps);
    } else
        sandbox_timer = NULL;
}

void sandbox_free() {
    if(sandbox_timer) furi_timer_free(sandbox_timer);

    gui_remove_view_port(sandbox_gui, sandbox_view_port);
    view_port_enabled_set(sandbox_view_port, false);
    view_port_free(sandbox_view_port);

    if(furi_mutex_acquire(sandbox_mutex, FuriWaitForever) == FuriStatusOk) {
        furi_mutex_free(sandbox_mutex);
    }
    furi_message_queue_free(sandbox_event_queue);
}
