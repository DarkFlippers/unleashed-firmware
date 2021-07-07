#include "fatfs.h"
#include "filesystem-api.h"
#include "sd-filesystem.h"
#include "menu/menu.h"
#include "menu/menu_item.h"
#include "cli/cli.h"
#include "api-hal-sd.h"

#include <gui/modules/dialog_ex.h>
#include <gui/modules/file_select.h>

typedef enum {
    FST_FAT12 = FS_FAT12,
    FST_FAT16 = FS_FAT16,
    FST_FAT32 = FS_FAT32,
    FST_EXFAT = FS_EXFAT,
} SDFsType;

typedef struct {
    SDFsType fs_type;
    uint32_t kb_total;
    uint32_t kb_free;
    uint16_t cluster_size;
    uint16_t sector_size;
    char label[34];
    SDError error;
} SDInfo;

typedef enum {
    SdAppEventTypeBack,
    SdAppEventTypeOK,
    SdAppEventTypeFormat,
    SdAppEventTypeInfo,
    SdAppEventTypeEject,
    SdAppEventTypeFileSelect,
    SdAppEventTypeCheckError,
    SdAppEventTypeShowError,
} SdAppEventType;

typedef struct {
    const char* path;
    const char* extension;
    char* result;
    uint8_t result_size;
    char* selected_filename;
} SdAppFileSelectData;

typedef struct {
    bool result;
} SdAppFileSelectResultEvent;

typedef struct {
    SdAppEventType type;
    union {
        SdAppFileSelectData file_select_data;
        const char* error_text;
    } payload;
} SdAppEvent;

static void sd_icon_draw_callback(Canvas* canvas, void* context);
bool sd_api_file_select(
    SdApp* sd_app,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    char* selected_filename);
void sd_api_check_error(SdApp* sd_app);
void sd_api_show_error(SdApp* sd_app, const char* error_text);

/******************* Allocators *******************/

FS_Api* fs_api_alloc() {
    FS_Api* fs_api = furi_alloc(sizeof(FS_Api));

    // fill file api
    fs_api->file.open = fs_file_open;
    fs_api->file.close = fs_file_close;
    fs_api->file.read = fs_file_read;
    fs_api->file.write = fs_file_write;
    fs_api->file.seek = fs_file_seek;
    fs_api->file.tell = fs_file_tell;
    fs_api->file.truncate = fs_file_truncate;
    fs_api->file.size = fs_file_size;
    fs_api->file.sync = fs_file_sync;
    fs_api->file.eof = fs_file_eof;

    // fill dir api
    fs_api->dir.open = fs_dir_open;
    fs_api->dir.close = fs_dir_close;
    fs_api->dir.read = fs_dir_read;
    fs_api->dir.rewind = fs_dir_rewind;

    // fill common api
    fs_api->common.info = fs_common_info;
    fs_api->common.remove = fs_common_remove;
    fs_api->common.rename = fs_common_rename;
    fs_api->common.set_attr = fs_common_set_attr;
    fs_api->common.mkdir = fs_common_mkdir;
    fs_api->common.set_time = fs_common_set_time;
    fs_api->common.get_fs_info = fs_get_fs_info;

    // fill errors api
    fs_api->error.get_desc = fs_error_get_desc;
    fs_api->error.get_internal_desc = fs_error_get_internal_desc;

    return fs_api;
}

SdApp* sd_app_alloc() {
    SdApp* sd_app = furi_alloc(sizeof(SdApp));

    // init inner fs data
    furi_check(_fs_init(&sd_app->info));

    sd_app->event_queue = osMessageQueueNew(8, sizeof(SdAppEvent), NULL);
    sd_app->result_receiver = osMessageQueueNew(1, sizeof(SdAppFileSelectResultEvent), NULL);

    // init icon view_port
    sd_app->icon.view_port = view_port_alloc();
    sd_app->icon.mounted = &I_SDcardMounted_11x8;
    sd_app->icon.fail = &I_SDcardFail_11x8;
    view_port_set_width(sd_app->icon.view_port, icon_get_width(sd_app->icon.mounted));
    view_port_draw_callback_set(sd_app->icon.view_port, sd_icon_draw_callback, sd_app);
    view_port_enabled_set(sd_app->icon.view_port, false);

    // init sd card api
    sd_app->sd_card_api.context = sd_app;
    sd_app->sd_card_api.file_select = sd_api_file_select;
    sd_app->sd_card_api.check_error = sd_api_check_error;
    sd_app->sd_card_api.show_error = sd_api_show_error;

    sd_app->sd_app_state = SdAppStateBackground;
    string_init(sd_app->text_holder);

    return sd_app;
}

/******************* Internal sd card related fns *******************/

void get_sd_info(SdApp* sd_app, SDInfo* sd_info) {
    uint32_t free_clusters, free_sectors, total_sectors;
    FATFS* fs;

    // clean data
    memset(sd_info, 0, sizeof(SDInfo));

    // get fs info
    _fs_lock(&sd_app->info);
    sd_info->error = f_getlabel(sd_app->info.path, sd_info->label, NULL);
    if(sd_info->error == SD_OK) {
        sd_info->error = f_getfree(sd_app->info.path, &free_clusters, &fs);
    }
    _fs_unlock(&sd_app->info);

    if(sd_info->error == SD_OK) {
        // calculate size
        total_sectors = (fs->n_fatent - 2) * fs->csize;
        free_sectors = free_clusters * fs->csize;

        uint16_t sector_size = _MAX_SS;
#if _MAX_SS != _MIN_SS
        sector_size = fs->ssize;
#endif

        sd_info->fs_type = fs->fs_type;

        sd_info->kb_total = total_sectors / 1024 * sector_size;
        sd_info->kb_free = free_sectors / 1024 * sector_size;
        sd_info->cluster_size = fs->csize;
        sd_info->sector_size = sector_size;
    }
}

const char* get_fs_type_text(SDFsType fs_type) {
    switch(fs_type) {
    case(FST_FAT12):
        return "FAT12";
        break;
    case(FST_FAT16):
        return "FAT16";
        break;
    case(FST_FAT32):
        return "FAT32";
        break;
    case(FST_EXFAT):
        return "EXFAT";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

void app_sd_format_internal(SdApp* sd_app) {
    uint8_t* work_area;

    _fs_lock(&sd_app->info);
    work_area = malloc(_MAX_SS);
    if(work_area == NULL) {
        sd_app->info.status = SD_NOT_ENOUGH_CORE;
    } else {
        sd_app->info.status = f_mkfs(sd_app->info.path, FM_ANY, 0, work_area, _MAX_SS);
        free(work_area);

        if(sd_app->info.status == SD_OK) {
            // set label and mount card
            f_setlabel("Flipper SD");
            sd_app->info.status = f_mount(&sd_app->info.fat_fs, sd_app->info.path, 1);
        }
    }

    _fs_unlock(&sd_app->info);
}

const NotificationSequence sd_sequence_success = {
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    NULL,
};

const NotificationSequence sd_sequence_error = {
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    NULL,
};

const NotificationSequence sd_sequence_eject = {
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    NULL,
};

const NotificationSequence sd_sequence_wait = {
    &message_red_255,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sd_sequence_wait_off = {
    &message_red_0,
    &message_blue_0,
    NULL,
};

void app_sd_notify_wait(SdApp* sd_app) {
    notification_message(sd_app->notifications, &sd_sequence_wait);
}

void app_sd_notify_wait_off(SdApp* sd_app) {
    notification_message(sd_app->notifications, &sd_sequence_wait_off);
}

void app_sd_notify_success(SdApp* sd_app) {
    notification_message(sd_app->notifications, &sd_sequence_success);
}

void app_sd_notify_eject(SdApp* sd_app) {
    notification_message(sd_app->notifications, &sd_sequence_eject);
}

void app_sd_notify_error(SdApp* sd_app) {
    notification_message(sd_app->notifications, &sd_sequence_error);
}

bool app_sd_mount_card(SdApp* sd_app) {
    bool result = false;
    const uint8_t max_init_counts = 10;
    uint8_t counter = max_init_counts;
    uint8_t bsp_result;

    _fs_lock(&sd_app->info);

    while(result == false && counter > 0 && hal_sd_detect()) {
        app_sd_notify_wait(sd_app);

        if((counter % 10) == 0) {
            // power reset sd card
            bsp_result = BSP_SD_Init(true);
        } else {
            bsp_result = BSP_SD_Init(false);
        }

        if(bsp_result) {
            // bsp error
            sd_app->info.status = SD_LOW_LEVEL_ERR;
        } else {
            sd_app->info.status = f_mount(&sd_app->info.fat_fs, sd_app->info.path, 1);

            if(sd_app->info.status == SD_OK || sd_app->info.status == SD_NO_FILESYSTEM) {
                FATFS* fs;
                uint32_t free_clusters;

                sd_app->info.status = f_getfree(sd_app->info.path, &free_clusters, &fs);

                if(sd_app->info.status == SD_OK || sd_app->info.status == SD_NO_FILESYSTEM) {
                    result = true;
                }
            }
        }
        app_sd_notify_wait_off(sd_app);

        if(!result) {
            delay(1000);
            FURI_LOG_E(
                "SD FILESYSTEM",
                "init(%d), error: %s\r\n",
                counter,
                fs_error_get_internal_desc(sd_app->info.status));

            counter--;
        }
    }

    _fs_unlock(&sd_app->info);
    return result;
}

void app_sd_unmount_card(SdApp* sd_app) {
    _fs_lock(&sd_app->info);

    // set status
    sd_app->info.status = SD_NO_CARD;
    view_port_enabled_set(sd_app->icon.view_port, false);

    // close files
    for(uint8_t index = 0; index < SD_FS_MAX_FILES; index++) {
        FileData* filedata = &sd_app->info.files[index];

        if(filedata->thread_id != NULL) {
            if(filedata->is_dir) {
                f_closedir(&filedata->data.dir);
            } else {
                f_close(&filedata->data.file);
            }
            filedata->thread_id = NULL;
        }
    }

    // unmount volume
    f_mount(0, sd_app->info.path, 0);

    _fs_unlock(&sd_app->info);
}

bool app_sd_make_path(const char* path) {
    furi_assert(path);

    if(*path) {
        char* file_path = strdup(path);
        // Make parent directories
        for(char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
            *p = '\0';
            SDError result = f_mkdir(file_path);

            if(result != SD_OK) {
                if(result != SD_EXIST) {
                    *p = '/';
                    free(file_path);
                    return false;
                }
            }
            *p = '/';
        }
        // Make origin directory
        SDError result = f_mkdir(file_path);
        if(result != SD_OK) {
            if(result != SD_EXIST) {
                free(file_path);
                return false;
            }
        }

        free(file_path);
    }

    return true;
}

/******************* Draw callbacks *******************/

static void sd_icon_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);
    SdApp* sd_app = context;

    switch(sd_app->info.status) {
    case SD_NO_CARD:
        break;
    case SD_OK:
        canvas_draw_icon(canvas, 0, 0, sd_app->icon.mounted);
        break;
    default:
        canvas_draw_icon(canvas, 0, 0, sd_app->icon.fail);
        break;
    }
}

/******************* SD-api callbacks *******************/

bool sd_api_file_select(
    SdApp* sd_app,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    char* selected_filename) {
    bool retval = false;

    SdAppEvent message = {
        .type = SdAppEventTypeFileSelect,
        .payload = {
            .file_select_data = {
                .path = path,
                .extension = extension,
                .result = result,
                .result_size = result_size,
                .selected_filename = selected_filename,
            }}};

    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);

    SdAppFileSelectResultEvent event;
    while(1) {
        osStatus_t event_status =
            osMessageQueueGet(sd_app->result_receiver, &event, NULL, osWaitForever);
        if(event_status == osOK) {
            retval = event.result;
            break;
        }
    }

    if(!retval) {
        sd_api_check_error(sd_app);
    }

    return retval;
}

void sd_api_check_error(SdApp* sd_app) {
    SdAppEvent message = {.type = SdAppEventTypeCheckError};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

void sd_api_show_error(SdApp* sd_app, const char* error_text) {
    SdAppEvent message = {.type = SdAppEventTypeShowError, .payload.error_text = error_text};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

/******************* View callbacks *******************/

void app_view_back_callback(void* context) {
    furi_assert(context);
    SdApp* sd_app = context;
    SdAppEvent message = {.type = SdAppEventTypeBack};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

void app_view_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    SdApp* sd_app = context;

    if(result == DialogExResultLeft) {
        SdAppEvent message = {.type = SdAppEventTypeBack};
        furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
    } else if(result == DialogExResultRight) {
        SdAppEvent message = {.type = SdAppEventTypeOK};
        furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
    }
}

void app_view_file_select_callback(bool result, void* context) {
    furi_assert(context);
    SdApp* sd_app = context;

    if(result) {
        SdAppEvent message = {.type = SdAppEventTypeOK};
        furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
    } else {
        SdAppEvent message = {.type = SdAppEventTypeBack};
        furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
    }
}

/******************* Menu callbacks *******************/

void app_sd_info_callback(void* context) {
    furi_assert(context);
    SdApp* sd_app = context;
    SdAppEvent message = {.type = SdAppEventTypeInfo};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

void app_sd_format_callback(void* context) {
    furi_assert(context);
    SdApp* sd_app = context;
    SdAppEvent message = {.type = SdAppEventTypeFormat};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

void app_sd_eject_callback(void* context) {
    furi_assert(context);
    SdApp* sd_app = context;
    SdAppEvent message = {.type = SdAppEventTypeEject};
    furi_check(osMessageQueuePut(sd_app->event_queue, &message, 0, osWaitForever) == osOK);
}

/******************* Cli callbacks *******************/

static void cli_sd_format(Cli* cli, string_t args, void* _ctx) {
    SdApp* sd_app = (SdApp*)_ctx;

    printf("Formatting SD card, all data will be lost. Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Formatting, please wait...\r\n");
        // format card
        app_sd_format_internal(sd_app);

        if(sd_app->info.status != SD_OK) {
            printf("SD card format error: ");
            printf(fs_error_get_internal_desc(sd_app->info.status));
            printf("\r\n");
        } else {
            printf("SD card was successfully formatted.\r\n");
        }
    } else {
        printf("Cancelled.\r\n");
    }
}

static void cli_sd_info(Cli* cli, string_t args, void* _ctx) {
    SdApp* sd_app = (SdApp*)_ctx;
    SDInfo sd_info;

    get_sd_info(sd_app, &sd_info);
    printf("SD Status: %s\r\n", fs_error_get_internal_desc(sd_app->info.status));

    if(sd_info.error == SD_OK) {
        const char* fs_type = get_fs_type_text(sd_info.fs_type);
        printf("Label: %s\r\n", sd_info.label);
        printf("Filesystem: %s\r\n", fs_type);
        printf("Cluster: %d sectors\r\n", sd_info.cluster_size);
        printf("Sector: %d bytes\r\n", sd_info.sector_size);
        printf("%lu KB total\r\n", sd_info.kb_total);
        printf("%lu KB free\r\n", sd_info.kb_free);
    } else {
        printf("SD Info error: %s\r\n", fs_error_get_internal_desc(sd_info.error));
    }
}

/******************* Test *******************/

bool try_to_alloc_view_holder(SdApp* sd_app, Gui* gui) {
    bool result = false;

    _fs_lock(&sd_app->info);

    if(sd_app->view_holder == NULL) {
        sd_app->view_holder = view_holder_alloc();
        view_holder_attach_to_gui(sd_app->view_holder, gui);
        view_holder_set_back_callback(sd_app->view_holder, app_view_back_callback, sd_app);
        result = true;
    }

    _fs_unlock(&sd_app->info);

    return result;
}

DialogEx* alloc_and_attach_dialog(SdApp* sd_app) {
    DialogEx* dialog = dialog_ex_alloc();
    dialog_ex_set_context(dialog, sd_app);
    dialog_ex_set_result_callback(dialog, app_view_dialog_callback);
    view_holder_set_view(sd_app->view_holder, dialog_ex_get_view(dialog));
    view_holder_set_free_callback(sd_app->view_holder, (FreeCallback)dialog_ex_free, dialog);
    return dialog;
}

FileSelect* alloc_and_attach_file_select(SdApp* sd_app) {
    FileSelect* file_select = file_select_alloc();
    file_select_set_callback(file_select, app_view_file_select_callback, sd_app);
    view_holder_set_view(sd_app->view_holder, file_select_get_view(file_select));
    view_holder_set_free_callback(
        sd_app->view_holder, (FreeCallback)file_select_free, file_select);
    return file_select;
}

void free_view_holder(SdApp* sd_app) {
    _fs_lock(&sd_app->info);

    if(sd_app->view_holder) {
        view_holder_free(sd_app->view_holder);
        sd_app->view_holder = NULL;
    }

    _fs_unlock(&sd_app->info);
}

void app_reset_state(SdApp* sd_app) {
    view_holder_stop(sd_app->view_holder);
    free_view_holder(sd_app);
    string_set_str(sd_app->text_holder, "");
    sd_app->sd_app_state = SdAppStateBackground;
}

/******************* Main app *******************/

int32_t sd_filesystem(void* p) {
    SdApp* sd_app = sd_app_alloc();
    FS_Api* fs_api = fs_api_alloc();

    Gui* gui = furi_record_open("gui");
    Cli* cli = furi_record_open("cli");
    sd_app->notifications = furi_record_open("notification");
    ValueMutex* menu_vm = furi_record_open("menu");

    gui_add_view_port(gui, sd_app->icon.view_port, GuiLayerStatusBarLeft);

    cli_add_command(cli, "sd_format", cli_sd_format, sd_app);
    cli_add_command(cli, "sd_info", cli_sd_info, sd_app);

    // add api record
    furi_record_create("sdcard", fs_api);

    // init menu
    // TODO menu icon
    MenuItem* menu_item;
    menu_item = menu_item_alloc_menu("SD Card", icon_animation_alloc(&I_SDcardMounted_11x8));

    menu_item_subitem_add(
        menu_item, menu_item_alloc_function("Info", NULL, app_sd_info_callback, sd_app));
    menu_item_subitem_add(
        menu_item, menu_item_alloc_function("Format", NULL, app_sd_format_callback, sd_app));
    menu_item_subitem_add(
        menu_item, menu_item_alloc_function("Eject", NULL, app_sd_eject_callback, sd_app));

    // add item to menu
    furi_check(menu_vm);
    with_value_mutex(
        menu_vm, (Menu * menu) { menu_item_add(menu, menu_item); });

    FURI_LOG_I("SD FILESYSTEM", "start");

    // add api record
    furi_record_create("sdcard", fs_api);
    furi_record_create("sdcard-ex", &sd_app->sd_card_api);

    // sd card cycle
    bool sd_was_present = true;

    // init detect pins
    hal_sd_detect_init();

    while(true) {
        if(sd_was_present) {
            if(hal_sd_detect()) {
                FURI_LOG_I("SD FILESYSTEM", "Card detected");
                app_sd_mount_card(sd_app);

                if(sd_app->info.status != SD_OK) {
                    FURI_LOG_E(
                        "SD FILESYSTEM",
                        "sd init error: %s",
                        fs_error_get_internal_desc(sd_app->info.status));
                    app_sd_notify_error(sd_app);
                } else {
                    FURI_LOG_I("SD FILESYSTEM", "sd init ok");
                    app_sd_notify_success(sd_app);
                }

                view_port_enabled_set(sd_app->icon.view_port, true);
                sd_was_present = false;

                if(!hal_sd_detect()) {
                    FURI_LOG_I("SD FILESYSTEM", "card removed");

                    view_port_enabled_set(sd_app->icon.view_port, false);
                    app_sd_unmount_card(sd_app);
                    sd_was_present = true;
                }
            }
        } else {
            if(!hal_sd_detect()) {
                FURI_LOG_I("SD FILESYSTEM", "card removed");

                view_port_enabled_set(sd_app->icon.view_port, false);
                app_sd_unmount_card(sd_app);
                sd_was_present = true;
                app_sd_notify_eject(sd_app);
            }
        }

        SdAppEvent event;
        osStatus_t event_status = osMessageQueueGet(sd_app->event_queue, &event, NULL, 1000);

        const uint8_t y_1_line = 32;
        const uint8_t y_2_line = 32;
        const uint8_t y_4_line = 26;

        if(event_status == osOK) {
            switch(event.type) {
            case SdAppEventTypeOK:
                switch(sd_app->sd_app_state) {
                case SdAppStateFormat: {
                    DialogEx* dialog = view_holder_get_free_context(sd_app->view_holder);
                    dialog_ex_set_left_button_text(dialog, NULL);
                    dialog_ex_set_right_button_text(dialog, NULL);
                    dialog_ex_set_header(
                        dialog, "Formatting...", 64, y_1_line, AlignCenter, AlignCenter);
                    dialog_ex_set_text(dialog, NULL, 0, 0, AlignCenter, AlignCenter);
                    sd_app->sd_app_state = SdAppStateFormatInProgress;
                    delay(100);
                    app_sd_format_internal(sd_app);
                    app_sd_notify_success(sd_app);
                    dialog_ex_set_left_button_text(dialog, "Back");
                    dialog_ex_set_header(
                        dialog, "SD card formatted", 64, 10, AlignCenter, AlignCenter);
                    dialog_ex_set_text(
                        dialog, "Press back to return", 64, y_1_line, AlignCenter, AlignCenter);
                    sd_app->sd_app_state = SdAppStateFormatCompleted;
                }; break;
                case SdAppStateEject: {
                    DialogEx* dialog = view_holder_get_free_context(sd_app->view_holder);
                    dialog_ex_set_right_button_text(dialog, NULL);
                    dialog_ex_set_header(
                        dialog, "SD card ejected", 64, 10, AlignCenter, AlignCenter);
                    dialog_ex_set_text(
                        dialog,
                        "Now the SD card\ncan be removed.",
                        64,
                        y_2_line,
                        AlignCenter,
                        AlignCenter);
                    sd_app->sd_app_state = SdAppStateEjected;
                    app_sd_unmount_card(sd_app);
                    app_sd_notify_eject(sd_app);
                }; break;
                case SdAppStateFileSelect: {
                    SdAppFileSelectResultEvent retval = {.result = true};
                    furi_check(
                        osMessageQueuePut(sd_app->result_receiver, &retval, 0, osWaitForever) ==
                        osOK);
                    app_reset_state(sd_app);
                }; break;
                default:
                    break;
                }
                break;
            case SdAppEventTypeBack:
                switch(sd_app->sd_app_state) {
                case SdAppStateFormatInProgress:
                    break;
                case SdAppStateFileSelect: {
                    SdAppFileSelectResultEvent retval = {.result = false};
                    furi_check(
                        osMessageQueuePut(sd_app->result_receiver, &retval, 0, osWaitForever) ==
                        osOK);
                    app_reset_state(sd_app);
                }; break;

                default:
                    app_reset_state(sd_app);
                    break;
                }
                break;
            case SdAppEventTypeFormat:
                if(try_to_alloc_view_holder(sd_app, gui)) {
                    DialogEx* dialog = alloc_and_attach_dialog(sd_app);
                    dialog_ex_set_left_button_text(dialog, "Back");
                    dialog_ex_set_right_button_text(dialog, "Format");
                    dialog_ex_set_header(
                        dialog, "Format SD card?", 64, 10, AlignCenter, AlignCenter);
                    dialog_ex_set_text(
                        dialog, "All data will be lost.", 64, y_1_line, AlignCenter, AlignCenter);
                    view_holder_start(sd_app->view_holder);
                    sd_app->sd_app_state = SdAppStateFormat;
                }
                break;
            case SdAppEventTypeInfo:
                if(try_to_alloc_view_holder(sd_app, gui)) {
                    DialogEx* dialog = alloc_and_attach_dialog(sd_app);
                    dialog_ex_set_left_button_text(dialog, "Back");

                    SDInfo sd_info;
                    get_sd_info(sd_app, &sd_info);

                    if(sd_info.error == SD_OK) {
                        string_printf(
                            sd_app->text_holder,
                            "Label: %s\nType: %s\n%lu KB total\n%lu KB free",
                            sd_info.label,
                            get_fs_type_text(sd_info.fs_type),
                            sd_info.kb_total,
                            sd_info.kb_free);
                        dialog_ex_set_text(
                            dialog,
                            string_get_cstr(sd_app->text_holder),
                            4,
                            y_4_line,
                            AlignLeft,
                            AlignCenter);
                        view_holder_start(sd_app->view_holder);
                    } else {
                        string_printf(
                            sd_app->text_holder,
                            "SD status: %s\n SD info: %s",
                            fs_error_get_internal_desc(_fs_status(&sd_app->info)),
                            fs_error_get_internal_desc(sd_info.error));
                        dialog_ex_set_header(dialog, "Error", 64, 10, AlignCenter, AlignCenter);
                        dialog_ex_set_text(
                            dialog,
                            string_get_cstr(sd_app->text_holder),
                            64,
                            y_2_line,
                            AlignCenter,
                            AlignCenter);
                        view_holder_start(sd_app->view_holder);
                    }

                    sd_app->sd_app_state = SdAppStateInfo;
                }
                break;
            case SdAppEventTypeEject:
                if(try_to_alloc_view_holder(sd_app, gui)) {
                    DialogEx* dialog = alloc_and_attach_dialog(sd_app);
                    dialog_ex_set_left_button_text(dialog, "Back");
                    dialog_ex_set_right_button_text(dialog, "Eject");
                    dialog_ex_set_header(
                        dialog, "Eject SD card?", 64, 10, AlignCenter, AlignCenter);
                    dialog_ex_set_text(
                        dialog,
                        "SD card will be\nunavailable",
                        64,
                        y_2_line,
                        AlignCenter,
                        AlignCenter);
                    view_holder_start(sd_app->view_holder);
                    sd_app->sd_app_state = SdAppStateEject;
                }
                break;
            case SdAppEventTypeFileSelect:
                if(!app_sd_make_path(event.payload.file_select_data.path)) {
                    SdAppFileSelectResultEvent retval = {.result = false};
                    furi_check(
                        osMessageQueuePut(sd_app->result_receiver, &retval, 0, osWaitForever) ==
                        osOK);
                    break;
                }
                if(try_to_alloc_view_holder(sd_app, gui)) {
                    FileSelect* file_select = alloc_and_attach_file_select(sd_app);
                    SdAppFileSelectData* file_select_data = &event.payload.file_select_data;

                    file_select_set_api(file_select, fs_api);
                    file_select_set_filter(
                        file_select, file_select_data->path, file_select_data->extension);
                    file_select_set_result_buffer(
                        file_select, file_select_data->result, file_select_data->result_size);
                    if(!file_select_init(file_select)) {
                        SdAppFileSelectResultEvent retval = {.result = false};
                        furi_check(
                            osMessageQueuePut(
                                sd_app->result_receiver, &retval, 0, osWaitForever) == osOK);
                        app_reset_state(sd_app);
                    } else {
                        sd_app->sd_app_state = SdAppStateFileSelect;
                        if(file_select_data->selected_filename != NULL) {
                            file_select_set_selected_file(
                                file_select, file_select_data->selected_filename);
                        }
                        view_holder_start(sd_app->view_holder);
                    }
                } else {
                    SdAppFileSelectResultEvent retval = {.result = false};
                    furi_check(
                        osMessageQueuePut(sd_app->result_receiver, &retval, 0, osWaitForever) ==
                        osOK);
                }
                break;
            case SdAppEventTypeCheckError:
                if(sd_app->info.status != SD_OK) {
                    if(try_to_alloc_view_holder(sd_app, gui)) {
                        DialogEx* dialog = alloc_and_attach_dialog(sd_app);
                        dialog_ex_set_left_button_text(dialog, "Back");
                        if(sd_app->info.status == SD_NO_CARD) {
                            dialog_ex_set_text(
                                dialog,
                                "SD card\nnot found",
                                88,
                                y_1_line,
                                AlignCenter,
                                AlignCenter);
                            dialog_ex_set_icon(dialog, 5, 6, &I_SDQuestion_35x43);
                        } else {
                            dialog_ex_set_text(
                                dialog, "SD card\nerror", 88, y_1_line, AlignCenter, AlignCenter);
                            dialog_ex_set_icon(dialog, 5, 10, &I_SDError_43x35);
                        }
                        sd_app->sd_app_state = SdAppStateCheckError;
                        view_holder_start(sd_app->view_holder);
                    }
                }
                break;
            case SdAppEventTypeShowError:
                if(try_to_alloc_view_holder(sd_app, gui)) {
                    DialogEx* dialog = alloc_and_attach_dialog(sd_app);
                    dialog_ex_set_left_button_text(dialog, "Back");
                    dialog_ex_set_text(
                        dialog, event.payload.error_text, 88, y_1_line, AlignCenter, AlignCenter);
                    dialog_ex_set_icon(dialog, 5, 6, &I_SDQuestion_35x43);
                    sd_app->sd_app_state = SdAppStateShowError;
                    view_holder_start(sd_app->view_holder);
                }
                break;
            }
        }
    }

    return 0;
}
