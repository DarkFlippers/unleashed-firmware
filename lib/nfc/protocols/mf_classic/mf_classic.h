#pragma once

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_CLASSIC_CMD_AUTH_KEY_A          (0x60U)
#define MF_CLASSIC_CMD_AUTH_KEY_B          (0x61U)
#define MF_CLASSIC_CMD_BACKDOOR_AUTH_KEY_A (0x64U)
#define MF_CLASSIC_CMD_BACKDOOR_AUTH_KEY_B (0x65U)
#define MF_CLASSIC_CMD_READ_BLOCK          (0x30U)
#define MF_CLASSIC_CMD_WRITE_BLOCK         (0xA0U)
#define MF_CLASSIC_CMD_VALUE_DEC           (0xC0U)
#define MF_CLASSIC_CMD_VALUE_INC           (0xC1U)
#define MF_CLASSIC_CMD_VALUE_RESTORE       (0xC2U)
#define MF_CLASSIC_CMD_VALUE_TRANSFER      (0xB0U)

#define MF_CLASSIC_CMD_HALT_MSB                (0x50)
#define MF_CLASSIC_CMD_HALT_LSB                (0x00)
#define MF_CLASSIC_CMD_ACK                     (0x0A)
#define MF_CLASSIC_CMD_NACK                    (0x00)
#define MF_CLASSIC_CMD_NACK_TRANSFER_INVALID   (0x04)
#define MF_CLASSIC_CMD_NACK_TRANSFER_CRC_ERROR (0x01)

#define MF_CLASSIC_TOTAL_SECTORS_MAX (40)
#define MF_CLASSIC_TOTAL_BLOCKS_MAX  (256)
#define MF_CLASSIC_READ_MASK_SIZE    (MF_CLASSIC_TOTAL_BLOCKS_MAX / 32)
#define MF_CLASSIC_BLOCK_SIZE        (16)
#define MF_CLASSIC_KEY_SIZE          (6)
#define MF_CLASSIC_ACCESS_BYTES_SIZE (4)

#define MF_CLASSIC_NT_SIZE (4)
#define MF_CLASSIC_NR_SIZE (4)
#define MF_CLASSIC_AR_SIZE (4)
#define MF_CLASSIC_AT_SIZE (4)

typedef enum {
    MfClassicErrorNone,
    MfClassicErrorNotPresent,
    MfClassicErrorProtocol,
    MfClassicErrorAuth,
    MfClassicErrorPartialRead,
    MfClassicErrorTimeout,
} MfClassicError;

typedef enum {
    MfClassicTypeMini,
    MfClassicType1k,
    MfClassicType4k,

    MfClassicTypeNum,
} MfClassicType;

typedef enum {
    MfClassicActionDataRead,
    MfClassicActionDataWrite,
    MfClassicActionDataInc,
    MfClassicActionDataDec,

    MfClassicActionKeyARead,
    MfClassicActionKeyAWrite,
    MfClassicActionKeyBRead,
    MfClassicActionKeyBWrite,
    MfClassicActionACRead,
    MfClassicActionACWrite,
} MfClassicAction;

typedef enum {
    MfClassicValueCommandIncrement,
    MfClassicValueCommandDecrement,
    MfClassicValueCommandRestore,

    MfClassicValueCommandInvalid,
} MfClassicValueCommand;

typedef struct {
    uint8_t data[MF_CLASSIC_BLOCK_SIZE];
} MfClassicBlock;

typedef enum {
    MfClassicKeyTypeA,
    MfClassicKeyTypeB,
} MfClassicKeyType;

typedef struct {
    uint8_t data[MF_CLASSIC_KEY_SIZE];
} MfClassicKey;

typedef struct {
    uint8_t data[MF_CLASSIC_ACCESS_BYTES_SIZE];
} MfClassicAccessBits;

typedef struct {
    uint8_t data[MF_CLASSIC_NT_SIZE];
} MfClassicNt;

typedef struct {
    uint8_t data[MF_CLASSIC_AT_SIZE];
} MfClassicAt;

typedef struct {
    uint8_t data[MF_CLASSIC_NR_SIZE];
} MfClassicNr;

typedef struct {
    uint8_t data[MF_CLASSIC_AR_SIZE];
} MfClassicAr;

typedef struct {
    uint8_t block_num;
    MfClassicKey key;
    MfClassicKeyType key_type;
    MfClassicNt nt;
    MfClassicNr nr;
    MfClassicAr ar;
    MfClassicAt at;
} MfClassicAuthContext;

typedef union {
    MfClassicBlock block;
    struct {
        MfClassicKey key_a;
        MfClassicAccessBits access_bits;
        MfClassicKey key_b;
    };
} MfClassicSectorTrailer;

typedef struct {
    uint64_t key_a_mask;
    MfClassicKey key_a[MF_CLASSIC_TOTAL_SECTORS_MAX];
    uint64_t key_b_mask;
    MfClassicKey key_b[MF_CLASSIC_TOTAL_SECTORS_MAX];
} MfClassicDeviceKeys;

typedef struct {
    Iso14443_3aData* iso14443_3a_data;
    MfClassicType type;
    uint32_t block_read_mask[MF_CLASSIC_READ_MASK_SIZE];
    uint64_t key_a_mask;
    uint64_t key_b_mask;
    MfClassicBlock block[MF_CLASSIC_TOTAL_BLOCKS_MAX];
} MfClassicData;

extern const NfcDeviceBase nfc_device_mf_classic;

MfClassicData* mf_classic_alloc(void);

void mf_classic_free(MfClassicData* data);

void mf_classic_reset(MfClassicData* data);

void mf_classic_copy(MfClassicData* data, const MfClassicData* other);

bool mf_classic_verify(MfClassicData* data, const FuriString* device_type);

bool mf_classic_load(MfClassicData* data, FlipperFormat* ff, uint32_t version);

bool mf_classic_save(const MfClassicData* data, FlipperFormat* ff);

bool mf_classic_is_equal(const MfClassicData* data, const MfClassicData* other);

const char* mf_classic_get_device_name(const MfClassicData* data, NfcDeviceNameType name_type);

const uint8_t* mf_classic_get_uid(const MfClassicData* data, size_t* uid_len);

bool mf_classic_set_uid(MfClassicData* data, const uint8_t* uid, size_t uid_len);

Iso14443_3aData* mf_classic_get_base_data(const MfClassicData* data);

uint8_t mf_classic_get_total_sectors_num(MfClassicType type);

uint16_t mf_classic_get_total_block_num(MfClassicType type);

uint8_t mf_classic_get_first_block_num_of_sector(uint8_t sector);

uint8_t mf_classic_get_blocks_num_in_sector(uint8_t sector);

uint8_t mf_classic_get_sector_trailer_num_by_sector(uint8_t sector);

uint8_t mf_classic_get_sector_trailer_num_by_block(uint8_t block);

MfClassicSectorTrailer*
    mf_classic_get_sector_trailer_by_sector(const MfClassicData* data, uint8_t sector_num);

bool mf_classic_is_sector_trailer(uint8_t block);

void mf_classic_set_sector_trailer_read(
    MfClassicData* data,
    uint8_t block_num,
    MfClassicSectorTrailer* sec_tr);

uint8_t mf_classic_get_sector_by_block(uint8_t block);

bool mf_classic_block_to_value(const MfClassicBlock* block, int32_t* value, uint8_t* addr);

void mf_classic_value_to_block(int32_t value, uint8_t addr, MfClassicBlock* block);

bool mf_classic_is_key_found(
    const MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type);

void mf_classic_set_key_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type,
    uint64_t key);

void mf_classic_set_key_not_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type);

MfClassicKey
    mf_classic_get_key(const MfClassicData* data, uint8_t sector_num, MfClassicKeyType key_type);

bool mf_classic_is_block_read(const MfClassicData* data, uint8_t block_num);

void mf_classic_set_block_read(MfClassicData* data, uint8_t block_num, MfClassicBlock* block_data);

bool mf_classic_is_sector_read(const MfClassicData* data, uint8_t sector_num);

void mf_classic_get_read_sectors_and_keys(
    const MfClassicData* data,
    uint8_t* sectors_read,
    uint8_t* keys_found);

bool mf_classic_is_card_read(const MfClassicData* data);

bool mf_classic_is_value_block(MfClassicSectorTrailer* sec_tr, uint8_t block_num);

bool mf_classic_is_allowed_access_data_block(
    MfClassicSectorTrailer* sec_tr,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicAction action);

bool mf_classic_is_allowed_access(
    MfClassicData* data,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicAction action);

#ifdef __cplusplus
}
#endif
