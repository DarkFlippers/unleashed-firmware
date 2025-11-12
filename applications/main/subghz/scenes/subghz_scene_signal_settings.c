#include "../subghz_i.h"
#include "subghz/types.h"
#include "../helpers/subghz_custom_event.h"
#include <lib/toolbox/value_index.h>

#define TAG "SubGhzSceneSignalSettings"

static uint32_t counter_mode = 0xff;

#define COUNTER_MODE_COUNT 7
static const char* const counter_mode_text[COUNTER_MODE_COUNT] = {
    "System",
    "Mode 1",
    "Mode 2",
    "Mode 3",
    "Mode 4",
    "Mode 5",
    "Mode 6",
};

static const int32_t counter_mode_value[COUNTER_MODE_COUNT] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
};

typedef struct {
    char* name;
    uint8_t mode_count;
} Protocols;

// List of protocols names and appropriate CounterMode counts
static Protocols protocols[] = {
    {"Nice FloR-S", 3},
    {"CAME Atomo", 4},
    {"Alutech AT-4N", 3},
    {"KeeLoq", 7},
};

#define PROTOCOLS_COUNT (sizeof(protocols) / sizeof(Protocols));

void subghz_scene_signal_settings_counter_mode_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, counter_mode_text[index]);
    counter_mode = counter_mode_value[index];
}

void subghz_scene_signal_settings_on_enter(void* context) {
    // When we open saved file we do some check and fill up subghz->file_path.
    // So now we use it to check is there CounterMode in file or not
    SubGhz* subghz = context;

    const char* file_path = furi_string_get_cstr(subghz->file_path);

    furi_assert(subghz);
    furi_assert(file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    FuriString* tmp_string = furi_string_alloc();

    uint32_t tmp_counter_mode = 0;
    counter_mode = 0xff;
    uint8_t mode_count = 1;

    // Open file and check is it contains allowed protocols and CounterMode variable - if not then CcounterMode will stay 0xff
    // if file contain allowed protocol but not contain CounterMode value then setup default CounterMode value = 0 and available CounterMode count for this protocol
    // if file contain CounterMode value then load it
    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Error open file %s", file_path);
    } else {
        flipper_format_read_string(fff_data_file, "Protocol", tmp_string);
        // compare available protocols names, load CounterMode value from file and setup variable_item_list values_count
        for(uint8_t i = 0; i < PROTOCOLS_COUNT i++) {
            if(!strcmp(furi_string_get_cstr(tmp_string), protocols[i].name)) {
                mode_count = protocols[i].mode_count;
                if(flipper_format_read_uint32(fff_data_file, "CounterMode", &tmp_counter_mode, 1)) {
                    counter_mode = (uint8_t)tmp_counter_mode;
                } else {
                    counter_mode = 0;
                }
            }
        }
    }
    FURI_LOG_D(TAG, "Current CounterMode value %li", counter_mode);

    furi_string_free(tmp_string);
    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    //Create and Enable/Disable variable_item_list depent from current CounterMode value
    VariableItemList* variable_item_list = subghz->variable_item_list;
    int32_t value_index;
    VariableItem* item;

    item = variable_item_list_add(
        variable_item_list,
        "Counter Mode",
        mode_count,
        subghz_scene_signal_settings_counter_mode_changed,
        subghz);
    value_index = value_index_int32(counter_mode, counter_mode_value, mode_count);

    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, counter_mode_text[value_index]);
    variable_item_set_locked(item, (counter_mode == 0xff), "Not available\nfor this\nprotocol !");

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_signal_settings_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(subghz->scene_manager);
        return true;
    } else
        return false;
}

void subghz_scene_signal_settings_on_exit(void* context) {
    SubGhz* subghz = context;
    const char* file_path = furi_string_get_cstr(subghz->file_path);

    furi_assert(subghz);
    furi_assert(file_path);

    // if ConterMode was changed from 0xff then we must update or write new value to file
    if(counter_mode != 0xff) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

        // check is the file available for update/insert CounterMode value
        if(flipper_format_file_open_existing(fff_data_file, file_path)) {
            if(flipper_format_insert_or_update_uint32(
                   fff_data_file, "CounterMode", &counter_mode, 1)) {
                FURI_LOG_D(
                    TAG, "Successfully updated/inserted CounterMode value %li", counter_mode);
            } else {
                FURI_LOG_E(TAG, "Error update/insert CounterMode value");
            }
        } else {
            FURI_LOG_E(TAG, "Error open file %s for writing", file_path);
        }

        flipper_format_file_close(fff_data_file);
        flipper_format_free(fff_data_file);
        furi_record_close(RECORD_STORAGE);
    }

    variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    variable_item_list_reset(subghz->variable_item_list);
}
