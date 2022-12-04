#include <furi.h>
#include <furi_hal.h>

#include <HEX_Viewer_icons.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <dialogs/dialogs.h>

#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <toolbox/stream/file_stream.h>

#define TAG "HexViewer"

#define HEX_VIEWER_APP_PATH_FOLDER "/any"
#define HEX_VIEWER_APP_EXTENSION "*"

#define HEX_VIEWER_BYTES_PER_LINE 4u
#define HEX_VIEWER_LINES_ON_SCREEN 4u
#define HEX_VIEWER_BUF_SIZE (HEX_VIEWER_LINES_ON_SCREEN * HEX_VIEWER_BYTES_PER_LINE)

typedef struct {
    uint8_t file_bytes[HEX_VIEWER_LINES_ON_SCREEN][HEX_VIEWER_BYTES_PER_LINE];
    uint32_t file_offset;
    uint32_t file_read_bytes;
    uint32_t file_size;
    Stream* stream;
    bool mode; // Print address or content
} HexViewerModel;

typedef struct {
    HexViewerModel* model;
    FuriMutex** mutex;

    FuriMessageQueue* input_queue;

    ViewPort* view_port;
    Gui* gui;
    Storage* storage;
} HexViewer;

static void render_callback(Canvas* canvas, void* ctx) {
    HexViewer* hex_viewer = ctx;
    furi_check(furi_mutex_acquire(hex_viewer->mutex, FuriWaitForever) == FuriStatusOk);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    elements_button_left(canvas, hex_viewer->model->mode ? "Addr" : "Text");
    elements_button_right(canvas, "Info");

    int ROW_HEIGHT = 12;
    int TOP_OFFSET = 10;
    int LEFT_OFFSET = 3;

    uint32_t line_count = hex_viewer->model->file_size / HEX_VIEWER_BYTES_PER_LINE;
    if(hex_viewer->model->file_size % HEX_VIEWER_BYTES_PER_LINE != 0) line_count += 1;
    uint32_t first_line_on_screen = hex_viewer->model->file_offset / HEX_VIEWER_BYTES_PER_LINE;
    if(line_count > HEX_VIEWER_LINES_ON_SCREEN) {
        uint8_t width = canvas_width(canvas);
        elements_scrollbar_pos(
            canvas,
            width,
            0,
            ROW_HEIGHT * HEX_VIEWER_LINES_ON_SCREEN,
            first_line_on_screen, // TODO
            line_count - (HEX_VIEWER_LINES_ON_SCREEN - 1));
    }

    char temp_buf[32];
    uint32_t row_iters = hex_viewer->model->file_read_bytes / HEX_VIEWER_BYTES_PER_LINE;
    if(hex_viewer->model->file_read_bytes % HEX_VIEWER_BYTES_PER_LINE != 0) row_iters += 1;

    for(uint32_t i = 0; i < row_iters; ++i) {
        uint32_t bytes_left_per_row =
            hex_viewer->model->file_read_bytes - i * HEX_VIEWER_BYTES_PER_LINE;
        bytes_left_per_row = MIN(bytes_left_per_row, HEX_VIEWER_BYTES_PER_LINE);

        if(hex_viewer->model->mode) {
            memcpy(temp_buf, hex_viewer->model->file_bytes[i], bytes_left_per_row);
            temp_buf[bytes_left_per_row] = '\0';
            for(uint32_t j = 0; j < bytes_left_per_row; ++j)
                if(!isprint((int)temp_buf[j])) temp_buf[j] = '.';

            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, LEFT_OFFSET, TOP_OFFSET + i * ROW_HEIGHT, temp_buf);
        } else {
            uint32_t addr = hex_viewer->model->file_offset + i * HEX_VIEWER_BYTES_PER_LINE;
            snprintf(temp_buf, 32, "%04lX", addr);

            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, LEFT_OFFSET, TOP_OFFSET + i * ROW_HEIGHT, temp_buf);
        }

        char* p = temp_buf;
        for(uint32_t j = 0; j < bytes_left_per_row; ++j)
            p += snprintf(p, 32, "%02X ", hex_viewer->model->file_bytes[i][j]);

        canvas_set_font(canvas, FontKeyboard);
        canvas_draw_str(canvas, LEFT_OFFSET + 41, TOP_OFFSET + i * ROW_HEIGHT, temp_buf);
    }

    furi_mutex_release(hex_viewer->mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    HexViewer* hex_viewer = ctx;
    if(input_event->type == InputTypeShort || input_event->type == InputTypeRepeat) {
        furi_message_queue_put(hex_viewer->input_queue, input_event, 0);
    }
}

static HexViewer* hex_viewer_alloc() {
    HexViewer* instance = malloc(sizeof(HexViewer));

    instance->model = malloc(sizeof(HexViewerModel));
    memset(instance->model, 0x0, sizeof(HexViewerModel));

    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, render_callback, instance);
    view_port_input_callback_set(instance->view_port, input_callback, instance);

    instance->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    instance->storage = furi_record_open(RECORD_STORAGE);

    return instance;
}

static void hex_viewer_free(HexViewer* instance) {
    furi_record_close(RECORD_STORAGE);

    gui_remove_view_port(instance->gui, instance->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(instance->view_port);

    furi_message_queue_free(instance->input_queue);

    furi_mutex_free(instance->mutex);

    if(instance->model->stream) buffered_file_stream_close(instance->model->stream);

    free(instance->model);
    free(instance);
}

static bool hex_viewer_open_file(HexViewer* hex_viewer, const char* file_path) {
    furi_assert(hex_viewer);
    furi_assert(file_path);

    hex_viewer->model->stream = buffered_file_stream_alloc(hex_viewer->storage);
    bool isOk = true;

    do {
        if(!buffered_file_stream_open(
               hex_viewer->model->stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Unable to open stream: %s", file_path);
            isOk = false;
            break;
        };

        hex_viewer->model->file_size = stream_size(hex_viewer->model->stream);
    } while(false);

    return isOk;
}

static bool hex_viewer_read_file(HexViewer* hex_viewer) {
    furi_assert(hex_viewer);
    furi_assert(hex_viewer->model->stream);
    furi_assert(hex_viewer->model->file_offset % HEX_VIEWER_BYTES_PER_LINE == 0);

    memset(hex_viewer->model->file_bytes, 0x0, HEX_VIEWER_BUF_SIZE);
    bool isOk = true;

    do {
        uint32_t offset = hex_viewer->model->file_offset;
        if(!stream_seek(hex_viewer->model->stream, offset, true)) {
            FURI_LOG_E(TAG, "Unable to seek stream");
            isOk = false;
            break;
        }

        hex_viewer->model->file_read_bytes = stream_read(
            hex_viewer->model->stream,
            (uint8_t*)hex_viewer->model->file_bytes,
            HEX_VIEWER_BUF_SIZE);
    } while(false);

    return isOk;
}

int32_t hex_viewer_app(void* p) {
    HexViewer* hex_viewer = hex_viewer_alloc();

    FuriString* file_path;
    file_path = furi_string_alloc();

    do {
        if(p && strlen(p)) {
            furi_string_set(file_path, (const char*)p);
        } else {
            furi_string_set(file_path, HEX_VIEWER_APP_PATH_FOLDER);

            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options, HEX_VIEWER_APP_EXTENSION, &I_hex_10px);
            browser_options.hide_ext = false;

            DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
            bool res = dialog_file_browser_show(dialogs, file_path, file_path, &browser_options);

            furi_record_close(RECORD_DIALOGS);
            if(!res) {
                FURI_LOG_I(TAG, "No file selected");
                break;
            }
        }

        FURI_LOG_I(TAG, "File selected: %s", furi_string_get_cstr(file_path));

        if(!hex_viewer_open_file(hex_viewer, furi_string_get_cstr(file_path))) break;
        hex_viewer_read_file(hex_viewer);

        InputEvent input;
        while(furi_message_queue_get(hex_viewer->input_queue, &input, FuriWaitForever) ==
              FuriStatusOk) {
            if(input.key == InputKeyBack) {
                break;
            } else if(input.key == InputKeyUp) {
                furi_check(furi_mutex_acquire(hex_viewer->mutex, FuriWaitForever) == FuriStatusOk);
                if(hex_viewer->model->file_offset > 0) {
                    hex_viewer->model->file_offset -= HEX_VIEWER_BYTES_PER_LINE;
                    if(!hex_viewer_read_file(hex_viewer)) break;
                }
                furi_mutex_release(hex_viewer->mutex);
            } else if(input.key == InputKeyDown) {
                furi_check(furi_mutex_acquire(hex_viewer->mutex, FuriWaitForever) == FuriStatusOk);
                uint32_t last_byte_on_screen =
                    hex_viewer->model->file_offset + hex_viewer->model->file_read_bytes;

                if(hex_viewer->model->file_size > last_byte_on_screen) {
                    hex_viewer->model->file_offset += HEX_VIEWER_BYTES_PER_LINE;
                    if(!hex_viewer_read_file(hex_viewer)) break;
                }
                furi_mutex_release(hex_viewer->mutex);
            } else if(input.key == InputKeyLeft) {
                furi_check(furi_mutex_acquire(hex_viewer->mutex, FuriWaitForever) == FuriStatusOk);
                hex_viewer->model->mode = !hex_viewer->model->mode;
                furi_mutex_release(hex_viewer->mutex);
            } else if(input.key == InputKeyRight) {
                FuriString* buffer;
                buffer = furi_string_alloc();
                furi_string_printf(
                    buffer,
                    "File path: %s\nFile size: %lu (0x%lX)",
                    furi_string_get_cstr(file_path),
                    hex_viewer->model->file_size,
                    hex_viewer->model->file_size);

                DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
                DialogMessage* message = dialog_message_alloc();
                dialog_message_set_header(message, "Hex Viewer v1.1", 16, 2, AlignLeft, AlignTop);
                dialog_message_set_icon(message, &I_hex_10px, 3, 2);
                dialog_message_set_text(
                    message, furi_string_get_cstr(buffer), 3, 16, AlignLeft, AlignTop);
                dialog_message_set_buttons(message, NULL, NULL, "Back");
                dialog_message_show(dialogs, message);

                furi_string_free(buffer);
                dialog_message_free(message);
                furi_record_close(RECORD_DIALOGS);
            }

            view_port_update(hex_viewer->view_port);
        }
    } while(false);

    furi_string_free(file_path);
    hex_viewer_free(hex_viewer);

    return 0;
}
