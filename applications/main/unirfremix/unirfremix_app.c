#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/path.h>
#include <applications/main/subghz/subghz_i.h>

#include <lib/subghz/protocols/raw.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/types.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/star_line.h>

#include <SubGHz_Remote_icons.h>

#define UNIRFMAP_FOLDER "/ext/subghz/unirf"
#define UNIRFMAP_EXTENSION ".txt"

#define TAG "UniRF Remix"

static const char* mfname;

static int kl_type;

void keeloq_reset_mfname() {
    mfname = "";
}

void keeloq_reset_kl_type() {
    kl_type = 0;
}

void star_line_reset_mfname() {
    mfname = "";
}

void star_line_reset_kl_type() {
    kl_type = 0;
}

typedef struct {
    uint32_t frequency;
    FuriString* name;

    FuriString* protocol;
    uint32_t repeat;

    uint8_t* data;
    size_t data_size;

    SubGhzProtocolDecoderBase* decoder;
} UniRFPreset;

typedef struct {
    FuriMutex* model_mutex;

    FuriMessageQueue* input_queue;

    ViewPort* view_port;
    Gui* gui;

    SubGhzSetting* setting;
    SubGhzEnvironment* environment;
    SubGhzReceiver* subghz_receiver;
    NotificationApp* notification;
    UniRFPreset* txpreset;

    FuriString* up_file;
    FuriString* down_file;
    FuriString* left_file;
    FuriString* right_file;
    FuriString* ok_file;

    FuriString* file_path;

    char* up_label;
    char* down_label;
    char* left_label;
    char* right_label;
    char* ok_label;

    int up_enabled;
    int down_enabled;
    int left_enabled;
    int right_enabled;
    int ok_enabled;

    char* send_status;
    int send_status_c;
    int processing;

    SubGhzTransmitter* tx_transmitter;
    FlipperFormat* tx_fff_data;
    const char* tx_file_path;
    int button;

    int file_result;
    bool tx_not_allowed;

    FuriString* signal;
} UniRFRemix;

UniRFPreset* unirfremix_preset_alloc(void) {
    UniRFPreset* preset = malloc(sizeof(UniRFPreset));
    preset->name = furi_string_alloc();
    preset->protocol = furi_string_alloc();
    preset->repeat = 200;
    return preset;
}

void unirfremix_preset_free(UniRFPreset* preset) {
    furi_string_free(preset->name);
    furi_string_free(preset->protocol);
    free(preset);
}

static char* char_to_str(char* str, int i) {
    char* converted = malloc(sizeof(char) * i + 1);
    memcpy(converted, str, i);

    converted[i] = '\0';

    return converted;
}

//get filename without path
static char* extract_filename(const char* name, int len) {
    FuriString* tmp;
    tmp = furi_string_alloc();

    //remove path
    path_extract_filename_no_ext(name, tmp);

    return char_to_str((char*)furi_string_get_cstr(tmp), len);
}

static void cfg_read_file_path(
    FlipperFormat* fff_file,
    FuriString* text_file_path,
    char** text_file_label,
    const char* read_key,
    int* is_enabled) {
    if(!flipper_format_read_string(fff_file, read_key, text_file_path)) {
        FURI_LOG_W(TAG, "Could not read %s string", read_key);
        *text_file_label = "N/A";
        *is_enabled = 0;
    } else {
        *text_file_label = extract_filename(furi_string_get_cstr(text_file_path), 16);
        FURI_LOG_D(TAG, "%s file: %s", read_key, furi_string_get_cstr(text_file_path));
        *is_enabled = 1;
    }
    flipper_format_rewind(fff_file);
}

static void cfg_read_file_label(
    FlipperFormat* fff_file,
    char** text_file_label,
    const char* read_key,
    bool is_enabled) {
    FuriString* temp_label = furi_string_alloc();

    if(!flipper_format_read_string(fff_file, read_key, temp_label)) {
        FURI_LOG_W(TAG, "Could not read %s string", read_key);
    } else {
        if(is_enabled == 1) {
            *text_file_label = char_to_str((char*)furi_string_get_cstr(temp_label), 16);
        }
        FURI_LOG_D(TAG, "%s label: %s", read_key, *text_file_label);
    }
    flipper_format_rewind(fff_file);
    furi_string_free(temp_label);
}

/*
 * check that map file exists
 * assign variables to values within map file
 * set missing filenames to N/A
 * set filename as label if label definitions are missing
 * set error flag if all buttons are N/A
 * set error flag if missing map file
 */

void unirfremix_cfg_set_check(UniRFRemix* app, FuriString* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    app->file_result = 1;

    app->up_enabled = 0;
    app->down_enabled = 0;
    app->left_enabled = 0;
    app->right_enabled = 0;
    app->ok_enabled = 0;

    //check that map file exists
    if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", furi_string_get_cstr(file_name));
    } else {
        //Filename Assignment/Check Start

        //assign variables to values within map file
        //set missing filenames to N/A
        cfg_read_file_path(fff_data_file, app->up_file, &app->up_label, "UP", &app->up_enabled);

        cfg_read_file_path(
            fff_data_file, app->down_file, &app->down_label, "DOWN", &app->down_enabled);

        cfg_read_file_path(
            fff_data_file, app->left_file, &app->left_label, "LEFT", &app->left_enabled);

        cfg_read_file_path(
            fff_data_file, app->right_file, &app->right_label, "RIGHT", &app->right_enabled);

        cfg_read_file_path(fff_data_file, app->ok_file, &app->ok_label, "OK", &app->ok_enabled);

        //File definitions are done.
        //File checks will follow after label assignment in order to close the universal_rf_map file without the need to reopen it again.

        //Label Assignment/Check Start

        cfg_read_file_label(fff_data_file, &app->up_label, "ULABEL", app->up_enabled);
        cfg_read_file_label(fff_data_file, &app->down_label, "DLABEL", app->down_enabled);
        cfg_read_file_label(fff_data_file, &app->left_label, "LLABEL", app->left_enabled);
        cfg_read_file_label(fff_data_file, &app->right_label, "RLABEL", app->right_enabled);
        cfg_read_file_label(fff_data_file, &app->ok_label, "OKLABEL", app->ok_enabled);
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    //File Existence Check
    //Check each file definition if not already set to "N/A"

    //determine if files exist.
    //determine whether or not to continue to launch app with missing variables
    //if 5 files are missing, throw error

    //if button is still enabled, check that file exists
    if(app->up_enabled == 1) {
        furi_string_set(file_name, app->up_file);
        fff_data_file = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            FURI_LOG_W(TAG, "Could not open UP file %s", furi_string_get_cstr(file_name));

            //disable button, and set label to "N/A"
            app->up_enabled = 0;
            app->up_label = "N/A";
        }

        //close the file
        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
    }

    if(app->down_enabled == 1) {
        furi_string_set(file_name, app->down_file);
        fff_data_file = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            FURI_LOG_W(TAG, "Could not open DOWN file %s", furi_string_get_cstr(file_name));

            app->down_enabled = 0;
            app->down_label = "N/A";
        }

        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
    }

    if(app->left_enabled == 1) {
        furi_string_set(file_name, app->left_file);
        fff_data_file = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            FURI_LOG_W(TAG, "Could not open LEFT file %s", furi_string_get_cstr(file_name));

            app->left_enabled = 0;
            app->left_label = "N/A";
        }

        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
    }

    if(app->right_enabled == 1) {
        furi_string_set(file_name, app->right_file);
        fff_data_file = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            FURI_LOG_W(TAG, "Could not open RIGHT file %s", furi_string_get_cstr(file_name));

            app->right_enabled = 0;
            app->right_label = "N/A";
        }

        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
    }

    if(app->ok_enabled == 1) {
        furi_string_set(file_name, app->ok_file);
        fff_data_file = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            FURI_LOG_W(TAG, "Could not open OK file %s", furi_string_get_cstr(file_name));

            app->ok_enabled = 0;
            app->ok_label = "N/A";
        }

        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
    }

    furi_record_close(RECORD_STORAGE);

    if(app->up_enabled == 0 && app->down_enabled == 0 && app->left_enabled == 0 &&
       app->right_enabled == 0 && app->ok_enabled == 0) {
        app->file_result = 1;
    } else {
        app->file_result = 2;
    }
}

static void unirfremix_end_send(UniRFRemix* app) {
    app->processing = 0;
}

bool unirfremix_set_preset(UniRFPreset* p, const char* preset) {
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        furi_string_set(p->name, "AM270");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        furi_string_set(p->name, "AM650");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        furi_string_set(p->name, "FM238");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        furi_string_set(p->name, "FM476");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        FURI_LOG_E(TAG, "Custom preset unsupported now");
        return false;
        // furi_string_set(p->name, "CUSTOM");
    } else {
        FURI_LOG_E(TAG, "Unsupported preset");
        return false;
    }
    return true;
}

bool unirfremix_key_load(
    UniRFPreset* preset,
    FlipperFormat* fff_file,
    FlipperFormat* fff_data,
    SubGhzSetting* setting,
    SubGhzReceiver* receiver,
    const char* path) {
    //
    if(!flipper_format_rewind(fff_file)) {
        FURI_LOG_E(TAG, "Rewind error");
        return false;
    }

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    bool res = false;

    do {
        // load frequency from file
        if(!flipper_format_read_uint32(fff_file, "Frequency", &preset->frequency, 1)) {
            FURI_LOG_W(TAG, "Cannot read frequency. Defaulting to 433.92 MHz");
            preset->frequency = 433920000;
        }

        // load preset from file
        if(!flipper_format_read_string(fff_file, "Preset", temp_str)) {
            FURI_LOG_W(TAG, "Could not read Preset. Defaulting to Ook650Async");
            furi_string_set(temp_str, "FuriHalSubGhzPresetOok650Async");
        }
        if(!unirfremix_set_preset(preset, furi_string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Could not set preset");
            break;
        }
        if(!strcmp(furi_string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            // TODO: check if preset is custom
            FURI_LOG_E(TAG, "Could not use custom preset");
            break;
        }
        size_t preset_index =
            subghz_setting_get_inx_preset_by_name(setting, furi_string_get_cstr(preset->name));
        preset->data = subghz_setting_get_preset_data(setting, preset_index);
        preset->data_size = subghz_setting_get_preset_data_size(setting, preset_index);

        // load protocol from file
        if(!flipper_format_read_string(fff_file, "Protocol", preset->protocol)) {
            FURI_LOG_E(TAG, "Could not read Protocol.");
            break;
        }
        if(!furi_string_cmp_str(preset->protocol, "RAW")) {
            subghz_protocol_raw_gen_fff_data(fff_data, path);
        } else {
            stream_copy_full(
                flipper_format_get_raw_stream(fff_file), flipper_format_get_raw_stream(fff_data));
        }

        // repeat
        if(!flipper_format_insert_or_update_uint32(fff_file, "Repeat", &preset->repeat, 1)) {
            FURI_LOG_E(TAG, "Unable to insert or update Repeat");
            break;
        }

        preset->decoder = subghz_receiver_search_decoder_base_by_name(
            receiver, furi_string_get_cstr(preset->protocol));
        if(preset->decoder) {
            if(!subghz_protocol_decoder_base_deserialize(preset->decoder, fff_data)) {
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol %s not found", furi_string_get_cstr(temp_str));
        }

        res = true;
    } while(0);

    furi_string_free(temp_str);

    return res;
}

// method modified from subghz_i.c
// https://github.com/flipperdevices/flipperzero-firmware/blob/b0daa601ad5b87427a45f9089c8b403a01f72c2a/applications/subghz/subghz_i.c#L417-L456
bool unirfremix_save_protocol_to_file(FlipperFormat* fff_file, const char* dev_file_name) {
    furi_assert(fff_file);
    furi_assert(dev_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(fff_file);

    bool saved = false;
    FuriString* file_dir;
    file_dir = furi_string_alloc();

    path_extract_dirname(dev_file_name, file_dir);
    do {
        flipper_format_delete_key(fff_file, "Repeat");
        //flipper_format_delete_key(fff_file, "Manufacture");

        if(!storage_simply_mkdir(storage, furi_string_get_cstr(file_dir))) {
            FURI_LOG_E(TAG, "(save) Cannot mkdir");
            break;
        }

        if(!storage_simply_remove(storage, dev_file_name)) {
            FURI_LOG_E(TAG, "(save) Cannot remove");
            break;
        }

        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, dev_file_name, FSOM_CREATE_ALWAYS);

        saved = true;
        FURI_LOG_D(TAG, "(save) OK Save");
    } while(0);
    furi_string_free(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void unirfremix_tx_stop(UniRFRemix* app) {
    if(app->processing == 0) {
        return;
    }

    if(!furi_string_cmp_str(app->txpreset->protocol, "RAW")) {
        while(!furi_hal_subghz_is_async_tx_complete()) {
            furi_delay_ms(15);
        }
    }

    //Stop TX
    furi_hal_subghz_stop_async_tx();
    //FURI_LOG_I(TAG, "TX Done!");
    subghz_transmitter_stop(app->tx_transmitter);

    FURI_LOG_D(TAG, "Checking if protocol is dynamic");
    const SubGhzProtocolRegistry* protocol_registry_items =
        subghz_environment_get_protocol_registry(app->environment);
    const SubGhzProtocol* proto = subghz_protocol_registry_get_by_name(
        protocol_registry_items, furi_string_get_cstr(app->txpreset->protocol));
    FURI_LOG_D(TAG, "Protocol-TYPE %d", proto->type);
    if(proto && proto->type == SubGhzProtocolTypeDynamic) {
        FURI_LOG_D(TAG, "Protocol is dynamic. Saving key");
        unirfremix_save_protocol_to_file(app->tx_fff_data, app->tx_file_path);

        keeloq_reset_mfname();
        keeloq_reset_kl_type();
        star_line_reset_mfname();
        star_line_reset_kl_type();
    }

    subghz_transmitter_free(app->tx_transmitter);
    furi_hal_subghz_idle();

    notification_message(app->notification, &sequence_blink_stop);

    unirfremix_preset_free(app->txpreset);
    flipper_format_free(app->tx_fff_data);
    unirfremix_end_send(app);
}

static bool unirfremix_send_sub(UniRFRemix* app, FlipperFormat* fff_data) {
    //
    bool res = false;
    do {
        if(!furi_hal_subghz_is_tx_allowed(app->txpreset->frequency)) {
            printf(
                "In your settings, only reception on this frequency (%lu) is allowed,\r\n"
                "the actual operation of the unirf app is not possible\r\n ",
                app->txpreset->frequency);
            app->tx_not_allowed = true;
            unirfremix_end_send(app);
            break;
        } else {
            app->tx_not_allowed = false;
        }

        app->tx_transmitter = subghz_transmitter_alloc_init(
            app->environment, furi_string_get_cstr(app->txpreset->protocol));
        if(!app->tx_transmitter) {
            break;
        }

        subghz_transmitter_deserialize(app->tx_transmitter, fff_data);

        furi_hal_subghz_reset();
        furi_hal_subghz_idle();
        furi_hal_subghz_load_custom_preset(app->txpreset->data);
        furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

        furi_hal_subghz_idle();

        furi_hal_subghz_set_frequency_and_path(app->txpreset->frequency);
        furi_hal_gpio_write(&gpio_cc1101_g0, false);
        furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

        if(!furi_hal_subghz_tx()) {
            FURI_LOG_E(TAG, "Sending not allowed");
            break;
        }

        //FURI_LOG_I(TAG, "Sending...");
        notification_message(app->notification, &sequence_blink_start_magenta);

        furi_hal_subghz_start_async_tx(subghz_transmitter_yield, app->tx_transmitter);

        res = true;
    } while(0);

    return res;
}

static void unirfremix_send_signal(UniRFRemix* app, Storage* storage, const char* path) {
    FURI_LOG_D(TAG, "Sending: %s", path);

    app->tx_file_path = path;

    app->tx_fff_data = flipper_format_string_alloc();

    app->txpreset = unirfremix_preset_alloc();

    // load settings/stream from .sub file
    FlipperFormat* fff_file = flipper_format_file_alloc(storage);
    bool open_ok = false;
    do {
        if(!flipper_format_file_open_existing(fff_file, path)) {
            FURI_LOG_E(TAG, "Could not open file %s", path);
            break;
        }
        if(!unirfremix_key_load(
               app->txpreset,
               fff_file,
               app->tx_fff_data,
               app->setting,
               app->subghz_receiver,
               path)) {
            FURI_LOG_E(TAG, "Could not load key");
            break;
        }
        open_ok = true;
    } while(0);
    flipper_format_free(fff_file);
    if(!open_ok) {
        FURI_LOG_E(TAG, "Could not load file!");
        return;
    }

    unirfremix_send_sub(app, app->tx_fff_data);
}

static void unirfremix_process_signal(UniRFRemix* app, FuriString* signal) {
    view_port_update(app->view_port);

    FURI_LOG_D(TAG, "signal = %s", furi_string_get_cstr(signal));

    if(strlen(furi_string_get_cstr(signal)) > 12) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        unirfremix_send_signal(app, storage, furi_string_get_cstr(signal));
        furi_record_close(RECORD_STORAGE);
    } else if(strlen(furi_string_get_cstr(signal)) < 10) {
        unirfremix_end_send(app);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    UniRFRemix* app = ctx;
    furi_check(furi_mutex_acquire(app->model_mutex, FuriWaitForever) == FuriStatusOk);

    //setup different canvas settings
    if(app->file_result == 1) {
        //if map has no valid filenames defined
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 62, 5, AlignCenter, AlignTop, "Config is incorrect.");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignTop, "Please configure map.");
        canvas_draw_str_aligned(canvas, 62, 60, AlignCenter, AlignBottom, "Press Back to Exit.");
    } else if(app->file_result == 3) {
        //if map has no valid filenames defined
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 62, 5, AlignCenter, AlignTop, "Checking config.");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignTop, "If app is stuck...");
        canvas_draw_str_aligned(canvas, 62, 60, AlignCenter, AlignBottom, "Press Back to Exit.");
    } else if(app->tx_not_allowed) {
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 62, 5, AlignCenter, AlignTop, "Transmission is blocked.");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 62, 15, AlignCenter, AlignTop, "Frequency is outside of");
        canvas_draw_str_aligned(canvas, 62, 25, AlignCenter, AlignTop, "default range.");
        canvas_draw_str_aligned(canvas, 62, 35, AlignCenter, AlignTop, "Check docs.");
        canvas_draw_str_aligned(canvas, 62, 60, AlignCenter, AlignBottom, "Press Back to Exit.");
    } else {
        //map found, draw all the things
        canvas_clear(canvas);

        //canvas_set_font(canvas, FontPrimary);
        //canvas_draw_str(canvas, 0, 10, "U: ");
        //canvas_draw_str(canvas, 0, 20, "L: ");
        //canvas_draw_str(canvas, 0, 30, "R: ");
        //canvas_draw_str(canvas, 0, 40, "D: ");
        //canvas_draw_str(canvas, 0, 50, "Ok: ");

        //PNGs are located in assets/icons/UniRFRemix before compiliation

        //Icons for Labels
        //canvas_draw_icon(canvas, 0, 0, &I_UniRFRemix_LeftAlignedButtons_9x64);
        canvas_draw_icon(canvas, 1, 5, &I_ButtonUp_7x4);
        canvas_draw_icon(canvas, 1, 15, &I_ButtonDown_7x4);
        canvas_draw_icon(canvas, 2, 23, &I_ButtonLeft_4x7);
        canvas_draw_icon(canvas, 2, 33, &I_ButtonRight_4x7);
        canvas_draw_icon(canvas, 0, 42, &I_Ok_btn_9x9);
        canvas_draw_icon(canvas, 0, 53, &I_back_10px);

        //Labels
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 10, 10, app->up_label);
        canvas_draw_str(canvas, 10, 20, app->down_label);
        canvas_draw_str(canvas, 10, 30, app->left_label);
        canvas_draw_str(canvas, 10, 40, app->right_label);
        canvas_draw_str(canvas, 10, 50, app->ok_label);

        canvas_draw_str_aligned(canvas, 11, 62, AlignLeft, AlignBottom, "Press=Exit.");

        //Status text and indicator
        canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, app->send_status);

        switch(app->send_status_c) {
        case 0:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            break;
        case 1:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            canvas_draw_icon(canvas, 116, 17, &I_Pin_arrow_up_7x9);
            break;
        case 2:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            canvas_draw_icon(canvas, 116, 17, &I_Pin_arrow_down_7x9);
            break;
        case 3:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            canvas_draw_icon(canvas, 115, 18, &I_Pin_arrow_right_9x7);
            break;
        case 4:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            canvas_draw_icon(canvas, 115, 18, &I_Pin_arrow_left_9x7);
            break;
        case 5:
            canvas_draw_icon(canvas, 113, 15, &I_Pin_cell_13x13);
            canvas_draw_icon(canvas, 116, 18, &I_Pin_star_7x7);
            break;
        default:
            break;
        }

        //Repeat indicator
        //canvas_draw_str_aligned(canvas, 125, 40, AlignRight, AlignBottom, "Repeat:");
        //canvas_draw_icon(canvas, 115, 39, &I_UniRFRemix_Repeat_12x14);
        //canvas_draw_str_aligned(canvas, 125, 62, AlignRight, AlignBottom, int_to_char(app->repeat));
    }

    furi_mutex_release(app->model_mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    UniRFRemix* app = ctx;
    furi_message_queue_put(app->input_queue, input_event, 0);
}

void unirfremix_subghz_alloc(UniRFRemix* app) {
    // load subghz presets
    app->setting = subghz_setting_alloc();
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user.txt"), false);

    // load mfcodes
    app->environment = subghz_environment_alloc();
    subghz_environment_load_keystore(app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(
        app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/nice_flor_s"));
    subghz_environment_set_protocol_registry(app->environment, (void*)&subghz_protocol_registry);

    app->subghz_receiver = subghz_receiver_alloc_init(app->environment);
}

UniRFRemix* unirfremix_alloc(void) {
    UniRFRemix* app = malloc(sizeof(UniRFRemix));

    furi_hal_power_suppress_charge_enter();

    app->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->input_queue = furi_message_queue_alloc(32, sizeof(InputEvent));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);

    // Open GUI and register view_port
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->notification = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

void unirfremix_free(UniRFRemix* app, bool with_subghz) {
    furi_hal_power_suppress_charge_exit();

    furi_string_free(app->up_file);
    furi_string_free(app->down_file);
    furi_string_free(app->left_file);
    furi_string_free(app->right_file);
    furi_string_free(app->ok_file);

    furi_string_free(app->file_path);
    furi_string_free(app->signal);

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    app->gui = NULL;

    furi_message_queue_free(app->input_queue);

    furi_mutex_free(app->model_mutex);

    if(with_subghz) {
        furi_hal_subghz_sleep();
        subghz_setting_free(app->setting);
        subghz_receiver_free(app->subghz_receiver);
        subghz_environment_free(app->environment);
    }

    furi_record_close(RECORD_NOTIFICATION);
    app->notification = NULL;

    free(app);
}

int32_t unirfremix_app(void* p) {
    UNUSED(p);
    UniRFRemix* app = unirfremix_alloc();

    app->file_path = furi_string_alloc();
    app->signal = furi_string_alloc();

    //setup variables before population
    app->up_file = furi_string_alloc();
    app->down_file = furi_string_alloc();
    app->left_file = furi_string_alloc();
    app->right_file = furi_string_alloc();
    app->ok_file = furi_string_alloc();

    app->file_result = 3;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, UNIRFMAP_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", UNIRFMAP_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    furi_string_set(app->file_path, UNIRFMAP_FOLDER);

    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, UNIRFMAP_EXTENSION, &I_sub1_10px);
    browser_options.base_path = UNIRFMAP_FOLDER;

    bool res = dialog_file_browser_show(dialogs, app->file_path, app->file_path, &browser_options);

    furi_record_close(RECORD_DIALOGS);
    if(!res) {
        FURI_LOG_E(TAG, "No file selected");
        unirfremix_free(app, false);
        return 255;
    } else {
        //check map and population variables
        unirfremix_cfg_set_check(app, app->file_path);
    }

    // init subghz stuff
    unirfremix_subghz_alloc(app);

    bool exit_loop = false;

    if(app->file_result == 2) {
        FURI_LOG_D(
            TAG,
            "U: %s - D: %s - L: %s - R: %s - O: %s ",
            furi_string_get_cstr(app->up_file),
            furi_string_get_cstr(app->down_file),
            furi_string_get_cstr(app->left_file),
            furi_string_get_cstr(app->right_file),
            furi_string_get_cstr(app->ok_file));

        //variables to control multiple button presses and status updates
        app->send_status = "Idle";
        app->send_status_c = 0;
        app->processing = 0;
        //app->repeat = 1;
        app->button = 0;

        //refresh screen to update variables before processing main screen or error screens
        furi_mutex_release(app->model_mutex);
        view_port_update(app->view_port);

        //input detect loop start
        InputEvent input;
        while(1) {
            furi_check(
                furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);
            FURI_LOG_D(
                TAG,
                "key: %s type: %s",
                input_get_key_name(input.key),
                input_get_type_name(input.type));

            switch(input.key) {
            case InputKeyUp:
                if(input.type == InputTypePress) {
                    if(app->up_enabled) {
                        if(app->processing == 0) {
                            furi_string_reset(app->signal);
                            furi_string_set(app->signal, app->up_file);
                            app->button = 1;
                            app->processing = 1;
                        }
                    }
                }
                if(input.type == InputTypeRelease) {
                    if(app->up_enabled) {
                        unirfremix_tx_stop(app);
                    }
                }
                break;

            case InputKeyDown:
                if(input.type == InputTypePress) {
                    if(app->down_enabled) {
                        if(app->processing == 0) {
                            furi_string_reset(app->signal);
                            furi_string_set(app->signal, app->down_file);
                            app->button = 2;
                            app->processing = 1;
                        }
                    }
                }
                if(input.type == InputTypeRelease) {
                    if(app->down_enabled) {
                        unirfremix_tx_stop(app);
                    }
                }
                break;

            case InputKeyRight:
                if(input.type == InputTypePress) {
                    if(app->right_enabled) {
                        if(app->processing == 0) {
                            furi_string_reset(app->signal);
                            furi_string_set(app->signal, app->right_file);
                            app->button = 3;
                            app->processing = 1;
                        }
                    }
                }
                if(input.type == InputTypeRelease) {
                    if(app->right_enabled) {
                        unirfremix_tx_stop(app);
                    }
                }
                break;

            case InputKeyLeft:
                if(input.type == InputTypePress) {
                    if(app->left_enabled) {
                        if(app->processing == 0) {
                            furi_string_reset(app->signal);
                            furi_string_set(app->signal, app->left_file);
                            app->button = 4;
                            app->processing = 1;
                        }
                    }
                }
                if(input.type == InputTypeRelease) {
                    if(app->left_enabled) {
                        unirfremix_tx_stop(app);
                    }
                }
                break;

            case InputKeyOk:
                if(input.type == InputTypePress) {
                    if(app->ok_enabled) {
                        if(app->processing == 0) {
                            furi_string_reset(app->signal);
                            furi_string_set(app->signal, app->ok_file);
                            app->button = 5;
                            app->processing = 1;
                        }
                    }
                }
                if(input.type == InputTypeRelease) {
                    if(app->ok_enabled) {
                        unirfremix_tx_stop(app);
                    }
                }
                break;

            case InputKeyBack:
                unirfremix_tx_stop(app);
                exit_loop = true;
                break;
            default:
                break;
            }

            if(app->processing == 0) {
                FURI_LOG_D(TAG, "processing 0");
                app->send_status = "Idle";
                app->send_status_c = 0;
                app->button = 0;
            } else if(app->processing == 1) {
                FURI_LOG_D(TAG, "processing 1");

                app->send_status = "Send";

                switch(app->button) {
                case 1:
                    app->send_status_c = 1;
                    break;
                case 2:
                    app->send_status_c = 2;
                    break;
                case 3:
                    app->send_status_c = 3;
                    break;
                case 4:
                    app->send_status_c = 4;
                    break;
                case 5:
                    app->send_status_c = 5;
                    break;
                default:
                    break;
                }

                app->processing = 2;

                unirfremix_process_signal(app, app->signal);
            }

            if(exit_loop == true) {
                furi_mutex_release(app->model_mutex);
                break;
            }

            furi_mutex_release(app->model_mutex);
            view_port_update(app->view_port);
        }
    } else if(app->file_result == 1 || app->file_result == 3) {
        //refresh screen to update variables before processing main screen or error screens
        view_port_update(app->view_port);

        InputEvent input;
        while(1) {
            furi_check(
                furi_message_queue_get(app->input_queue, &input, FuriWaitForever) == FuriStatusOk);
            FURI_LOG_D(
                TAG,
                "key: %s type: %s",
                input_get_key_name(input.key),
                input_get_type_name(input.type));

            switch(input.key) {
            case InputKeyRight:
                break;
            case InputKeyLeft:
                break;
            case InputKeyUp:
                break;
            case InputKeyDown:
                break;
            case InputKeyOk:
                break;
            case InputKeyBack:
                exit_loop = true;
                break;
            default:
                break;
            }

            if(exit_loop == true) {
                furi_mutex_release(app->model_mutex);
                break;
            }

            furi_mutex_release(app->model_mutex);
            view_port_update(app->view_port);
        }
    } else {
        furi_mutex_release(app->model_mutex);
    }

    // remove & free all stuff created by app
    unirfremix_free(app, true);

    return 0;
}