#include "../subghz_i.h"
#include <lib/subghz/protocols/faac_slh.h>
#include <lib/subghz/protocols/keeloq.h>

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
    uint32_t fix_part, cnt, seed;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            SubGhzCustomEvent state =
                scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType);

            switch(state) {
            case SubmenuIndexBFTClone:
                fix_part = subghz->txrx->secure_data->fix[0] << 24 |
                           subghz->txrx->secure_data->fix[1] << 16 |
                           subghz->txrx->secure_data->fix[2] << 8 |
                           subghz->txrx->secure_data->fix[3];

                cnt = subghz->txrx->secure_data->cnt[0] << 8 | subghz->txrx->secure_data->cnt[1];

                seed = subghz->txrx->secure_data->seed[0] << 24 |
                       subghz->txrx->secure_data->seed[1] << 16 |
                       subghz->txrx->secure_data->seed[2] << 8 |
                       subghz->txrx->secure_data->seed[3];

                subghz->txrx->transmitter =
                    subghz_transmitter_alloc_init(subghz->txrx->environment, "KeeLoq");
                if(subghz->txrx->transmitter) {
                    subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
                    subghz_protocol_keeloq_bft_create_data(
                        subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                        subghz->txrx->fff_data,
                        fix_part & 0x0FFFFFFF,
                        fix_part >> 28,
                        cnt,
                        seed,
                        "BFT",
                        subghz->txrx->preset);

                    uint8_t seed_data[sizeof(uint32_t)] = {0};
                    for(size_t i = 0; i < sizeof(uint32_t); i++) {
                        seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
                    }

                    flipper_format_write_hex(
                        subghz->txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));

                    flipper_format_write_string_cstr(subghz->txrx->fff_data, "Manufacture", "BFT");

                    generated_protocol = true;
                }

                subghz_transmitter_free(subghz->txrx->transmitter);

                if(!generated_protocol) {
                    furi_string_set(
                        subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
                consumed = true;
                break;
            case SubmenuIndexFaacSLH_433:
            case SubmenuIndexFaacSLH_868:
                fix_part = subghz->txrx->secure_data->fix[0] << 24 |
                           subghz->txrx->secure_data->fix[1] << 16 |
                           subghz->txrx->secure_data->fix[2] << 8 |
                           subghz->txrx->secure_data->fix[3];

                cnt = subghz->txrx->secure_data->cnt[0] << 16 |
                      subghz->txrx->secure_data->cnt[1] << 8 | subghz->txrx->secure_data->cnt[2];

                seed = subghz->txrx->secure_data->seed[0] << 24 |
                       subghz->txrx->secure_data->seed[1] << 16 |
                       subghz->txrx->secure_data->seed[2] << 8 |
                       subghz->txrx->secure_data->seed[3];

                subghz->txrx->transmitter =
                    subghz_transmitter_alloc_init(subghz->txrx->environment, "Faac SLH");
                if(subghz->txrx->transmitter) {
                    SubGhzCustomEvent state =
                        scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType);

                    if(state == SubmenuIndexFaacSLH_433) {
                        subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
                    } else if(state == SubmenuIndexFaacSLH_868) {
                        subghz_preset_init(subghz, "AM650", 868350000, NULL, 0);
                    }
                    subghz_protocol_faac_slh_create_data(
                        subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                        subghz->txrx->fff_data,
                        fix_part >> 4,
                        fix_part & 0xf,
                        (cnt & 0xFFFFF),
                        seed,
                        "FAAC_SLH",
                        subghz->txrx->preset);
                    // RogueMaster dont steal!
                    uint8_t seed_data[sizeof(uint32_t)] = {0};
                    for(size_t i = 0; i < sizeof(uint32_t); i++) {
                        seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
                    }

                    flipper_format_write_hex(
                        subghz->txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));

                    generated_protocol = true;
                }

                subghz_transmitter_free(subghz->txrx->transmitter);

                if(!generated_protocol) {
                    furi_string_set(
                        subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
                consumed = true;
                break;

            default:
                break;
            }
        }

        if(generated_protocol) {
            subghz_file_name_clear(subghz);
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneSetType, SubGhzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        }
    }
    return consumed;
}

void subghz_scene_set_seed_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
