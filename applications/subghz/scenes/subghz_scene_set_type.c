#include "../subghz_i.h"
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/faac_slh.h>
#include <lib/subghz/blocks/math.h>
#include <dolphin/dolphin.h>
#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>

#define TAG "SubGhzSetType"

enum SubmenuIndex {
    SubmenuIndexFaacSLH,
    SubmenuIndexPricenton,
    SubmenuIndexNiceFlo12bit,
    SubmenuIndexNiceFlo24bit,
    SubmenuIndexCAME12bit,
    SubmenuIndexCAME24bit,
    SubmenuIndexCAMETwee,
    SubmenuIndexNeroSketch,
    SubmenuIndexNeroRadio,
    SubmenuIndexGateTX,
    SubmenuIndexDoorHan_315_00,
    SubmenuIndexDoorHan_433_92,
};

bool subghz_scene_set_type_submenu_gen_data_protocol(
    void* context,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit) {
    furi_assert(context);
    SubGhz* subghz = context;

    bool res = false;

    subghz->txrx->decoder_result =
        subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, protocol_name);

    if(subghz->txrx->decoder_result == NULL) {
        string_set(subghz->error_str, "Protocol not found");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(subghz->txrx->fff_data);
        stream_clean(fff_data_stream);
        if(!subghz_protocol_decoder_base_serialize(
               subghz->txrx->decoder_result,
               subghz->txrx->fff_data,
               subghz_setting_get_default_frequency(subghz->setting),
               FuriHalSubGhzPresetOok650Async)) {
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
        "Faac SLH_868",
        SubmenuIndexFaacSLH,
        subghz_scene_set_type_submenu_callback,
        subghz);
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
        case SubmenuIndexFaacSLH:
            subghz->txrx->fix_data->fix_len = 4;
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFix);
            uint32_t fix_part = subghz->txrx->fix_data->fix[0] << 24 | subghz->txrx->fix_data->fix[1] << 16 |
                           subghz->txrx->fix_data->fix[2] << 8 | subghz->txrx->fix_data->fix[3];
            
            uint16_t cnt = subghz->txrx->cnt_data->cnt[0] << 8 | subghz->txrx->cnt_data->cnt[1] << 16;
            
            uint32_t seed = subghz->txrx->seed_data->seed[0] << 24 | subghz->txrx->seed_data->seed[1] << 16 |
                            subghz->txrx->seed_data->seed[2] << 8 | subghz->txrx->seed_data->seed[3];
            subghz->txrx->transmitter =
                subghz_transmitter_alloc_init(subghz->txrx->environment, "Faac SLH");
            if(subghz->txrx->transmitter) {
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
            break;
        case SubmenuIndexPricenton:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "Princeton", key, 24)) {
                uint32_t te = 400;
                flipper_format_update_uint32(subghz->txrx->fff_data, "TE", (uint32_t*)&te, 1);
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "Nice FLO", key, 12)) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "Nice FLO", key, 24)) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "CAME", key, 12)) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "CAME", key, 24)) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAMETwee:
            key = (key & 0x0FFFFFF0);
            key = 0x003FFF7200000000 | (key ^ 0xE0E0E0EE);
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "CAME TWEE", key, 54)) {
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
            if(subghz_scene_set_type_submenu_gen_data_protocol(subghz, "GateTX", rev_key, 24)) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexDoorHan_433_92:
            subghz->txrx->transmitter =
                subghz_transmitter_alloc_init(subghz->txrx->environment, "KeeLoq");
            if(subghz->txrx->transmitter) {
                subghz_protocol_keeloq_create_data(
                    subghz->txrx->transmitter->protocol_instance,
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x2,
                    0x0003,
                    "DoorHan",
                    subghz_setting_get_default_frequency(subghz->setting),
                    FuriHalSubGhzPresetOok650Async);
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
            break;
        case SubmenuIndexDoorHan_315_00:
            subghz->txrx->transmitter =
                subghz_transmitter_alloc_init(subghz->txrx->environment, "KeeLoq");
            if(subghz->txrx->transmitter) {
                subghz_protocol_keeloq_create_data(
                    subghz->txrx->transmitter->protocol_instance,
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x2,
                    0x0003,
                    "DoorHan",
                    315000000,
                    FuriHalSubGhzPresetOok650Async);
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
            break;
        default:
            return false;
            break;
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

    return false;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
