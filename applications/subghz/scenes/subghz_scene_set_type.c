#include "../subghz_i.h"
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/secplus_v1.h>
#include <lib/subghz/protocols/secplus_v2.h>
#include <lib/subghz/blocks/math.h>
#include <dolphin/dolphin.h>
#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/registry.h>

#define TAG "SubGhzSetType"

bool subghz_scene_set_type_submenu_gen_data_protocol(
    void* context,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t frequency,
    const char* preset_name) {
    furi_assert(context);
    SubGhz* subghz = context;

    bool res = false;

    subghz_preset_init(subghz, preset_name, frequency, NULL, 0);
    subghz->txrx->decoder_result =
        subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, protocol_name);

    if(subghz->txrx->decoder_result == NULL) {
        string_set_str(subghz->error_str, "Protocol not\nfound!");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(subghz->txrx->fff_data);
        stream_clean(fff_data_stream);
        if(!subghz_protocol_decoder_base_serialize(
               subghz->txrx->decoder_result, subghz->txrx->fff_data, subghz->txrx->preset)) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(subghz->txrx->fff_data, "Bit", &bit, 1)) {
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(subghz->txrx->fff_data, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
        res = true;
    } while(false);
    return res;
}

void subghz_scene_set_type_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_set_type_on_enter(void* context) {
    SubGhz* subghz = context;

    submenu_add_item(
        subghz->submenu,
        "Princeton_433",
        SubmenuIndexPricenton,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice Flo 12bit_433",
        SubmenuIndexNiceFlo12bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice Flo 24bit_433",
        SubmenuIndexNiceFlo24bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 12bit_433",
        SubmenuIndexCAME12bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 24bit_433",
        SubmenuIndexCAME24bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Linear_300",
        SubmenuIndexLinear_300_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME TWEE",
        SubmenuIndexCAMETwee,
        subghz_scene_set_type_submenu_callback,
        subghz);
    // submenu_add_item(
    //     subghz->submenu, "Nero Sketch", SubmenuIndexNeroSketch, subghz_scene_set_type_submenu_callback, subghz);
    // submenu_add_item(
    //     subghz->submenu, "Nero Radio", SubmenuIndexNeroRadio, subghz_scene_set_type_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Gate TX_433",
        SubmenuIndexGateTX,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "DoorHan_315",
        SubmenuIndexDoorHan_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "DoorHan_433",
        SubmenuIndexDoorHan_433_92,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "LiftMaster_315",
        SubmenuIndexLiftMaster_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "LiftMaster_390",
        SubmenuIndexLiftMaster_390_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0_310",
        SubmenuIndexSecPlus_v2_310_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0_315",
        SubmenuIndexSecPlus_v2_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0_390",
        SubmenuIndexSecPlus_v2_390_00,
        subghz_scene_set_type_submenu_callback,
        subghz);

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        //ToDo Fix
        uint32_t key = subghz_random_serial();
        switch(event.event) {
        case SubmenuIndexPricenton:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 433920000, "AM650")) {
                uint32_t te = 400;
                flipper_format_update_uint32(subghz->txrx->fff_data, "TE", (uint32_t*)&te, 1);
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 12, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 24, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_CAME_NAME, key, 12, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_CAME_NAME, key, 24, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexLinear_300_00:
            key = (key & 0x3FF);
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_LINEAR_NAME, key, 10, 300000000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAMETwee:
            key = (key & 0x0FFFFFF0);
            key = 0x003FFF7200000000 | (key ^ 0xE0E0E0EE);
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_CAME_TWEE_NAME, key, 54, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        // case SubmenuIndexNeroSketch:
        //     /* code */
        //     break;
        // case SubmenuIndexNeroRadio:
        //     /* code */
        //     break;
        case SubmenuIndexGateTX:
            key = (key & 0x00F0FF00) | 0xF << 16 | 0x40; //btn 0xF, 0xC, 0xA, 0x6 (?)
            uint64_t rev_key = subghz_protocol_blocks_reverse_key(key, 24);
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_GATE_TX_NAME, rev_key, 24, 433920000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexDoorHan_433_92:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
            subghz_preset_init(
                subghz, "AM650", subghz_setting_get_default_frequency(subghz->setting), NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_keeloq_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x2,
                    0x0003,
                    "DoorHan",
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                string_set_str(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexDoorHan_315_00:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
            subghz_preset_init(subghz, "AM650", 315000000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_keeloq_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x2,
                    0x0003,
                    "DoorHan",
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                string_set_str(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexLiftMaster_315_00:
            while(!subghz_protocol_secplus_v1_check_fixed(key)) {
                key = subghz_random_serial();
            }
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz,
                   SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
                   (uint64_t)key << 32 | 0xE6000000,
                   42,
                   315000000,
                   "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexLiftMaster_390_00:
            while(!subghz_protocol_secplus_v1_check_fixed(key)) {
                key = subghz_random_serial();
            }
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz,
                   SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
                   (uint64_t)key << 32 | 0xE6000000,
                   42,
                   390000000,
                   "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexSecPlus_v2_310_00:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
            subghz_preset_init(subghz, "AM650", 310000000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_secplus_v2_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key,
                    0x68,
                    0xE500000,
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            break;
        case SubmenuIndexSecPlus_v2_315_00:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
            subghz_preset_init(subghz, "AM650", 315000000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_secplus_v2_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key,
                    0x68,
                    0xE500000,
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            break;
        case SubmenuIndexSecPlus_v2_390_00:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
            subghz_preset_init(subghz, "AM650", 390000000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_secplus_v2_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key,
                    0x68,
                    0xE500000,
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            break;
        default:
            return false;
            break;
        }

        scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

        if(generated_protocol) {
            subghz_file_name_clear(subghz);
            DOLPHIN_DEED(DolphinDeedSubGhzAddManually);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        }
    }

    return false;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
