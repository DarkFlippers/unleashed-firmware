#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"

#include <machine/endian.h>

#define TAG "SubGhzSetSerial"

void subghz_scene_set_serial_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_serial_on_enter(void* context) {
    SubGhz* subghz = context;

    uint8_t* byte_ptr = NULL;
    uint8_t byte_count = 0;

    switch(subghz->gen_info->type) {
    case GenFaacSLH:
        byte_ptr = (uint8_t*)&subghz->gen_info->faac_slh.serial;
        byte_count = sizeof(subghz->gen_info->faac_slh.serial);
        break;
    case GenKeeloq:
        byte_ptr = (uint8_t*)&subghz->gen_info->keeloq.serial;
        byte_count = sizeof(subghz->gen_info->keeloq.serial);
        break;
    case GenCameAtomo:
        byte_ptr = (uint8_t*)&subghz->gen_info->came_atomo.serial;
        byte_count = sizeof(subghz->gen_info->came_atomo.serial);
        break;
    case GenKeeloqBFT:
        byte_ptr = (uint8_t*)&subghz->gen_info->keeloq_bft.serial;
        byte_count = sizeof(subghz->gen_info->keeloq_bft.serial);
        break;
    case GenAlutechAt4n:
        byte_ptr = (uint8_t*)&subghz->gen_info->alutech_at_4n.serial;
        byte_count = sizeof(subghz->gen_info->alutech_at_4n.serial);
        break;
    case GenSomfyTelis:
        byte_ptr = (uint8_t*)&subghz->gen_info->somfy_telis.serial;
        byte_count = sizeof(subghz->gen_info->somfy_telis.serial);
        break;
    case GenKingGatesStylo4k:
        byte_ptr = (uint8_t*)&subghz->gen_info->kinggates_stylo_4k.serial;
        byte_count = sizeof(subghz->gen_info->kinggates_stylo_4k.serial);
        break;
    case GenNiceFlorS:
        byte_ptr = (uint8_t*)&subghz->gen_info->nice_flor_s.serial;
        byte_count = sizeof(subghz->gen_info->nice_flor_s.serial);
        break;
    case GenSecPlus2:
        byte_ptr = (uint8_t*)&subghz->gen_info->sec_plus_2.serial;
        byte_count = sizeof(subghz->gen_info->sec_plus_2.serial);
        break;
    case GenPhoenixV2:
        byte_ptr = (uint8_t*)&subghz->gen_info->phoenix_v2.serial;
        byte_count = sizeof(subghz->gen_info->phoenix_v2.serial);
        break;
    // Not needed for these types
    case GenData:
    case GenSecPlus1:
    default:
        furi_crash("Not implemented");
        break;
    }

    furi_assert(byte_ptr);
    furi_assert(byte_count > 0);

    *((uint32_t*)byte_ptr) = __bswap32(*((uint32_t*)byte_ptr)); // Convert

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter SERIAL in hex");
    byte_input_set_result_callback(
        byte_input,
        subghz_scene_set_serial_byte_input_callback,
        NULL,
        subghz,
        byte_ptr,
        byte_count);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_serial_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            // Swap bytes
            switch(subghz->gen_info->type) {
            case GenFaacSLH:
                subghz->gen_info->faac_slh.serial = __bswap32(subghz->gen_info->faac_slh.serial);
                break;
            case GenKeeloq:
                subghz->gen_info->keeloq.serial = __bswap32(subghz->gen_info->keeloq.serial);
                break;
            case GenCameAtomo:
                subghz->gen_info->came_atomo.serial =
                    __bswap32(subghz->gen_info->came_atomo.serial);
                break;
            case GenKeeloqBFT:
                subghz->gen_info->keeloq_bft.serial =
                    __bswap32(subghz->gen_info->keeloq_bft.serial);
                break;
            case GenAlutechAt4n:
                subghz->gen_info->alutech_at_4n.serial =
                    __bswap32(subghz->gen_info->alutech_at_4n.serial);
                break;
            case GenSomfyTelis:
                subghz->gen_info->somfy_telis.serial =
                    __bswap32(subghz->gen_info->somfy_telis.serial);
                break;
            case GenKingGatesStylo4k:
                subghz->gen_info->kinggates_stylo_4k.serial =
                    __bswap32(subghz->gen_info->kinggates_stylo_4k.serial);
                break;
            case GenNiceFlorS:
                subghz->gen_info->nice_flor_s.serial =
                    __bswap32(subghz->gen_info->nice_flor_s.serial);
                break;
            case GenSecPlus2:
                subghz->gen_info->sec_plus_2.serial =
                    __bswap32(subghz->gen_info->sec_plus_2.serial);
                break;
            case GenPhoenixV2:
                subghz->gen_info->phoenix_v2.serial =
                    __bswap32(subghz->gen_info->phoenix_v2.serial);
                break;
            // Not needed for these types
            case GenData:
            case GenSecPlus1:
            default:
                furi_crash("Not implemented");
                break;
            }

            switch(subghz->gen_info->type) {
            case GenFaacSLH:
            case GenKeeloq:
            case GenKeeloqBFT:
            case GenAlutechAt4n:
            case GenSomfyTelis:
            case GenKingGatesStylo4k:
            case GenNiceFlorS:
            case GenSecPlus2:
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetButton);
                break;
            case GenCameAtomo:
            case GenPhoenixV2:
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetCounter);
                break;
            // Not needed for these types
            case GenData:
            case GenSecPlus1:
            default:
                furi_crash("Not implemented");
                break;
            }

            consumed = true;
        }
    }
    return consumed;
}

void subghz_scene_set_serial_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
