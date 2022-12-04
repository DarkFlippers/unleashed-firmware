#include "ifttt_virtual_button.h"

#define IFTTT_FOLDER "/ext/ifttt"
#define IFTTT_CONFIG_FOLDER "/ext/ifttt/config"
const char* CONFIG_FILE_PATH = "/ext/ifttt/config/config.settings";

#define FLIPPERZERO_SERIAL_BAUD 115200
typedef enum ESerialCommand { ESerialCommand_Config } ESerialCommand;

Settings save_settings(Settings settings) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    if(flipper_format_file_open_existing(file, CONFIG_FILE_PATH)) {
        flipper_format_update_string_cstr(file, CONF_SSID, settings.save_ssid);
        flipper_format_update_string_cstr(file, CONF_PASSWORD, settings.save_password);
        flipper_format_update_string_cstr(file, CONF_KEY, settings.save_key);
        flipper_format_update_string_cstr(file, CONF_EVENT, settings.save_event);
    } else {
    }
    flipper_format_file_close(file);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return settings;
}

void save_settings_file(FlipperFormat* file, Settings* settings) {
    flipper_format_write_header_cstr(file, CONFIG_FILE_HEADER, CONFIG_FILE_VERSION);
    flipper_format_write_comment_cstr(file, "Enter here the SSID of the wifi network");
    flipper_format_write_string_cstr(file, CONF_SSID, settings->save_ssid);
    flipper_format_write_comment_cstr(file, "Enter here the PASSWORD of the wifi network");
    flipper_format_write_string_cstr(file, CONF_PASSWORD, settings->save_password);
    flipper_format_write_comment_cstr(file, "Enter here the WEBHOOKS of your IFTTT account");
    flipper_format_write_string_cstr(file, CONF_KEY, settings->save_key);
    flipper_format_write_comment_cstr(file, "Enter here the EVENT name of your trigger");
    flipper_format_write_string_cstr(file, CONF_EVENT, settings->save_event);
}

Settings* load_settings() {
    Settings* settings = malloc(sizeof(Settings));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    FuriString* string_value;
    string_value = furi_string_alloc();
    FuriString* text_ssid_value;
    text_ssid_value = furi_string_alloc();
    FuriString* text_password_value;
    text_password_value = furi_string_alloc();
    FuriString* text_key_value;
    text_key_value = furi_string_alloc();
    FuriString* text_event_value;
    text_event_value = furi_string_alloc();

    if(storage_common_stat(storage, CONFIG_FILE_PATH, NULL) != FSE_OK) {
        if(!flipper_format_file_open_new(file, CONFIG_FILE_PATH)) {
            flipper_format_file_close(file);
        } else {
            settings->save_ssid = malloc(1);
            settings->save_password = malloc(1);
            settings->save_key = malloc(1);
            settings->save_event = malloc(1);

            settings->save_ssid[0] = '\0';
            settings->save_password[0] = '\0';
            settings->save_key[0] = '\0';
            settings->save_event[0] = '\0';

            save_settings_file(file, settings);
            flipper_format_file_close(file);
        }
    } else {
        if(!flipper_format_file_open_existing(file, CONFIG_FILE_PATH)) {
            flipper_format_file_close(file);
        } else {
            uint32_t value;
            if(!flipper_format_read_header(file, string_value, &value)) {
            } else {
                if(flipper_format_read_string(file, CONF_SSID, text_ssid_value)) {
                    settings->save_ssid = malloc(furi_string_size(text_ssid_value) + 1);
                    strcpy(settings->save_ssid, furi_string_get_cstr(text_ssid_value));
                }
                if(flipper_format_read_string(file, CONF_PASSWORD, text_password_value)) {
                    settings->save_password = malloc(furi_string_size(text_password_value) + 1);
                    strcpy(settings->save_password, furi_string_get_cstr(text_password_value));
                }
                if(flipper_format_read_string(file, CONF_KEY, text_key_value)) {
                    settings->save_key = malloc(furi_string_size(text_key_value) + 1);
                    strcpy(settings->save_key, furi_string_get_cstr(text_key_value));
                }
                if(flipper_format_read_string(file, CONF_EVENT, text_event_value)) {
                    settings->save_event = malloc(furi_string_size(text_event_value) + 1);
                    strcpy(settings->save_event, furi_string_get_cstr(text_event_value));
                }
            }
            flipper_format_file_close(file);
        }
    }

    furi_string_free(text_ssid_value);
    furi_string_free(text_password_value);
    furi_string_free(text_key_value);
    furi_string_free(text_event_value);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return settings;
}

void send_serial_command_config(ESerialCommand command, Settings* settings) {
    uint8_t data[1] = {0};

    char config_tmp[100];
    strcpy(config_tmp, "config,");
    strcat(config_tmp, settings->save_key);
    char config_tmp2[5];
    strcpy(config_tmp2, config_tmp);
    strcat(config_tmp2, ",");
    char config_tmp3[100];
    strcpy(config_tmp3, config_tmp2);
    strcat(config_tmp3, settings->save_ssid);
    char config_tmp4[5];
    strcpy(config_tmp4, config_tmp3);
    strcat(config_tmp4, ",");
    char config_tmp5[100];
    strcpy(config_tmp5, config_tmp4);
    strcat(config_tmp5, settings->save_password);
    char config_tmp6[5];
    strcpy(config_tmp6, config_tmp5);
    strcat(config_tmp6, ",");
    char config[350];
    strcpy(config, config_tmp6);
    strcat(config, settings->save_event);

    int length = strlen(config);
    for(int i = 0; i < length; i++) {
        switch(command) {
        case ESerialCommand_Config:
            data[0] = config[i];
            break;
        default:
            return;
        };

        furi_hal_uart_tx(FuriHalUartIdUSART1, data, 1);
    }
}

static bool ifttt_virtual_button_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    VirtualButtonApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool ifttt_virtual_button_back_event_callback(void* context) {
    furi_assert(context);
    VirtualButtonApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void ifttt_virtual_button_tick_event_callback(void* context) {
    furi_assert(context);
    VirtualButtonApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

VirtualButtonApp* ifttt_virtual_button_app_alloc(uint32_t first_scene) {
    VirtualButtonApp* app = malloc(sizeof(VirtualButtonApp));

    // Records
    app->gui = furi_record_open(RECORD_GUI);
    app->power = furi_record_open(RECORD_POWER);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&virtual_button_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, ifttt_virtual_button_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, ifttt_virtual_button_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, ifttt_virtual_button_tick_event_callback, 2000);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->sen_view = send_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, VirtualButtonAppViewSendView, send_view_get_view(app->sen_view));

    app->abou_view = about_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, VirtualButtonAppViewAboutView, about_view_get_view(app->abou_view));

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, VirtualButtonAppViewSubmenu, submenu_get_view(app->submenu));
    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, VirtualButtonAppViewDialog, dialog_ex_get_view(app->dialog));

    // Set first scene
    scene_manager_next_scene(app->scene_manager, first_scene);
    return app;
}

void ifttt_virtual_button_app_free(VirtualButtonApp* app) {
    furi_assert(app);

    free(app->settings.save_ssid);
    free(app->settings.save_password);
    free(app->settings.save_key);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, VirtualButtonAppViewSendView);
    send_view_free(app->sen_view);
    view_dispatcher_remove_view(app->view_dispatcher, VirtualButtonAppViewAboutView);
    about_view_free(app->abou_view);
    view_dispatcher_remove_view(app->view_dispatcher, VirtualButtonAppViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, VirtualButtonAppViewDialog);
    dialog_ex_free(app->dialog);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    // Records
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t ifttt_virtual_button_app(void* p) {
    UNUSED(p);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, IFTTT_FOLDER)) {
    }
    if(!storage_simply_mkdir(storage, IFTTT_CONFIG_FOLDER)) {
    }
    furi_record_close(RECORD_STORAGE);

    uint32_t first_scene = VirtualButtonAppSceneStart;
    VirtualButtonApp* app = ifttt_virtual_button_app_alloc(first_scene);
    memcpy(&app->settings, load_settings(), sizeof(Settings));
    send_serial_command_config(ESerialCommand_Config, &(app->settings));

    view_dispatcher_run(app->view_dispatcher);
    ifttt_virtual_button_app_free(app);
    return 0;
}