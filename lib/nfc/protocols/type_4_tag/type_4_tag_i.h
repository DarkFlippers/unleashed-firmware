#pragma once

#include "type_4_tag.h"

// ISO SELECT FILE command and parameters
#define TYPE_4_TAG_ISO_SELECT_CMD         0x00, 0xA4
#define TYPE_4_TAG_ISO_SELECT_P1_BY_NAME  (0x04)
#define TYPE_4_TAG_ISO_SELECT_P1_BY_EF_ID (0x02)
#define TYPE_4_TAG_ISO_SELECT_P1_BY_ID    (0x00)
#define TYPE_4_TAG_ISO_SELECT_P2_EMPTY    (0x0C)
#define TYPE_4_TAG_ISO_SELECT_LE_EMPTY    (0x00)

// ISO READ BINARY command and parameters
#define TYPE_4_TAG_ISO_READ_CMD          0x00, 0xB0
#define TYPE_4_TAG_ISO_READ_P1_ID_MASK   (1 << 7)
#define TYPE_4_TAG_ISO_READ_P_BEGINNING  0x00, 0x00
#define TYPE_4_TAG_ISO_READ_P_OFFSET_MAX (32767U)
#define TYPE_4_TAG_ISO_READ_LE_FULL      (0x00)

// ISO UPDATE BINARY command and parameters
#define TYPE_4_TAG_ISO_WRITE_CMD         0x00, 0xD6
#define TYPE_4_TAG_ISO_WRITE_P1_ID_MASK  (1 << 7)
#define TYPE_4_TAG_ISO_WRITE_P_BEGINNING 0x00, 0x00
#define TYPE_4_TAG_ISO_WRITE_LE_EMPTY    (0x00)

// Common APDU parameters and values
#define TYPE_4_TAG_ISO_STATUS_LEN        (2U)
#define TYPE_4_TAG_ISO_STATUS_SUCCESS    0x90, 0x00
#define TYPE_4_TAG_ISO_STATUS_OFFSET_ERR 0x6B, 0x00
#define TYPE_4_TAG_ISO_STATUS_NOT_FOUND  0x6A, 0x82
#define TYPE_4_TAG_ISO_STATUS_NO_SUPPORT 0x6A, 0x81
#define TYPE_4_TAG_ISO_STATUS_BAD_PARAMS 0x6A, 0x86
#define TYPE_4_TAG_ISO_STATUS_NO_CMD     0x68, 0x00
#define TYPE_4_TAG_ISO_RW_CHUNK_LEN      (255U)

// Common IDs and Names, note:
// MF = Master File (PICC/Card Level)
// DF = Dedicated File (Application)
// EF = Elementary File (File)
#define TYPE_4_TAG_ISO_NAME_LEN   (7U)
#define TYPE_4_TAG_ISO_MF_NAME    0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00
#define TYPE_4_TAG_ISO_DF_NAME    0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01
#define TYPE_4_TAG_ISO_ID_LEN     (2U)
#define TYPE_4_TAG_ISO_MF_ID      (0x3F00)
#define TYPE_4_TAG_ISO_DF_ID      (0xE110)
#define TYPE_4_TAG_T4T_CC_EF_ID   (0xE103)
#define TYPE_4_TAG_T4T_NDEF_EF_ID (0xE104)

// Capability Container parsing parameters
#define TYPE_4_TAG_T4T_CC_VNO          (0x20)
#define TYPE_4_TAG_T4T_CC_RW_LOCK_NONE (0x00)
#define TYPE_4_TAG_T4T_CC_MIN_SIZE     (sizeof(Type4TagCc) + sizeof(Type4TagCcTlv))

// Implementation-specific sizes and defaults
// 4a layer adds 1..3 byte prefix, 3a layer adds 2 byte suffix and has 256 byte buffer
#define TYPE_4_TAG_BUF_SIZE          (256U - 3U - 2U)
// Read returns 2 byte status trailer, write sends 5 byte command header
#define TYPE_4_TAG_CHUNK_LEN         MIN(TYPE_4_TAG_BUF_SIZE - 5U, TYPE_4_TAG_ISO_RW_CHUNK_LEN)
#define TYPE_4_TAG_DEFAULT_NDEF_SIZE TYPE_4_TAG_MF_DESFIRE_NDEF_SIZE

extern const uint8_t type_4_tag_iso_mf_name[TYPE_4_TAG_ISO_NAME_LEN];
extern const uint8_t type_4_tag_iso_df_name[TYPE_4_TAG_ISO_NAME_LEN];

// Capability Container parsing structures

typedef enum FURI_PACKED {
    Type4TagCcTlvTypeNdefFileCtrl = 0x04,
    Type4TagCcTlvTypeProprietaryFileCtrl = 0x05,
} Type4TagCcTlvType;

typedef struct FURI_PACKED {
    uint16_t file_id;
    uint16_t max_len;
    uint8_t read_perm;
    uint8_t write_perm;
} Type4TagCcTlvNdefFileCtrl;

typedef union FURI_PACKED {
    Type4TagCcTlvNdefFileCtrl ndef_file_ctrl;
} Type4TagCcTlvValue;

typedef struct FURI_PACKED {
    Type4TagCcTlvType type;
    uint8_t len;
    Type4TagCcTlvValue value;
} Type4TagCcTlv;

typedef struct FURI_PACKED {
    uint16_t len;
    uint8_t t4t_vno;
    uint16_t mle;
    uint16_t mlc;
    Type4TagCcTlv tlv[];
} Type4TagCc;

// Internal helpers

Type4TagError type_4_tag_process_error(Iso14443_4aError error);

void type_4_tag_cc_dump(const Type4TagData* data, uint8_t* buf, size_t len);

Type4TagError type_4_tag_cc_parse(Type4TagData* data, const uint8_t* buf, size_t len);

// Load internal Type4Tag structures

bool type_4_tag_ndef_data_load(Type4TagData* data, FlipperFormat* ff);

// Save internal Type4Tag structures

bool type_4_tag_ndef_data_save(const Type4TagData* data, FlipperFormat* ff);
