#include "../camera_suite.h"
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>

struct CameraSuiteViewStart {
    View* view;
    CameraSuiteViewStartCallback callback;
    void* context;
};

typedef struct {
    int some_value;
} CameraSuiteViewStartModel;

void camera_suite_view_start_set_callback(
    CameraSuiteViewStart* instance,
    CameraSuiteViewStartCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void camera_suite_view_start_draw(Canvas* canvas, CameraSuiteViewStartModel* model) {
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "Camera Suite");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 22, AlignCenter, AlignTop, "Flipper Zero");
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "ESP32 CAM");
    elements_button_center(canvas, "Start");
}

static void camera_suite_view_start_model_init(CameraSuiteViewStartModel* const model) {
    model->some_value = 1;
}

bool camera_suite_view_start_input(InputEvent* event, void* context) {
    furi_assert(context);
    CameraSuiteViewStart* instance = context;
    if(event->type == InputTypeRelease) {
        switch(event->key) {
        case InputKeyBack:
            // Exit application.
            with_view_model(
                instance->view,
                CameraSuiteViewStartModel * model,
                {
                    UNUSED(model);
                    instance->callback(CameraSuiteCustomEventStartBack, instance->context);
                },
                true);
            break;
        case InputKeyOk:
            // Start the application.
            with_view_model(
                instance->view,
                CameraSuiteViewStartModel * model,
                {
                    UNUSED(model);
                    instance->callback(CameraSuiteCustomEventStartOk, instance->context);
                },
                true);
            break;
        case InputKeyMAX:
        case InputKeyLeft:
        case InputKeyRight:
        case InputKeyUp:
        case InputKeyDown:
            // Do nothing.
            break;
        }
    }
    return true;
}

void camera_suite_view_start_exit(void* context) {
    furi_assert(context);
}

void camera_suite_view_start_enter(void* context) {
    furi_assert(context);
    CameraSuiteViewStart* instance = (CameraSuiteViewStart*)context;
    with_view_model(
        instance->view,
        CameraSuiteViewStartModel * model,
        { camera_suite_view_start_model_init(model); },
        true);
}

CameraSuiteViewStart* camera_suite_view_start_alloc() {
    CameraSuiteViewStart* instance = malloc(sizeof(CameraSuiteViewStart));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(CameraSuiteViewStartModel));
    // furi_assert crashes in events without this
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)camera_suite_view_start_draw);
    view_set_input_callback(instance->view, camera_suite_view_start_input);

    with_view_model(
        instance->view,
        CameraSuiteViewStartModel * model,
        { camera_suite_view_start_model_init(model); },
        true);

    return instance;
}

void camera_suite_view_start_free(CameraSuiteViewStart* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, CameraSuiteViewStartModel * model, { UNUSED(model); }, true);
    view_free(instance->view);
    free(instance);
}

View* camera_suite_view_start_get_view(CameraSuiteViewStart* instance) {
    furi_assert(instance);
    return instance->view;
}
