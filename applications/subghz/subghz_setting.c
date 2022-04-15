#include "subghz_setting.h"
#include "subghz_i.h"

#include <furi.h>
#include <m-list.h>
#include <lib/flipper_format/flipper_format.h>

#define TAG "SubGhzSetting"

#define SUBGHZ_SETTING_FILE_VERSION 1
#define SUBGHZ_SETTING_FILE_TYPE "Flipper SubGhz Setting File"

typedef enum {
    SubGhzSettingStateNoLoad = 0,
    SubGhzSettingStateLoadFrequencyDefault,
    SubGhzSettingStateOkLoad,
} SubGhzSettingState;

static const uint32_t subghz_frequencies[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequencies[] = {
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};
static const uint32_t subghz_frequency_default_index = 9;

static const uint32_t subghz_frequencies_region_eu_ru[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequencies_region_eu_ru[] = {
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};
static const uint32_t subghz_frequency_default_index_region_eu_ru = 9;

static const uint32_t subghz_frequencies_region_us_ca_au[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequencies_region_us_ca_au[] = {
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};
static const uint32_t subghz_frequency_default_index_region_us_ca_au = 9;

static const uint32_t subghz_frequencies_region_jp[] = {
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
    0,
};
static const uint32_t subghz_hopper_frequencies_region_jp[] = {
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
    0,
};
static const uint32_t subghz_frequency_default_index_region_jp = 9;

LIST_DEF(FrequenciesList, uint32_t)

struct SubGhzSetting {
    FrequenciesList_t frequencies;
    FrequenciesList_t hopper_frequencies;
    size_t frequencies_count;
    size_t hopper_frequencies_count;
    uint32_t frequency_default_index;
};

SubGhzSetting* subghz_setting_alloc(void) {
    SubGhzSetting* instance = malloc(sizeof(SubGhzSetting));
    FrequenciesList_init(instance->frequencies);
    FrequenciesList_init(instance->hopper_frequencies);
    return instance;
}

void subghz_setting_free(SubGhzSetting* instance) {
    furi_assert(instance);
    FrequenciesList_clear(instance->frequencies);
    FrequenciesList_clear(instance->hopper_frequencies);
    free(instance);
}

void subghz_setting_load_default(
    SubGhzSetting* instance,
    const uint32_t frequencies[],
    const uint32_t hopper_frequencies[],
    const uint32_t frequency_default_index) {
    furi_assert(instance);
    size_t i = 0;
    FrequenciesList_clear(instance->frequencies);
    FrequenciesList_clear(instance->hopper_frequencies);
    i = 0;
    while(frequencies[i]) {
        FrequenciesList_push_back(instance->frequencies, frequencies[i]);
        i++;
    }
    instance->frequencies_count = i;

    i = 0;
    while(hopper_frequencies[i]) {
        FrequenciesList_push_back(instance->hopper_frequencies, hopper_frequencies[i]);
        i++;
    }
    instance->hopper_frequencies_count = i;

    instance->frequency_default_index = frequency_default_index;
}

void subghz_setting_load(SubGhzSetting* instance, const char* file_path) {
    furi_assert(instance);

    FrequenciesList_clear(instance->frequencies);
    FrequenciesList_clear(instance->hopper_frequencies);

    Storage* storage = furi_record_open("storage");
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;
    SubGhzSettingState loading = SubGhzSettingStateNoLoad;
    uint16_t i = 0;

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

            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            i = 0;
            while(flipper_format_read_uint32(
                fff_data_file, "Frequency", (uint32_t*)&temp_data32, 1)) {
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Frequency loaded %lu", temp_data32);
                    FrequenciesList_push_back(instance->frequencies, temp_data32);
                    i++;
                } else {
                    FURI_LOG_E(TAG, "Frequency not supported %lu", temp_data32);
                }
            }
            instance->frequencies_count = i;

            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            i = 0;
            while(flipper_format_read_uint32(
                fff_data_file, "Hopper_frequency", (uint32_t*)&temp_data32, 1)) {
                if(furi_hal_subghz_is_frequency_valid(temp_data32)) {
                    FURI_LOG_I(TAG, "Hopper frequency loaded %lu", temp_data32);
                    FrequenciesList_push_back(instance->hopper_frequencies, temp_data32);
                    i++;
                } else {
                    FURI_LOG_E(TAG, "Hopper frequency not supported %lu", temp_data32);
                }
            }
            instance->hopper_frequencies_count = i;

            if(!flipper_format_rewind(fff_data_file)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            if(!flipper_format_read_uint32(
                   fff_data_file, "Frequency_default", (uint32_t*)&temp_data32, 1)) {
                FURI_LOG_E(TAG, "Frequency default missing");
                break;
            }

            for(i = 0; i < instance->frequencies_count; i++) {
                if(subghz_setting_get_frequency(instance, i) == temp_data32) {
                    instance->frequency_default_index = i;
                    FURI_LOG_I(TAG, "Frequency default index %lu", i);
                    loading = SubGhzSettingStateLoadFrequencyDefault;
                    break;
                }
            }

            if(loading == SubGhzSettingStateLoadFrequencyDefault) {
                loading = SubGhzSettingStateOkLoad;
            } else {
                FURI_LOG_E(TAG, "Frequency default index missing");
            }
        } while(false);
    }

    string_clear(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close("storage");

    if(loading != SubGhzSettingStateOkLoad) {
        switch(furi_hal_version_get_hw_region()) {
        case FuriHalVersionRegionEuRu:
            subghz_setting_load_default(
                instance,
                subghz_frequencies_region_eu_ru,
                subghz_hopper_frequencies_region_eu_ru,
                subghz_frequency_default_index_region_eu_ru);
            break;
        case FuriHalVersionRegionUsCaAu:
            subghz_setting_load_default(
                instance,
                subghz_frequencies_region_us_ca_au,
                subghz_hopper_frequencies_region_us_ca_au,
                subghz_frequency_default_index_region_us_ca_au);
            break;
        case FuriHalVersionRegionJp:
            subghz_setting_load_default(
                instance,
                subghz_frequencies_region_jp,
                subghz_hopper_frequencies_region_jp,
                subghz_frequency_default_index_region_jp);
            break;

        default:
            subghz_setting_load_default(
                instance,
                subghz_frequencies,
                subghz_hopper_frequencies,
                subghz_frequency_default_index);
            break;
        }
    }
}

size_t subghz_setting_get_frequency_count(SubGhzSetting* instance) {
    furi_assert(instance);
    return instance->frequencies_count;
}

size_t subghz_setting_get_hopper_frequency_count(SubGhzSetting* instance) {
    furi_assert(instance);
    return instance->hopper_frequencies_count;
}

uint32_t subghz_setting_get_frequency(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    return *FrequenciesList_get(instance->frequencies, idx);
}

uint32_t subghz_setting_get_hopper_frequency(SubGhzSetting* instance, size_t idx) {
    furi_assert(instance);
    return *FrequenciesList_get(instance->hopper_frequencies, idx);
}

uint32_t subghz_setting_get_frequency_default_index(SubGhzSetting* instance) {
    furi_assert(instance);
    return instance->frequency_default_index;
}

uint32_t subghz_setting_get_default_frequency(SubGhzSetting* instance) {
    furi_assert(instance);
    return *FrequenciesList_get(instance->frequencies, instance->frequency_default_index);
}
