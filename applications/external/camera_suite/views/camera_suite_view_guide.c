#include "../camera_suite.h"
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <dolphin/dolphin.h>

struct CameraSuiteViewGuide {
    View* view;
    CameraSuiteViewGuideCallback callback;
    void* context;
};

typedef struct {
    int some_value;
} CameraSuiteViewGuideModel;

void camera_suite_view_guide_set_callback(
    CameraSuiteViewGuide* instance,
    CameraSuiteViewGuideCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void camera_suite_view_guide_draw(Canvas* canvas, CameraSuiteViewGuideModel* model) {
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Guide");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 12, AlignLeft, AlignTop, "Left = Toggle Invert");
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, "Right = Toggle Dithering");
    canvas_draw_str_aligned(canvas, 0, 32, AlignLeft, AlignTop, "Up = Contrast Up");
    canvas_draw_str_aligned(canvas, 0, 42, AlignLeft, AlignTop, "Down = Contrast Down");
    // TODO: Possibly update to take picture instead.
    canvas_draw_str_aligned(canvas, 0, 52, AlignLeft, AlignTop, "Center = Toggle Dither Type");
}

static void camera_suite_view_guide_model_init(CameraSuiteViewGuideModel* const model) {
    model->some_value = 1;
}

bool camera_suite_view_guide_input(InputEvent* event, void* context) {
    furi_assert(context);
    CameraSuiteViewGuide* instance = context;
    if(event->type == InputTypeRelease) {
        switch(event->key) {
        case InputKeyBack:
            with_view_model(
                instance->view,
                CameraSuiteViewGuideModel * model,
                {
                    UNUSED(model);
                    instance->callback(CameraSuiteCustomEventSceneGuideBack, instance->context);
                },
                true);
            break;
        case InputKeyLeft:
        case InputKeyRight:
        case InputKeyUp:
        case InputKeyDown:
        case InputKeyOk:
        case InputKeyMAX:
            // Do nothing.
            break;
        }
    }
    return true;
}

void camera_suite_view_guide_exit(void* context) {
    furi_assert(context);
}

void camera_suite_view_guide_enter(void* context) {
    furi_assert(context);
    CameraSuiteViewGuide* instance = (CameraSuiteViewGuide*)context;
    with_view_model(
        instance->view,
        CameraSuiteViewGuideModel * model,
        { camera_suite_view_guide_model_init(model); },
        true);
}

CameraSuiteViewGuide* camera_suite_view_guide_alloc() {
    CameraSuiteViewGuide* instance = malloc(sizeof(CameraSuiteViewGuide));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(CameraSuiteViewGuideModel));
    view_set_context(instance->view, instance); // furi_assert crashes in events without this
    view_set_draw_callback(instance->view, (ViewDrawCallback)camera_suite_view_guide_draw);
    view_set_input_callback(instance->view, camera_suite_view_guide_input);
    view_set_enter_callback(instance->view, camera_suite_view_guide_enter);
    view_set_exit_callback(instance->view, camera_suite_view_guide_exit);

    with_view_model(
        instance->view,
        CameraSuiteViewGuideModel * model,
        { camera_suite_view_guide_model_init(model); },
        true);

    return instance;
}

void camera_suite_view_guide_free(CameraSuiteViewGuide* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, CameraSuiteViewGuideModel * model, { UNUSED(model); }, true);
    view_free(instance->view);
    free(instance);
}

View* camera_suite_view_guide_get_view(CameraSuiteViewGuide* instance) {
    furi_assert(instance);
    return instance->view;
}
