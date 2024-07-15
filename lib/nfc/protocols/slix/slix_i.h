#pragma once

#include "slix.h"

#include <nfc/protocols/iso15693_3/iso15693_3_i.h>
#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLIX_NXP_MANUFACTURER_CODE (0x04U)

#define SLIX_LOCK_BITS_AFI   (1U << 0)
#define SLIX_LOCK_BITS_EAS   (1U << 1)
#define SLIX_LOCK_BITS_DSFID (1U << 2)
#define SLIX_LOCK_BITS_PPL   (1U << 3)

#define SLIX_CMD_CUSTOM_START                   (0xA2U)
#define SLIX_CMD_SET_EAS                        (0xA2U)
#define SLIX_CMD_RESET_EAS                      (0xA3U)
#define SLIX_CMD_LOCK_EAS                       (0xA4U)
#define SLIX_CMD_EAS_ALARM                      (0xA5U)
#define SLIX_CMD_PASSWORD_PROTECT_EAS_AFI       (0xA6U)
#define SLIX_CMD_WRITE_EAS_ID                   (0xA7U)
#define SLIX_CMD_GET_NXP_SYSTEM_INFORMATION     (0xABU)
#define SLIX_CMD_INVENTORY_PAGE_READ            (0xB0U)
#define SLIX_CMD_INVENTORY_PAGE_READ_FAST       (0xB1U)
#define SLIX_CMD_GET_RANDOM_NUMBER              (0xB2U)
#define SLIX_CMD_SET_PASSWORD                   (0xB3U)
#define SLIX_CMD_WRITE_PASSWORD                 (0xB4U)
#define SLIX_CMD_64_BIT_PASSWORD_PROTECTION     (0xB5U)
#define SLIX_CMD_PROTECT_PAGE                   (0xB6U)
#define SLIX_CMD_LOCK_PAGE_PROTECTION_CONDITION (0xB7U)
#define SLIX_CMD_DESTROY                        (0xB9U)
#define SLIX_CMD_ENABLE_PRIVACY                 (0xBAU)
#define SLIX_CMD_STAY_QUIET_PERSISTENT          (0xBCU)
#define SLIX_CMD_READ_SIGNATURE                 (0xBDU)
#define SLIX_CMD_CUSTOM_END                     (0xBEU)
#define SLIX_CMD_CUSTOM_COUNT                   (SLIX_CMD_CUSTOM_END - SLIX_CMD_CUSTOM_START)

#define SLIX_TYPE_FEATURES_SLIX (SLIX_TYPE_FEATURE_EAS)
#define SLIX_TYPE_FEATURES_SLIX_S                                                   \
    (SLIX_TYPE_FEATURE_READ | SLIX_TYPE_FEATURE_WRITE | SLIX_TYPE_FEATURE_PRIVACY | \
     SLIX_TYPE_FEATURE_DESTROY | SLIX_TYPE_FEATURE_EAS)
#define SLIX_TYPE_FEATURES_SLIX_L \
    (SLIX_TYPE_FEATURE_PRIVACY | SLIX_TYPE_FEATURE_DESTROY | SLIX_TYPE_FEATURE_EAS)
#define SLIX_TYPE_FEATURES_SLIX2                                                       \
    (SLIX_TYPE_FEATURE_READ | SLIX_TYPE_FEATURE_WRITE | SLIX_TYPE_FEATURE_PRIVACY |    \
     SLIX_TYPE_FEATURE_DESTROY | SLIX_TYPE_FEATURE_EAS | SLIX_TYPE_FEATURE_SIGNATURE | \
     SLIX_TYPE_FEATURE_PROTECTION | SLIX_TYPE_FEATURE_NFC_SYSTEM_INFO)

#define SLIX2_FEATURE_FLAGS                                                                       \
    (SLIX_FEATURE_FLAG_UM_PP | SLIX_FEATURE_FLAG_COUNTER | SLIX_FEATURE_FLAG_EAS_ID |             \
     SLIX_FEATURE_FLAG_EAS_PP | SLIX_FEATURE_FLAG_AFI_PP | SLIX_FEATURE_FLAG_INVENTORY_READ_EXT | \
     SLIX_FEATURE_FLAG_EAS_IR | SLIX_FEATURE_FLAG_ORIGINALITY_SIG |                               \
     SLIX_FEATURE_FLAG_PERSISTENT_QUIET | SLIX_FEATURE_FLAG_PRIVACY | SLIX_FEATURE_FLAG_DESTROY)

typedef union {
    struct {
        uint16_t value;
        uint8_t reserved;
        uint8_t protection;
    };
    uint8_t bytes[SLIX_BLOCK_SIZE];
} SlixCounter;

// Same behaviour as iso15693_3_error_response_parse
bool slix_error_response_parse(SlixError* error, const BitBuffer* buf);

SlixError slix_process_iso15693_3_error(Iso15693_3Error iso15693_3_error);

SlixError slix_get_nxp_system_info_response_parse(SlixData* data, const BitBuffer* buf);

SlixError slix_read_signature_response_parse(SlixSignature data, const BitBuffer* buf);

SlixError slix_get_random_number_response_parse(SlixRandomNumber* data, const BitBuffer* buf);

// Setters
void slix_set_password(SlixData* data, SlixPasswordType password_type, SlixPassword password);

void slix_set_privacy_mode(SlixData* data, bool set);

void slix_increment_counter(SlixData* data);

#ifdef __cplusplus
}
#endif
