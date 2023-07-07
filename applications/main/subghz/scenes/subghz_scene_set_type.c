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
        "Faac SLH 868MHz",
        SubmenuIndexFaacSLH_868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Faac SLH 433MHz",
        SubmenuIndexFaacSLH_433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "BFT [Manual] 433MHz",
        SubmenuIndexBFTClone,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "BFT Mitto 433MHz",
        SubmenuIndexBFTMitto,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Somfy Telis 433MHz",
        SubmenuIndexSomfyTelis,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "AN-Motors AT4 433MHz",
        SubmenuIndexANMotorsAT4,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Alutech AT4N 433MHz",
        SubmenuIndexAlutechAT4N,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: DoorHan 315MHz",
        SubmenuIndexDoorHan_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: DoorHan 433MHz",
        SubmenuIndexDoorHan_433_92,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Beninca 433MHz",
        SubmenuIndexBeninca433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Beninca 868MHz",
        SubmenuIndexBeninca868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Allmatic 433MHz",
        SubmenuIndexAllmatic433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Allmatic 868MHz",
        SubmenuIndexAllmatic868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Centurion 433MHz",
        SubmenuIndexCenturion433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Sommer 434MHz",
        SubmenuIndexSommer_FM_434,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Sommer 868MHz",
        SubmenuIndexSommer_FM_868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Stilmatic 433MHz",
        SubmenuIndexStilmatic,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: IronLogic 433MHz",
        SubmenuIndexIronLogic,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: DTM Neo 433MHz",
        SubmenuIndexDTMNeo433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Gibidi 433MHz",
        SubmenuIndexGibidi433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: GSN 433MHz",
        SubmenuIndexGSN,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Aprimatic 433MHz",
        SubmenuIndexAprimatic,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Elmes (PL) 433MHz",
        SubmenuIndexElmesElectronic,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: Nice Smilo 433MHz",
        SubmenuIndexNiceSmilo_433_92,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice FloR-S 433MHz",
        SubmenuIndexNiceFlorS_433_92,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice One 433MHz",
        SubmenuIndexNiceOne_433_92,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice Flo 12bit 433MHz",
        SubmenuIndexNiceFlo12bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Nice Flo 24bit 433MHz",
        SubmenuIndexNiceFlo24bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 12bit 433MHz",
        SubmenuIndexCAME12bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 24bit 433MHz",
        SubmenuIndexCAME24bit,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 12bit 868MHz",
        SubmenuIndexCAME12bit868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME 24bit 868MHz",
        SubmenuIndexCAME24bit868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME TWEE 433MHz",
        SubmenuIndexCAMETwee,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME Atomo 433MHz",
        SubmenuIndexCameAtomo433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "CAME Atomo 868MHz",
        SubmenuIndexCameAtomo868,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "KL: CAME Space 433MHz",
        SubmenuIndexCAMESpace,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Princeton 315MHz",
        SubmenuIndexPricenton315,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Princeton 433MHz",
        SubmenuIndexPricenton433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "BETT 433MHz",
        SubmenuIndexBETT_433,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Linear 300MHz",
        SubmenuIndexLinear_300_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    // submenu_add_item(
    //     subghz->submenu, "Nero Sketch", SubmenuIndexNeroSketch, subghz_scene_set_type_submenu_callback, subghz);
    // submenu_add_item(
    //     subghz->submenu, "Nero Radio", SubmenuIndexNeroRadio, subghz_scene_set_type_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Gate TX 433MHz",
        SubmenuIndexGateTX,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+1.0 315MHz",
        SubmenuIndexLiftMaster_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+1.0 390MHz",
        SubmenuIndexLiftMaster_390_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+1.0 433MHz",
        SubmenuIndexLiftMaster_433_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0 310MHz",
        SubmenuIndexSecPlus_v2_310_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0 315MHz",
        SubmenuIndexSecPlus_v2_315_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0 390MHz",
        SubmenuIndexSecPlus_v2_390_00,
        subghz_scene_set_type_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Security+2.0 433MHz",
        SubmenuIndexSecPlus_v2_433_00,
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
        uint32_t key = (uint32_t)rand();
        switch(event.event) {
        case SubmenuIndexFaacSLH_868:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFix);
            break;
        case SubmenuIndexFaacSLH_433:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFix);
            break;
        case SubmenuIndexBFTClone:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFix);
            break;
        case SubmenuIndexPricenton433:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 400);
            break;
        case SubmenuIndexPricenton315:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                subghz->txrx, "AM650", 315000000, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 400);
            break;
        case SubmenuIndexNiceFlo12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 12);
            break;
        case SubmenuIndexNiceFlo24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_NICE_FLO_NAME, key, 24);
            break;
        case SubmenuIndexCAME12bit:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_CAME_NAME, key, 12);
            break;
        case SubmenuIndexCAME24bit:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_CAME_NAME, key, 24);
            break;
        case SubmenuIndexCAME12bit868:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 868350000, SUBGHZ_PROTOCOL_CAME_NAME, key, 12);
            break;
        case SubmenuIndexCAME24bit868:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 868350000, SUBGHZ_PROTOCOL_CAME_NAME, key, 24);
            break;
        case SubmenuIndexLinear_300_00:
            key = (key & 0x3FF);
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 300000000, SUBGHZ_PROTOCOL_LINEAR_NAME, key, 10);
            break;
        case SubmenuIndexBETT_433:
            key = (key & 0x0000FFF0);
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx, "AM650", 433920000, SUBGHZ_PROTOCOL_BETT_NAME, key, 18);
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
        case SubmenuIndexBeninca433:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                (key & 0x000FFF00) | 0x00800080,
                0x1,
                0x0005,
                "Beninca");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexBeninca868:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                868350000,
                (key & 0x000FFF00) | 0x00800080,
                0x1,
                0x0005,
                "Beninca");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexAllmatic433:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                (key & 0x00FFFF00) | 0x01000011,
                0xC,
                0x0005,
                "Beninca");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexAllmatic868:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                868350000,
                (key & 0x00FFFF00) | 0x01000011,
                0xC,
                0x0005,
                "Beninca");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexCenturion433:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, (key & 0x0000FFFF), 0x2, 0x0003, "Centurion");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexElmesElectronic:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                (key & 0x00FFFFFF) | 0x02000000,
                0x2,
                0x0003,
                "Elmes_Poland");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexANMotorsAT4:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                (key & 0x000FFFFF) | 0x04700000,
                0x2,
                0x0021,
                "AN-Motors");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexAprimatic:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                (key & 0x000FFFFF) | 0x00600000,
                0x4,
                0x0003,
                "Aprimatic");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexGibidi433:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "Gibidi");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexGSN:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x0FFFFFFF, 0x2, 0x0003, "GSN");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexIronLogic:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x00FFFFF0, 0x4, 0x0005, "IronLogic");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexStilmatic:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x0FFFFFFF, 0x1, 0x0003, "Stilmatic");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexSommer_FM_434:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "FM476", 434420000, key & 0x0FFFFFFF, 0x4, 0x0003, "Sommer(fsk476)");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexSommer_FM_868:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "FM476", 868800000, key & 0x0FFFFFFF, 0x4, 0x0003, "Sommer(fsk476)");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexDTMNeo433:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x000FFFFF, 0x2, 0x0005, "DTM_Neo");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexCAMESpace:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "Came_Space");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexCameAtomo433:
            generated_protocol = subghz_txrx_gen_came_atomo_protocol(
                subghz->txrx, "AM650", 433920000, (key & 0x0FFFFFFF) | 0x10000000, 0x0003);
            break;
        case SubmenuIndexCameAtomo868:
            generated_protocol = subghz_txrx_gen_came_atomo_protocol(
                subghz->txrx, "AM650", 868350000, (key & 0x0FFFFFFF) | 0x10000000, 0x0003);
            break;
        case SubmenuIndexBFTMitto:
            generated_protocol = subghz_txrx_gen_keeloq_bft_protocol(
                subghz->txrx,
                "AM650",
                433920000,
                key & 0x000FFFFF,
                0x2,
                0x0002,
                key & 0x000FFFFF,
                "BFT");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexAlutechAT4N:
            generated_protocol = subghz_txrx_gen_alutech_at_4n_protocol(
                subghz->txrx, "AM650", 433920000, (key & 0x000FFFFF) | 0x00100000, 0x44, 0x0003);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexSomfyTelis:
            generated_protocol = subghz_txrx_gen_somfy_telis_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003);
            break;
        case SubmenuIndexDoorHan_433_92:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x0FFFFFFF, 0x2, 0x0003, "DoorHan");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexDoorHan_315_00:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 315000000, key & 0x0FFFFFFF, 0x2, 0x0003, "DoorHan");
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexNiceFlorS_433_92:
            generated_protocol = subghz_txrx_gen_nice_flor_s_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x0FFFFFFF, 0x1, 0x0003, false);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexNiceOne_433_92:
            generated_protocol = subghz_txrx_gen_nice_flor_s_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x0FFFFFFF, 0x1, 0x0003, true);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexNiceSmilo_433_92:
            generated_protocol = subghz_txrx_gen_keeloq_protocol(
                subghz->txrx, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "NICE_Smilo");
            if(!generated_protocol) {
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
        case SubmenuIndexLiftMaster_433_00:
            generated_protocol =
                subghz_txrx_gen_secplus_v1_protocol(subghz->txrx, "AM650", 433920000);
            break;
        case SubmenuIndexSecPlus_v2_310_00:
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 310000000, key, 0x68, 0xE500000);
            break;
        case SubmenuIndexSecPlus_v2_315_00:
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 315000000, key, 0x68, 0xE500000);
            break;
        case SubmenuIndexSecPlus_v2_390_00:
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 390000000, key, 0x68, 0xE500000);
            break;
        case SubmenuIndexSecPlus_v2_433_00:
            generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
                subghz->txrx, "AM650", 433920000, key, 0x68, 0xE500000);
            break;
        default:
            return false;
            break;
        }

        scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

        if(generated_protocol) {
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
