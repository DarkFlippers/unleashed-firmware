#include "../subghz_i.h"
#include "subghz/types.h"
#include "../helpers/subghz_custom_event.h"
#include <lib/toolbox/value_index.h>
#include <machine/endian.h>
#include <toolbox/strint.h>

#define TAG "SubGhzSceneSignalSettings"

static uint32_t counter_mode = 0xff;
static uint32_t loaded_counter32 = 0x0;
static uint32_t counter32 = 0x0;
static uint16_t counter16 = 0x0;
static uint8_t byte_count = 0;
static uint8_t* byte_ptr = NULL;
static uint8_t hex_char_lenght = 0;
static FuriString* byte_input_text;

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

// our special case function based on strint_to_uint32 from strint.c
StrintParseError strint_to_uint32_base16(const char* str, uint32_t* out, uint8_t* lenght) {
    // skip whitespace
    while(((*str >= '\t') && (*str <= '\r')) || *str == ' ') {
        str++;
    }

    // read digits
    uint32_t limit = UINT32_MAX;
    uint32_t mul_limit = limit / 16;
    uint32_t result = 0;
    int read_total = 0;

    while(*str != 0) {
        int digit_value;
        if(*str >= '0' && *str <= '9') {
            digit_value = *str - '0';
        } else if(*str >= 'A' && *str <= 'Z') {
            digit_value = *str - 'A' + 10;
        } else if(*str >= 'a' && *str <= 'z') {
            digit_value = *str - 'a' + 10;
        } else {
            break;
        }

        if(digit_value >= 16) {
            break;
        }

        if(result > mul_limit) return StrintParseOverflowError;
        result *= 16;
        if(result > limit - digit_value) return StrintParseOverflowError; //-V658
        result += digit_value;

        read_total++;
        str++;
    }

    if(read_total == 0) {
        result = 0;
        *lenght = 0;
        return StrintParseAbsentError;
    }

    if(out) *out = result;
    if(lenght) *lenght = read_total;
    return StrintParseNoError;
}

void subghz_scene_signal_settings_counter_mode_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, counter_mode_text[index]);
    counter_mode = counter_mode_value[index];
}

void subghz_scene_signal_settings_byte_input_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_signal_settings_variable_item_list_enter_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;

    // when we click OK on "Edit counter" item
    if(index == 1) {
        furi_string_cat_printf(byte_input_text, "%i", hex_char_lenght * 4);
        furi_string_cat_str(byte_input_text, "-bit counter in HEX");

        // Setup byte_input view
        ByteInput* byte_input = subghz->byte_input;
        byte_input_set_header_text(byte_input, furi_string_get_cstr(byte_input_text));

        byte_input_set_result_callback(
            byte_input,
            subghz_scene_signal_settings_byte_input_callback,
            NULL,
            subghz,
            byte_ptr,
            byte_count);
        view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
    }
}

void subghz_scene_signal_settings_on_enter(void* context) {
    SubGhz* subghz = context;

    // ### Counter mode section ###

    // When we open saved file we do some check and fill up subghz->file_path.
    // So now we use it to check is there CounterMode in file or not
    const char* file_path = furi_string_get_cstr(subghz->file_path);

    furi_assert(subghz);
    furi_assert(file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    FuriString* tmp_text = furi_string_alloc_set_str("");

    uint32_t tmp_counter_mode = 0;
    counter_mode = 0xff;
    uint8_t mode_count = 1;

    // Open file and check is it contains allowed protocols and CounterMode variable - if not then CcounterMode will stay 0xff
    // if file contain allowed protocol but not contain CounterMode value then setup default CounterMode value = 0 and available CounterMode count for this protocol
    // if file contain CounterMode value then load it
    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Error open file %s", file_path);
    } else {
        flipper_format_read_string(fff_data_file, "Protocol", tmp_text);
        // compare available protocols names, load CounterMode value from file and setup variable_item_list values_count
        for(uint8_t i = 0; i < PROTOCOLS_COUNT i++) {
            if(!strcmp(furi_string_get_cstr(tmp_text), protocols[i].name)) {
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

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    // ### Counter edit section ###
    FuriString* textCnt = furi_string_alloc_set_str("");
    byte_input_text = furi_string_alloc_set_str("Enter ");
    furi_string_reset(tmp_text);

    bool counter_not_available = true;
    SubGhzProtocolDecoderBase* decoder = subghz_txrx_get_decoder(subghz->txrx);

    // deserialaze and decode loaded sugbhz file and take information string from decoder
    if(subghz_protocol_decoder_base_deserialize(decoder, subghz_txrx_get_fff_data(subghz->txrx)) ==
       SubGhzProtocolStatusOk) {
        subghz_protocol_decoder_base_get_string(decoder, tmp_text);
    } else {
        FURI_LOG_E(TAG, "Cant deserialize this subghz file");
    }

    // In protocols output we allways have HEX format for "Cnt:" output (text formating like ...Cnt:%05lX\r\n")
    // we take 8 simbols starting from  "Cnt:........"
    // at first we search "Cnt:????" that mean for this protocol counter cannot be decoded

    int8_t place = furi_string_search_str(tmp_text, "Cnt:??", 0);
    if(place > 0) {
        counter_mode = 0xff;
        FURI_LOG_D(
            TAG, "Founded Cnt:???? - Counter mode and edit not available for this protocol");
    } else {
        place = furi_string_search_str(tmp_text, "Cnt:", 0);
        if(place > 0) {
            // defence from memory leaks. Check can we take 8 symbols after 'Cnt:' ?
            // if from current place to end of stirngs more than 8 symbols - ok, if not - just take symbols from current place to end of string.
            // +4 - its 'Cnt:' lenght
            uint8_t n_symbols_taken = 8;
            if(sizeof(tmp_text) - (place + 4) < 8) {
                n_symbols_taken = sizeof(tmp_text) - (place + 4);
            }
            furi_string_set_n(textCnt, tmp_text, place + 4, n_symbols_taken);
            furi_string_trim(textCnt);
            FURI_LOG_D(
                TAG,
                "Taked 8 bytes hex value starting after 'Cnt:' - %s",
                furi_string_get_cstr(textCnt));

            // trim and convert 8 simbols string to uint32 by base 16 (hex);
            // later we use loaded_counter in subghz_scene_signal_settings_on_event to check is there 0 or not - special case

            if(strint_to_uint32_base16(
                   furi_string_get_cstr(textCnt), &loaded_counter32, &hex_char_lenght) ==
               StrintParseNoError) {
                counter_not_available = false;

                // calculate and roundup number of hex bytes do display counter in byte_input (every 2 hex simbols = 1 byte for view)
                // later must be used in byte_input to restrict number of available byte to edit
                // cnt_byte_count = (hex_char_lenght + 1) / 2;

                FURI_LOG_D(
                    TAG,
                    "Result of conversion from String to uint_32 DEC %li, HEX %lX, HEX lenght %i symbols",
                    loaded_counter32,
                    loaded_counter32,
                    hex_char_lenght);

                // Check is there byte_count more than 2 hex bytes long (16 bit) or not (32bit)
                // To show hex value we must correct revert bytes for ByteInput view
                if(hex_char_lenght > 4) {
                    counter32 = loaded_counter32;
                    furi_string_printf(tmp_text, "%lX", counter32);
                    counter32 = __bswap32(counter32);
                    byte_ptr = (uint8_t*)&counter32;
                    byte_count = 4;
                } else {
                    counter16 = loaded_counter32;
                    furi_string_printf(tmp_text, "%X", counter16);
                    counter16 = __bswap16(counter16);
                    byte_ptr = (uint8_t*)&counter16;
                    byte_count = 2;
                }
            } else {
                FURI_LOG_E(TAG, "Cant convert text counter value");
            };

        } else {
            FURI_LOG_D(TAG, "Counter editor not available for this protocol");
        }
    }

    furi_assert(byte_ptr);
    furi_assert(byte_count > 0);

    //Create and Enable/Disable variable_item_list depent from current values
    VariableItemList* variable_item_list = subghz->variable_item_list;
    int32_t value_index;
    VariableItem* item;

    // variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    // variable_item_list_reset(subghz->variable_item_list);

    variable_item_list_set_enter_callback(
        variable_item_list,
        subghz_scene_signal_settings_variable_item_list_enter_callback,
        subghz);

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

    item = variable_item_list_add(variable_item_list, "Edit Counter", 1, NULL, subghz);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, furi_string_get_cstr(tmp_text));
    variable_item_set_locked(item, (counter_not_available), "Not available\nfor this\nprotocol !");

    furi_string_free(tmp_text);
    furi_string_free(textCnt);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_signal_settings_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    int32_t tmp_counter = 0;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            switch(byte_count) {
            case 2:
                // when signal has Cnt:00 we can step only to 0000+FFFF = FFFF, but we need 0000 for next step
                // for this case we must use +1 additional step to increace Cnt from FFFF to 0000.

                // save current user definded counter increase value (mult)
                tmp_counter = furi_hal_subghz_get_rolling_counter_mult();

                // increase signal counter to max value - at result it must be 0000 in most cases
                // but can be FFFF in case Cnt:0000 (for this we have +1 additional step below)
                furi_hal_subghz_set_rolling_counter_mult(0xFFFF);
                subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                subghz_txrx_stop(subghz->txrx);

                // if file Cnt:00 then do +1 additional step
                if(loaded_counter32 == 0) {
                    furi_hal_subghz_set_rolling_counter_mult(1);
                    subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                    subghz_txrx_stop(subghz->txrx);
                }

                // at this point we must have signal Cnt:00
                // convert back after byte_input and do one send with our new mult (counter16) - at end we must have signal Cnt = counter16
                counter16 = __bswap16(counter16);

                furi_hal_subghz_set_rolling_counter_mult(counter16);
                subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                subghz_txrx_stop(subghz->txrx);

                // restore user definded counter increase value (mult)
                furi_hal_subghz_set_rolling_counter_mult(tmp_counter);

                break;
            case 4:
                // the same for 32 bit Counter
                tmp_counter = furi_hal_subghz_get_rolling_counter_mult();

                furi_hal_subghz_set_rolling_counter_mult(0xFFFFFFF);
                subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                subghz_txrx_stop(subghz->txrx);

                if(loaded_counter32 == 0) {
                    furi_hal_subghz_set_rolling_counter_mult(1);
                    subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                    subghz_txrx_stop(subghz->txrx);
                }

                counter32 = __bswap32(counter32);

                furi_hal_subghz_set_rolling_counter_mult((counter32 & 0xFFFFFFF));
                subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                subghz_txrx_stop(subghz->txrx);

                furi_hal_subghz_set_rolling_counter_mult(tmp_counter);
                break;
            default:
                break;
            }

            scene_manager_previous_scene(subghz->scene_manager);
            return true;

        } else {
            if(event.type == SceneManagerEventTypeBack) {
                scene_manager_previous_scene(subghz->scene_manager);
                return true;
            }
        }
    }
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

    // Clear views
    variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    variable_item_list_reset(subghz->variable_item_list);
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
    furi_string_free(byte_input_text);
}
