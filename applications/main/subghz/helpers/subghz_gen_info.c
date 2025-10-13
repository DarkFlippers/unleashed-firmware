#include "subghz_gen_info.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/blocks/math.h>

void subghz_gen_info_reset(GenInfo* gen_info) {
    furi_assert(gen_info);
    memset(gen_info, 0, sizeof(GenInfo));
}

void subghz_scene_set_type_fill_generation_infos(GenInfo* infos_dest, SetType type) {
    GenInfo gen_info = {0};
    uint64_t key = (uint64_t)rand();

    uint64_t gangqi_key;
    subghz_txrx_gen_serial_gangqi(&gangqi_key);

    uint64_t marantec_key;
    subghz_txrx_gen_key_marantec(&marantec_key);

    switch(type) {
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
    case SetTypeZKTeco430:
        gen_info = (GenInfo){
            .type = GenData,
            .mod = "AM650",
            .freq = 430500000,
            .data.name = SUBGHZ_PROTOCOL_PRINCETON_NAME,
            .data.key = (key & 0x00FFFF00) | 0x30, // btn 0x30(UP), 0x03(STOP), 0x0C(DOWN)
            .data.bits = 24,
            .data.te = 357};
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
    case SetTypeRoger_433:
        gen_info = (GenInfo){
            .type = GenData,
            .mod = "AM650",
            .freq = 433920000,
            .data.name = SUBGHZ_PROTOCOL_ROGER_NAME,
            .data.key = (key & 0xFFFF000) | 0x0000101, // button code 0x1 and (crc?) is 0x01
            .data.bits = 28,
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
            .data.name = SUBGHZ_PROTOCOL_GANGQI_NAME, // Add button 0xD arm and crc sum to the end
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
    case SetTypeReversRB2_433:
        gen_info = (GenInfo){
            .type = GenData,
            .mod = "AM650",
            .freq = 433920000,
            .data.name = SUBGHZ_PROTOCOL_REVERSRB2_NAME, // 64bits no buttons
            .data.key = (key & 0x00000FFFFFFFF000) | 0xFFFFF00000000000 | 0x0000000000000A00,
            .data.bits = 64,
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
    case SetTypeMarantec_433:
        gen_info = (GenInfo){
            .type = GenData,
            .mod = "AM650",
            .freq = 433920000,
            .data.name =
                SUBGHZ_PROTOCOL_MARANTEC_NAME, // Button code is 0x4 and crc sum to the end
            .data.key = marantec_key,
            .data.bits = 49,
            .data.te = 0};
        break;
    case SetTypeMarantec_868:
        gen_info = (GenInfo){
            .type = GenData,
            .mod = "AM650",
            .freq = 868350000,
            .data.name =
                SUBGHZ_PROTOCOL_MARANTEC_NAME, // Button code is 0x4 and crc sum to the end
            .data.key = marantec_key,
            .data.bits = 49,
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
            .faac_slh.seed = (uint32_t)key,
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
            .faac_slh.seed = (uint32_t)key,
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
    case SetTypeComunello433:
        gen_info = (GenInfo){
            .type = GenKeeloq,
            .mod = "AM650",
            .freq = 433920000,
            .keeloq.serial = key & 0x00FFFFFF,
            .keeloq.btn = 0x08,
            .keeloq.cnt = 0x05,
            .keeloq.manuf = "Comunello"};
        break;
    case SetTypeComunello868:
        gen_info = (GenInfo){
            .type = GenKeeloq,
            .mod = "AM650",
            .freq = 868460000,
            .keeloq.serial = key & 0x00FFFFFF,
            .keeloq.btn = 0x08,
            .keeloq.cnt = 0x05,
            .keeloq.manuf = "Comunello"};
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
    case SetTypeJollyMotors433:
        gen_info = (GenInfo){
            .type = GenKeeloq,
            .mod = "AM650",
            .freq = 433920000,
            .keeloq.serial = (key & 0x000FFFFF),
            .keeloq.btn = 0x02,
            .keeloq.cnt = 0x03,
            .keeloq.manuf = "Jolly_Motors"};
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
            .came_atomo.serial = (key & 0x0FFFFFFF) | 0x10000000,
            .came_atomo.cnt = 0x03};
        break;
    case SetTypeCameAtomo868:
        gen_info = (GenInfo){
            .type = GenCameAtomo,
            .mod = "AM650",
            .freq = 868350000,
            .came_atomo.serial = (key & 0x0FFFFFFF) | 0x10000000,
            .came_atomo.cnt = 0x03};
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
    case SetTypeMotorline433:
        gen_info = (GenInfo){
            .type = GenKeeloq,
            .mod = "AM650",
            .freq = 433920000,
            .keeloq.serial = key & 0x0FFFFFFF,
            .keeloq.btn = 0x01,
            .keeloq.cnt = 0x03,
            .keeloq.manuf = "Motorline"};
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
    case SetTypePhoenix_V2_433:
        gen_info = (GenInfo){
            .type = GenPhoenixV2,
            .mod = "AM650",
            .freq = 433920000,
            .phoenix_v2.serial = (key & 0x0FFFFFFF) | 0xB0000000,
            .phoenix_v2.cnt = 0x025D};
        break;
    default:
        furi_crash("Not implemented");
        break;
    }
    *infos_dest = gen_info;
}
