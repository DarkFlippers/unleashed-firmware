#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"

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
        subghz->secure_data->seed,
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
                fix_part = subghz->secure_data->fix[0] << 24 | subghz->secure_data->fix[1] << 16 |
                           subghz->secure_data->fix[2] << 8 | subghz->secure_data->fix[3];

                cnt = subghz->secure_data->cnt[0] << 8 | subghz->secure_data->cnt[1];

                seed = subghz->secure_data->seed[0] << 24 | subghz->secure_data->seed[1] << 16 |
                       subghz->secure_data->seed[2] << 8 | subghz->secure_data->seed[3];

                generated_protocol = subghz_txrx_gen_keeloq_bft_protocol(
                    subghz->txrx,
                    "AM650",
                    433920000,
                    fix_part & 0x0FFFFFFF,
                    fix_part >> 28,
                    cnt,
                    seed,
                    "BFT");

                if(!generated_protocol) {
                    furi_string_set(
                        subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
                consumed = true;
                break;
            case SubmenuIndexFaacSLH_433:
            case SubmenuIndexFaacSLH_868:
                fix_part = subghz->secure_data->fix[0] << 24 | subghz->secure_data->fix[1] << 16 |
                           subghz->secure_data->fix[2] << 8 | subghz->secure_data->fix[3];

                cnt = subghz->secure_data->cnt[0] << 16 | subghz->secure_data->cnt[1] << 8 |
                      subghz->secure_data->cnt[2];

                seed = subghz->secure_data->seed[0] << 24 | subghz->secure_data->seed[1] << 16 |
                       subghz->secure_data->seed[2] << 8 | subghz->secure_data->seed[3];

                if(state == SubmenuIndexFaacSLH_433) {
                    generated_protocol = subghz_txrx_gen_faac_slh_protocol(
                        subghz->txrx,
                        "AM650",
                        433920000,
                        fix_part >> 4,
                        fix_part & 0xf,
                        (cnt & 0xFFFFF),
                        seed,
                        "FAAC_SLH");
                } else if(state == SubmenuIndexFaacSLH_868) {
                    generated_protocol = subghz_txrx_gen_faac_slh_protocol(
                        subghz->txrx,
                        "AM650",
                        868350000,
                        fix_part >> 4,
                        fix_part & 0xf,
                        (cnt & 0xFFFFF),
                        seed,
                        "FAAC_SLH");
                }

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

        // Reset Seed, Fix, Cnt in secure data after successful or unsuccessful generation
        memset(subghz->secure_data->seed, 0, sizeof(subghz->secure_data->seed));
        memset(subghz->secure_data->cnt, 0, sizeof(subghz->secure_data->cnt));
        memset(subghz->secure_data->fix, 0, sizeof(subghz->secure_data->fix));

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
