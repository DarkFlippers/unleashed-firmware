#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"
#include <lib/subghz/blocks/math.h>
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhzSetType"

void subghz_scene_set_type_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_set_type_on_enter(void* context) {
    SubGhz* subghz = context;

    submenu_add_item(
        subghz->submenu,
        "Princeton_433",
        SubmenuIndexPricenton_433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Princeton_315",
        SubmenuIndexPricenton_315,
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
    SubGhzProtocolStatus generated_protocol = SubGhzProtocolStatusError;

    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t key = (uint32_t)rand();
        switch(event.event) {
        case SubmenuIndexPricenton_433:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 400);
            break;
        case SubmenuIndexPricenton_315:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                subghz->txrx, "AM650", 315000000, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 400);
            break;
        case SubmenuIndexNiceFlo12bit:
            key = (key & 0x00000FF0) | 0x1; //btn 0x1, 0x2, 0x4
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 12);
            break;
        case SubmenuIndexNiceFlo24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 24);
            break;
        case SubmenuIndexCAME12bit:
            key = (key & 0x00000FF0) | 0x1; //btn 0x1, 0x2, 0x4
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_CAME_NAME, key, 12);
            break;
        case SubmenuIndexCAME24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_CAME_NAME, key, 24);
            break;
        case SubmenuIndexLinear_300_00:
            key = (key & 0x3FF);
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 300000000, SUBGHZ_PROTOCOL_LINEAR_NAME, key, 10);
            break;
        case SubmenuIndexCAMETwee:
            key = (key & 0x0FFFFFF0);
            key = 0x003FFF7200000000 | (key ^ 0xE0E0E0EE);

            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_CAME_TWEE_NAME, key, 54);
            break;
        case SubmenuIndexGateTX:
            key = (key & 0x00F0FF00) | 0xF << 16 | 0x40; //btn 0xF, 0xC, 0xA, 0x6 (?)
            uint64_t rev_key = subghz_protocol_blocks_reverse_key(key, 24);
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_GATE_TX_NAME, rev_key, 24);
            break;
        case SubmenuIndexDoorHan_433_92:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, "DoorHan", key, 0x2, 0x0003);
            if(generated_protocol != SubGhzProtocolStatusOk) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexDoorHan_315_00:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 315000000, "DoorHan", key, 0x2, 0x0003);
            if(generated_protocol != SubGhzProtocolStatusOk) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexLiftMaster_315_00:
            generated_protocol =
                subghz_txrx_gen_secplus_v1_protocol(subghz->txrx, "AM650", 315000000);
            break;
        case SubmenuIndexLiftMaster_390_00:
            generated_protocol =
                subghz_txrx_gen_secplus_v1_protocol(subghz->txrx, "AM650", 390000000);
            break;
        case SubmenuIndexSecPlus_v2_310_00:
            key = (key & 0x7FFFF3FC); // 850LM pairing
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 310000000, key, 0x68, 0xE500000);
            break;
        case SubmenuIndexSecPlus_v2_315_00:
            key = (key & 0x7FFFF3FC); // 850LM pairing
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 315000000, key, 0x68, 0xE500000);
            break;
        case SubmenuIndexSecPlus_v2_390_00:
            key = (key & 0x7FFFF3FC); // 850LM pairing
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 390000000, key, 0x68, 0xE500000);
            break;
        default:
            return false;
            break;
        }

        scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

        if(generated_protocol == SubGhzProtocolStatusOk) {
            subghz_file_name_clear(subghz);
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
