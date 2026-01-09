#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"
#include "../helpers/subghz_gen_info.h"
#include "subghz_scene_start.h"
#include <lib/subghz/blocks/math.h>

#define TAG "SubGhzSetType"

void subghz_scene_set_type_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

static const char* submenu_names[SetTypeMAX] = {
    [SetTypeFaacSLH_868] = "FAAC SLH 868MHz",
    [SetTypeFaacSLH_433] = "FAAC SLH 433MHz",
    [SetTypeBFTMitto] = "BFT Mitto 433MHz",
    [SetTypeSomfyTelis] = "Somfy Telis 433MHz",
    [SetTypeANMotorsAT4] = "AN-Motors AT4 433MHz",
    [SetTypeAlutechAT4N] = "Alutech AT4N 433MHz",
    [SetTypeRoger_433] = "Roger 433MHz",
    [SetTypePhoenix_V2_433] = "V2 Phoenix 433MHz",
    [SetTypeKingGatesStylo4k] = "KingGates Stylo4 433MHz",
    [SetTypeHCS101_433_92] = "KL: HCS101 433MHz",
    [SetTypeDoorHan_315_00] = "KL: DoorHan 315MHz",
    [SetTypeDoorHan_433_92] = "KL: DoorHan 433MHz",
    [SetTypeBeninca433] = "KL: Beninca 433MHz",
    [SetTypeBeninca868] = "KL: Beninca 868MHz",
    [SetTypeComunello433] = "KL: Comunello 433MHz",
    [SetTypeComunello868] = "KL: Comunello 868MHz",
    [SetTypeAllmatic433] = "KL: Allmatic 433MHz",
    [SetTypeAllmatic868] = "KL: Allmatic 868MHz",
    [SetTypeMotorline433] = "KL: Motorline 433MHz",
    [SetTypeCenturion433] = "KL: Centurion 433MHz",
    [SetTypeMonarch433] = "KL: Monarch 433MHz",
    [SetTypeJollyMotors433] = "KL: Jolly Mot. 433MHz",
    [SetTypeSommer_FM_434] = "KL: Sommer 434MHz",
    [SetTypeSommer_FM_868] = "KL: Sommer 868MHz",
    [SetTypeSommer_FM12K_434] = "KL: Sommer fm2 434Mhz",
    [SetTypeSommer_FM12K_868] = "KL: Sommer fm2 868Mhz",
    [SetTypeStilmatic] = "KL: Stilmatic 433MHz",
    [SetTypeIronLogic] = "KL: IronLogic 433MHz",
    [SetTypeIronLogicSmart] = "KL: IronLogic SM 433MHz",
    [SetTypeDeaMio433] = "KL: DEA Mio 433MHz",
    [SetTypeDTMNeo433] = "KL: DTM Neo 433MHz",
    [SetTypeGibidi433] = "KL: Gibidi 433MHz",
    [SetTypeGSN] = "KL: GSN 433MHz",
    [SetTypeAprimatic] = "KL: Aprimatic 433MHz",
    [SetTypeElmesElectronic] = "KL: Elmes (PL) 433MHz",
    [SetTypeNormstahl_433_92] = "KL: Normstahl 433MHz",
    [SetTypeJCM_433_92] = "KL: JCM Tech 433MHz",
    [SetTypeNovoferm_433_92] = "KL: Novoferm 433MHz",
    [SetTypeHormannEcoStar_433_92] = "KL: Hor. EcoStar 433MHz",
    [SetTypeCardinS449_433FM] = "KL: Cardin S449 433MHz",
    [SetTypeFAACRCXT_433_92] = "KL: FAAC RC,XT 433MHz",
    [SetTypeFAACRCXT_868] = "KL: FAAC RC,XT 868MHz",
    [SetTypeGeniusBravo433] = "KL: Genius Bravo 433MHz",
    [SetTypeNiceMHouse_433_92] = "KL: Mhouse 433MHz",
    [SetTypeNiceSmilo_433_92] = "KL: Nice Smilo 433MHz",
    [SetTypeNiceFlorS_433_92] = "Nice FloR-S 433MHz",
    [SetTypeNiceOne_433_92] = "Nice One 433MHz",
    [SetTypeNiceFlo12bit] = "Nice Flo 12bit 433MHz",
    [SetTypeNiceFlo24bit] = "Nice Flo 24bit 433MHz",
    [SetTypeCAME12bit] = "CAME 12bit 433MHz",
    [SetTypeCAME24bit] = "CAME 24bit 433MHz",
    [SetTypeCAME12bit868] = "CAME 12bit 868MHz",
    [SetTypeCAME24bit868] = "CAME 24bit 868MHz",
    [SetTypeCAMETwee] = "CAME TWEE 433MHz",
    [SetTypeCameAtomo433] = "CAME Atomo 433MHz",
    [SetTypeCameAtomo868] = "CAME Atomo 868MHz",
    [SetTypeCAMESpace] = "KL: CAME Space 433MHz",
    [SetTypePricenton315] = "Princeton 315MHz",
    [SetTypePricenton433] = "Princeton 433MHz",
    [SetTypeZKTeco430] = "ZKTeco 430MHz",
    [SetTypeGangQi_433] = "GangQi 433MHz",
    [SetTypeHollarm_433] = "Hollarm 433MHz",
    [SetTypeReversRB2_433] = "Revers RB2 433MHz",
    [SetTypeMarantec24_868] = "Marantec24 868MHz",
    [SetTypeMarantec_433] = "Marantec 433MHz",
    [SetTypeMarantec_868] = "Marantec 868MHz",
    [SetTypeBETT_433] = "BETT 433MHz",
    [SetTypeLinear_300_00] = "Linear 300MHz",
    // [SetTypeNeroSketch] = "Nero Sketch", // Deleted in OFW
    // [SetTypeNeroRadio] = "Nero Radio", // Deleted in OFW
    [SetTypeGateTX] = "Gate TX 433MHz",
    [SetTypeSecPlus_v1_315_00] = "Security+1.0 315MHz",
    [SetTypeSecPlus_v1_390_00] = "Security+1.0 390MHz",
    [SetTypeSecPlus_v1_433_00] = "Security+1.0 433MHz",
    [SetTypeSecPlus_v2_310_00] = "Security+2.0 310MHz",
    [SetTypeSecPlus_v2_315_00] = "Security+2.0 315MHz",
    [SetTypeSecPlus_v2_390_00] = "Security+2.0 390MHz",
    [SetTypeSecPlus_v2_433_00] = "Security+2.0 433MHz",
};

void subghz_scene_set_type_on_enter(void* context) {
    SubGhz* subghz = context;

    for(SetType i = 0; i < SetTypeMAX; i++) {
        submenu_add_item(
            subghz->submenu, submenu_names[i], i, subghz_scene_set_type_submenu_callback, subghz);
    }

    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_set_type_generate_protocol_from_infos(SubGhz* subghz) {
    GenInfo gen_info = *subghz->gen_info;
    bool generated_protocol = false;
    switch(gen_info.type) {
    case GenData:
        if(gen_info.data.te) {
            generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                subghz->txrx,
                gen_info.mod,
                gen_info.freq,
                gen_info.data.name,
                gen_info.data.key,
                gen_info.data.bits,
                gen_info.data.te);
        } else {
            generated_protocol = subghz_txrx_gen_data_protocol(
                subghz->txrx,
                gen_info.mod,
                gen_info.freq,
                gen_info.data.name,
                gen_info.data.key,
                gen_info.data.bits);
        }
        break;
    case GenFaacSLH:
        generated_protocol = subghz_txrx_gen_faac_slh_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.faac_slh.serial,
            gen_info.faac_slh.btn,
            gen_info.faac_slh.cnt,
            gen_info.faac_slh.seed,
            gen_info.faac_slh.manuf);
        break;
    case GenKeeloq:
        generated_protocol = subghz_txrx_gen_keeloq_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.keeloq.serial,
            gen_info.keeloq.btn,
            gen_info.keeloq.cnt,
            gen_info.keeloq.manuf);
        break;
    case GenCameAtomo:
        generated_protocol = subghz_txrx_gen_came_atomo_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.came_atomo.serial,
            gen_info.came_atomo.cnt);
        break;
    case GenKeeloqBFT:
        generated_protocol = subghz_txrx_gen_keeloq_bft_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.keeloq_bft.serial,
            gen_info.keeloq_bft.btn,
            gen_info.keeloq_bft.cnt,
            gen_info.keeloq_bft.seed,
            gen_info.keeloq_bft.manuf);
        break;
    case GenAlutechAt4n:
        generated_protocol = subghz_txrx_gen_alutech_at_4n_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.alutech_at_4n.serial,
            gen_info.alutech_at_4n.btn,
            gen_info.alutech_at_4n.cnt);
        break;
    case GenSomfyTelis:
        generated_protocol = subghz_txrx_gen_somfy_telis_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.somfy_telis.serial,
            gen_info.somfy_telis.btn,
            gen_info.somfy_telis.cnt);
        break;
    case GenKingGatesStylo4k:
        generated_protocol = subghz_txrx_gen_kinggates_stylo_4k_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.kinggates_stylo_4k.serial,
            gen_info.kinggates_stylo_4k.btn,
            gen_info.kinggates_stylo_4k.cnt);
        break;
    case GenNiceFlorS:
        generated_protocol = subghz_txrx_gen_nice_flor_s_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.nice_flor_s.serial,
            gen_info.nice_flor_s.btn,
            gen_info.nice_flor_s.cnt,
            gen_info.nice_flor_s.nice_one);
        break;
    case GenSecPlus1:
        generated_protocol =
            subghz_txrx_gen_secplus_v1_protocol(subghz->txrx, gen_info.mod, gen_info.freq);
        break;
    case GenSecPlus2:
        generated_protocol = subghz_txrx_gen_secplus_v2_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.sec_plus_2.serial,
            gen_info.sec_plus_2.btn,
            gen_info.sec_plus_2.cnt);
        break;
    case GenPhoenixV2:
        generated_protocol = subghz_txrx_gen_phoenix_v2_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.phoenix_v2.serial,
            gen_info.phoenix_v2.cnt);
        break;
    default:
        furi_crash("Not implemented");
        break;
    }

    if(generated_protocol) {
        subghz_file_name_clear(subghz);
        scene_manager_set_scene_state(
            subghz->scene_manager, SubGhzSceneSetType, SubGhzCustomEventManagerSet);
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
    } else {
        furi_string_set(subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
    }
    return generated_protocol;
}

bool subghz_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event >= SetTypeMAX) {
            return false;
        }
        scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

        subghz_gen_info_reset(subghz->gen_info);
        subghz_scene_set_type_fill_generation_infos(subghz->gen_info, event.event);

        if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart) ==
           SubmenuIndexAddManually) {
            generated_protocol = subghz_scene_set_type_generate_protocol_from_infos(subghz);
        } else if(
            scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart) ==
            SubmenuIndexAddManuallyAdvanced) {
            switch(subghz->gen_info->type) {
            case GenData: // Key (u64)
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetKey);
                break;
            case GenSecPlus1: // None
                return subghz_scene_set_type_generate_protocol_from_infos(subghz);
            case GenFaacSLH: // Serial (u32), Button (u8), Counter (u32), Seed (u32)
            case GenKeeloq: // Serial (u32), Button (u8), Counter (u16)
            case GenCameAtomo: // Serial (u32), Counter (u16)
            case GenKeeloqBFT: // Serial (u32), Button (u8), Counter (u16), Seed (u32)
            case GenAlutechAt4n: // Serial (u32), Button (u8), Counter (u16)
            case GenSomfyTelis: // Serial (u32), Button (u8), Counter (u16)
            case GenKingGatesStylo4k: // Serial (u32), Button (u8), Counter (u16)
            case GenNiceFlorS: // Serial (u32), Button (u8), Counter (u16)
            case GenSecPlus2: // Serial (u32), Button (u8), Counter (u32)
            case GenPhoenixV2: // Serial (u32), Counter (u16)
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetSerial);
                break;
            }
            return true;
        }
    }

    return generated_protocol;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
