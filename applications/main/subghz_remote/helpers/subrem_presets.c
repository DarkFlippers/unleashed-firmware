#include "subrem_presets.h"

SubRemSubFilePreset* subrem_sub_file_preset_alloc() {
    SubRemSubFilePreset* sub_preset = malloc(sizeof(SubRemSubFilePreset));

    sub_preset->fff_data = flipper_format_string_alloc();
    sub_preset->file_path = furi_string_alloc();
    sub_preset->protocaol_name = furi_string_alloc();
    sub_preset->label = furi_string_alloc_set_str("N/A");

    sub_preset->type = SubGhzProtocolTypeUnknown;
    sub_preset->load_state = SubRemLoadSubStateNotSet;

    return sub_preset;
}

void subrem_sub_file_preset_free(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_free(sub_preset->label);
    furi_string_free(sub_preset->protocaol_name);
    furi_string_free(sub_preset->file_path);
    flipper_format_free(sub_preset->fff_data);

    free(sub_preset);
}

void subrem_sub_file_preset_reset(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_set_str(sub_preset->label, "N/A");
    furi_string_reset(sub_preset->protocaol_name);
    furi_string_reset(sub_preset->file_path);

    Stream* fff_data_stream = flipper_format_get_raw_stream(sub_preset->fff_data);
    stream_clean(fff_data_stream);

    sub_preset->type = SubGhzProtocolTypeUnknown;
    sub_preset->load_state = SubRemLoadSubStateNotSet;
}
