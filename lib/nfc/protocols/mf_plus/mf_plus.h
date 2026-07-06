#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_PLUS_UID_SIZE_MAX (7)
#define MF_PLUS_BATCH_SIZE   (5)

#define MF_PLUS_CMD_GET_VERSION (0x60)

typedef enum {
    MfPlusErrorNone,
    MfPlusErrorUnknown,
    MfPlusErrorNotPresent,
    MfPlusErrorProtocol,
    MfPlusErrorAuth,
    MfPlusErrorPartialRead,
    MfPlusErrorTimeout,
    // The card answered but refused the command with a non-OK status byte (e.g. 0x0B "command not
    // available at current state"). Distinct from MfPlusErrorProtocol (a comms/RF fault): a
    // rejection is deterministic and driven by card state, so callers may retry in another mode.
    MfPlusErrorRejected,
} MfPlusError;

typedef enum {
    MfPlusTypePlus,
    MfPlusTypeEV1,
    MfPlusTypeEV2,
    MfPlusTypeS,
    MfPlusTypeSE,
    MfPlusTypeX,

    MfPlusTypeUnknown,
    MfPlusTypeNum,
} MfPlusType;

typedef enum {
    MfPlusSize1K,
    MfPlusSize2K,
    MfPlusSize4K,

    MfPlusSizeUnknown,
    MfPlusSizeNum,
} MfPlusSize;

typedef enum {
    MfPlusSecurityLevel0,
    MfPlusSecurityLevel1,
    MfPlusSecurityLevel2,
    MfPlusSecurityLevel3,

    MfPlusSecurityLevelUnknown,
    MfPlusSecurityLevelNum,
} MfPlusSecurityLevel;

typedef struct {
    uint8_t hw_vendor;
    uint8_t hw_type;
    uint8_t hw_subtype;
    uint8_t hw_major;
    uint8_t hw_minor;
    uint8_t hw_storage;
    uint8_t hw_proto;

    uint8_t sw_vendor;
    uint8_t sw_type;
    uint8_t sw_subtype;
    uint8_t sw_major;
    uint8_t sw_minor;
    uint8_t sw_storage;
    uint8_t sw_proto;

    uint8_t uid[MF_PLUS_UID_SIZE_MAX];
    uint8_t batch[MF_PLUS_BATCH_SIZE];
    uint8_t prod_week;
    uint8_t prod_year;
} MfPlusVersion;

#define MF_PLUS_BLOCK_SIZE     (16)
#define MF_PLUS_KEY_SIZE       (16) // AES-128
#define MF_PLUS_SIGNATURE_SIZE (56) // EV1/EV2 originality signature (Read_Sig, 0x3C)

// 4K is the largest layout: 40 sectors, 256 blocks. Smaller cards use a prefix of these.
#define MF_PLUS_MAX_SECTORS          (40)
#define MF_PLUS_MAX_BLOCKS           (256)
#define MF_PLUS_BLOCK_READ_MASK_SIZE (MF_PLUS_MAX_BLOCKS / 32) // 256-bit "block was read" bitmap
#define MF_PLUS_CONFIG_BLOCK_NUM     (4) // 0xB000..0xB003

typedef struct {
    uint8_t data[MF_PLUS_BLOCK_SIZE];
} MfPlusBlock;

typedef struct {
    uint8_t data[MF_PLUS_KEY_SIZE];
} MfPlusKey;

typedef enum {
    MfPlusKeyTypeA = 0,
    MfPlusKeyTypeB = 1,
} MfPlusKeyType;

// Admin keys in the 0x90xx keyspace. Like sector keys, they are recovered by a successful
// AES authentication (dictionary attack), never read out.
typedef enum {
    MfPlusAdminKeyCardMaster, // 0x9000
    MfPlusAdminKeyCardConfig, // 0x9001
    MfPlusAdminKeyL3Switch, // 0x9003 (L3 switch; 0x9002 is the separate SL2->SL3 key)
    MfPlusAdminKeySL1CardAuth, // 0x9004

    MfPlusAdminKeyNum,
} MfPlusAdminKeyType;

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    MfPlusVersion version;
    MfPlusType type;
    MfPlusSize size;
    MfPlusSecurityLevel security_level;
    FuriString* device_name;

    // Originality signature (Read_Sig / 0x3C). Level-independent: EV1/EV2 expose it at any
    // security level; EV0 parts have none (signature_present stays false).
    bool signature_present;
    uint8_t signature[MF_PLUS_SIGNATURE_SIZE];

    // The blocks, keys and config below are SL3-only content: populated when
    // security_level == SL3, zeroed otherwise. In the saved file they sit behind a "Data format
    // version" key; legacy metadata-only dumps lack it and load as identity-only.
    MfPlusBlock block[MF_PLUS_MAX_BLOCKS];
    uint32_t block_read_mask[MF_PLUS_BLOCK_READ_MASK_SIZE]; // per-block "was read" bitmap

    // Sector AES keys, recovered by dictionary auth. Stored separately from block data
    // (unlike Classic, where keys live in the sector trailer) because SL3 keys are in the
    // 0x40xx keyspace, not data memory.
    MfPlusKey key_a[MF_PLUS_MAX_SECTORS];
    MfPlusKey key_b[MF_PLUS_MAX_SECTORS];
    uint64_t key_a_mask; // per-sector "key A recovered" bitmap
    uint64_t key_b_mask;

    // Admin keys (0x90xx) and the readable config blocks (0xB0xx).
    MfPlusKey admin_key[MfPlusAdminKeyNum];
    uint8_t admin_key_mask; // one bit per MfPlusAdminKeyType
    MfPlusBlock config_block[MF_PLUS_CONFIG_BLOCK_NUM];
    uint8_t config_read_mask; // one bit per config block
} MfPlusData;

extern const NfcDeviceBase nfc_device_mf_plus;

// SL3 sector count for the card size (0 if unknown). Public because the app (an external FAP) sizes
// its dictionary-attack progress view from it, mirroring mf_classic_get_total_sectors_num.
uint8_t mf_plus_get_sector_count(MfPlusSize size);

// True once every data block of the card has been captured (full read vs partial recovery). Public
// for the same reason (the dictionary-attack scene grades its outcome with it).
bool mf_plus_is_card_read(const MfPlusData* data);

// Count fully-read sectors (all their blocks captured) and recovered sector keys, for the
// read-progress display. Mirrors mf_classic_get_read_sectors_and_keys.
void mf_plus_get_read_sectors_and_keys(
    const MfPlusData* data,
    uint8_t* sectors_read,
    uint8_t* keys_found);

// SL3 block count for the card size (0 if unknown).
uint16_t mf_plus_get_block_count(MfPlusSize size);

// Known-tracking READ accessors, public so the app (an external FAP) can render recovered SL3
// content; the matching set_* variants stay internal to the poller/serializer. Mirrors the
// mf_classic accessor surface.
bool mf_plus_is_block_read(const MfPlusData* data, uint16_t block_num);

bool mf_plus_is_key_found(const MfPlusData* data, uint8_t sector, MfPlusKeyType key_type);

bool mf_plus_is_admin_key_found(const MfPlusData* data, MfPlusAdminKeyType type);

bool mf_plus_is_config_block_read(const MfPlusData* data, uint8_t index);

// Human-readable name for an admin key type, e.g. "Card Master Key".
const char* mf_plus_get_admin_key_name(MfPlusAdminKeyType type);

MfPlusData* mf_plus_alloc(void);

void mf_plus_free(MfPlusData* data);

void mf_plus_reset(MfPlusData* data);

void mf_plus_copy(MfPlusData* data, const MfPlusData* other);

bool mf_plus_verify(MfPlusData* data, const FuriString* device_type);

bool mf_plus_load(MfPlusData* data, FlipperFormat* ff, uint32_t version);

bool mf_plus_save(const MfPlusData* data, FlipperFormat* ff);

bool mf_plus_is_equal(const MfPlusData* data, const MfPlusData* other);

const char* mf_plus_get_device_name(const MfPlusData* data, NfcDeviceNameType name_type);

const uint8_t* mf_plus_get_uid(const MfPlusData* data, size_t* uid_len);

bool mf_plus_set_uid(MfPlusData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* mf_plus_get_base_data(const MfPlusData* data);

#ifdef __cplusplus
}
#endif
