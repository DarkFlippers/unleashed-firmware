#include "loading.h"

#include <gui/icon_animation.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <input/input.h>

#include <furi.h>
#include <assets_icons.h>
#include <stdint.h>

struct Loading {
    View* view;
};

typedef struct {
    IconAnimation* icon;
    FuriString* text;
    bool progress_shown;
    float progress;
} LoadingModel;

#define LOADING_ICON_SIZE  24
#define LOADING_BAR_WIDTH  64
#define LOADING_BAR_HEIGHT 9
//keeps the pair centred as a block once the bar joins the animation
#define LOADING_BAR_GAP    4
#define LOADING_BAR_BLOCK  (LOADING_ICON_SIZE + LOADING_BAR_GAP + LOADING_BAR_HEIGHT)

//with a label: spinner on the left, label and bar centred in the space to its right
#define LOADING_LABEL_ICON_X      12
#define LOADING_LABEL_LEFT        (LOADING_LABEL_ICON_X + LOADING_ICON_SIZE)
//half a 2-line FontPrimary label
#define LOADING_LABEL_HALF_HEIGHT 10
//keeps label + gap + bar centred as one block
#define LOADING_LABEL_BAR_SHIFT   ((LOADING_BAR_GAP + LOADING_BAR_HEIGHT) / 2)
#define LOADING_LABEL_BAR_Y       (LOADING_LABEL_HALF_HEIGHT + LOADING_BAR_GAP)

static void loading_draw_label(Canvas* canvas, LoadingModel* model) {
    const uint8_t center_x = (LOADING_LABEL_LEFT + canvas_width(canvas)) / 2;
    const uint8_t icon_y = canvas_height(canvas) / 2 - LOADING_ICON_SIZE / 2;
    canvas_draw_icon(canvas, LOADING_LABEL_ICON_X, icon_y, &A_Loading_24);
    canvas_draw_icon_animation(canvas, LOADING_LABEL_ICON_X, icon_y, model->icon);

    const uint8_t text_y =
        canvas_height(canvas) / 2 - (model->progress_shown ? LOADING_LABEL_BAR_SHIFT : 0);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, center_x, text_y, AlignCenter, AlignCenter, furi_string_get_cstr(model->text));

    if(model->progress_shown) {
        elements_progress_bar(
            canvas,
            center_x - LOADING_BAR_WIDTH / 2,
            text_y + LOADING_LABEL_BAR_Y,
            LOADING_BAR_WIDTH,
            model->progress);
    }
}

static void loading_draw_callback(Canvas* canvas, void* _model) {
    LoadingModel* model = (LoadingModel*)_model;

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), canvas_height(canvas));
    canvas_set_color(canvas, ColorBlack);

    if(!furi_string_empty(model->text)) {
        loading_draw_label(canvas, model);
        return;
    }

    const uint8_t x = canvas_width(canvas) / 2 - LOADING_ICON_SIZE / 2;
    const uint8_t block = model->progress_shown ? LOADING_BAR_BLOCK : LOADING_ICON_SIZE;
    const uint8_t y = canvas_height(canvas) / 2 - block / 2;

    canvas_draw_icon(canvas, x, y, &A_Loading_24);

    canvas_draw_icon_animation(canvas, x, y, model->icon);

    if(model->progress_shown) {
        elements_progress_bar(
            canvas,
            canvas_width(canvas) / 2 - LOADING_BAR_WIDTH / 2,
            y + LOADING_ICON_SIZE + LOADING_BAR_GAP,
            LOADING_BAR_WIDTH,
            model->progress);
    }
}

static bool loading_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    furi_assert(context);
    return true;
}

static void loading_enter_callback(void* context) {
    furi_assert(context);
    Loading* instance = context;
    LoadingModel* model = view_get_model(instance->view);
    /* using Loading View in conjunction with several
     * Stack View obligates to reassign
     * Update callback, as it can be rewritten
     */
    view_tie_icon_animation(instance->view, model->icon);
    icon_animation_start(model->icon);
    view_commit_model(instance->view, false);
}

static void loading_exit_callback(void* context) {
    furi_assert(context);
    Loading* instance = context;
    LoadingModel* model = view_get_model(instance->view);
    icon_animation_stop(model->icon);
    view_commit_model(instance->view, false);
}

Loading* loading_alloc(void) {
    Loading* instance = malloc(sizeof(Loading));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(LoadingModel));
    LoadingModel* model = view_get_model(instance->view);
    model->icon = icon_animation_alloc(&A_Loading_24);
    model->text = furi_string_alloc();
    model->progress_shown = false;
    model->progress = 0.0f;
    view_tie_icon_animation(instance->view, model->icon);
    view_commit_model(instance->view, false);

    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, loading_draw_callback);
    view_set_input_callback(instance->view, loading_input_callback);
    view_set_enter_callback(instance->view, loading_enter_callback);
    view_set_exit_callback(instance->view, loading_exit_callback);

    return instance;
}

void loading_free(Loading* instance) {
    furi_check(instance);

    LoadingModel* model = view_get_model(instance->view);
    icon_animation_free(model->icon);
    furi_string_free(model->text);
    view_commit_model(instance->view, false);

    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* loading_get_view(Loading* instance) {
    furi_check(instance);
    return instance->view;
}

void loading_set_progress(Loading* instance, float progress) {
    furi_check(instance);
    const float clamped = CLAMP(progress, 1.0f, 0.0f);
    bool changed = false;
    with_view_model(
        instance->view,
        LoadingModel * model,
        {
            //the redraw is worth skipping: callers report once per file or key, and
            //would otherwise queue hundreds of identical frames
            changed = !model->progress_shown || (model->progress != clamped);
            model->progress_shown = true;
            model->progress = clamped;
        },
        changed);
}

void loading_reset_progress(Loading* instance) {
    furi_check(instance);
    bool changed = false;
    with_view_model(
        instance->view,
        LoadingModel * model,
        {
            changed = model->progress_shown;
            model->progress_shown = false;
            model->progress = 0.0f;
        },
        changed);
}

void loading_set_text(Loading* instance, const char* text) {
    furi_check(instance);
    if(!text) text = "";
    bool changed = false;
    with_view_model(
        instance->view,
        LoadingModel * model,
        {
            changed = !furi_string_equal_str(model->text, text);
            furi_string_set(model->text, text);
        },
        changed);
}
