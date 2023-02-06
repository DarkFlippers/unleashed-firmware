#include "spi_mem_view_detect.h"
#include "spi_mem_manager_icons.h"
#include <gui/elements.h>

struct SPIMemDetectView {
    View* view;
    IconAnimation* icon;
    SPIMemDetectViewCallback callback;
    void* cb_ctx;
};

typedef struct {
    IconAnimation* icon;
} SPIMemDetectViewModel;

View* spi_mem_view_detect_get_view(SPIMemDetectView* app) {
    return app->view;
}

static void spi_mem_view_detect_draw_callback(Canvas* canvas, void* context) {
    SPIMemDetectViewModel* model = context;
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_icon_animation(canvas, 0, 0, model->icon);
    canvas_draw_str_aligned(canvas, 64, 26, AlignLeft, AlignCenter, "Detecting");
    canvas_draw_str_aligned(canvas, 64, 36, AlignLeft, AlignCenter, "SPI chip...");
}

static void spi_mem_view_detect_enter_callback(void* context) {
    SPIMemDetectView* app = context;
    with_view_model(
        app->view, SPIMemDetectViewModel * model, { icon_animation_start(model->icon); }, false);
}

static void spi_mem_view_detect_exit_callback(void* context) {
    SPIMemDetectView* app = context;
    with_view_model(
        app->view, SPIMemDetectViewModel * model, { icon_animation_stop(model->icon); }, false);
}

SPIMemDetectView* spi_mem_view_detect_alloc() {
    SPIMemDetectView* app = malloc(sizeof(SPIMemDetectView));
    app->view = view_alloc();
    view_set_context(app->view, app);
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(SPIMemDetectViewModel));
    with_view_model(
        app->view,
        SPIMemDetectViewModel * model,
        {
            model->icon = icon_animation_alloc(&A_ChipLooking_64x64);
            view_tie_icon_animation(app->view, model->icon);
        },
        false);
    view_set_draw_callback(app->view, spi_mem_view_detect_draw_callback);
    view_set_enter_callback(app->view, spi_mem_view_detect_enter_callback);
    view_set_exit_callback(app->view, spi_mem_view_detect_exit_callback);
    return app;
}

void spi_mem_view_detect_free(SPIMemDetectView* app) {
    with_view_model(
        app->view, SPIMemDetectViewModel * model, { icon_animation_free(model->icon); }, false);
    view_free(app->view);
    free(app);
}
