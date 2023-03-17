#include "spi_mem_view_progress.h"
#include <gui/elements.h>

struct SPIMemProgressView {
    View* view;
    SPIMemProgressViewCallback callback;
    void* cb_ctx;
};

typedef enum {
    SPIMemProgressViewTypeRead,
    SPIMemProgressViewTypeVerify,
    SPIMemProgressViewTypeWrite,
    SPIMemProgressViewTypeUnknown
} SPIMemProgressViewType;

typedef struct {
    size_t chip_size;
    size_t file_size;
    size_t blocks_written;
    size_t block_size;
    float progress;
    SPIMemProgressViewType view_type;
} SPIMemProgressViewModel;

View* spi_mem_view_progress_get_view(SPIMemProgressView* app) {
    return app->view;
}

static void spi_mem_view_progress_draw_progress(Canvas* canvas, float progress) {
    FuriString* progress_str = furi_string_alloc();
    if(progress > 1.0) progress = 1.0;
    furi_string_printf(progress_str, "%d %%", (int)(progress * 100));
    elements_progress_bar(canvas, 13, 35, 100, progress);
    canvas_draw_str_aligned(
        canvas, 64, 25, AlignCenter, AlignTop, furi_string_get_cstr(progress_str));
    furi_string_free(progress_str);
}

static void
    spi_mem_view_progress_read_draw_callback(Canvas* canvas, SPIMemProgressViewModel* model) {
    canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Reading dump");
    spi_mem_view_progress_draw_progress(canvas, model->progress);
    elements_button_left(canvas, "Cancel");
}

static void
    spi_mem_view_progress_draw_size_warning(Canvas* canvas, SPIMemProgressViewModel* model) {
    if(model->file_size > model->chip_size) {
        canvas_draw_str_aligned(canvas, 64, 13, AlignCenter, AlignTop, "Size clamped to chip!");
    }
    if(model->chip_size > model->file_size) {
        canvas_draw_str_aligned(canvas, 64, 13, AlignCenter, AlignTop, "Size clamped to file!");
    }
}

static void
    spi_mem_view_progress_verify_draw_callback(Canvas* canvas, SPIMemProgressViewModel* model) {
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Verifying dump");
    spi_mem_view_progress_draw_size_warning(canvas, model);
    spi_mem_view_progress_draw_progress(canvas, model->progress);
    elements_button_center(canvas, "Skip");
}

static void
    spi_mem_view_progress_write_draw_callback(Canvas* canvas, SPIMemProgressViewModel* model) {
    canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Writing dump");
    spi_mem_view_progress_draw_size_warning(canvas, model);
    spi_mem_view_progress_draw_progress(canvas, model->progress);
    elements_button_left(canvas, "Cancel");
}

static void spi_mem_view_progress_draw_callback(Canvas* canvas, void* context) {
    SPIMemProgressViewModel* model = context;
    SPIMemProgressViewType view_type = model->view_type;
    if(view_type == SPIMemProgressViewTypeRead) {
        spi_mem_view_progress_read_draw_callback(canvas, model);
    } else if(view_type == SPIMemProgressViewTypeVerify) {
        spi_mem_view_progress_verify_draw_callback(canvas, model);
    } else if(view_type == SPIMemProgressViewTypeWrite) {
        spi_mem_view_progress_write_draw_callback(canvas, model);
    }
}

static bool
    spi_mem_view_progress_read_write_input_callback(InputEvent* event, SPIMemProgressView* app) {
    bool success = false;
    if(event->type == InputTypeShort && event->key == InputKeyLeft) {
        if(app->callback) {
            app->callback(app->cb_ctx);
        }
        success = true;
    }
    return success;
}

static bool
    spi_mem_view_progress_verify_input_callback(InputEvent* event, SPIMemProgressView* app) {
    bool success = false;
    if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(app->callback) {
            app->callback(app->cb_ctx);
        }
        success = true;
    }
    return success;
}

static bool spi_mem_view_progress_input_callback(InputEvent* event, void* context) {
    SPIMemProgressView* app = context;
    bool success = false;
    SPIMemProgressViewType view_type;
    with_view_model(
        app->view, SPIMemProgressViewModel * model, { view_type = model->view_type; }, true);
    if(view_type == SPIMemProgressViewTypeRead) {
        success = spi_mem_view_progress_read_write_input_callback(event, app);
    } else if(view_type == SPIMemProgressViewTypeVerify) {
        success = spi_mem_view_progress_verify_input_callback(event, app);
    } else if(view_type == SPIMemProgressViewTypeWrite) {
        success = spi_mem_view_progress_read_write_input_callback(event, app);
    }
    return success;
}

SPIMemProgressView* spi_mem_view_progress_alloc() {
    SPIMemProgressView* app = malloc(sizeof(SPIMemProgressView));
    app->view = view_alloc();
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(SPIMemProgressViewModel));
    view_set_context(app->view, app);
    view_set_draw_callback(app->view, spi_mem_view_progress_draw_callback);
    view_set_input_callback(app->view, spi_mem_view_progress_input_callback);
    spi_mem_view_progress_reset(app);
    return app;
}

void spi_mem_view_progress_free(SPIMemProgressView* app) {
    view_free(app->view);
    free(app);
}

void spi_mem_view_progress_set_read_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx) {
    app->callback = callback;
    app->cb_ctx = cb_ctx;
    with_view_model(
        app->view,
        SPIMemProgressViewModel * model,
        { model->view_type = SPIMemProgressViewTypeRead; },
        true);
}

void spi_mem_view_progress_set_verify_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx) {
    app->callback = callback;
    app->cb_ctx = cb_ctx;
    with_view_model(
        app->view,
        SPIMemProgressViewModel * model,
        { model->view_type = SPIMemProgressViewTypeVerify; },
        true);
}

void spi_mem_view_progress_set_write_callback(
    SPIMemProgressView* app,
    SPIMemProgressViewCallback callback,
    void* cb_ctx) {
    app->callback = callback;
    app->cb_ctx = cb_ctx;
    with_view_model(
        app->view,
        SPIMemProgressViewModel * model,
        { model->view_type = SPIMemProgressViewTypeWrite; },
        true);
}

void spi_mem_view_progress_set_chip_size(SPIMemProgressView* app, size_t chip_size) {
    with_view_model(
        app->view, SPIMemProgressViewModel * model, { model->chip_size = chip_size; }, true);
}

void spi_mem_view_progress_set_file_size(SPIMemProgressView* app, size_t file_size) {
    with_view_model(
        app->view, SPIMemProgressViewModel * model, { model->file_size = file_size; }, true);
}

void spi_mem_view_progress_set_block_size(SPIMemProgressView* app, size_t block_size) {
    with_view_model(
        app->view, SPIMemProgressViewModel * model, { model->block_size = block_size; }, true);
}

static size_t spi_mem_view_progress_set_total_size(SPIMemProgressViewModel* model) {
    size_t total_size = model->chip_size;
    if((model->chip_size > model->file_size) && model->view_type != SPIMemProgressViewTypeRead) {
        total_size = model->file_size;
    }
    return total_size;
}

void spi_mem_view_progress_inc_progress(SPIMemProgressView* app) {
    with_view_model(
        app->view,
        SPIMemProgressViewModel * model,
        {
            size_t total_size = spi_mem_view_progress_set_total_size(model);
            if(total_size == 0) total_size = 1;
            model->blocks_written++;
            model->progress =
                ((float)model->block_size * (float)model->blocks_written) / ((float)total_size);
        },
        true);
}

void spi_mem_view_progress_reset(SPIMemProgressView* app) {
    with_view_model(
        app->view,
        SPIMemProgressViewModel * model,
        {
            model->blocks_written = 0;
            model->block_size = 0;
            model->chip_size = 0;
            model->file_size = 0;
            model->progress = 0;
            model->view_type = SPIMemProgressViewTypeUnknown;
        },
        true);
}
