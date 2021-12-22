#include "../subghz_i.h"
#include "../lib/subghz/protocols/subghz_protocol_keeloq.h"

enum SubmenuIndex {
    SubmenuIndexPricenton,
    SubmenuIndexNiceFlo12bit,
    SubmenuIndexNiceFlo24bit,
    SubmenuIndexCAME12bit,
    SubmenuIndexCAME24bit,
    SubmenuIndexCAMETwee,
    SubmenuIndexNeroSketch,
    SubmenuIndexNeroRadio,
    SubmenuIndexGateTX,
    SubmenuIndexDoorHan,
};

bool subghz_scene_set_type_submenu_to_find_protocol(void* context, const char* protocol_name) {
    SubGhz* subghz = context;
    subghz->txrx->protocol_result = subghz_parser_get_by_name(subghz->txrx->parser, protocol_name);
    if(subghz->txrx->protocol_result == NULL) {
        string_set(subghz->error_str, "Protocol not found");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }
    return true;
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
        "DoorHan_433",
        SubmenuIndexDoorHan,
        subghz_scene_set_type_submenu_callback,
        subghz);

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewMenu);
}

bool subghz_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t key = subghz_random_serial();
        switch(event.event) {
        case SubmenuIndexPricenton:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "Princeton")) {
                subghz->txrx->protocol_result->code_last_count_bit = 24;
                key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
                subghz->txrx->protocol_result->code_last_found = key;
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo12bit:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "Nice FLO")) {
                subghz->txrx->protocol_result->code_last_count_bit = 12;
                key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
                subghz->txrx->protocol_result->code_last_found = key;
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlo24bit:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "Nice FLO")) {
                subghz->txrx->protocol_result->code_last_count_bit = 24;
                key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
                subghz->txrx->protocol_result->code_last_found = key;
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME12bit:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "CAME")) {
                subghz->txrx->protocol_result->code_last_count_bit = 12;
                key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
                subghz->txrx->protocol_result->code_last_found = key;
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME24bit:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "CAME")) {
                subghz->txrx->protocol_result->code_last_count_bit = 24;
                key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
                subghz->txrx->protocol_result->code_last_found = key;
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAMETwee:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "CAME TWEE")) {
                subghz->txrx->protocol_result->code_last_count_bit = 54;
                key = (key & 0x0FFFFFF0);
                subghz->txrx->protocol_result->code_last_found = 0x003FFF7200000000 |
                                                                 (key ^ 0xE0E0E0EE);
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
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "GateTX")) {
                subghz->txrx->protocol_result->code_last_count_bit = 24;
                key = (key & 0x00F0FF00) | 0xF << 16 | 0x40; //btn 0xF, 0xC, 0xA, 0x6 (?)
                subghz->txrx->protocol_result->code_last_found =
                    subghz_protocol_common_reverse_key(
                        key, subghz->txrx->protocol_result->code_last_count_bit);
                generated_protocol = true;
            }
            break;
        case SubmenuIndexDoorHan:
            if(subghz_scene_set_type_submenu_to_find_protocol(subghz, "KeeLoq")) {
                subghz->txrx->protocol_result->code_last_count_bit = 64;
                subghz->txrx->protocol_result->serial = key & 0x0FFFFFFF;
                subghz->txrx->protocol_result->btn = 0x2; //btn 0x1, 0x2, 0x4, 0x8
                subghz->txrx->protocol_result->cnt = 0x0003;
                if(subghz_protocol_keeloq_set_manufacture_name(
                       subghz->txrx->protocol_result, "DoorHan")) {
                    subghz->txrx->protocol_result->code_last_found =
                        subghz_protocol_keeloq_gen_key(subghz->txrx->protocol_result);
                    generated_protocol = true;
                } else {
                    generated_protocol = false;
                    string_set(
                        subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
            }
            break;
        default:
            return false;
            break;
        }

        if(generated_protocol) {
            subghz->txrx->frequency = subghz_frequencies[subghz_frequencies_433_92];
            subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
            subghz_file_name_clear(subghz);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            return true;
        }
    }

    return false;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_clean(subghz->submenu);
}
