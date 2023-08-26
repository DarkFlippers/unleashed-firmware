#include "subrem_presets.h"

#define TAG "SubRemPresets"

SubRemSubFilePreset* subrem_sub_file_preset_alloc() {
    SubRemSubFilePreset* sub_preset = malloc(sizeof(SubRemSubFilePreset));

    sub_preset->fff_data = flipper_format_string_alloc();
    sub_preset->file_path = furi_string_alloc();
    sub_preset->protocaol_name = furi_string_alloc();
    sub_preset->label = furi_string_alloc();

    sub_preset->freq_preset.name = furi_string_alloc();

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

    furi_string_free(sub_preset->freq_preset.name);

    free(sub_preset);
}

void subrem_sub_file_preset_reset(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_set_str(sub_preset->label, "");
    furi_string_reset(sub_preset->protocaol_name);
    furi_string_reset(sub_preset->file_path);

    Stream* fff_data_stream = flipper_format_get_raw_stream(sub_preset->fff_data);
    stream_clean(fff_data_stream);

    sub_preset->type = SubGhzProtocolTypeUnknown;
    sub_preset->load_state = SubRemLoadSubStateNotSet;
}

SubRemLoadSubState subrem_sub_preset_load(
    SubRemSubFilePreset* sub_preset,
    SubGhzTxRx* txrx,
    FlipperFormat* fff_data_file) {
    furi_assert(sub_preset);
    furi_assert(txrx);
    furi_assert(fff_data_file);

    Stream* fff_data_stream = flipper_format_get_raw_stream(sub_preset->fff_data);

    SubRemLoadSubState ret;
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32;
    uint32_t repeat = 200;

    ret = SubRemLoadSubStateError;

    do {
        stream_clean(fff_data_stream);
        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }

        if(((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        SubGhzSetting* setting = subghz_txrx_get_setting(txrx);

        //Load frequency or using default from settings
        ret = SubRemLoadSubStateErrorFreq;
        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            FURI_LOG_W(TAG, "Cannot read frequency. Set default frequency");
            sub_preset->freq_preset.frequency = subghz_setting_get_default_frequency(setting);
        } else if(!subghz_txrx_radio_device_is_frequency_valid(txrx, temp_data32)) {
            FURI_LOG_E(TAG, "Frequency not supported on chosen radio module");
            break;
        }
        sub_preset->freq_preset.frequency = temp_data32;

        //Load preset
        ret = SubRemLoadSubStateErrorMod;
        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            break;
        }

        furi_string_set_str(
            temp_str, subghz_txrx_get_preset_name(txrx, furi_string_get_cstr(temp_str)));
        if(!strcmp(furi_string_get_cstr(temp_str), "")) {
            break;
        }

        if(!strcmp(furi_string_get_cstr(temp_str), "CUSTOM")) {
            FURI_LOG_E(TAG, "CUSTOM preset is not supported");
            break;
            // TODO Custom preset loading logic if need
            // sub_preset->freq_preset.preset_index =
            //     subghz_setting_get_inx_preset_by_name(setting, furi_string_get_cstr(temp_str));
        }

        furi_string_set(sub_preset->freq_preset.name, temp_str);

        // Load protocol
        ret = SubRemLoadSubStateErrorProtocol;
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }

        FlipperFormat* fff_data = sub_preset->fff_data;
        if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
            //if RAW
            subghz_protocol_raw_gen_fff_data(
                fff_data,
                furi_string_get_cstr(sub_preset->file_path),
                subghz_txrx_radio_device_get_name(txrx));
        } else {
            stream_copy_full(
                flipper_format_get_raw_stream(fff_data_file),
                flipper_format_get_raw_stream(fff_data));
        }

        if(subghz_txrx_load_decoder_by_name_protocol(txrx, furi_string_get_cstr(temp_str))) {
            SubGhzProtocolStatus status =
                subghz_protocol_decoder_base_deserialize(subghz_txrx_get_decoder(txrx), fff_data);
            if(status != SubGhzProtocolStatusOk) {
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol not found");
            break;
        }

        const SubGhzProtocol* protocol = subghz_txrx_get_decoder(txrx)->protocol;

        if(protocol->flag & SubGhzProtocolFlag_Send) {
            if((protocol->type == SubGhzProtocolTypeStatic) ||
               (protocol->type == SubGhzProtocolTypeDynamic) ||
               (protocol->type == SubGhzProtocolTypeBinRAW) ||
               (protocol->type == SubGhzProtocolTypeRAW)) {
                sub_preset->type = protocol->type;
            } else {
                FURI_LOG_E(TAG, "Unsuported Protocol");
                break;
            }

            furi_string_set(sub_preset->protocaol_name, temp_str);
        } else {
            FURI_LOG_E(TAG, "Protocol does not support transmission");
            break;
        }

        if(!flipper_format_insert_or_update_uint32(fff_data, "Repeat", &repeat, 1)) {
            FURI_LOG_E(TAG, "Unable Repeat");
            break;
        }

        ret = SubRemLoadSubStateOK;

#if FURI_DEBUG
        FURI_LOG_I(TAG, "%-16s - protocol Loaded", furi_string_get_cstr(sub_preset->label));
#endif
    } while(false);

    furi_string_free(temp_str);
    sub_preset->load_state = ret;
    return ret;
}
