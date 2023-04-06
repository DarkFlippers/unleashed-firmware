#include "../subghz_i.h"
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/nice_flor_s.h>
#include <lib/subghz/protocols/faac_slh.h>
#include <lib/subghz/protocols/secplus_v1.h>
#include <lib/subghz/protocols/secplus_v2.h>
#include <lib/subghz/blocks/math.h>
#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/protocol_items.h>

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
        furi_string_set(subghz->error_str, "Protocol not\nfound!");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(subghz->txrx->fff_data);
        stream_clean(fff_data_stream);
        if(subghz_protocol_decoder_base_serialize(
               subghz->txrx->decoder_result, subghz->txrx->fff_data, subghz->txrx->preset) !=
           SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(subghz->txrx->fff_data, "Bit", &bit, 1)) {
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(subghz->txrx->fff_data, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
        res = true;
    } while(false);
    return res;
}

bool subghz_scene_set_type_submenu_gen_data_keeloq(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name) {
    SubGhz* subghz = context;

    bool res = false;

    subghz->txrx->transmitter =
        subghz_transmitter_alloc_init(subghz->txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_preset_init(subghz, preset_name, frequency, NULL, 0);

    if(subghz->txrx->transmitter &&
       subghz_protocol_keeloq_create_data(
           subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
           subghz->txrx->fff_data,
           serial,
           btn,
           cnt,
           manufacture_name,
           subghz->txrx->preset)) {
        flipper_format_write_string_cstr(subghz->txrx->fff_data, "Manufacture", manufacture_name);
        res = true;
    }

    subghz_transmitter_free(subghz->txrx->transmitter);
    if(!res) {
        furi_string_set(subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
    }
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
        SubmenuIndexPricenton,
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
        //ToDo Fix
        uint32_t key = subghz_random_serial();
        switch(event.event) {
        case SubmenuIndexFaacSLH_868:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFixFaac);
            break;
        case SubmenuIndexFaacSLH_433:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFixFaac);
            break;
        case SubmenuIndexBFTClone:
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFixBft);
            break;
        case SubmenuIndexPricenton:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 433920000, "AM650")) {
                uint32_t te = 400;
                flipper_format_update_uint32(subghz->txrx->fff_data, "TE", (uint32_t*)&te, 1);
                generated_protocol = true;
            }
            break;
        case SubmenuIndexPricenton315:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_PRINCETON_NAME, key, 24, 315000000, "AM650")) {
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
        case SubmenuIndexCAME12bit868:
            key = (key & 0x0000FFF0) | 0x1; //btn 0x1, 0x2, 0x4
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_CAME_NAME, key, 12, 868350000, "AM650")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAME24bit868:
            key = (key & 0x00FFFFF0) | 0x4; //btn 0x1, 0x2, 0x4, 0x8
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_CAME_NAME, key, 24, 868350000, "AM650")) {
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
        case SubmenuIndexBETT_433:
            key = (key & 0x0000FFF0);
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz, SUBGHZ_PROTOCOL_BETT_NAME, key, 18, 433920000, "AM650")) {
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
        case SubmenuIndexBeninca433:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   433920000,
                   (key & 0x000FFF00) | 0x00800080,
                   0x1,
                   0x0005,
                   "Beninca")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexBeninca868:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   868350000,
                   (key & 0x000FFF00) | 0x00800080,
                   0x1,
                   0x0005,
                   "Beninca")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexAllmatic433:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   433920000,
                   (key & 0x00FFFF00) | 0x01000011,
                   0xC,
                   0x0005,
                   "Beninca")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexAllmatic868:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   868350000,
                   (key & 0x00FFFF00) | 0x01000011,
                   0xC,
                   0x0005,
                   "Beninca")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexElmesElectronic:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   433920000,
                   (key & 0x00FFFFFF) | 0x02000000,
                   0x2,
                   0x0003,
                   "Elmes_Poland")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexANMotorsAT4:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   433920000,
                   (key & 0x000FFFFF) | 0x04700000,
                   0x2,
                   0x0021,
                   "AN-Motors")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexAprimatic:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz,
                   "AM650",
                   433920000,
                   (key & 0x000FFFFF) | 0x00600000,
                   0x4,
                   0x0003,
                   "Aprimatic")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexGibidi433:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "Gibidi")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexGSN:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x0FFFFFFF, 0x2, 0x0003, "GSN")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexIronLogic:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x00FFFFF0, 0x4, 0x0005, "IronLogic")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexSommer_FM_434:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "FM476", 434420000, key & 0x0FFFFFFF, 0x4, 0x0003, "Sommer(fsk476)")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexSommer_FM_868:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "FM476", 868800000, key & 0x0FFFFFFF, 0x4, 0x0003, "Sommer(fsk476)")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexDTMNeo433:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x000FFFFF, 0x2, 0x0005, "DTM_Neo")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexCAMESpace:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "Came_Space")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexBFTMitto:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_keeloq_bft_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x000FFFFF,
                    0x2,
                    0x0002,
                    key & 0x000FFFFF,
                    "BFT",
                    subghz->txrx->preset);

                uint8_t seed_data[sizeof(uint32_t)] = {0};
                for(size_t i = 0; i < sizeof(uint32_t); i++) {
                    seed_data[sizeof(uint32_t) - i - 1] = ((key & 0x000FFFFF) >> i * 8) & 0xFF;
                }

                flipper_format_write_hex(
                    subghz->txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));

                flipper_format_write_string_cstr(subghz->txrx->fff_data, "Manufacture", "BFT");

                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexAlutechAT4N:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_alutech_at_4n_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    (key & 0x000FFFFF) | 0x00100000,
                    0x44,
                    0x0003,
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexSomfyTelis:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_somfy_telis_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x00FFFFFF,
                    0x2,
                    0x0003,
                    subghz->txrx->preset);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexDoorHan_433_92:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x0FFFFFFF, 0x2, 0x0003, "DoorHan")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexDoorHan_315_00:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 315000000, key & 0x0FFFFFFF, 0x2, 0x0003, "DoorHan")) {
                generated_protocol = true;
            }
            break;
        case SubmenuIndexNiceFlorS_433_92:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_nice_flor_s_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x1,
                    0x0003,
                    subghz->txrx->preset,
                    false);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexNiceOne_433_92:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
            if(subghz->txrx->transmitter) {
                subghz_protocol_nice_flor_s_create_data(
                    subghz_transmitter_get_protocol_instance(subghz->txrx->transmitter),
                    subghz->txrx->fff_data,
                    key & 0x0FFFFFFF,
                    0x1,
                    0x0003,
                    subghz->txrx->preset,
                    true);
                generated_protocol = true;
            } else {
                generated_protocol = false;
            }
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            break;
        case SubmenuIndexNiceSmilo_433_92:
            if(subghz_scene_set_type_submenu_gen_data_keeloq(
                   subghz, "AM650", 433920000, key & 0x00FFFFFF, 0x2, 0x0003, "NICE_Smilo")) {
                generated_protocol = true;
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
        case SubmenuIndexLiftMaster_433_00:
            while(!subghz_protocol_secplus_v1_check_fixed(key)) {
                key = subghz_random_serial();
            }
            if(subghz_scene_set_type_submenu_gen_data_protocol(
                   subghz,
                   SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
                   (uint64_t)key << 32 | 0xE6000000,
                   42,
                   433920000,
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
        case SubmenuIndexSecPlus_v2_433_00:
            subghz->txrx->transmitter = subghz_transmitter_alloc_init(
                subghz->txrx->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
            subghz_preset_init(subghz, "AM650", 433920000, NULL, 0);
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
