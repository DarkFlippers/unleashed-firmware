#include "../subghz_i.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/faac_slh.h>

#define TAG "SubGhzSetSeed"

void subghz_scene_set_seed_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_seed_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter SEED in hex");
    byte_input_set_result_callback(
        byte_input,
        subghz_scene_set_seed_byte_input_callback,
        NULL,
        subghz,
        subghz->txrx->secure_data->seed,
        4);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_seed_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    bool generated_protocol = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            uint32_t fix_part = subghz->txrx->secure_data->fix[0] << 24 | subghz->txrx->secure_data->fix[1] << 16 |
                           subghz->txrx->secure_data->fix[2] << 8 | subghz->txrx->secure_data->fix[3];
            FURI_LOG_I(TAG, "fix: %8X", fix_part);
            uint16_t cnt = subghz->txrx->secure_data->cnt[0] << 8 | subghz->txrx->secure_data->cnt[1];
            FURI_LOG_I(TAG, "cnt: %8X", cnt);
            uint32_t seed = subghz->txrx->secure_data->seed[0] << 24 | subghz->txrx->secure_data->seed[1] << 16 |
                            subghz->txrx->secure_data->seed[2] << 8 | subghz->txrx->secure_data->seed[3];
            FURI_LOG_I(TAG, "seed: %8X", seed);
            subghz->txrx->transmitter =
                subghz_transmitter_alloc_init(subghz->txrx->environment, "Faac SLH");
            if(subghz->txrx->transmitter && flipper_format_update_uint32(subghz->txrx->fff_data, "SEED", (uint32_t*)&seed, 4)) {
                subghz_protocol_faac_slh_create_data(
                    subghz->txrx->transmitter->protocol_instance,
                    subghz->txrx->fff_data,
                    fix_part >> 4,
                    fix_part & 0xf,
                    cnt,
                    seed,
                    "FAAC_SLH",
                    868350000,
                    FuriHalSubGhzPresetOok650Async);
                
                FURI_LOG_I(TAG, "SEED (set_seed_on_event): %8X\n", seed);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            consumed = true;
        }
        if(generated_protocol) {
            subghz_file_name_clear(subghz);
            DOLPHIN_DEED(DolphinDeedSubGhzAddManually);
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneSetType, SubGhzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        }
    }
    return consumed;;
}

void subghz_scene_set_seed_on_exit(void* context) {
    SubGhz* subghz = context;
    uint32_t seed = subghz->txrx->secure_data->seed[0] << 24 | subghz->txrx->secure_data->seed[1] << 16 |
                            subghz->txrx->secure_data->seed[2] << 8 | subghz->txrx->secure_data->seed[3];
    FURI_LOG_I(TAG, "SEED (set_seed_on_exit): %8X\n", seed);
    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
