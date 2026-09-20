#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

#include <core/memmgr.h>
#include <core/memmgr_heap.h>

#define TAG "OomTest"

#define OOM_TEST_BLOCK_SIZE    1024u
#define OOM_TEST_SAFETY_MARGIN 8192u
#define OOM_TEST_MAX_BLOCKS    256u

typedef struct {
    FuriMessageQueue* input_queue;
    ViewPort* view_port;
    size_t free_heap;
    size_t max_block;
    size_t leaked_bytes;
} OomTest;

static void oom_test_refresh_stats(OomTest* app) {
    app->free_heap = memmgr_get_free_heap();
    app->max_block = memmgr_heap_get_max_free_block();
}

static void oom_test_fragment(OomTest* app) {
    void** blocks = malloc(sizeof(void*) * OOM_TEST_MAX_BLOCKS);
    size_t count = 0;

    while(count < OOM_TEST_MAX_BLOCKS &&
          memmgr_heap_get_max_free_block() > (OOM_TEST_BLOCK_SIZE + OOM_TEST_SAFETY_MARGIN)) {
        blocks[count] = malloc(OOM_TEST_BLOCK_SIZE);
        count++;
    }

    size_t kept = 0;
    for(size_t i = 0; i < count; i++) {
        if(i % 2 == 1) {
            free(blocks[i]);
        } else {
            kept++;
        }
    }

    app->leaked_bytes += kept * OOM_TEST_BLOCK_SIZE;

    free(blocks);

    oom_test_refresh_stats(app);
    FURI_LOG_W(
        TAG,
        "Fragmented: free %zu, max block %zu, leaked %zu",
        app->free_heap,
        app->max_block,
        app->leaked_bytes);
}

static void oom_test_draw_callback(Canvas* canvas, void* ctx) {
    OomTest* app = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "OOM Reboot Test");

    canvas_set_font(canvas, FontSecondary);

    char line[32];
    snprintf(line, sizeof(line), "Free:      %zu B", app->free_heap);
    canvas_draw_str(canvas, 2, 22, line);
    snprintf(line, sizeof(line), "Max block: %zu B", app->max_block);
    canvas_draw_str(canvas, 2, 33, line);
    snprintf(line, sizeof(line), "Leaked:    %zu B", app->leaked_bytes);
    canvas_draw_str(canvas, 2, 44, line);

    canvas_draw_str(canvas, 2, 62, "OK: fragment   Back: exit");
}

static void oom_test_input_callback(InputEvent* input_event, void* ctx) {
    OomTest* app = ctx;
    furi_message_queue_put(app->input_queue, input_event, FuriWaitForever);
}

int32_t oom_test_main(void* p) {
    UNUSED(p);

    OomTest* app = malloc(sizeof(OomTest));
    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->leaked_bytes = 0;

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, oom_test_draw_callback, app);
    view_port_input_callback_set(app->view_port, oom_test_input_callback, app);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, app->view_port, GuiLayerFullscreen);

    oom_test_refresh_stats(app);

    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(app->input_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                if(event.key == InputKeyOk) {
                    oom_test_fragment(app);
                } else if(event.key == InputKeyBack) {
                    running = false;
                }
            }
        }
        view_port_update(app->view_port);
    }

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    free(app);

    return 0;
}
