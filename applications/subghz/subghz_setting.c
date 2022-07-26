#include "subghz_setting.h"
#include "subghz_i.h"

#include <furi.h>
#include <m-list.h>
#include <lib/flipper_format/flipper_format.h>

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

LIST_DEF(FrequencyList, uint32_t)

#define M_OPL_FrequencyList_t() LIST_OPLIST(FrequencyList)

struct SubGhzSetting {
    FrequencyList_t frequencies;
    FrequencyList_t hopper_frequencies;
};

SubGhzSetting* subghz_setting_alloc(void) {
    SubGhzSetting* instance = malloc(sizeof(SubGhzSetting));
    FrequencyList_init(instance->frequencies);
    FrequencyList_init(instance->hopper_frequencies);
    return instance;
}

void subghz_setting_free(SubGhzSetting* instance) {
    furi_assert(instance);
    FrequencyList_clear(instance->frequencies);
    FrequencyList_clear(instance->hopper_frequencies);
    free(instance);
}

static void subghz_setting_load_default_region(
    SubGhzSetting* instance,
    const uint32_t frequencies[],
    const uint32_t hopper_frequencies[]) {
    furi_assert(instance);

    FrequencyList_reset(instance->frequencies);
    FrequencyList_reset(instance->hopper_frequencies);

    while(*frequencies) {
        FrequencyList_push_back(instance->frequencies, *frequencies);
        frequencies++;
    }

    while(*hopper_frequencies) {
        FrequencyList_push_back(instance->hopper_frequencies, *hopper_frequencies);
        hopper_frequencies++;
    }
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
            flipper_format_read_bool(fff_data_file, "add_standard_frequencies", &temp_bool, 1);
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
                fff_data_file, "frequency", (uint32_t*)&temp_data32, 1)) {
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
                fff_data_file, "hopper_frequency", (uint32_t*)&temp_data32, 1)) {
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
            if(flipper_format_read_uint32(fff_data_file, "default_frequency", &temp_data32, 1)) {
                for
                    M_EACH(frequency, instance->frequencies, FrequencyList_t) {
                        *frequency &= FREQUENCY_MASK;
                        if(*frequency == temp_data32) {
                            *frequency |= FREQUENCY_FLAG_DEFAULT;
                        }
                    }
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
