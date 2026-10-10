#pragma once

#include <nfc/protocols/nfc_device_base_i.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Texkom (Техком) keys are 13.56 MHz tags that implement none of the ISO standards: there is no
 * anticollision and no command set. While in a field the tag load-modulates a short impulse over
 * and over, and the payload sits in the gaps between impulses.
 *
 * Every variant carries the same 8-byte frame:
 *
 *     FF FF <type> <uid 4 bytes> <crc>
 *
 * MMBIT is the exception: its type byte is FF and so is the byte after it, leaving 3 bytes of UID.
 */

#define TEXKOM_FRAME_SIZE (8U)
#define TEXKOM_UID_SIZE   (4U)

/** MMBIT spends one byte of the UID field on a second FF marker. */
#define TEXKOM_MMBIT_UID_SIZE (3U)

/** Offset of the UID inside the frame, per variant. */
#define TEXKOM_UID_OFFSET       (3U)
#define TEXKOM_MMBIT_UID_OFFSET (4U)

typedef enum {
    /** No frame yet. A decode never produces this, and no saved card holds it. */
    TexkomTypeUnknown,
    TexkomTypeTk13,
    TexkomTypeTk15,
    TexkomTypeTk17,
    TexkomTypeMmbit,

    TexkomTypeNum,
} TexkomType;

typedef struct {
    uint8_t frame[TEXKOM_FRAME_SIZE];
    TexkomType type;
} TexkomData;

extern const NfcDeviceBase nfc_device_texkom;

TexkomData* texkom_alloc(void);

void texkom_free(TexkomData* data);

void texkom_reset(TexkomData* data);

void texkom_copy(TexkomData* data, const TexkomData* other);

bool texkom_load(TexkomData* data, FlipperFormat* ff, uint32_t version);

bool texkom_save(const TexkomData* data, FlipperFormat* ff);

bool texkom_is_equal(const TexkomData* data, const TexkomData* other);

const char* texkom_get_device_name(const TexkomData* data, NfcDeviceNameType name_type);

const uint8_t* texkom_get_uid(const TexkomData* data, size_t* uid_len);

bool texkom_set_uid(TexkomData* data, const uint8_t* uid, size_t uid_len);

TexkomData* texkom_get_base_data(const TexkomData* data);

/**
 * @brief Does the frame's stored checksum match the one over its UID?
 *
 * The four variants use three different checksums between them, and the checksum travels in the
 * frame, so a wrong one is something a reader can see.
 *
 * @param[in] data pointer to the instance to be queried.
 * @returns true if the checksum is correct, false otherwise.
 */
bool texkom_is_crc_valid(const TexkomData* data);

/**
 * @brief What the frame's last byte should hold for the checksum to be correct.
 *
 * The whole byte, not just the checksum field: MMBIT keeps payload in the low nibble, so the
 * value returned here is directly comparable with the byte that is stored.
 *
 * @param[in] data pointer to the instance to be queried.
 * @param[out] crc the expected byte, written only when this returns true.
 * @returns true if the variant has a checksum at all, false otherwise.
 */
bool texkom_get_expected_crc(const TexkomData* data, uint8_t* crc);

#ifdef __cplusplus
}
#endif
