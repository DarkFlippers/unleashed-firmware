#include "../storage_settings.h"
#include <furi_hal.h>

#define BENCH_DATA_SIZE 4096
#define BENCH_COUNT 6
#define BENCH_REPEATS 4
#define BENCH_FILE EXT_PATH("rwfiletest.bin")

static void
    storage_settings_scene_benchmark_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static bool storage_settings_scene_bench_write(
    Storage* api,
    uint16_t size,
    const uint8_t* data,
    uint32_t* speed) {
    File* file = storage_file_alloc(api);
    bool result = true;
    if(storage_file_open(file, BENCH_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint32_t ticks;
        ticks = furi_get_tick();

        for(size_t repeat = 0; repeat < BENCH_REPEATS; repeat++) {
            for(size_t i = 0; i < BENCH_DATA_SIZE / size; i++) {
                if(storage_file_write(file, (data + i * size), size) != size) {
                    result = false;
                    break;
                }
            }
        }

        ticks = furi_get_tick() - ticks;
        *speed = BENCH_DATA_SIZE * furi_kernel_get_tick_frequency() * BENCH_REPEATS;
        *speed /= ticks;
        *speed /= 1024;
    }
    storage_file_close(file);
    storage_file_free(file);
    return result;
}

static bool
    storage_settings_scene_bench_read(Storage* api, uint16_t size, uint8_t* data, uint32_t* speed) {
    File* file = storage_file_alloc(api);
    bool result = true;
    *speed = -1;

    if(storage_file_open(file, BENCH_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint32_t ticks;
        ticks = furi_get_tick();

        for(size_t repeat = 0; repeat < BENCH_REPEATS; repeat++) {
            for(size_t i = 0; i < BENCH_DATA_SIZE / size; i++) {
                if(storage_file_read(file, (data + i * size), size) != size) {
                    result = false;
                    break;
                }
            }
        }

        ticks = furi_get_tick() - ticks;
        *speed = BENCH_DATA_SIZE * furi_kernel_get_tick_frequency() * BENCH_REPEATS;
        *speed /= ticks;
        *speed /= 1024;
    }
    storage_file_close(file);
    storage_file_free(file);
    return result;
}

static void storage_settings_scene_benchmark(StorageSettings* app) {
    DialogEx* dialog_ex = app->dialog_ex;
    uint8_t* bench_data;
    dialog_ex_set_header(dialog_ex, "Preparing Data...", 64, 32, AlignCenter, AlignCenter);

    bench_data = malloc(BENCH_DATA_SIZE);
    for(size_t i = 0; i < BENCH_DATA_SIZE; i++) {
        bench_data[i] = (uint8_t)i;
    }

    uint16_t bench_size[BENCH_COUNT] = {1, 8, 32, 256, 512, 1024};
    uint32_t bench_w_speed[BENCH_COUNT] = {0, 0, 0, 0, 0, 0};
    uint32_t bench_r_speed[BENCH_COUNT] = {0, 0, 0, 0, 0, 0};

    dialog_ex_set_header(dialog_ex, "Benchmarking...", 64, 32, AlignCenter, AlignCenter);
    for(size_t i = 0; i < BENCH_COUNT; i++) {
        if(!storage_settings_scene_bench_write(
               app->fs_api, bench_size[i], bench_data, &bench_w_speed[i]))
            break;

        if(i > 0) string_cat_printf(app->text_string, "\n");
        string_cat_printf(app->text_string, "%ub : W %luK ", bench_size[i], bench_w_speed[i]);
        dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex, string_get_cstr(app->text_string), 0, 32, AlignLeft, AlignCenter);

        if(!storage_settings_scene_bench_read(
               app->fs_api, bench_size[i], bench_data, &bench_r_speed[i]))
            break;

        string_cat_printf(app->text_string, "R %luK", bench_r_speed[i]);
        dialog_ex_set_text(
            dialog_ex, string_get_cstr(app->text_string), 0, 32, AlignLeft, AlignCenter);
    }

    free(bench_data);
}

void storage_settings_scene_benchmark_on_enter(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    FS_Error sd_status = storage_sd_status(app->fs_api);
    scene_manager_set_scene_state(app->scene_manager, StorageSettingsBenchmark, sd_status);

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, storage_settings_scene_benchmark_dialog_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);

    if(sd_status != FSE_OK) {
        dialog_ex_set_icon(dialog_ex, 72, 14, &I_DolphinFirstStart8_56x51);
        dialog_ex_set_header(dialog_ex, "SD Card Not Mounted", 64, 3, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog_ex, "Try to reinsert\nor format SD\ncard.", 3, 19, AlignLeft, AlignTop);
        dialog_ex_set_center_button_text(dialog_ex, "Ok");
    } else {
        storage_settings_scene_benchmark(app);
        notification_message(app->notification, &sequence_blink_green_100);
    }
}

bool storage_settings_scene_benchmark_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    FS_Error sd_status =
        scene_manager_get_scene_state(app->scene_manager, StorageSettingsBenchmark);

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultCenter:
            consumed = scene_manager_previous_scene(app->scene_manager);
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack && sd_status != FSE_OK) {
        consumed = true;
    }

    return consumed;
}

void storage_settings_scene_benchmark_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(dialog_ex);

    string_reset(app->text_string);
}
