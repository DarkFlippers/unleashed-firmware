#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"
#include <lib/subghz/blocks/math.h>
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhzSetType"

void subghz_scene_set_type_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

static const char* submenu_names[SetTypeMAX] = {
    [SetTypeFaacSLH_Manual_868] = "FAAC SLH [Man.] 868MHz",
    [SetTypeFaacSLH_Manual_433] = "FAAC SLH [Man.] 433MHz",
    [SetTypeBFTClone] = "BFT [Manual] 433MHz",
    [SetTypeFaacSLH_868] = "FAAC SLH 868MHz",
    [SetTypeFaacSLH_433] = "FAAC SLH 433MHz",
    [SetTypeBFTMitto] = "BFT Mitto 433MHz",
    [SetTypeSomfyTelis] = "Somfy Telis 433MHz",
    [SetTypeANMotorsAT4] = "AN-Motors AT4 433MHz",
    [SetTypeAlutechAT4N] = "Alutech AT4N 433MHz",
    [SetTypeHCS101_433_92] = "KL: HCS101 433MHz",
    [SetTypeDoorHan_315_00] = "KL: DoorHan 315MHz",
    [SetTypeDoorHan_433_92] = "KL: DoorHan 433MHz",
    [SetTypeBeninca433] = "KL: Beninca 433MHz",
    [SetTypeBeninca868] = "KL: Beninca 868MHz",
    [SetTypeAllmatic433] = "KL: Allmatic 433MHz",
    [SetTypeAllmatic868] = "KL: Allmatic 868MHz",
    [SetTypeCenturion433] = "KL: Centurion 433MHz",
    [SetTypeMonarch433] = "KL: Monarch 433MHz",
    [SetTypeSommer_FM_434] = "KL: Sommer 434MHz",
    [SetTypeSommer_FM_868] = "KL: Sommer 868MHz",
    [SetTypeSommer_FM238_434] = "KL: Sommer fm2 434Mhz",
    [SetTypeSommer_FM238_868] = "KL: Sommer fm2 868Mhz",
    [SetTypeStilmatic] = "KL: Stilmatic 433MHz",
    [SetTypeIronLogic] = "KL: IronLogic 433MHz",
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
    [SetTypeGangQi_433] = "GangQi 433MHz",
    [SetTypeHollarm_433] = "Hollarm 433MHz",
    [SetTypeMarantec24_868] = "Marantec24 868MHz",
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

typedef enum {
    GenData,
    GenFaacSLH,
    GenKeeloq,
    GenCameAtomo,
    GenKeeloqBFT,
    GenAlutechAt4n,
    GenSomfyTelis,
    GenNiceFlorS,
    GenSecPlus1,
    GenSecPlus2,
} GenType;

typedef struct {
    GenType type;
    const char* mod;
    uint32_t freq;
    union {
        struct {
            const char* name;
            uint64_t key;
            uint8_t bits;
            uint16_t te;
        } data;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
            uint32_t seed;
            const char* manuf;
        } faac_slh;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
            const char* manuf;
        } keeloq;
        struct {
            uint32_t serial;
            uint8_t cnt;
        } came_atomo;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
            uint32_t seed;
            const char* manuf;
        } keeloq_bft;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
        } alutech_at_4n;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
        } somfy_telis;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint8_t cnt;
            bool nice_one;
        } nice_flor_s;
        struct {
            uint32_t serial;
            uint8_t btn;
            uint32_t cnt;
        } sec_plus_2;
    };
} GenInfo;

bool subghz_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event >= SetTypeMAX) {
            return false;
        }
        scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSetType, event.event);

        if(event.event == SetTypeFaacSLH_Manual_868 || event.event == SetTypeFaacSLH_Manual_433 ||
           event.event == SetTypeBFTClone) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetFix);
            return true;
        }

        uint64_t key = (uint64_t)rand();

        uint64_t gangqi_key;
        subghz_txrx_gen_serial_gangqi(&gangqi_key);

        GenInfo gen_info = {0};
        switch(event.event) {
        case SetTypePricenton433:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_PRINCETON_NAME,
                .data.key = (key & 0x00FFFFF0) | 0x4, // btn 0x1, 0x2, 0x4, 0x8
                .data.bits = 24,
                .data.te = 400};
            break;
        case SetTypePricenton315:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 315000000,
                .data.name = SUBGHZ_PROTOCOL_PRINCETON_NAME,
                .data.key = (key & 0x00FFFFF0) | 0x4, // btn 0x1, 0x2, 0x4, 0x8
                .data.bits = 24,
                .data.te = 400};
            break;
        case SetTypeNiceFlo12bit:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_NICE_FLO_NAME,
                .data.key = (key & 0x00000FF0) | 0x1, // btn 0x1, 0x2, 0x4
                .data.bits = 12,
                .data.te = 0};
            break;
        case SetTypeNiceFlo24bit:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_NICE_FLO_NAME,
                .data.key = (key & 0x00FFFFF0) | 0x4, // btn 0x1, 0x2, 0x4, 0x8
                .data.bits = 24,
                .data.te = 0};
            break;
        case SetTypeCAME12bit:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_CAME_NAME,
                .data.key = (key & 0x00000FF0) | 0x1, // btn 0x1, 0x2, 0x4
                .data.bits = 12,
                .data.te = 0};
            break;
        case SetTypeCAME24bit:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_CAME_NAME,
                .data.key = (key & 0x00FFFFF0) | 0x4, // btn 0x1, 0x2, 0x4, 0x8
                .data.bits = 24,
                .data.te = 0};
            break;
        case SetTypeCAME12bit868:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 868350000,
                .data.name = SUBGHZ_PROTOCOL_CAME_NAME,
                .data.key = (key & 0x00000FF0) | 0x1, // btn 0x1, 0x2, 0x4
                .data.bits = 12,
                .data.te = 0};
            break;
        case SetTypeCAME24bit868:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 868350000,
                .data.name = SUBGHZ_PROTOCOL_CAME_NAME,
                .data.key = (key & 0x00FFFFF0) | 0x4, // btn 0x1, 0x2, 0x4, 0x8
                .data.bits = 24,
                .data.te = 0};
            break;
        case SetTypeLinear_300_00:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 300000000,
                .data.name = SUBGHZ_PROTOCOL_LINEAR_NAME,
                .data.key = (key & 0x3FF),
                .data.bits = 10,
                .data.te = 0};
            break;
        case SetTypeBETT_433:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_BETT_NAME,
                .data.key = (key & 0x0000FFF0),
                .data.bits = 18,
                .data.te = 0};
            break;
        case SetTypeCAMETwee:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_CAME_TWEE_NAME,
                .data.key = 0x003FFF7200000000 | ((key & 0x0FFFFFF0) ^ 0xE0E0E0EE), // ????
                .data.bits = 54,
                .data.te = 0};
            break;
        case SetTypeGateTX:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_GATE_TX_NAME, // btn 0xF, 0xC, 0xA, 0x6 (?)
                .data.key = subghz_protocol_blocks_reverse_key((key & 0x00F0FF00) | 0xF0040, 24),
                .data.bits = 24,
                .data.te = 0};
            break;
        case SetTypeGangQi_433:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name =
                    SUBGHZ_PROTOCOL_GANGQI_NAME, // Add button 0xD arm and crc sum to the end
                .data.key = gangqi_key,
                .data.bits = 34,
                .data.te = 0};
            break;
        case SetTypeHollarm_433:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 433920000,
                .data.name = SUBGHZ_PROTOCOL_HOLLARM_NAME, // Add button 0x2 and crc sum to the end
                .data.key = (key & 0x000FFF0000) | 0xF0B0002200 |
                            ((((((key & 0x000FFF0000) | 0xF0B0002200) >> 32) & 0xFF) +
                              ((((key & 0x000FFF0000) | 0xF0B0002200) >> 24) & 0xFF) +
                              ((((key & 0x000FFF0000) | 0xF0B0002200) >> 16) & 0xFF) +
                              ((((key & 0x000FFF0000) | 0xF0B0002200) >> 8) & 0xFF)) &
                             0xFF),
                .data.bits = 42,
                .data.te = 0};
            break;
        case SetTypeMarantec24_868:
            gen_info = (GenInfo){
                .type = GenData,
                .mod = "AM650",
                .freq = 868350000,
                .data.name = SUBGHZ_PROTOCOL_MARANTEC24_NAME, // Add button code 0x8 to the end
                .data.key = (key & 0xFFFFF0) | 0x000008,
                .data.bits = 24,
                .data.te = 0};
            break;
        case SetTypeFaacSLH_433:
            gen_info = (GenInfo){
                .type = GenFaacSLH,
                .mod = "AM650",
                .freq = 433920000,
                .faac_slh.serial = ((key & 0x00FFFFF0) | 0xA0000006) >> 4,
                .faac_slh.btn = 0x06,
                .faac_slh.cnt = 0x02,
                .faac_slh.seed = key,
                .faac_slh.manuf = "FAAC_SLH"};
            break;
        case SetTypeFaacSLH_868:
            gen_info = (GenInfo){
                .type = GenFaacSLH,
                .mod = "AM650",
                .freq = 868350000,
                .faac_slh.serial = ((key & 0x00FFFFF0) | 0xA0000006) >> 4,
                .faac_slh.btn = 0x06,
                .faac_slh.cnt = 0x02,
                .faac_slh.seed = (key & 0x0FFFFFFF),
                .faac_slh.manuf = "FAAC_SLH"};
            break;
        case SetTypeBeninca433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x000FFF00) | 0x00800080,
                .keeloq.btn = 0x01,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "Beninca"};
            break;
        case SetTypeBeninca868:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 868350000,
                .keeloq.serial = (key & 0x000FFF00) | 0x00800080,
                .keeloq.btn = 0x01,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "Beninca"};
            break;
        case SetTypeAllmatic433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x00FFFF00) | 0x01000011,
                .keeloq.btn = 0x0C,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "Beninca"};
            break;
        case SetTypeAllmatic868:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 868350000,
                .keeloq.serial = (key & 0x00FFFF00) | 0x01000011,
                .keeloq.btn = 0x0C,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "Beninca"};
            break;
        case SetTypeCenturion433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0000FFFF),
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Centurion"};
            break;
        case SetTypeMonarch433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0000FFFF),
                .keeloq.btn = 0x0A,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Monarch"};
            break;
        case SetTypeElmesElectronic:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x00FFFFFF) | 0x02000000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Elmes_Poland"};
            break;
        case SetTypeANMotorsAT4:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x000FFFFF) | 0x04700000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x21,
                .keeloq.manuf = "AN-Motors"};
            break;
        case SetTypeAprimatic:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x000FFFFF) | 0x00600000,
                .keeloq.btn = 0x08,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Aprimatic"};
            break;
        case SetTypeGibidi433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Gibidi"};
            break;
        case SetTypeGSN:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x0FFFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "GSN"};
            break;
        case SetTypeIronLogic:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFF0,
                .keeloq.btn = 0x04,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "IronLogic"};
            break;
        case SetTypeStilmatic:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x0FFFFFFF,
                .keeloq.btn = 0x01,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Stilmatic"};
            break;
        case SetTypeSommer_FM_434:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "FM476",
                .freq = 434420000,
                .keeloq.serial = (key & 0x0000FFFF) | 0x01700000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Sommer(fsk476)"};
            break;
        case SetTypeSommer_FM_868:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "FM476",
                .freq = 868800000,
                .keeloq.serial = (key & 0x0000FFFF) | 0x01700000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Sommer(fsk476)"};
            break;
        case SetTypeSommer_FM238_434:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "FM238",
                .freq = 434420000,
                .keeloq.serial = key & 0x0000FFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Sommer(fsk476)"};
            break;
        case SetTypeSommer_FM238_868:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "FM238",
                .freq = 868800000,
                .keeloq.serial = key & 0x0000FFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Sommer(fsk476)"};
            break;
        case SetTypeDTMNeo433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x000FFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x05,
                .keeloq.manuf = "DTM_Neo"};
            break;
        case SetTypeCAMESpace:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x04,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Came_Space"};
            break;
        case SetTypeCameAtomo433:
            gen_info = (GenInfo){
                .type = GenCameAtomo,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0FFFFFFF) | 0x10000000,
                .keeloq.cnt = 0x03};
            break;
        case SetTypeCameAtomo868:
            gen_info = (GenInfo){
                .type = GenCameAtomo,
                .mod = "AM650",
                .freq = 868350000,
                .keeloq.serial = (key & 0x0FFFFFFF) | 0x10000000,
                .keeloq.cnt = 0x03};
            break;
        case SetTypeBFTMitto:
            gen_info = (GenInfo){
                .type = GenKeeloqBFT,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq_bft.serial = key & 0x000FFFFF,
                .keeloq_bft.btn = 0x02,
                .keeloq_bft.cnt = 0x02,
                .keeloq_bft.seed = key & 0x000FFFFF,
                .keeloq_bft.manuf = "BFT"};
            break;
        case SetTypeAlutechAT4N:
            gen_info = (GenInfo){
                .type = GenAlutechAt4n,
                .mod = "AM650",
                .freq = 433920000,
                .alutech_at_4n.serial = (key & 0x000FFFFF) | 0x00100000,
                .alutech_at_4n.btn = 0x44,
                .alutech_at_4n.cnt = 0x03};
            break;
        case SetTypeSomfyTelis:
            gen_info = (GenInfo){
                .type = GenSomfyTelis,
                .mod = "AM650",
                .freq = 433420000,
                .somfy_telis.serial = key & 0x00FFFFFF,
                .somfy_telis.btn = 0x02,
                .somfy_telis.cnt = 0x03};
            break;
        case SetTypeDoorHan_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x0FFFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "DoorHan"};
            break;
        case SetTypeDoorHan_315_00:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 315000000,
                .keeloq.serial = key & 0x0FFFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "DoorHan"};
            break;
        case SetTypeNiceFlorS_433_92:
            gen_info = (GenInfo){
                .type = GenNiceFlorS,
                .mod = "AM650",
                .freq = 433920000,
                .nice_flor_s.serial = key & 0x0FFFFFFF,
                .nice_flor_s.btn = 0x01,
                .nice_flor_s.cnt = 0x03,
                .nice_flor_s.nice_one = false};
            break;
        case SetTypeNiceOne_433_92:
            gen_info = (GenInfo){
                .type = GenNiceFlorS,
                .mod = "AM650",
                .freq = 433920000,
                .nice_flor_s.serial = key & 0x0FFFFFFF,
                .nice_flor_s.btn = 0x01,
                .nice_flor_s.cnt = 0x03,
                .nice_flor_s.nice_one = true};
            break;
        case SetTypeNiceSmilo_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "NICE_Smilo"};
            break;
        case SetTypeNiceMHouse_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x09,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "NICE_MHOUSE"};
            break;
        case SetTypeDeaMio433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0FFFF000) | 0x00000869,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Dea_Mio"};
            break;
        case SetTypeGeniusBravo433:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x06,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Genius_Bravo"};
            break;
        case SetTypeJCM_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x00FFFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "JCM_Tech"};
            break;
        case SetTypeNovoferm_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0000FFFF) | 0x018F0000,
                .keeloq.btn = 0x01,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Novoferm"};
            break;
        case SetTypeHormannEcoStar_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x000FFFFF) | 0x02200000,
                .keeloq.btn = 0x04,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "EcoStar"};
            break;
        case SetTypeFAACRCXT_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = (key & 0x0000FFFF) | 0x00100000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "FAAC_RC,XT"};
            break;
        case SetTypeFAACRCXT_868:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 868350000,
                .keeloq.serial = (key & 0x0000FFFF) | 0x00100000,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "FAAC_RC,XT"};
            break;
        case SetTypeNormstahl_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x0000FFFF,
                .keeloq.btn = 0x04,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "Normstahl"};
            break;
        case SetTypeHCS101_433_92:
            gen_info = (GenInfo){
                .type = GenKeeloq,
                .mod = "AM650",
                .freq = 433920000,
                .keeloq.serial = key & 0x000FFFFF,
                .keeloq.btn = 0x02,
                .keeloq.cnt = 0x03,
                .keeloq.manuf = "HCS101"};
            break;
        case SetTypeSecPlus_v1_315_00:
            gen_info = (GenInfo){.type = GenSecPlus1, .mod = "AM650", .freq = 315000000};
            break;
        case SetTypeSecPlus_v1_390_00:
            gen_info = (GenInfo){.type = GenSecPlus1, .mod = "AM650", .freq = 390000000};
            break;
        case SetTypeSecPlus_v1_433_00:
            gen_info = (GenInfo){.type = GenSecPlus1, .mod = "AM650", .freq = 433920000};
            break;
        case SetTypeSecPlus_v2_310_00:
            gen_info = (GenInfo){
                .type = GenSecPlus2,
                .mod = "AM650",
                .freq = 310000000,
                .sec_plus_2.serial = (key & 0x7FFFF3FC), // 850LM pairing
                .sec_plus_2.btn = 0x68,
                .sec_plus_2.cnt = 0xE500000};
            break;
        case SetTypeSecPlus_v2_315_00:
            gen_info = (GenInfo){
                .type = GenSecPlus2,
                .mod = "AM650",
                .freq = 315000000,
                .sec_plus_2.serial = (key & 0x7FFFF3FC), // 850LM pairing
                .sec_plus_2.btn = 0x68,
                .sec_plus_2.cnt = 0xE500000};
            break;
        case SetTypeSecPlus_v2_390_00:
            gen_info = (GenInfo){
                .type = GenSecPlus2,
                .mod = "AM650",
                .freq = 390000000,
                .sec_plus_2.serial = (key & 0x7FFFF3FC), // 850LM pairing
                .sec_plus_2.btn = 0x68,
                .sec_plus_2.cnt = 0xE500000};
            break;
        case SetTypeSecPlus_v2_433_00:
            gen_info = (GenInfo){
                .type = GenSecPlus2,
                .mod = "AM650",
                .freq = 433920000,
                .sec_plus_2.serial = (key & 0x7FFFF3FC), // 850LM pairing
                .sec_plus_2.btn = 0x68,
                .sec_plus_2.cnt = 0xE500000};
            break;
        default:
            furi_crash("Not implemented");
            break;
        }

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
        default:
            furi_crash("Not implemented");
            break;
        }

        if(generated_protocol) {
            subghz_file_name_clear(subghz);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
        } else {
            furi_string_set(
                subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
        }
    }

    return generated_protocol;
}

void subghz_scene_set_type_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
