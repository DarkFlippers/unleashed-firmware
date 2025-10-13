#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"

#include <machine/endian.h>

#define TAG "SubGhzSetSeed"

void subghz_scene_set_seed_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_seed_on_enter(void* context) {
    SubGhz* subghz = context;

    uint8_t* byte_ptr = NULL;
    uint8_t byte_count = 0;

    switch(subghz->gen_info->type) {
    case GenFaacSLH:
        byte_ptr = (uint8_t*)&subghz->gen_info->faac_slh.seed;
        byte_count = sizeof(subghz->gen_info->faac_slh.seed);
        break;
    case GenKeeloqBFT:
        byte_ptr = (uint8_t*)&subghz->gen_info->keeloq_bft.seed;
        byte_count = sizeof(subghz->gen_info->keeloq_bft.seed);
        break;
    // Not needed for these types
    case GenKeeloq:
    case GenAlutechAt4n:
    case GenSomfyTelis:
    case GenNiceFlorS:
    case GenSecPlus2:
    case GenPhoenixV2:
    case GenData:
    case GenSecPlus1:
    case GenCameAtomo:
    default:
        furi_crash("Not implemented");
        break;
    }

    furi_assert(byte_ptr);
    furi_assert(byte_count > 0);

    *((uint32_t*)byte_ptr) = __bswap32(*((uint32_t*)byte_ptr)); // Convert

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter SEED in hex");
    byte_input_set_result_callback(
        byte_input, subghz_scene_set_seed_byte_input_callback, NULL, subghz, byte_ptr, byte_count);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_seed_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    bool generated_protocol = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            switch(subghz->gen_info->type) {
            case GenFaacSLH:
                subghz->gen_info->faac_slh.seed = __bswap32(subghz->gen_info->faac_slh.seed);
                generated_protocol = subghz_txrx_gen_faac_slh_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->faac_slh.serial,
                    subghz->gen_info->faac_slh.btn,
                    subghz->gen_info->faac_slh.cnt,
                    subghz->gen_info->faac_slh.seed,
                    subghz->gen_info->faac_slh.manuf);
                break;
            case GenKeeloqBFT:
                subghz->gen_info->keeloq_bft.seed = __bswap32(subghz->gen_info->keeloq_bft.seed);
                generated_protocol = subghz_txrx_gen_keeloq_bft_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->keeloq_bft.serial,
                    subghz->gen_info->keeloq_bft.btn,
                    subghz->gen_info->keeloq_bft.cnt,
                    subghz->gen_info->keeloq_bft.seed,
                    subghz->gen_info->keeloq_bft.manuf);
                break;
            // Not needed for these types
            case GenKeeloq:
            case GenAlutechAt4n:
            case GenSomfyTelis:
            case GenNiceFlorS:
            case GenSecPlus2:
            case GenPhoenixV2:
            case GenData:
            case GenSecPlus1:
            case GenCameAtomo:
            default:
                furi_crash("Not implemented");
                break;
            }

            consumed = true;

            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            } else {
                subghz_file_name_clear(subghz);

                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneSetType, SubGhzCustomEventManagerSet);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            }
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
