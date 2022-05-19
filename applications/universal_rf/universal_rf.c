#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <flipper_format/flipper_format_i.h>
#include <string.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/toolbox/path.h>
#include <notification/notification_messages.h>

#define TAG "UniveralRFRemote"

typedef struct {
    bool press[5];
} RemoteAppState;

static void remote_reset_state(RemoteAppState* state) {
    state->press[0] = 0;
    state->press[1] = 0;
    state->press[2] = 0;
    state->press[3] = 0;
    state->press[4] = 0;
}
static string_t up_file;
static string_t down_file;
static string_t left_file;
static string_t right_file;
static string_t ok_file;

static char* subString(char* someString, int n) {
    char* new = malloc(sizeof(char) * n + 1);
    strncpy(new, someString, n);
    new[n] = '\0';
    return(new);
}

static char* file_stub(const char* file_name) {
    string_t filename;
    string_init(filename);
    path_extract_filename_no_ext(file_name, filename);

    return(subString((char*)string_get_cstr(filename), 8));
}

static void remote_send_signal(uint32_t frequency, string_t signal, string_t protocol) {
    uint32_t repeat = 1;
    frequency = frequency ? frequency : 433920000;
    FURI_LOG_D(TAG, "file to send: %s", string_get_cstr(signal));

    if(strlen(string_get_cstr(signal)) < 10) {
        return;
    }

    string_t flipper_format_string;
    if(strcmp(string_get_cstr(protocol), "RAW") == 0) {
        string_init_printf(flipper_format_string, "File_name: %s", string_get_cstr(signal));
    } else {
        return;
    }
    NotificationApp* notification = furi_record_open("notification");
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    Stream* stream = flipper_format_get_raw_stream(flipper_format);
    stream_clean(stream);
    stream_write_cstring(stream, string_get_cstr(flipper_format_string));

    SubGhzEnvironment* environment = subghz_environment_alloc();

    SubGhzTransmitter* transmitter =
        subghz_transmitter_alloc_init(environment, string_get_cstr(protocol));
    subghz_transmitter_deserialize(transmitter, flipper_format);

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok270Async);
    furi_hal_subghz_set_frequency_and_path(frequency);
    FURI_LOG_D(
        TAG, "Transmitting at %lu, repeat %lu. Press CTRL+C to stop\r\n", frequency, repeat);

    furi_hal_power_suppress_charge_enter();
    notification_message(notification, &sequence_set_vibro_on);

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, transmitter);

    while(!(furi_hal_subghz_is_async_tx_complete())) {
        FURI_LOG_D(TAG, ".");
        fflush(stdout);
        osDelay(333);
    }
    notification_message(notification, &sequence_reset_vibro);

    furi_record_close("notification");

    furi_hal_subghz_stop_async_tx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    flipper_format_free(flipper_format);
    subghz_transmitter_free(transmitter);
    subghz_environment_free(environment);
}

static void remote_render_callback(Canvas* canvas, void* ctx) {
    RemoteAppState* state = (RemoteAppState*)acquire_mutex((ValueMutex*)ctx, 25);
    canvas_clear(canvas);
    char strings[5][20];
    string_t signal;
    string_init(signal);
    sprintf(strings[0], "Ok: %s", file_stub(string_get_cstr(ok_file)));
    sprintf(strings[1], "L: %s", file_stub(string_get_cstr(left_file)));
    sprintf(strings[2], "R: %s", file_stub(string_get_cstr(right_file)));
    sprintf(strings[3], "U: %s", file_stub(string_get_cstr(up_file)));
    sprintf(strings[4], "D: %s", file_stub(string_get_cstr(down_file)));

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Univeral Remote");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 24, strings[1]);
    canvas_draw_str(canvas, 85, 24, strings[2]);
    canvas_draw_str(canvas, 0, 36, strings[3]);
    canvas_draw_str(canvas, 85, 36, strings[4]);
    canvas_draw_str(canvas, 0, 48, strings[0]);

    if(state->press[0]) {
        string_cat_printf(signal, "%s", string_get_cstr(right_file));
    }

    else if(state->press[1]) {
        string_cat_printf(signal, "%s", string_get_cstr(left_file));

    } else if(state->press[2]) {
        string_cat_printf(signal, "%s", string_get_cstr(up_file));

    } else if(state->press[3]) {
        string_cat_printf(signal, "%s", string_get_cstr(down_file));

    }

    else if(state->press[4]) {
        string_cat_printf(signal, "%s", string_get_cstr(ok_file));
    }
    FURI_LOG_D(TAG, "signal = %s", string_get_cstr(signal));

    if(strlen(string_get_cstr(signal)) > 12) {
        string_t file_name;
        string_init(file_name);
        string_t protocol;
        string_init(protocol);
        string_set(file_name, string_get_cstr(signal));
        Storage* storage = furi_record_open("storage");
        FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
        uint32_t frequency_str;
        flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name));
        flipper_format_read_uint32(fff_data_file, "Frequency", (uint32_t*)&frequency_str, 1);
        if(!flipper_format_read_string(fff_data_file, "Protocol", protocol)) {
            FURI_LOG_D(TAG, "Could not read Protocol");
            string_set(protocol, "RAW");
        }
        flipper_format_free(fff_data_file);
        furi_record_close("storage");
        FURI_LOG_D(TAG, "%lu", frequency_str);
        remote_send_signal(frequency_str, signal, protocol);
    }

    canvas_draw_str(canvas, 10, 63, "[back] - skip, hold to exit");
    remote_reset_state(state);
    release_mutex((ValueMutex*)ctx, state);
}

static void remote_input_callback(InputEvent* input_event, void* ctx) {
	if (input_event->type == InputTypeRelease) {
		osMessageQueueId_t event_queue = ctx;
		osMessageQueuePut(event_queue, input_event, 0, osWaitForever);
	}
}

int32_t universal_rf_remote_app(void* p) {
    UNUSED(p);
    osMessageQueueId_t event_queue = osMessageQueueNew(32, sizeof(InputEvent), NULL);
    furi_check(event_queue);
    string_init(up_file);
    string_init(down_file);
    string_init(left_file);
    string_init(right_file);
    string_init(ok_file);

    string_t file_name;
    string_init(file_name);
    string_set(file_name, "/ext/subghz/assets/universal_rf_map");
    Storage* storage = furi_record_open("storage");
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
        FURI_LOG_D(TAG, "Could not open file %s", string_get_cstr(file_name));
    }

    if(!flipper_format_read_string(fff_data_file, "UP", up_file)) {
        FURI_LOG_D(TAG, "Could not read UP string");
    }
    if(!flipper_format_read_string(fff_data_file, "DOWN", down_file)) {
        FURI_LOG_D(TAG, "Could not read DOWN string");
    }
    if(!flipper_format_read_string(fff_data_file, "LEFT", left_file)) {
        FURI_LOG_D(TAG, "Could not read LEFT string");
    }
    if(!flipper_format_read_string(fff_data_file, "RIGHT", right_file)) {
        FURI_LOG_D(TAG, "Could not read RIGHT string");
    }
    if(!flipper_format_read_string(fff_data_file, "OK", ok_file)) {
        FURI_LOG_D(TAG, "Could not read OK string");
    }
    flipper_format_free(fff_data_file);
    furi_record_close("storage");
    FURI_LOG_I(
        TAG,
        "%s %s %s %s %s ",
        string_get_cstr(up_file),
        string_get_cstr(down_file),
        string_get_cstr(left_file),
        string_get_cstr(right_file),
        string_get_cstr(ok_file));

    RemoteAppState _state = {{false, false, false, false, false}};

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(RemoteAppState))) {
        FURI_LOG_D(TAG, "cannot create mutex");
        return(0);
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, remote_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, remote_input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    while(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK) {
        RemoteAppState* state = (RemoteAppState*)acquire_mutex_block(&state_mutex);
        FURI_LOG_D(
            TAG,
            "key: %s type: %s",
            input_get_key_name(event.key),
            input_get_type_name(event.type));

        if(event.key == InputKeyRight) {
                state->press[0] = true;
        } else if(event.key == InputKeyLeft) {
                state->press[1] = true;
        } else if(event.key == InputKeyUp) {
                state->press[2] = true;
        } else if(event.key == InputKeyDown) {
                state->press[3] = true;
        } else if(event.key == InputKeyOk) {
                state->press[4] = true;
        } else if(event.key == InputKeyBack) {
                release_mutex(&state_mutex, state);
                break;
        }
        release_mutex(&state_mutex, state);
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    delete_mutex(&state_mutex);

    furi_record_close("gui");

    return(0);
}