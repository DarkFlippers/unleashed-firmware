#include "view_display_test.h"

typedef struct {
    uint32_t test;
    uint32_t size;
    uint32_t counter;
    bool flip_flop;
} ViewDisplayTestModel;

struct ViewDisplayTest {
    View* view;
    FuriTimer* timer;
};

static void view_display_test_draw_callback_intro(Canvas* canvas, void* _model) {
    UNUSED(_model);
    canvas_draw_str(canvas, 12, 24, "Use < and > to switch tests");
    canvas_draw_str(canvas, 12, 36, "Use ^ and v to switch size");
    canvas_draw_str(canvas, 32, 48, "Use (o) to flip");
}

static void view_display_test_draw_callback_fill(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    if(model->flip_flop) {
        uint8_t width = canvas_width(canvas);
        uint8_t height = canvas_height(canvas);
        canvas_draw_box(canvas, 0, 0, width, height);
    }
}

static void view_display_test_draw_callback_hstripe(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    uint8_t block = 1 + model->size;
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);

    for(uint8_t y = model->flip_flop * block; y < height; y += 2 * block) {
        canvas_draw_box(canvas, 0, y, width, block);
    }
}

static void view_display_test_draw_callback_vstripe(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    uint8_t block = 1 + model->size;
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);

    for(uint8_t x = model->flip_flop * block; x < width; x += 2 * block) {
        canvas_draw_box(canvas, x, 0, block, height);
    }
}

static void view_display_test_draw_callback_check(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    uint8_t block = 1 + model->size;
    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);

    bool flip_flop = model->flip_flop;
    for(uint8_t x = 0; x < width; x += block) {
        bool last_flip_flop = flip_flop;
        for(uint8_t y = 0; y < height; y += block) {
            if(flip_flop) {
                canvas_draw_box(canvas, x, y, block, block);
            }
            flip_flop = !flip_flop;
        }
        if(last_flip_flop == flip_flop) {
            flip_flop = !flip_flop;
        }
    }
}

static void view_display_test_draw_callback_move(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    uint8_t block = 1 + model->size;
    uint8_t width = canvas_width(canvas) - block;
    uint8_t height = canvas_height(canvas) - block;

    uint8_t x = model->counter % width;
    if((model->counter / width) % 2) {
        x = width - x;
    }

    uint8_t y = model->counter % height;
    if((model->counter / height) % 2) {
        y = height - y;
    }

    canvas_draw_box(canvas, x, y, block, block);
}

const ViewDrawCallback view_display_test_tests[] = {
    view_display_test_draw_callback_intro,
    view_display_test_draw_callback_fill,
    view_display_test_draw_callback_hstripe,
    view_display_test_draw_callback_vstripe,
    view_display_test_draw_callback_check,
    view_display_test_draw_callback_move,
};

static void view_display_test_draw_callback(Canvas* canvas, void* _model) {
    ViewDisplayTestModel* model = _model;
    view_display_test_tests[model->test](canvas, _model);
}

static bool view_display_test_input_callback(InputEvent* event, void* context) {
    ViewDisplayTest* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        with_view_model(
            instance->view, (ViewDisplayTestModel * model) {
                if(event->key == InputKeyLeft && model->test > 0) {
                    model->test--;
                    consumed = true;
                } else if(
                    event->key == InputKeyRight &&
                    model->test < (COUNT_OF(view_display_test_tests) - 1)) {
                    model->test++;
                    consumed = true;
                } else if(event->key == InputKeyDown && model->size > 0) {
                    model->size--;
                    consumed = true;
                } else if(event->key == InputKeyUp && model->size < 24) {
                    model->size++;
                    consumed = true;
                } else if(event->key == InputKeyOk) {
                    model->flip_flop = !model->flip_flop;
                    consumed = true;
                }
                return consumed;
            });
    }

    return consumed;
}

static void view_display_test_enter(void* context) {
    ViewDisplayTest* instance = context;
    furi_timer_start(instance->timer, furi_kernel_get_tick_frequency() / 32);
}

static void view_display_test_exit(void* context) {
    ViewDisplayTest* instance = context;
    furi_timer_stop(instance->timer);
}

static void view_display_test_timer_callback(void* context) {
    ViewDisplayTest* instance = context;
    with_view_model(
        instance->view, (ViewDisplayTestModel * model) {
            model->counter++;
            return true;
        });
}

ViewDisplayTest* view_display_test_alloc() {
    ViewDisplayTest* instance = malloc(sizeof(ViewDisplayTest));

    instance->view = view_alloc();
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLockFree, sizeof(ViewDisplayTestModel));
    view_set_draw_callback(instance->view, view_display_test_draw_callback);
    view_set_input_callback(instance->view, view_display_test_input_callback);
    view_set_enter_callback(instance->view, view_display_test_enter);
    view_set_exit_callback(instance->view, view_display_test_exit);

    instance->timer =
        furi_timer_alloc(view_display_test_timer_callback, FuriTimerTypePeriodic, instance);

    return instance;
}

void view_display_test_free(ViewDisplayTest* instance) {
    furi_assert(instance);

    furi_timer_free(instance->timer);
    view_free(instance->view);
    free(instance);
}

View* view_display_test_get_view(ViewDisplayTest* instance) {
    furi_assert(instance);
    return instance->view;
}
