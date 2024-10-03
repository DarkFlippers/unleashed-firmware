#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

#define BUFFER_SIZE (32U)

typedef struct {
    FuriPubSub* input;
    FuriPubSubSubscription* input_subscription;
    Gui* gui;
    Canvas* canvas;
    bool stop;
    uint32_t counter;
} DirectDraw;

static void gui_input_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);

    DirectDraw* instance = ctx;
    const InputEvent* event = value;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        instance->stop = true;
    }
}

static DirectDraw* direct_draw_alloc(void) {
    DirectDraw* instance = malloc(sizeof(DirectDraw));

    instance->input = furi_record_open(RECORD_INPUT_EVENTS);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->canvas = gui_direct_draw_acquire(instance->gui);

    instance->input_subscription =
        furi_pubsub_subscribe(instance->input, gui_input_events_callback, instance);

    return instance;
}

static void direct_draw_free(DirectDraw* instance) {
    furi_pubsub_unsubscribe(instance->input, instance->input_subscription);

    gui_direct_draw_release(instance->gui);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_INPUT_EVENTS);

    free(instance);
}

static void direct_draw_block(Canvas* canvas, uint32_t size, uint32_t counter) {
    size += 16;
    uint8_t width = canvas_width(canvas) - size;
    uint8_t height = canvas_height(canvas) - size;

    uint8_t x = counter % width;
    if((counter / width) % 2) {
        x = width - x;
    }

    uint8_t y = counter % height;
    if((counter / height) % 2) {
        y = height - y;
    }

    canvas_draw_box(canvas, x, y, size, size);
}

static void direct_draw_run(DirectDraw* instance) {
    size_t start = DWT->CYCCNT;
    size_t counter = 0;
    float fps = 0;

    furi_thread_set_current_priority(FuriThreadPriorityIdle);

    do {
        size_t elapsed = DWT->CYCCNT - start;
        char buffer[BUFFER_SIZE] = {0};

        if(elapsed >= 64000000) {
            fps = (float)counter / ((float)elapsed / 64000000.0f);

            start = DWT->CYCCNT;
            counter = 0;
        }
        snprintf(buffer, BUFFER_SIZE, "FPS: %.1f", (double)fps);

        canvas_reset(instance->canvas);
        canvas_set_color(instance->canvas, ColorXOR);
        direct_draw_block(instance->canvas, instance->counter % 16, instance->counter);
        direct_draw_block(instance->canvas, instance->counter * 2 % 16, instance->counter * 2);
        direct_draw_block(instance->canvas, instance->counter * 3 % 16, instance->counter * 3);
        direct_draw_block(instance->canvas, instance->counter * 4 % 16, instance->counter * 4);
        direct_draw_block(instance->canvas, instance->counter * 5 % 16, instance->counter * 5);
        canvas_draw_str(instance->canvas, 10, 10, buffer);
        canvas_commit(instance->canvas);

        counter++;
        instance->counter++;
        furi_thread_yield();
    } while(!instance->stop);
}

int32_t direct_draw_app(void* p) {
    UNUSED(p);

    DirectDraw* instance = direct_draw_alloc();
    direct_draw_run(instance);
    direct_draw_free(instance);

    return 0;
}
