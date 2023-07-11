#include "../camera_suite.h"
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <dolphin/dolphin.h>
#include "../helpers/camera_suite_haptic.h"
#include "../helpers/camera_suite_speaker.h"
#include "../helpers/camera_suite_led.h"

struct CameraSuiteViewStyle2 {
    View* view;
    CameraSuiteViewStyle2Callback callback;
    void* context;
};

typedef struct {
    int screen_text;
} CameraSuiteViewStyle2Model;

char buttonText[11][14] = {
    "",
    "Press Up",
    "Press Down",
    "Press Left",
    "Press Right",
    "Press Ok",
    "Release Up",
    "Release Down",
    "Release Left",
    "Release Right",
    "Release Ok",
};

void camera_suite_view_style_2_set_callback(
    CameraSuiteViewStyle2* instance,
    CameraSuiteViewStyle2Callback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void camera_suite_view_style_2_draw(Canvas* canvas, CameraSuiteViewStyle2Model* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, "Scene 2: Input Examples");
    canvas_set_font(canvas, FontSecondary);
    char* strInput = malloc(15);
    strcpy(strInput, buttonText[model->screen_text]);
    canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, strInput);
    free(strInput);
}

static void camera_suite_view_style_2_model_init(CameraSuiteViewStyle2Model* const model) {
    model->screen_text = 0;
}

bool camera_suite_view_style_2_input(InputEvent* event, void* context) {
    furi_assert(context);
    CameraSuiteViewStyle2* instance = context;
    if(event->type == InputTypeRelease) {
        switch(event->key) {
        case InputKeyBack:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    UNUSED(model);
                    camera_suite_stop_all_sound(instance->context);
                    instance->callback(CameraSuiteCustomEventSceneStyle2Back, instance->context);
                    camera_suite_play_long_bump(instance->context);
                },
                true);
            break;
        case InputKeyUp:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 6;
                    camera_suite_play_bad_bump(instance->context);
                    camera_suite_stop_all_sound(instance->context);
                    camera_suite_led_set_rgb(instance->context, 255, 0, 255);
                },
                true);
            break;
        case InputKeyDown:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 7;
                    camera_suite_play_bad_bump(instance->context);
                    camera_suite_stop_all_sound(instance->context);
                    camera_suite_led_set_rgb(instance->context, 255, 255, 0);
                },
                true);
            break;
        case InputKeyLeft:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 8;
                    camera_suite_play_bad_bump(instance->context);
                    camera_suite_stop_all_sound(instance->context);
                    camera_suite_led_set_rgb(instance->context, 0, 255, 255);
                },
                true);
            break;
        case InputKeyRight:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 9;
                    camera_suite_play_bad_bump(instance->context);
                    camera_suite_stop_all_sound(instance->context);
                    camera_suite_led_set_rgb(instance->context, 255, 0, 0);
                },
                true);
            break;
        case InputKeyOk:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 10;
                    camera_suite_play_bad_bump(instance->context);
                    camera_suite_stop_all_sound(instance->context);
                    camera_suite_led_set_rgb(instance->context, 255, 255, 255);
                },
                true);
            break;
        case InputKeyMAX:
            break;
        }
    } else if(event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyUp:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 1;
                    camera_suite_play_happy_bump(instance->context);
                    camera_suite_play_input_sound(instance->context);
                },
                true);
            break;
        case InputKeyDown:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 2;
                    camera_suite_play_happy_bump(instance->context);
                    camera_suite_play_input_sound(instance->context);
                },
                true);
            break;
        case InputKeyLeft:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 3;
                    camera_suite_play_happy_bump(instance->context);
                    camera_suite_play_input_sound(instance->context);
                },
                true);
            break;
        case InputKeyRight:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 4;
                    camera_suite_play_happy_bump(instance->context);
                    camera_suite_play_input_sound(instance->context);
                },
                true);
            break;
        case InputKeyOk:
            with_view_model(
                instance->view,
                CameraSuiteViewStyle2Model * model,
                {
                    model->screen_text = 5;
                    camera_suite_play_happy_bump(instance->context);
                    camera_suite_play_input_sound(instance->context);
                },
                true);
            break;
        case InputKeyBack:
        case InputKeyMAX:
            break;
        }
    }

    return true;
}

void camera_suite_view_style_2_exit(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    camera_suite_stop_all_sound(app);
    //camera_suite_led_reset(app);
}

void camera_suite_view_style_2_enter(void* context) {
    furi_assert(context);
    dolphin_deed(DolphinDeedPluginStart);
}

CameraSuiteViewStyle2* camera_suite_view_style_2_alloc() {
    CameraSuiteViewStyle2* instance = malloc(sizeof(CameraSuiteViewStyle2));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(CameraSuiteViewStyle2Model));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)camera_suite_view_style_2_draw);
    view_set_input_callback(instance->view, camera_suite_view_style_2_input);
    //view_set_enter_callback(instance->view, camera_suite_view_style_2_enter);
    view_set_exit_callback(instance->view, camera_suite_view_style_2_exit);

    with_view_model(
        instance->view,
        CameraSuiteViewStyle2Model * model,
        { camera_suite_view_style_2_model_init(model); },
        true);

    return instance;
}

void camera_suite_view_style_2_free(CameraSuiteViewStyle2* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* camera_suite_view_style_2_get_view(CameraSuiteViewStyle2* instance) {
    furi_assert(instance);

    return instance->view;
}
