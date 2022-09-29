#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <assets_icons.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/path.h>
#include <applications/main/subghz/subghz_i.h>

#include <lib/subghz/protocols/raw.h>
#include <lib/subghz/protocols/registry.h>
#include <lib/subghz/types.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/star_line.h>

#define UNIRFMAP_FOLDER "/ext/unirf"
#define UNIRFMAP_EXTENSION ".txt"

#define TAG "UniRF Remix"

typedef struct {
    uint32_t frequency;
    string_t name;

    string_t protocol;
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

    string_t up_file;
    string_t down_file;
    string_t left_file;
    string_t right_file;
    string_t ok_file;
    string_t empty;

    string_t up_l;
    string_t left_l;
    string_t right_l;
    string_t down_l;
    string_t ok_l;

    string_t file_path;

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
    int file_blank;

    string_t signal;
} UniRFRemix;

UniRFPreset* unirfremix_preset_alloc(void) {
    UniRFPreset* preset = malloc(sizeof(UniRFPreset));
    string_init(preset->name);
    string_init(preset->protocol);
    preset->repeat = 200;
    return preset;
}

void unirfremix_preset_free(UniRFPreset* preset) {
    string_clear(preset->name);
    string_clear(preset->protocol);
    free(preset);
}

static char* char_to_str(char* str, int i) {
    char* converted = malloc(sizeof(char) * i + 1);
    memcpy(converted, str, i);

    converted[i] = '\0';

    return converted;
}
/*
static const char* int_to_char(int number) {
    switch(number) {
    case 0:
        return "0";
    case 1:
        return "1";
    case 2:
        return "2";
    case 3:
        return "3";
    case 4:
        return "4";
    case 5:
        return "5";
    case 6:
        return "6";
    case 7:
        return "7";
    case 8:
        return "8";
    case 9:
        return "9";
    default:
        return "0";
    }
}
*/
//get filename without path
static char* extract_filename(const char* name, int len) {
    string_t tmp;
    string_init(tmp);

    //remove path
    path_extract_filename_no_ext(name, tmp);

    return char_to_str((char*)string_get_cstr(tmp), len);
}

/*
 * check that map file exists
 * assign variables to values within map file
 * set missing filenames to N/A
 * set filename as label if label definitions are missing
 * set error flag if all buttons are N/A
 * set error flag if missing map file
 */

void unirfremix_cfg_set_check(UniRFRemix* app, string_t file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    app->file_result = 3;
    app->file_blank = 0;

    app->up_enabled = 1;
    app->down_enabled = 1;
    app->left_enabled = 1;
    app->right_enabled = 1;
    app->ok_enabled = 1;

    int label_len = 16;

    //check that map file exists
    if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", string_get_cstr(file_name));
    } else {
        //Filename Assignment/Check Start

        //assign variables to values within map file
        //set missing filenames to N/A
        if(!flipper_format_read_string(fff_data_file, "UP", app->up_file)) {
            FURI_LOG_W(TAG, "Could not read UP string");
            //increment file_blank for processing later
            app->file_blank++;
            //set label to "N/A"
            app->up_label = "N/A";
            //disable the ability to process the signal on button press
            app->up_enabled = 0;
        } else {
            //check name length for proper screen fit
            //then set filename as label. Might be replaced with defined label later on below.
            app->up_label = extract_filename(string_get_cstr(app->up_file), label_len);
            FURI_LOG_I(TAG, "UP file: %s", string_get_cstr(app->up_file));
        }

        //Repeat process for Down
        if(!flipper_format_read_string(fff_data_file, "DOWN", app->down_file)) {
            FURI_LOG_W(TAG, "Could not read DOWN string");
            app->file_blank++;
            app->down_label = "N/A";
            app->down_enabled = 0;
        } else {
            app->down_label = extract_filename(string_get_cstr(app->down_file), label_len);
            FURI_LOG_I(TAG, "DOWN file: %s", string_get_cstr(app->down_file));
        }

        //Repeat process for Left
        if(!flipper_format_read_string(fff_data_file, "LEFT", app->left_file)) {
            FURI_LOG_W(TAG, "Could not read LEFT string");
            app->file_blank++;
            app->left_label = "N/A";
            app->left_enabled = 0;
        } else {
            app->left_label = extract_filename(string_get_cstr(app->left_file), label_len);
            FURI_LOG_I(TAG, "LEFT file: %s", string_get_cstr(app->left_file));
        }

        //Repeat process for Right
        if(!flipper_format_read_string(fff_data_file, "RIGHT", app->right_file)) {
            FURI_LOG_W(TAG, "Could not read RIGHT string");
            app->file_blank++;
            app->right_label = "N/A";
            app->right_enabled = 0;
        } else {
            app->right_label = extract_filename(string_get_cstr(app->right_file), label_len);
            FURI_LOG_I(TAG, "RIGHT file: %s", string_get_cstr(app->right_file));
        }

        //Repeat process for Ok
        if(!flipper_format_read_string(fff_data_file, "OK", app->ok_file)) {
            FURI_LOG_W(TAG, "Could not read OK string");
            app->file_blank++;
            app->ok_label = "N/A";
            app->ok_enabled = 0;
        } else {
            app->ok_label = extract_filename(string_get_cstr(app->ok_file), label_len);
            FURI_LOG_I(TAG, "OK file: %s", string_get_cstr(app->ok_file));
        }

        //File definitions are done.
        //File checks will follow after label assignment in order to close the universal_rf_map file without the need to reopen it again.

        //Label Assignment/Check Start

        //assign variables to values within map file
        if(!flipper_format_read_string(fff_data_file, "ULABEL", app->up_l)) {
            FURI_LOG_W(TAG, "Could not read ULABEL string");
            //if Up button is disabled, set the label to "N/A";
            if(app->up_enabled == 0) {
                app->up_label = "N/A";
            }
        } else {
            //check if button is disabled, and set label to "N/A" from missing map definition above
            if(app->up_enabled == 0) {
                app->up_label = "N/A";
            } else {
                //set label from map to variable and shrink to fit screen
                app->up_label = char_to_str((char*)string_get_cstr(app->up_l), label_len);
            }
            FURI_LOG_I(TAG, "UP label: %s", app->up_label);
        }

        if(!flipper_format_read_string(fff_data_file, "DLABEL", app->down_l)) {
            FURI_LOG_W(TAG, "Could not read DLABEL string");
            if(app->down_enabled == 0) {
                app->down_label = "N/A";
            }
        } else {
            if(app->down_enabled == 0) {
                app->down_label = "N/A";
            } else {
                app->down_label = char_to_str((char*)string_get_cstr(app->down_l), label_len);
            }
            FURI_LOG_I(TAG, "DOWN label: %s", app->down_label);
        }

        if(!flipper_format_read_string(fff_data_file, "LLABEL", app->left_l)) {
            FURI_LOG_W(TAG, "Could not read LLABEL string");
            if(app->left_enabled == 0) {
                app->left_label = "N/A";
            }
        } else {
            if(app->left_enabled == 0) {
                app->left_label = "N/A";
            } else {
                app->left_label = char_to_str((char*)string_get_cstr(app->left_l), label_len);
            }
            FURI_LOG_I(TAG, "LEFT label: %s", app->left_label);
        }

        if(!flipper_format_read_string(fff_data_file, "RLABEL", app->right_l)) {
            FURI_LOG_W(TAG, "Could not read RLABEL string");
            if(app->right_enabled == 0) {
                app->right_label = "N/A";
            }
        } else {
            if(app->right_enabled == 0) {
                app->right_label = "N/A";
            } else {
                app->right_label = char_to_str((char*)string_get_cstr(app->right_l), label_len);
            }
            FURI_LOG_I(TAG, "RIGHT label: %s", app->right_label);
        }

        if(!flipper_format_read_string(fff_data_file, "OKLABEL", app->ok_l)) {
            FURI_LOG_W(TAG, "Could not read OKLABEL string");
            if(app->ok_enabled == 0) {
                app->ok_label = "N/A";
            }
        } else {
            if(app->ok_enabled == 0) {
                app->ok_label = "N/A";
            } else {
                app->ok_label = char_to_str((char*)string_get_cstr(app->ok_l), label_len);
            }
            FURI_LOG_I(TAG, "OK label: %s", app->ok_label);
        }

        app->file_result = 2;
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    //File Existence Check
    //Check each file definition if not already set to "N/A"

    //determine if files exist.
    //determine whether or not to continue to launch app with missing variables
    //if 5 files are missing, throw error

    FURI_LOG_D(TAG, "app->file_blank: %d", app->file_blank);

    if(app->file_blank == 5) {
        //trigger invalid file error screen
        app->file_result = 1;
    } else {
        //check all files
        //reset app->file_blank to redetermine if error needs to be thrown
        app->file_blank = 0;

        //if button is still enabled, check that file exists
        if(app->up_enabled == 1) {
            string_set(file_name, app->up_file);
            fff_data_file = flipper_format_file_alloc(storage);

            if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
                FURI_LOG_W(TAG, "Could not open UP file %s", string_get_cstr(file_name));

                //disable button, and set label to "N/A"
                app->up_enabled = 0;
                app->up_label = "N/A";
                app->file_blank++;
            }

            //close the file
            flipper_format_file_close(fff_data_file);
            flipper_format_free(fff_data_file);
        }

        if(app->down_enabled == 1) {
            string_set(file_name, app->down_file);
            fff_data_file = flipper_format_file_alloc(storage);

            if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
                FURI_LOG_W(TAG, "Could not open DOWN file %s", string_get_cstr(file_name));

                app->down_enabled = 0;
                app->down_label = "N/A";
                app->file_blank++;
            }

            flipper_format_file_close(fff_data_file);
            flipper_format_free(fff_data_file);
        }

        if(app->left_enabled == 1) {
            string_set(file_name, app->left_file);
            fff_data_file = flipper_format_file_alloc(storage);

            if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
                FURI_LOG_W(TAG, "Could not open LEFT file %s", string_get_cstr(file_name));

                app->left_enabled = 0;
                app->left_label = "N/A";
                app->file_blank++;
            }

            flipper_format_file_close(fff_data_file);
            flipper_format_free(fff_data_file);
        }

        if(app->right_enabled == 1) {
            string_set(file_name, app->right_file);
            fff_data_file = flipper_format_file_alloc(storage);

            if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
                FURI_LOG_W(TAG, "Could not open RIGHT file %s", string_get_cstr(file_name));

                app->right_enabled = 0;
                app->right_label = "N/A";
                app->file_blank++;
            }

            flipper_format_file_close(fff_data_file);
            flipper_format_free(fff_data_file);
        }

        if(app->ok_enabled == 1) {
            string_set(file_name, app->ok_file);
            fff_data_file = flipper_format_file_alloc(storage);

            if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
                FURI_LOG_W(TAG, "Could not open OK file %s", string_get_cstr(file_name));

                app->ok_enabled = 0;
                app->ok_label = "N/A";
                app->file_blank++;
            }

            flipper_format_file_close(fff_data_file);
            flipper_format_free(fff_data_file);
        }

        furi_record_close(RECORD_STORAGE);

        if(app->file_blank == 5) {
            app->file_result = 1;
        } else {
            app->file_result = 2;
        }
    }
}

static void unirfremix_end_send(UniRFRemix* app) {
    app->processing = 0;
}

bool unirfremix_set_preset(UniRFPreset* p, const char* preset) {
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        string_set(p->name, "AM270");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        string_set(p->name, "AM650");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        string_set(p->name, "FM238");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        string_set(p->name, "FM476");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        FURI_LOG_E(TAG, "Custom preset unsupported now");
        return false;
        // string_set(p->name, "CUSTOM");
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

    string_t temp_str;
    string_init(temp_str);

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
            string_set(temp_str, "FuriHalSubGhzPresetOok650Async");
        }
        if(!unirfremix_set_preset(preset, string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Could not set preset");
            break;
        }
        if(!strcmp(string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            // TODO: check if preset is custom
            FURI_LOG_E(TAG, "Could not use custom preset");
            break;
        }
        size_t preset_index =
            subghz_setting_get_inx_preset_by_name(setting, string_get_cstr(preset->name));
        preset->data = subghz_setting_get_preset_data(setting, preset_index);
        preset->data_size = subghz_setting_get_preset_data_size(setting, preset_index);

        // load protocol from file
        if(!flipper_format_read_string(fff_file, "Protocol", preset->protocol)) {
            FURI_LOG_E(TAG, "Could not read Protocol.");
            break;
        }
        if(!string_cmp_str(preset->protocol, "RAW")) {
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
            receiver, string_get_cstr(preset->protocol));
        if(preset->decoder) {
            if(!subghz_protocol_decoder_base_deserialize(preset->decoder, fff_data)) {
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol %s not found", string_get_cstr(temp_str));
        }

        res = true;
    } while(0);

    string_clear(temp_str);

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
    string_t file_dir;
    string_init(file_dir);

    path_extract_dirname(dev_file_name, file_dir);
    do {
        flipper_format_delete_key(fff_file, "Repeat");
        flipper_format_delete_key(fff_file, "Manufacture");

        if(!storage_simply_mkdir(storage, string_get_cstr(file_dir))) {
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
    string_clear(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void unirfremix_tx_stop(UniRFRemix* app) {
    if(app->processing == 0) {
        return;
    }

    if(!string_cmp_str(app->txpreset->protocol, "RAW")) {
        while(!furi_hal_subghz_is_async_tx_complete()) {
            furi_delay_ms(15);
        }
    }

    //Stop TX
    furi_hal_subghz_stop_async_tx();
    FURI_LOG_I(TAG, "TX Done!");
    subghz_transmitter_stop(app->tx_transmitter);

    FURI_LOG_D(TAG, "Checking if protocol is dynamic");
    const SubGhzProtocol* registry =
        subghz_protocol_registry_get_by_name(string_get_cstr(app->txpreset->protocol));
    FURI_LOG_D(TAG, "Protocol-TYPE %d", registry->type);
    if(registry && registry->type == SubGhzProtocolTypeDynamic) {
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
            app->environment, string_get_cstr(app->txpreset->protocol));
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
        furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_write(&gpio_cc1101_g0, true);

        if(!furi_hal_subghz_tx()) {
            FURI_LOG_E(TAG, "Sending not allowed");
            break;
        }

        FURI_LOG_I(TAG, "Sending...");
        notification_message(app->notification, &sequence_blink_start_magenta);

        furi_hal_subghz_start_async_tx(subghz_transmitter_yield, app->tx_transmitter);

        res = true;
    } while(0);

    return res;
}

static void unirfremix_send_signal(UniRFRemix* app, Storage* storage, const char* path) {
    FURI_LOG_I(TAG, "Sending: %s", path);

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

static void unirfremix_process_signal(UniRFRemix* app, string_t signal) {
    view_port_update(app->view_port);

    FURI_LOG_I(TAG, "signal = %s", string_get_cstr(signal));

    if(strlen(string_get_cstr(signal)) > 12) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        unirfremix_send_signal(app, storage, string_get_cstr(signal));
        furi_record_close(RECORD_STORAGE);
    } else if(strlen(string_get_cstr(signal)) < 10) {
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
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user"));

    // load mfcodes
    app->environment = subghz_environment_alloc();
    subghz_environment_load_keystore(app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(
        app->environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        app->environment, EXT_PATH("subghz/assets/nice_flor_s"));

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

    string_clear(app->up_file);
    string_clear(app->down_file);
    string_clear(app->left_file);
    string_clear(app->right_file);
    string_clear(app->ok_file);
    string_clear(app->empty);

    string_clear(app->up_l);
    string_clear(app->down_l);
    string_clear(app->left_l);
    string_clear(app->right_l);
    string_clear(app->ok_l);

    string_clear(app->file_path);
    string_clear(app->signal);

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

    string_init(app->file_path);

    //setup variables before population
    string_init(app->up_file);
    string_init(app->down_file);
    string_init(app->left_file);
    string_init(app->right_file);
    string_init(app->ok_file);
    string_init(app->empty);

    string_init(app->up_l);
    string_init(app->down_l);
    string_init(app->left_l);
    string_init(app->right_l);
    string_init(app->ok_l);

    app->file_result = 3;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, UNIRFMAP_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", UNIRFMAP_FOLDER);
    }
    furi_record_close(RECORD_STORAGE);

    string_set_str(app->file_path, UNIRFMAP_FOLDER);

    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, UNIRFMAP_EXTENSION, &I_sub1_10px);

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
            string_get_cstr(app->up_file),
            string_get_cstr(app->down_file),
            string_get_cstr(app->left_file),
            string_get_cstr(app->right_file),
            string_get_cstr(app->ok_file));

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
                            *app->signal = *app->empty;
                            *app->signal = *app->up_file;
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
                            *app->signal = *app->empty;
                            *app->signal = *app->down_file;
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
                            *app->signal = *app->empty;
                            *app->signal = *app->right_file;
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
                            *app->signal = *app->empty;
                            *app->signal = *app->left_file;
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
                            *app->signal = *app->empty;
                            *app->signal = *app->ok_file;
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
    } else if(app->file_result == 1) {
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