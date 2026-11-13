#include <subghz/subghz_i.h>
#include "subghz_txrx_create_protocol_key.h"
#include "subghz_add_manually_i.h"
#include <subghz/helpers/subghz_gen_info.h>
#include <subghz/scenes/subghz_scene_start.h>
#include <lib/subghz/blocks/math.h>

#define TAG "SubGhzSetType"

void subghz_scene_set_type_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

/* NULL means the type has no row of its own: it only supplies the data for a band offered
   under another row - see set_type_alt_bands in the frequency scene. */
static const char* submenu_names[SetTypeMAX] = {
    [SetTypeAllmatic] = "KL: Allmatic",
    [SetTypeAlutechAT4N] = "Alutech AT4N",
    [SetTypeANMotorsAT4] = "AN-Motors AT4",
    [SetTypeAprimatic] = "KL: Aprimatic",
    [SetTypeATA_PTX4] = "KL: ATA PTX4",
    [SetTypeBelGate] = "BelGate",
    [SetTypeBeninca] = "KL: Beninca",
    [SetTypeBenincaARC] = "Beninca ARC",
    [SetTypeBETT] = "BETT",
    [SetTypeBFTMitto] = "BFT Mitto",
    [SetTypeCAME12bit] = "CAME 12bit",
    [SetTypeCAME24bit] = "CAME 24bit",
    [SetTypeCameAtomo] = "CAME Atomo",
    [SetTypeCAMESpace] = "KL: CAME Space",
    [SetTypeCAMETwee] = "CAME TWEE",
    [SetTypeCardinS449FM] = "KL: Cardin S449",
    [SetTypeCenturion] = "KL: Centurion",
    [SetTypeClemsa_Mutancode] = "KL: Clm.Mutancode",
    [SetTypeComunello] = "KL: Comunello",
    [SetTypeDeaMio] = "KL: DEA Mio",
    [SetTypeDitecGOL4] = "Ditec GOL4",
    [SetTypeDoorHan] = "KL: DoorHan",
    [SetTypeDoormatic] = "KL: Doormatic",
    [SetTypeDTMNeo] = "KL: DTM Neo",
    [SetTypeElmesElectronic] = "KL: Elmes (PL)",
    [SetTypeElvox] = "KL: Elvox",
    [SetTypeErreka] = "Erreka",
    [SetTypeET_Blue] = "KL: ET Blue",
    [SetTypeET_Blue_Mix] = "KL: ET Blue Mix",
    [SetTypeFAACRCXT] = "KL: FAAC RC,XT",
    [SetTypeFaacSLH] = "FAAC SLH",
    [SetTypeFaacTm] = "FAAC TM",
    [SetTypeFadini] = "KL: Fadini",
    [SetTypeGangQi] = "GangQi",
    [SetTypeGateTX] = "Gate TX",
    [SetTypeGenius] = "Genius",
    [SetTypeGeniusBravo] = "KL: Genius TX4RC",
    [SetTypeGibidi] = "KL: Gibidi",
    [SetTypeGSN] = "KL: GSN",
    [SetTypeHCS101] = "KL: HCS101",
    [SetTypeHollarm] = "Hollarm",
    [SetTypeHomeGate] = "KL: HomeGate",
    [SetTypeHormannEcoStar] = "KL: Hor. EcoStar",
    [SetTypeIronLogic] = "KL: IronLogic",
    [SetTypeIronLogicSmart] = "KL: IronLogic SM",
    [SetTypeJarolift] = "Jarolift",
    [SetTypeJCM] = "KL: JCM Tech",
    [SetTypeJollyMotors] = "KL: Jolly Mot.",
    [SetTypeKEY] = "KL: KEY",
    [SetTypeKingGatesStylo4k] = "KingGates Stylo4k",
    [SetTypeKorn] = "Korn",
    [SetTypeLinear] = "Linear",
    [SetTypeMarantec] = "Marantec",
    [SetTypeMarantec24] = "Marantec24",
    [SetTypeMc_Garcia] = "KL: Mc Garcia",
    [SetTypeNiceMHouse] = "KL: Mhouse",
    [SetTypeMonarch] = "KL: Monarch",
    [SetTypeMotorline] = "KL: Motorline",
    [SetTypeNiceFlo12bit] = "Nice Flo 12bit",
    [SetTypeNiceFlo24bit] = "Nice Flo 24bit",
    [SetTypeNiceFlorS] = "Nice FloR-S",
    [SetTypeNiceOCode] = "Nice O-Code",
    [SetTypeNiceOCodeOne] = "Nice O-Code One",
    [SetTypeNiceOCodeCert] = "Nice O-Code+Crt",
    [SetTypeNiceOne] = "Nice One",
    [SetTypeNiceSmilo] = "KL: Nice Smilo",
    [SetTypeNordIce] = "Nord ICE",
    [SetTypeNormstahl] = "KL: Normstahl",
    [SetTypeNovoferm] = "KL: Novoferm",
    [SetTypePhantomHiland] = "KL:Phantom(Hiland)",
    [SetTypePrastel] = "Prastel",
    [SetTypePricenton] = "Princeton",
    [SetTypeProteco] = "Proteco",
    [SetTypePujol] = "KL: Pujol",
    [SetTypePujol_Vario] = "KL: Pujol Vario",
    [SetTypeReversRB2] = "Revers RB2",
    [SetTypeRoger] = "Roger",
    [SetTypeSeav] = "KL: Seav",
    [SetTypeSecPlus_v1] = "Security+1.0",
    [SetTypeSecPlus_v2] = "Security+2.0",
    [SetTypeSomfyKeytis] = "Somfy Keytis",
    [SetTypeSomfyTelis] = "Somfy Telis",
    [SetTypeSommer_FM476] = NULL, // Sommer's other modulation, offered under its row
    [SetTypeSommer] = "KL: Sommer",
    [SetTypeStilmatic] = "KL: Stilmatic",
    [SetTypeSuperrollo] = "KL: Superrollo",
    [SetTypeTelcomaEdge] = "Telcoma EDGE",
    [SetTypePhoenix_V2] = "V2 Phoenix",
    [SetTypeVerex] = "KL: Verex",
    [SetTypeWisniowski] = "KL: Wisniowski",
    [SetTypeZKTeco] = "ZKTeco",
};

void subghz_scene_set_type_on_enter(void* context) {
    SubGhz* subghz = context;

    for(SetType i = 0; i < SetTypeMAX; i++) {
        if(!submenu_names[i]) {
            continue;
        }
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
    case GenKeeloqSeed:
        generated_protocol = subghz_txrx_gen_keeloq_seed_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.keeloq_seed.serial,
            gen_info.keeloq_seed.btn,
            gen_info.keeloq_seed.cnt,
            gen_info.keeloq_seed.seed,
            gen_info.keeloq_seed.manuf);
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
    case GenSomfyKeytis:
        generated_protocol = subghz_txrx_gen_somfy_keytis_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.somfy_keytis.serial,
            gen_info.somfy_keytis.btn,
            gen_info.somfy_keytis.cnt);
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
    case GenBenincaARC:
        generated_protocol = subghz_txrx_gen_beninca_arc_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.beninca_arc.serial,
            gen_info.beninca_arc.btn,
            gen_info.beninca_arc.cnt);
        break;
    case GenBelKorn:
        generated_protocol = subghz_txrx_gen_belkorn_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.belkorn.model,
            gen_info.belkorn.serial,
            gen_info.belkorn.btn,
            gen_info.belkorn.cnt);
        break;
    case GenJarolift:
        generated_protocol = subghz_txrx_gen_jarolift_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.jarolift.serial,
            gen_info.jarolift.btn,
            gen_info.jarolift.cnt);
        break;
    case GenDitecGOL4:
        generated_protocol = subghz_txrx_gen_ditec_gol4_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.ditec_gol4.serial,
            gen_info.ditec_gol4.btn,
            gen_info.ditec_gol4.cnt);
        break;
    case GenNiceFlorS:
        generated_protocol = subghz_txrx_gen_nice_flor_s_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.nice_flor_s.serial,
            gen_info.nice_flor_s.btn,
            gen_info.nice_flor_s.cnt,
            gen_info.nice_flor_s.nice_one,
            gen_info.nice_flor_s.o_code,
            gen_info.nice_flor_s.ic,
            0);
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
    case GenPrastel:
        generated_protocol = subghz_txrx_gen_prastel_protocol(
            subghz->txrx,
            gen_info.mod,
            gen_info.freq,
            gen_info.prastel.serial,
            gen_info.prastel.btn,
            gen_info.prastel.cnt);
        break;
    default:
        furi_crash("Not implemented");
        break;
    }

    if(generated_protocol) {
        subghz_file_name_clear(subghz);
        scene_manager_set_scene_state(
            subghz->scene_manager, SubGhzSceneSaveName, SubGhzCustomEventManagerSet);
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
    } else {
        furi_string_set(subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
    }
    return generated_protocol;
}

const char* subghz_set_type_name(SetType type) {
    furi_check(type < SetTypeMAX);
    return submenu_names[type];
}

void subghz_scene_set_type_fill(SubGhz* subghz, SetType type) {
    subghz_gen_info_reset(subghz->gen_info);
    subghz_scene_set_type_fill_generation_infos(subghz->gen_info, type);
}

bool subghz_scene_set_type_proceed(SubGhz* subghz) {
    bool generated_protocol = false;

    if((subghz->gen_info->type == GenNiceFlorS) && subghz->gen_info->nice_flor_s.cert_wanted) {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetSerial);
        return true;
    }

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
        case GenKeeloqSeed: // Serial (u32), Button (u8), Counter (u16), Seed (u32)
        case GenAlutechAt4n: // Serial (u32), Button (u8), Counter (u16)
        case GenSomfyTelis: // Serial (u32), Button (u8), Counter (u16)
        case GenSomfyKeytis: // Serial (u32), Button (u8), Counter (u16)
        case GenKingGatesStylo4k: // Serial (u32), Button (u8), Counter (u16)
        case GenBenincaARC: // Serial (u32), Button (u8), Counter (u32)
        case GenJarolift: // Serial (u32), Button (u4), Counter (u16)
        case GenDitecGOL4: // Serial (u32), Button (u4), Counter (u16)
        case GenNiceFlorS: // Serial (u32), Button (u8), Counter (u16), IC (u16, O-Code)
        case GenSecPlus2: // Serial (u32), Button (u8), Counter (u32)
        case GenPhoenixV2: // Serial (u32), Counter (u16)
        case GenPrastel: // Serial (u32), Button (u8), Counter (u16)
        case GenBelKorn: // Serial (u32), Button (u8), Counter (u16)
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetSerial);
            break;
        }
        return true;
    }

    return generated_protocol;
}

bool subghz_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type != SceneManagerEventTypeCustom) {
        return false;
    }
    if(event.event >= SetTypeMAX) {
        return false;
    }
    scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

    // The band comes next, and it can pick another variant's data, so nothing is filled here
    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFreq);
    return true;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
