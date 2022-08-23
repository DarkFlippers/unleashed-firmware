#include "subghz_setting.h"
#include "subghz_i.h"

#include <furi.h>
#include <m-list.h>
#include "furi_hal_subghz_configs.h"

#define TAG "SubGhzSetting"

#define SUBGHZ_SETTING_FILE_TYPE "Flipper SubGhz Setting File"
#define SUBGHZ_SETTING_FILE_VERSION 1

#define FREQUENCY_FLAG_DEFAULT (1 << 31)
#define FREQUENCY_MASK (0xFFFFFFFF ^ FREQUENCY_FLAG_DEFAULT)

/* Default */
static const uint32_t subghz_frequency_list[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    310000000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000 | FREQUENCY_FLAG_DEFAULT, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};

static const uint32_t subghz_hopper_frequency_list[] = {
    310000000,
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};

/* Europe and Russia */
static const uint32_t subghz_frequency_list_region_eu_ru[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    310000000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000 | FREQUENCY_FLAG_DEFAULT, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequency_list_region_eu_ru[] = {
    310000000,
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};

/* Region 0 */
static const uint32_t subghz_frequency_list_region_us_ca_au[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    310000000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000 | FREQUENCY_FLAG_DEFAULT, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequency_list_region_us_ca_au[] = {
    310000000,
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};

static const uint32_t subghz_frequency_list_region_jp[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    310000000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000 | FREQUENCY_FLAG_DEFAULT, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequency_list_region_jp[] = {
    310000000,
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};

typedef struct {
    string_t custom_preset_name;
    uint8_t* custom_preset_data;
    size_t custom_preset_data_size;
} SubGhzSettingCustomPresetItem;

ARRAY_DEF(SubGhzSettingCustomPresetItemArray, SubGhzSettingCustomPresetItem, M_POD_OPLIST)

#define M_OPL_SubGhzSettingCustomPresetItemArray_t() \
    ARRAY_OPLIST(SubGhzSettingCustomPresetItemArray, M_POD_OPLIST)

LIST_DEF(FrequencyList, uint32_t)

#define M_OPL_FrequencyList_t() LIST_OPLIST(FrequencyList)

typedef struct {
    SubGhzSettingCustomPresetItemArray_t data;
} SubGhzSettingCustomPresetStruct;

struct SubGhzSetting {
    FrequencyList_t frequencies;
    FrequencyList_t hopper_frequencies;
    SubGhzSettingCustomPresetStruct* preset;
};

SubGhzSetting* subghz_setting_alloc(void) {
    SubGhzSetting* instance = malloc(sizeof(SubGhzSetting));
    FrequencyList_init(instance->frequencies);
    FrequencyList_init(instance->hopper_frequencies);
    instance->preset = malloc(sizeof(SubGhzSettingCustomPresetStruct));
    SubGhzSettingCustomPresetItemArray_init(instance->preset->data);
    return instance;
}

static void subghz_setting_preset_reset(SubGhzSetting* instance) {
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            string_clear(item->custom_preset_name);
            free(item->custom_preset_data);
        }
    SubGhzSettingCustomPresetItemArray_reset(instance->preset->data);
}

void subghz_setting_free(SubGhzSetting* instance) {
    furi_assert(instance);
    FrequencyList_clear(instance->frequencies);
    FrequencyList_clear(instance->hopper_frequencies);
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            string_clear(item->custom_preset_name);
            free(item->custom_preset_data);
        }
    SubGhzSettingCustomPresetItemArray_clear(instance->preset->data);
    free(instance->preset);
    free(instance);
}

static void subghz_setting_load_default_preset(
    SubGhzSetting* instance,
    const char* preset_name,
    const uint8_t* preset_data,
    const uint8_t preset_pa_table[8]) {
    furi_assert(instance);
    furi_assert(preset_data);
    uint32_t preset_data_count = 0;
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_push_raw(instance->preset->data);

    string_init(item->custom_preset_name);
    string_set(item->custom_preset_name, preset_name);

    while(preset_data[preset_data_count]) {
        preset_data_count += 2;
    }
    preset_data_count += 2;
    item->custom_preset_data_size = sizeof(uint8_t) * preset_data_count + sizeof(uint8_t) * 8;
    item->custom_preset_data = malloc(item->custom_preset_data_size);
    //load preset register
    memcpy(&item->custom_preset_data[0], &preset_data[0], preset_data_count);
    //load pa table
    memcpy(&item->custom_preset_data[preset_data_count], &preset_pa_table[0], 8);
}

static void subghz_setting_load_default_region(
    SubGhzSetting* instance,
    const uint32_t frequencies[],
    const uint32_t hopper_frequencies[]) {
    furi_assert(instance);

    FrequencyList_reset(instance->frequencies);
    FrequencyList_reset(instance->hopper_frequencies);
    subghz_setting_preset_reset(instance);

    while(*frequencies) {
        FrequencyList_push_back(instance->frequencies, *frequencies);
        frequencies++;
    }

    while(*hopper_frequencies) {
        FrequencyList_push_back(instance->hopper_frequencies, *hopper_frequencies);
        hopper_frequencies++;
    }

    subghz_setting_load_default_preset(
        instance,
        "AM270",
        (uint8_t*)furi_hal_subghz_preset_ook_270khz_async_regs,
        furi_hal_subghz_preset_ook_async_patable);
    subghz_setting_load_default_preset(
        instance,
        "AM650",
        (uint8_t*)furi_hal_subghz_preset_ook_650khz_async_regs,
        furi_hal_subghz_preset_ook_async_patable);
    subghz_setting_load_default_preset(
        instance,
        "FM238",
        (uint8_t*)furi_hal_subghz_preset_2fsk_dev2_38khz_async_regs,
        furi_hal_subghz_preset_2fsk_async_patable);
    subghz_setting_load_default_preset(
        instance,
        "FM476",
        (uint8_t*)furi_hal_subghz_preset_2fsk_dev47_6khz_async_regs,
        furi_hal_subghz_preset_2fsk_async_patable);
}

void subghz_setting_load_default(SubGhzSetting* instance) {
    switch(furi_hal_version_get_hw_region()) {
    case FuriHalVersionRegionEuRu:
        subghz_setting_load_default_region(
            instance,
            subghz_frequency_list_region_eu_ru,
            subghz_hopper_frequency_list_region_eu_ru);
        break;
    case FuriHalVersionRegionUsCaAu:
        subghz_setting_load_default_region(
            instance,
            subghz_frequency_list_region_us_ca_au,
            subghz_hopper_frequency_list_region_us_ca_au);
        break;
    case FuriHalVersionRegionJp:
        subghz_setting_load_default_region(
            instance, subghz_frequency_list_region_jp, subghz_hopper_frequency_list_region_jp);
        break;

    default:
        subghz_setting_load_default_region(
            instance, subghz_frequency_list, subghz_hopper_frequency_list);
        break;
    }
}

void subghz_setting_load(SubGhzSetting* instance, const char* file_path) {
    furi_assert(instance);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;
    bool temp_bool;

    subghz_setting_load_default(instance);

    if(file_path) {
        do {
            if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
                FURI_LOG_E(TAG, "Error open file %s", file_path);
                break;
            }

            if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
                FURI_LOG_E(TAG, "Missing or incorrect header");
                break;
            }

            if((!strcmp(string_get_cstr(temp_str), SUBGHZ_SETTING_FILE_TYPE)) &&
               temp_data32 == SUBGHZ_SETTING_FILE_VERSION) {
            } else {
                FURI_LOG_E(TAG, "Type or version mismatch");
                break;
            }

            // Standard frequencies (optional)
            temp_bool = true;
            flipper_format_read_bool(fff_data_file, "Add_standard_frequencies", &temp_bool, 1);
            if(!temp_bool) {
                FURI_LOG_I(TAG, "Removing standard frequencies");
                FrequencyList_reset(instance->frequencies);
                FrequencyList_reset(instance->hopper_frequencies);
            } else {
                FURI_LOG_I(TAG, "Keeping standard frequencies");
            }

            // Load frequencies
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            while(flipper_format_read_uint32(
                fff_data_file, "Frequency", (uint32_t*)&temp_data32, 1)) {
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Frequency loaded %lu", temp_data32);
                    FrequencyList_push_back(instance->frequencies, temp_data32);
                } else {
                    FURI_LOG_E(TAG, "Frequency not supported %lu", temp_data32);
                }
            }

            // Load hopper frequencies
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            while(flipper_format_read_uint32(
                fff_data_file, "Hopper_frequency", (uint32_t*)&temp_data32, 1)) {
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Hopper frequency loaded %lu", temp_data32);
                    FrequencyList_push_back(instance->hopper_frequencies, temp_data32);
                } else {
                    FURI_LOG_E(TAG, "Hopper frequency not supported %lu", temp_data32);
                }
            }

            // Default frequency (optional)
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            if(flipper_format_read_uint32(fff_data_file, "Default_frequency", &temp_data32, 1)) {
                for
                    M_EACH(frequency, instance->frequencies, FrequencyList_t) {
                        *frequency &= FREQUENCY_MASK;
                        if(*frequency == temp_data32) {
                            *frequency |= FREQUENCY_FLAG_DEFAULT;
                        }
                    }
            }

            // custom preset (optional)
            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            while(flipper_format_read_string(fff_data_file, "Custom_preset_name", temp_str)) {
                FURI_LOG_I(TAG, "Custom preset loaded %s", string_get_cstr(temp_str));
                subghz_setting_load_custom_preset(
                    instance, string_get_cstr(temp_str), fff_data_file);
            }

        } while(false);
    }

    string_clear(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    if(!FrequencyList_size(instance->frequencies) ||
       !FrequencyList_size(instance->hopper_frequencies)) {
        FURI_LOG_E(TAG, "Error loading user settings, loading default settings");
        subghz_setting_load_default(instance);
    }
}

size_t subghz_setting_get_frequency_count(SubGhzSetting* instance) {
    furi_assert(instance);
    return FrequencyList_size(instance->frequencies);
}

size_t subghz_setting_get_hopper_frequency_count(SubGhzSetting* instance) {
    furi_assert(instance);
    return FrequencyList_size(instance->hopper_frequencies);
}

size_t subghz_setting_get_preset_count(SubGhzSetting* instance) {
    furi_assert(instance);
    return SubGhzSettingCustomPresetItemArray_size(instance->preset->data);
}

const char* subghz_setting_get_preset_name(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return string_get_cstr(item->custom_preset_name);
}

int subghz_setting_get_inx_preset_by_name(SubGhzSetting* instance, const char* preset_name) {
    furi_assert(instance);
    size_t idx = 0;
    for
        M_EACH(item, instance->preset->data, SubGhzSettingCustomPresetItemArray_t) {
            if(strcmp(string_get_cstr(item->custom_preset_name), preset_name) == 0) {
                return idx;
            }
            idx++;
        }
    furi_crash("SubGhz: No name preset.");
    return -1;
}

bool subghz_setting_load_custom_preset(
    SubGhzSetting* instance,
    const char* preset_name,
    FlipperFormat* fff_data_file) {
    furi_assert(instance);
    furi_assert(preset_name);
    uint32_t temp_data32;
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_push_raw(instance->preset->data);
    string_init(item->custom_preset_name);
    string_set(item->custom_preset_name, preset_name);
    do {
        if(!flipper_format_get_value_count(fff_data_file, "Custom_preset_data", &temp_data32))
            break;
        if(!temp_data32 || (temp_data32 % 2)) {
            FURI_LOG_E(TAG, "Integrity error Custom_preset_data");
            break;
        }
        item->custom_preset_data_size = sizeof(uint8_t) * temp_data32;
        item->custom_preset_data = malloc(item->custom_preset_data_size);
        if(!flipper_format_read_hex(
               fff_data_file,
               "Custom_preset_data",
               item->custom_preset_data,
               item->custom_preset_data_size)) {
            FURI_LOG_E(TAG, "Missing Custom_preset_data");
            break;
        }
        return true;
    } while(true);
    return false;
}

bool subghz_setting_delete_custom_preset(SubGhzSetting* instance, const char* preset_name) {
    furi_assert(instance);
    furi_assert(preset_name);
    SubGhzSettingCustomPresetItemArray_it_t it;
    SubGhzSettingCustomPresetItemArray_it_last(it, instance->preset->data);
    while(!SubGhzSettingCustomPresetItemArray_end_p(it)) {
        SubGhzSettingCustomPresetItem* item = SubGhzSettingCustomPresetItemArray_ref(it);
        if(strcmp(string_get_cstr(item->custom_preset_name), preset_name) == 0) {
            string_clear(item->custom_preset_name);
            free(item->custom_preset_data);
            SubGhzSettingCustomPresetItemArray_remove(instance->preset->data, it);
            return true;
        }
        SubGhzSettingCustomPresetItemArray_previous(it);
    }
    return false;
}

uint8_t* subghz_setting_get_preset_data(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return item->custom_preset_data;
}

size_t subghz_setting_get_preset_data_size(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    SubGhzSettingCustomPresetItem* item =
        SubGhzSettingCustomPresetItemArray_get(instance->preset->data, idx);
    return item->custom_preset_data_size;
}

uint8_t* subghz_setting_get_preset_data_by_name(SubGhzSetting* instance, const char* preset_name) {
    furi_assert(instance);
    SubGhzSettingCustomPresetItem* item = SubGhzSettingCustomPresetItemArray_get(
        instance->preset->data, subghz_setting_get_inx_preset_by_name(instance, preset_name));
    return item->custom_preset_data;
}

uint32_t subghz_setting_get_frequency(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    uint32_t* ret = FrequencyList_get(instance->frequencies, idx);
    if(ret) {
        return (*ret) & FREQUENCY_MASK;
    } else {
        return 0;
    }
}

uint32_t subghz_setting_get_hopper_frequency(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    uint32_t* ret = FrequencyList_get(instance->hopper_frequencies, idx);
    if(ret) {
        return *ret;
    } else {
        return 0;
    }
}

uint32_t subghz_setting_get_frequency_default_index(SubGhzSetting* instance) {
    furi_assert(instance);
    for(size_t i = 0; i < FrequencyList_size(instance->frequencies); i++) {
        uint32_t frequency = *FrequencyList_get(instance->frequencies, i);
        if(frequency & FREQUENCY_FLAG_DEFAULT) {
            return i;
        }
    }
    return 0;
}

uint32_t subghz_setting_get_default_frequency(SubGhzSetting* instance) {
    furi_assert(instance);
    return subghz_setting_get_frequency(
        instance, subghz_setting_get_frequency_default_index(instance));
}
