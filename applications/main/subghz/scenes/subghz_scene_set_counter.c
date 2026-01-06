#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"

#include <machine/endian.h>

#define TAG "SubGhzSetCounter"

void subghz_scene_set_counter_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_counter_on_enter(void* context) {
    SubGhz* subghz = context;

    uint8_t* byte_ptr = NULL;
    uint8_t byte_count = 0;

    switch(subghz->gen_info->type) {
    case GenFaacSLH:
        byte_ptr = (uint8_t*)&subghz->gen_info->faac_slh.cnt;
        byte_count = sizeof(subghz->gen_info->faac_slh.cnt);
        break;
    case GenKeeloq:
        byte_ptr = (uint8_t*)&subghz->gen_info->keeloq.cnt;
        byte_count = sizeof(subghz->gen_info->keeloq.cnt);
        break;
    case GenCameAtomo:
        byte_ptr = (uint8_t*)&subghz->gen_info->came_atomo.cnt;
        byte_count = sizeof(subghz->gen_info->came_atomo.cnt);
        break;
    case GenKeeloqBFT:
        byte_ptr = (uint8_t*)&subghz->gen_info->keeloq_bft.cnt;
        byte_count = sizeof(subghz->gen_info->keeloq_bft.cnt);
        break;
    case GenAlutechAt4n:
        byte_ptr = (uint8_t*)&subghz->gen_info->alutech_at_4n.cnt;
        byte_count = sizeof(subghz->gen_info->alutech_at_4n.cnt);
        break;
    case GenSomfyTelis:
        byte_ptr = (uint8_t*)&subghz->gen_info->somfy_telis.cnt;
        byte_count = sizeof(subghz->gen_info->somfy_telis.cnt);
        break;
    case GenKingGatesStylo4k:
        byte_ptr = (uint8_t*)&subghz->gen_info->kinggates_stylo_4k.cnt;
        byte_count = sizeof(subghz->gen_info->kinggates_stylo_4k.cnt);
        break;
    case GenNiceFlorS:
        byte_ptr = (uint8_t*)&subghz->gen_info->nice_flor_s.cnt;
        byte_count = sizeof(subghz->gen_info->nice_flor_s.cnt);
        break;
    case GenSecPlus2:
        byte_ptr = (uint8_t*)&subghz->gen_info->sec_plus_2.cnt;
        byte_count = sizeof(subghz->gen_info->sec_plus_2.cnt);
        break;
    case GenPhoenixV2:
        byte_ptr = (uint8_t*)&subghz->gen_info->phoenix_v2.cnt;
        byte_count = sizeof(subghz->gen_info->phoenix_v2.cnt);
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

    if(byte_count == 2) {
        *((uint16_t*)byte_ptr) = __bswap16(*((uint16_t*)byte_ptr)); // Convert
    } else if(byte_count == 4) {
        *((uint32_t*)byte_ptr) = __bswap32(*((uint32_t*)byte_ptr)); // Convert
    }

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter COUNTER in hex");

    byte_input_set_result_callback(
        byte_input,
        subghz_scene_set_counter_byte_input_callback,
        NULL,
        subghz,
        byte_ptr,
        byte_count);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_counter_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            // Swap bytes
            switch(subghz->gen_info->type) {
            case GenFaacSLH:
                subghz->gen_info->faac_slh.cnt = __bswap32(subghz->gen_info->faac_slh.cnt);
                break;
            case GenKeeloq:
                subghz->gen_info->keeloq.cnt = __bswap16(subghz->gen_info->keeloq.cnt);
                break;
            case GenCameAtomo:
                subghz->gen_info->came_atomo.cnt = __bswap16(subghz->gen_info->came_atomo.cnt);
                break;
            case GenKeeloqBFT:
                subghz->gen_info->keeloq_bft.cnt = __bswap16(subghz->gen_info->keeloq_bft.cnt);
                break;
            case GenAlutechAt4n:
                subghz->gen_info->alutech_at_4n.cnt =
                    __bswap16(subghz->gen_info->alutech_at_4n.cnt);
                break;
            case GenSomfyTelis:
                subghz->gen_info->somfy_telis.cnt = __bswap16(subghz->gen_info->somfy_telis.cnt);
                break;
            case GenKingGatesStylo4k:
                subghz->gen_info->kinggates_stylo_4k.cnt =
                    __bswap16(subghz->gen_info->kinggates_stylo_4k.cnt);
                break;
            case GenNiceFlorS:
                subghz->gen_info->nice_flor_s.cnt = __bswap16(subghz->gen_info->nice_flor_s.cnt);
                break;
            case GenSecPlus2:
                subghz->gen_info->sec_plus_2.cnt = __bswap32(subghz->gen_info->sec_plus_2.cnt);
                break;
            case GenPhoenixV2:
                subghz->gen_info->phoenix_v2.cnt = __bswap16(subghz->gen_info->phoenix_v2.cnt);
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
            case GenKeeloqBFT:
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetSeed);
                return true;
            case GenKeeloq:
                generated_protocol = subghz_txrx_gen_keeloq_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->keeloq.serial,
                    subghz->gen_info->keeloq.btn,
                    subghz->gen_info->keeloq.cnt,
                    subghz->gen_info->keeloq.manuf);
                break;
            case GenCameAtomo:
                generated_protocol = subghz_txrx_gen_came_atomo_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->came_atomo.serial,
                    subghz->gen_info->came_atomo.cnt);
                break;
            case GenAlutechAt4n:
                generated_protocol = subghz_txrx_gen_alutech_at_4n_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->alutech_at_4n.serial,
                    subghz->gen_info->alutech_at_4n.btn,
                    subghz->gen_info->alutech_at_4n.cnt);
                break;
            case GenSomfyTelis:
                generated_protocol = subghz_txrx_gen_somfy_telis_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->somfy_telis.serial,
                    subghz->gen_info->somfy_telis.btn,
                    subghz->gen_info->somfy_telis.cnt);
                break;
            case GenKingGatesStylo4k:
                generated_protocol = subghz_txrx_gen_kinggates_stylo_4k_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->kinggates_stylo_4k.serial,
                    subghz->gen_info->kinggates_stylo_4k.btn,
                    subghz->gen_info->kinggates_stylo_4k.cnt);
                break;
            case GenNiceFlorS:
                generated_protocol = subghz_txrx_gen_nice_flor_s_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->nice_flor_s.serial,
                    subghz->gen_info->nice_flor_s.btn,
                    subghz->gen_info->nice_flor_s.cnt,
                    subghz->gen_info->nice_flor_s.nice_one);
                break;
            case GenSecPlus2:
                generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->sec_plus_2.serial,
                    subghz->gen_info->sec_plus_2.btn,
                    subghz->gen_info->sec_plus_2.cnt);
                break;
            case GenPhoenixV2:
                generated_protocol = subghz_txrx_gen_phoenix_v2_protocol(
                    subghz->txrx,
                    subghz->gen_info->mod,
                    subghz->gen_info->freq,
                    subghz->gen_info->phoenix_v2.serial,
                    subghz->gen_info->phoenix_v2.cnt);
                break;
            // Not needed for these types
            case GenData:
            case GenSecPlus1:
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

void subghz_scene_set_counter_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
