#include <furi.h>
#include <gui/gui.h>
#include <network/network.h>
#include <storage/storage.h>
#include <string.h>

/* Companion-side HTTP download: GET a GIF straight to SD, then play it back.
 *
 * Network API used: network_http_request() WITH save_path - the companion
 * writes the response body to the SD card itself, so the only event is the
 * final NetworkEventHttpResponse (saved_to_file set on success). Compare with
 * bad_apple.c, where the body streams through NetworkEventReceived instead.
 */

#define BAD_APPLE_REQUEST_ID 13
#define BAD_APPLE_URL \
    "https://raw.githubusercontent.com/apfxtech/apfxtech/main/bad_apple_128x64_stretch.gif"
#define BAD_APPLE_TIMEOUT_MS   30000
#define BAD_APPLE_SAVE_PATH    EXT_PATH("bad_apple_128x64.gif")
#define BAD_APPLE_MIN_DELAY_MS 80

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define DISP_W        128
#define GIF_MAX_CODES 4096
#define GIF_STACK_MAX 4096

typedef uint8_t Framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

typedef enum {
    BadAppleInit,
    BadAppleNoBridge,
    BadAppleDownloading,
    BadAppleDownloaded,
    BadApplePlaying,
    BadAppleDone,
    BadAppleError,
} BadAppleState;

typedef struct {
    FuriMutex* mutex;
    BadAppleState state;
    NetworkError error;
    uint32_t http_status;
    uint32_t frames;
    uint16_t delay_ms;
    char status_message[32];
    bool play_requested;
    Framebuffer fb;
    Storage* storage;
    File* file;
} BadAppleApp;

static void bad_apple_pack_ssd(const Framebuffer fb, uint8_t* ssd) {
    for(int page = 0; page < (SCREEN_HEIGHT / 8); ++page) {
        const uint8_t* r0 = fb[page * 8 + 0];
        const uint8_t* r1 = fb[page * 8 + 1];
        const uint8_t* r2 = fb[page * 8 + 2];
        const uint8_t* r3 = fb[page * 8 + 3];
        const uint8_t* r4 = fb[page * 8 + 4];
        const uint8_t* r5 = fb[page * 8 + 5];
        const uint8_t* r6 = fb[page * 8 + 6];
        const uint8_t* r7 = fb[page * 8 + 7];
        uint8_t* out = ssd + page * DISP_W;
        for(int col = 0; col < SCREEN_WIDTH; ++col) {
            out[col] =
                (uint8_t)((r0[col] != 0) | ((r1[col] != 0) << 1) | ((r2[col] != 0) << 2) |
                          ((r3[col] != 0) << 3) | ((r4[col] != 0) << 4) | ((r5[col] != 0) << 5) |
                          ((r6[col] != 0) << 6) | ((r7[col] != 0) << 7));
        }
    }
}

/* Called from the RPC session thread; demuxed by the request id. */
static void bad_apple_event_callback(const NetworkEvent* event, void* context) {
    if(event->connection_id != BAD_APPLE_REQUEST_ID) return;
    BadAppleApp* app = context;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    /* save_path is set, so the companion writes the body to SD itself: there are
     * no NetworkEventReceived chunks to handle, only the final response. */
    if(event->type == NetworkEventHttpResponse) {
        app->http_status = event->http_status;
        if(event->error != NetworkErrorNone) {
            app->state = BadAppleError;
            app->error = event->error;
        } else if(event->http_status != 200) {
            app->state = BadAppleError;
            app->error = NetworkErrorInternal;
            snprintf(
                app->status_message,
                sizeof(app->status_message),
                "HTTP %lu",
                (unsigned long)event->http_status);
        } else if(!event->saved_to_file) {
            app->state = BadAppleError;
            app->error = NetworkErrorFileError;
        } else {
            app->state = BadAppleDownloaded;
            app->play_requested = true;
        }
    }
    furi_mutex_release(app->mutex);
}

static void bad_apple_draw_callback(Canvas* canvas, void* context) {
    BadAppleApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    if(app->state == BadApplePlaying || app->state == BadAppleDone) {
        uint8_t* ssd = canvas_get_buffer(canvas);
        if(ssd) bad_apple_pack_ssd(app->fb, ssd);
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 11, AlignCenter, AlignBottom, "Bad Apple (save)");
        canvas_set_font(canvas, FontSecondary);
        const char* status = "Starting...";
        if(app->state == BadAppleNoBridge)
            status = "No USB/BLE connection";
        else if(app->state == BadAppleDownloading)
            status = "Downloading on companion...";
        else if(app->state == BadAppleDownloaded)
            status = "Preparing playback...";
        else if(app->state == BadAppleError) {
            status = app->status_message[0] ? app->status_message :
                                              network_error_to_string(app->error);
        }
        canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignBottom, status);
        canvas_draw_str_aligned(canvas, 64, 58, AlignCenter, AlignBottom, BAD_APPLE_SAVE_PATH);
    }

    furi_mutex_release(app->mutex);
}

static void bad_apple_input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

typedef struct {
    File* file;
    uint8_t block[255];
    uint8_t block_size;
    uint8_t block_pos;
    uint8_t bit_count;
    uint32_t bit_buffer;
} GifStream;

typedef struct {
    uint16_t* prefix;
    uint8_t* suffix;
    uint8_t* stack;
} GifWork;

static bool gif_read_u8(File* file, uint8_t* value) {
    return storage_file_read(file, value, 1) == 1;
}

static bool gif_read_le16(File* file, uint16_t* value) {
    uint8_t b[2];
    if(storage_file_read(file, b, sizeof(b)) != sizeof(b)) return false;
    *value = (uint16_t)b[0] | ((uint16_t)b[1] << 8);
    return true;
}

static bool gif_skip(File* file, uint32_t bytes) {
    return storage_file_seek(file, storage_file_tell(file) + bytes, true);
}

static bool gif_skip_sub_blocks(File* file) {
    uint8_t size = 0;
    do {
        if(!gif_read_u8(file, &size)) return false;
        if(size && !gif_skip(file, size)) return false;
    } while(size);
    return true;
}

static bool gif_stream_next_byte(GifStream* stream, uint8_t* value) {
    if(stream->block_pos >= stream->block_size) {
        if(!gif_read_u8(stream->file, &stream->block_size)) return false;
        stream->block_pos = 0;
        if(stream->block_size == 0) return false;
        if(storage_file_read(stream->file, stream->block, stream->block_size) !=
           stream->block_size)
            return false;
    }
    *value = stream->block[stream->block_pos++];
    return true;
}

static int32_t gif_stream_read_code(GifStream* stream, uint8_t code_size) {
    while(stream->bit_count < code_size) {
        uint8_t next = 0;
        if(!gif_stream_next_byte(stream, &next)) return -1;
        stream->bit_buffer |= ((uint32_t)next) << stream->bit_count;
        stream->bit_count += 8;
    }

    uint32_t code = stream->bit_buffer & ((1UL << code_size) - 1);
    stream->bit_buffer >>= code_size;
    stream->bit_count -= code_size;
    return code;
}

static uint8_t gif_palette_luma(const uint8_t* palette, uint16_t index) {
    const uint8_t* rgb = palette + index * 3;
    return (uint8_t)(((uint16_t)rgb[0] * 30 + (uint16_t)rgb[1] * 59 + (uint16_t)rgb[2] * 11) /
                     100);
}

static bool gif_draw_index(
    BadAppleApp* app,
    uint16_t* pixel,
    uint16_t left,
    uint16_t top,
    uint16_t width,
    uint16_t height,
    bool interlace,
    const uint8_t* palette,
    uint16_t palette_size,
    int16_t transparent,
    uint8_t color_index) {
    if(*pixel >= (uint32_t)width * height) return true;
    if(color_index < palette_size && color_index != transparent) {
        uint16_t row = *pixel / width;
        uint16_t col = *pixel % width;
        if(interlace) {
            static const uint8_t starts[] = {0, 4, 2, 1};
            static const uint8_t steps[] = {8, 8, 4, 2};
            uint16_t mapped = row;
            uint16_t count = 0;
            for(uint8_t pass = 0; pass < 4; pass++) {
                for(uint16_t y = starts[pass]; y < height; y += steps[pass]) {
                    if(count++ == row) {
                        mapped = y;
                        pass = 4;
                        break;
                    }
                }
            }
            row = mapped;
        }
        uint16_t x = left + col;
        uint16_t y = top + row;
        if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
            app->fb[y][x] = gif_palette_luma(palette, color_index) < 128;
        }
    }
    (*pixel)++;
    return *pixel >= (uint32_t)width * height;
}

static bool gif_decode_image(
    BadAppleApp* app,
    File* file,
    GifWork* work,
    uint16_t left,
    uint16_t top,
    uint16_t width,
    uint16_t height,
    bool interlace,
    const uint8_t* palette,
    uint16_t palette_size,
    int16_t transparent) {
    uint8_t min_code_size = 0;
    if(!gif_read_u8(file, &min_code_size) || min_code_size > 8) return false;

    GifStream stream = {.file = file};
    uint16_t* prefix = work->prefix;
    uint8_t* suffix = work->suffix;
    uint8_t* stack = work->stack;

    int32_t clear_code = 1 << min_code_size;
    int32_t end_code = clear_code + 1;
    int32_t next_code = end_code + 1;
    uint8_t code_size = min_code_size + 1;
    int32_t old_code = -1;
    uint8_t first = 0;
    uint16_t pixel = 0;

    for(int32_t i = 0; i < clear_code; i++) {
        prefix[i] = 0;
        suffix[i] = i;
    }

    for(;;) {
        int32_t code = gif_stream_read_code(&stream, code_size);
        if(code < 0) break;
        if(code == clear_code) {
            code_size = min_code_size + 1;
            next_code = end_code + 1;
            old_code = -1;
            continue;
        }
        if(code == end_code) break;

        int32_t in_code = code;
        uint16_t stack_len = 0;
        if(code >= next_code) {
            if(old_code < 0) {
                return false;
            }
            stack[stack_len++] = first;
            code = old_code;
        }

        while(code >= clear_code) {
            if(code >= GIF_MAX_CODES || stack_len >= GIF_STACK_MAX) {
                return false;
            }
            stack[stack_len++] = suffix[code];
            code = prefix[code];
        }

        first = suffix[code];
        stack[stack_len++] = first;

        while(stack_len) {
            if(gif_draw_index(
                   app,
                   &pixel,
                   left,
                   top,
                   width,
                   height,
                   interlace,
                   palette,
                   palette_size,
                   transparent,
                   stack[--stack_len])) {
                break;
            }
        }

        if(old_code >= 0 && next_code < GIF_MAX_CODES) {
            prefix[next_code] = old_code;
            suffix[next_code] = first;
            next_code++;
            if(next_code == (1 << code_size) && code_size < 12) code_size++;
        }
        old_code = in_code;
        if(pixel >= (uint32_t)width * height) break;
    }

    gif_skip_sub_blocks(file);
    return true;
}

static bool
    bad_apple_play_gif(BadAppleApp* app, ViewPort* view_port, FuriMessageQueue* event_queue) {
    if(!storage_file_open(app->file, BAD_APPLE_SAVE_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
        return false;

    uint8_t header[13];
    bool ok = storage_file_read(app->file, header, sizeof(header)) == sizeof(header) &&
              memcmp(header, "GIF", 3) == 0;
    uint8_t global_palette[256 * 3];
    uint16_t global_palette_size = 0;
    if(ok && (header[10] & 0x80)) {
        global_palette_size = 1 << ((header[10] & 0x07) + 1);
        ok = storage_file_read(app->file, global_palette, global_palette_size * 3) ==
             global_palette_size * 3;
    }
    uint32_t frames_start = storage_file_tell(app->file);

    uint16_t delay_ms = 80;
    int16_t transparent = -1;
    bool processing = true;
    GifWork work = {
        .prefix = malloc(sizeof(uint16_t) * GIF_MAX_CODES),
        .suffix = malloc(sizeof(uint8_t) * GIF_MAX_CODES),
        .stack = malloc(GIF_STACK_MAX),
    };
    if(!work.prefix || !work.suffix || !work.stack) ok = false;
    while(ok && processing) {
        InputEvent event;
        if(furi_message_queue_get(event_queue, &event, 1) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyBack) {
                processing = false;
                break;
            }
        }

        uint8_t sep = 0;
        if(!gif_read_u8(app->file, &sep)) break;
        if(sep == 0x3B) {
            if(frames_start && storage_file_seek(app->file, frames_start, true)) {
                delay_ms = BAD_APPLE_MIN_DELAY_MS;
                transparent = -1;
                continue;
            }
            break;
        }
        if(sep == 0x21) {
            uint8_t label = 0;
            if(!gif_read_u8(app->file, &label)) {
                ok = false;
                break;
            }
            if(label == 0xF9) {
                uint8_t block_size = 0;
                uint8_t packed = 0;
                uint16_t delay_cs = 0;
                uint8_t trans = 0;
                uint8_t terminator = 0;
                ok = gif_read_u8(app->file, &block_size) && block_size == 4 &&
                     gif_read_u8(app->file, &packed) && gif_read_le16(app->file, &delay_cs) &&
                     gif_read_u8(app->file, &trans) && gif_read_u8(app->file, &terminator);
                if(!ok) break;
                delay_ms = delay_cs ? delay_cs * 10 : BAD_APPLE_MIN_DELAY_MS;
                if(delay_ms < BAD_APPLE_MIN_DELAY_MS) delay_ms = BAD_APPLE_MIN_DELAY_MS;
                transparent = (packed & 0x01) ? trans : -1;
            } else {
                ok = gif_skip_sub_blocks(app->file);
            }
        } else if(sep == 0x2C) {
            uint16_t left = 0;
            uint16_t top = 0;
            uint16_t width = 0;
            uint16_t height = 0;
            uint8_t packed = 0;
            ok = gif_read_le16(app->file, &left) && gif_read_le16(app->file, &top) &&
                 gif_read_le16(app->file, &width) && gif_read_le16(app->file, &height) &&
                 gif_read_u8(app->file, &packed);
            if(!ok) break;

            uint8_t local_palette[256 * 3];
            const uint8_t* palette = global_palette;
            uint16_t palette_size = global_palette_size;
            if(packed & 0x80) {
                palette_size = 1 << ((packed & 0x07) + 1);
                ok = storage_file_read(app->file, local_palette, palette_size * 3) ==
                     palette_size * 3;
                palette = local_palette;
            }
            if(!ok || !palette_size) break;

            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->state = BadApplePlaying;
            app->delay_ms = delay_ms;
            furi_mutex_release(app->mutex);

            ok = gif_decode_image(
                app,
                app->file,
                &work,
                left,
                top,
                width,
                height,
                packed & 0x40,
                palette,
                palette_size,
                transparent);
            if(!ok) break;

            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->frames++;
            furi_mutex_release(app->mutex);
            view_port_update(view_port);
            furi_delay_ms(delay_ms);
        } else {
            ok = false;
        }
    }

    free(work.prefix);
    free(work.suffix);
    free(work.stack);
    storage_file_close(app->file);
    return ok;
}

int32_t network_bad_apple_save_app(void* p) {
    UNUSED(p);

    BadAppleApp* app = malloc(sizeof(BadAppleApp));
    memset(app, 0, sizeof(BadAppleApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->state = BadAppleInit;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->storage = furi_record_open(RECORD_STORAGE);
    app->file = storage_file_alloc(app->storage);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_set_event_callback(network, bad_apple_event_callback, app);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, bad_apple_draw_callback, app);
    view_port_input_callback_set(view_port, bad_apple_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    /* save_path tells the companion to write the response body to the SD card
     * itself; the firmware only waits for the final NetworkEventHttpResponse. */
    const NetworkHttpRequest request = {
        .method = NetworkHttpMethodGet,
        .url = BAD_APPLE_URL,
        .save_path = BAD_APPLE_SAVE_PATH,
        .timeout_ms = BAD_APPLE_TIMEOUT_MS,
    };

    /* false = no live companion session (USB/BLE); the download runs on the
     * companion and finishes in the event callback. */
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    bool requested = network_http_request(network, BAD_APPLE_REQUEST_ID, &request);
    app->state = requested ? BadAppleDownloading : BadAppleNoBridge;
    furi_mutex_release(app->mutex);

    InputEvent event;
    for(bool processing = true; processing;) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyBack) processing = false;
        }

        furi_mutex_acquire(app->mutex, FuriWaitForever);
        bool play_requested = app->play_requested;
        app->play_requested = false;
        furi_mutex_release(app->mutex);

        if(play_requested) {
            if(storage_file_is_open(app->file)) storage_file_close(app->file);
            bool ok = bad_apple_play_gif(app, view_port, event_queue);
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->state = ok ? BadAppleDone : BadAppleError;
            if(!ok) app->error = NetworkErrorInternal;
            furi_mutex_release(app->mutex);
        }

        view_port_update(view_port);
    }

    /* Unsubscribe before closing the record so a late event cannot reach a
     * freed context. */
    network_set_event_callback(network, NULL, NULL);
    furi_record_close(RECORD_NETWORK);

    if(storage_file_is_open(app->file)) storage_file_close(app->file);
    storage_file_free(app->file);
    furi_record_close(RECORD_STORAGE);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
