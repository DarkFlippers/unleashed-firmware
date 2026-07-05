#include "mf_plus_i.h"

#include <toolbox/hex.h>

#define MF_PLUS_FFF_VERSION_KEY \
    MF_PLUS_FFF_PICC_PREFIX " " \
                            "Version"

#define MF_PLUS_T1_TK_VALUE_LEN 7

#define MF_PLUS_FFF_SECURITY_LEVEL_KEY "Security Level"
#define MF_PLUS_FFF_CARD_TYPE_KEY      "Card Type"
#define MF_PLUS_FFF_MEMORY_SIZE_KEY    "Memory Size"

#define TAG "MfPlus"

const uint8_t mf_plus_ats_t1_tk_values[][MF_PLUS_T1_TK_VALUE_LEN] = {
    {0xC1, 0x05, 0x2F, 0x2F, 0x00, 0x35, 0xC7}, // Mifare Plus S
    {0xC1, 0x05, 0x2F, 0x2F, 0x01, 0xBC, 0xD6}, // Mifare Plus X
    {0xC1, 0x05, 0x21, 0x30, 0x00, 0xF6, 0xD1}, // Mifare Plus SE
    {0xC1, 0x05, 0x21, 0x30, 0x10, 0xF6, 0xD1}, // Mifare Plus SE
};

// Map the ATS historical bytes to a MIFARE Plus product, or MfPlusTypeUnknown if they match no
// known S/X/SE entry. A non-table-length count never matches (and is NULL-safe).
static MfPlusType mf_plus_type_from_ats(const uint8_t* historical_bytes, size_t len) {
    if(len != MF_PLUS_T1_TK_VALUE_LEN) return MfPlusTypeUnknown;
    if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[0], len) == 0) return MfPlusTypeS;
    if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[1], len) == 0) return MfPlusTypeX;
    if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[2], len) == 0 ||
       memcmp(historical_bytes, mf_plus_ats_t1_tk_values[3], len) == 0)
        return MfPlusTypeSE;
    return MfPlusTypeUnknown;
}

// SL0/SL3 expose no product-independent size byte, so once the card is confirmed Plus fall back to
// the ATQA size coding: bit 2 (0x0004) = 2K, bit 1 (0x0002) = 4K. Size refinement only, never type
// identification (same as PM3 hf mfp info).
static MfPlusSize mf_plus_size_from_atqa(const uint8_t atqa[2]) {
    const uint16_t atqa_value = atqa[0] | ((uint16_t)atqa[1] << 8);
    if(atqa_value & 0x0004) return MfPlusSize2K;
    if(atqa_value & 0x0002) return MfPlusSize4K;
    return MfPlusSizeUnknown;
}

MfPlusError mf_plus_get_type_from_version(
    const Iso14443_4aData* iso14443_4a_data,
    MfPlusSecurityLevel probed_security_level,
    MfPlusData* mf_plus_data) {
    furi_assert(iso14443_4a_data);
    furi_assert(mf_plus_data);

    MfPlusError error = MfPlusErrorProtocol;

    if((mf_plus_data->version.hw_type & 0x0F) == 0x02) {
        error = MfPlusErrorNone;
        // Mifare Plus EV1/EV2

        // Revision
        switch(mf_plus_data->version.hw_major) {
        case 0x11:
            mf_plus_data->type = MfPlusTypeEV1;
            FURI_LOG_D(TAG, "Mifare Plus EV1");
            break;
        case 0x22:
            mf_plus_data->type = MfPlusTypeEV2;
            FURI_LOG_D(TAG, "Mifare Plus EV2");
            break;
        default:
            mf_plus_data->type = MfPlusTypeUnknown;
            FURI_LOG_D(TAG, "Unknown Mifare Plus EV type");
            break;
        }

        // Storage size
        switch(mf_plus_data->version.hw_storage) {
        case 0x16:
            mf_plus_data->size = MfPlusSize2K;
            FURI_LOG_D(TAG, "2K");
            break;
        case 0x18:
            mf_plus_data->size = MfPlusSize4K;
            FURI_LOG_D(TAG, "4K");
            break;
        default:
            // No recognized storage byte (e.g. probe-only/older silicon) -> fall back to ATQA.
            mf_plus_data->size = mf_plus_size_from_atqa(iso14443_4a_data->iso14443_3a_data->atqa);
            FURI_LOG_D(TAG, "Unknown storage size; size from ATQA");
            break;
        }

        // Security level from the SAK bit map (AN10833): 0x20 -> SL3, 0x10/0x11 -> SL2,
        // 0x08/0x18 -> SL1. GetVersion already gave the authoritative type/size, so the SAK here
        // only refines SL; an unrecognized SAK leaves it Unknown rather than guessing SL1 (which
        // previously collapsed every non-0x20 card, mislabelling genuine SL2 as SL1).
        const uint8_t sak = iso14443_4a_data->iso14443_3a_data->sak;
        switch(sak) {
        case 0x20:
            // GetVersion already confirmed Plus here; the probe only refines SL0 vs SL3. Keep the
            // prior SL3 default when the probe was inconclusive.
            if(probed_security_level != MfPlusSecurityLevelUnknown) {
                mf_plus_data->security_level = probed_security_level;
                FURI_LOG_D(
                    TAG,
                    "Mifare Plus EV1/2 SL%d (probe)",
                    probed_security_level == MfPlusSecurityLevel0 ? 0 : 3);
            } else {
                mf_plus_data->security_level = MfPlusSecurityLevel3;
                FURI_LOG_D(TAG, "Mifare Plus EV1/2 SL3 (default)");
            }
            break;
        case 0x10:
        case 0x11:
            mf_plus_data->security_level = MfPlusSecurityLevel2;
            FURI_LOG_D(TAG, "Mifare Plus EV1/2 SL2");
            break;
        case 0x08:
        case 0x18:
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            FURI_LOG_D(TAG, "Mifare Plus EV1/2 SL1");
            break;
        default:
            mf_plus_data->security_level = MfPlusSecurityLevelUnknown;
            FURI_LOG_D(TAG, "Mifare Plus EV1/2 unknown SL (SAK 0x%02X)", sak);
            break;
        }
    }

    return error;
}

MfPlusError mf_plus_get_type_from_iso4(
    const Iso14443_4aData* iso4_data,
    MfPlusSecurityLevel probed_security_level,
    MfPlusData* mf_plus_data) {
    furi_assert(iso4_data);
    furi_assert(mf_plus_data);

    MfPlusError error = MfPlusErrorProtocol;

    const uint8_t sak = iso4_data->iso14443_3a_data->sak;
    const size_t historical_bytes_len = simple_array_get_count(iso4_data->ats_data.t1_tk);
    const bool ats_matchable = (historical_bytes_len == MF_PLUS_T1_TK_VALUE_LEN);
    const uint8_t* historical_bytes =
        ats_matchable ? simple_array_cget_data(iso4_data->ats_data.t1_tk) : NULL;

    // SAK 0x20 is shared with DESFire and cannot separate SL0 from SL3. When the poller's active
    // probe resolved the level (non-Unknown), it has already confirmed "Plus, not DESFire", so we
    // may report a Plus regardless of the ATS. Handle that here, before the ATS-table gate below,
    // so an untabled or short ATS still yields a generic Plus instead of Unknown. 2K vs 4K needs the
    // AN10833-forbidden ATQA nibble, so non-SE size stays Unknown.
    if(sak == 0x20 && probed_security_level != MfPlusSecurityLevelUnknown) {
        const MfPlusType ats_type = mf_plus_type_from_ats(historical_bytes, historical_bytes_len);
        mf_plus_data->type = (ats_type == MfPlusTypeUnknown) ? MfPlusTypePlus : ats_type;
        mf_plus_data->size = (ats_type == MfPlusTypeSE) ?
                                 MfPlusSize1K :
                                 mf_plus_size_from_atqa(iso4_data->iso14443_3a_data->atqa);
        mf_plus_data->security_level = probed_security_level;
        FURI_LOG_D(
            TAG,
            "Mifare Plus SL%d (SAK 20, probe-confirmed)",
            probed_security_level == MfPlusSecurityLevel0 ? 0 : 3);
        return MfPlusErrorNone;
    }

    if(!ats_matchable) {
        return MfPlusErrorProtocol;
    }

    switch(sak) {
    case 0x08:
        if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[0], historical_bytes_len) == 0) {
            // Mifare Plus S 2K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus S 2K SL1");
            error = MfPlusErrorNone;
        } else if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[1], historical_bytes_len) == 0) {
            // Mifare Plus X 2K SL1
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus X 2K SL1");
            error = MfPlusErrorNone;
        } else if(
            memcmp(historical_bytes, mf_plus_ats_t1_tk_values[2], historical_bytes_len) == 0 ||
            memcmp(historical_bytes, mf_plus_ats_t1_tk_values[3], historical_bytes_len) == 0) {
            // Mifare Plus SE 1K SL1
            mf_plus_data->type = MfPlusTypeSE;
            mf_plus_data->size = MfPlusSize1K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus SE 1K SL1");
            error = MfPlusErrorNone;
        } else {
            FURI_LOG_D(TAG, "Sak 08 but no known Mifare Plus type");
        }

        break;
    case 0x10:
        // SAK 0x10 is Plus 2K SL2 (AN10833). The bare SAK does not justify the S/X/SE product, so
        // report a generic Plus rather than over-claiming X.
        mf_plus_data->type = MfPlusTypePlus;
        mf_plus_data->size = MfPlusSize2K;
        mf_plus_data->security_level = MfPlusSecurityLevel2;
        FURI_LOG_D(TAG, "Mifare Plus 2K SL2");
        error = MfPlusErrorNone;

        break;
    case 0x11:
        // SAK 0x11 is Plus 4K SL2 (AN10833); generic Plus for the same reason as 0x10.
        mf_plus_data->type = MfPlusTypePlus;
        mf_plus_data->size = MfPlusSize4K;
        mf_plus_data->security_level = MfPlusSecurityLevel2;
        FURI_LOG_D(TAG, "Mifare Plus 4K SL2");
        error = MfPlusErrorNone;

        break;
    case 0x18:
        if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[0], historical_bytes_len) == 0) {
            // Mifare Plus S 4K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus S 4K SL1");
            error = MfPlusErrorNone;
        } else if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[1], historical_bytes_len) == 0) {
            // Mifare Plus X 4K SL1
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus X 4K SL1");
            error = MfPlusErrorNone;
        } else {
            FURI_LOG_D(TAG, "Sak 18 but no known Mifare Plus type");
        }

        break;
    case 0x20: {
        // Reached only when the probe gave no usable result: keep the strict ATS-table gate so a
        // DESFire (also SAK 0x20) is not hijacked. SE is always 1K; other products take their size
        // from ATQA (the only SL3 size signal).
        const MfPlusType ats_type = mf_plus_type_from_ats(historical_bytes, historical_bytes_len);
        if(ats_type != MfPlusTypeUnknown) {
            mf_plus_data->type = ats_type;
            mf_plus_data->size = (ats_type == MfPlusTypeSE) ?
                                     MfPlusSize1K :
                                     mf_plus_size_from_atqa(iso4_data->iso14443_3a_data->atqa);
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus SL3 (SAK 20, ATS-matched)");
            error = MfPlusErrorNone;
        } else {
            FURI_LOG_D(TAG, "Sak 20 but no known Mifare Plus type");
        }
        break;
    }
    }

    return error;
}

MfPlusError mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusVersion);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusVersion));
    } else {
        memset(data, 0, sizeof(MfPlusVersion));
    }

    return can_parse ? MfPlusErrorNone : MfPlusErrorProtocol;
}

bool mf_plus_version_load(MfPlusVersion* data, FlipperFormat* ff) {
    return flipper_format_read_hex(
        ff, MF_PLUS_FFF_VERSION_KEY, (uint8_t*)data, sizeof(MfPlusVersion));
}

bool mf_plus_security_level_load(MfPlusSecurityLevel* data, FlipperFormat* ff) {
    FuriString* security_level_string = furi_string_alloc();
    flipper_format_read_string(ff, MF_PLUS_FFF_SECURITY_LEVEL_KEY, security_level_string);

    // Take the last character of the string
    char security_level_char = furi_string_get_char(
        security_level_string, furi_string_utf8_length(security_level_string) - 1);

    switch(security_level_char) {
    case '0':
        *data = MfPlusSecurityLevel0;
        break;
    case '1':
        *data = MfPlusSecurityLevel1;
        break;
    case '2':
        *data = MfPlusSecurityLevel2;
        break;
    case '3':
        *data = MfPlusSecurityLevel3;
        break;
    default:
        *data = MfPlusSecurityLevelUnknown;
        break;
    }

    furi_string_free(security_level_string);

    return true;
}

bool mf_plus_type_load(MfPlusType* data, FlipperFormat* ff) {
    FuriString* type_string = furi_string_alloc();
    flipper_format_read_string(ff, MF_PLUS_FFF_CARD_TYPE_KEY, type_string);

    if(furi_string_equal_str(type_string, "Mifare Plus")) {
        *data = MfPlusTypePlus;
    } else if(furi_string_equal_str(type_string, "Mifare Plus X")) {
        *data = MfPlusTypeX;
    } else if(furi_string_equal_str(type_string, "Mifare Plus S")) {
        *data = MfPlusTypeS;
    } else if(furi_string_equal_str(type_string, "Mifare Plus SE")) {
        *data = MfPlusTypeSE;
    } else if(furi_string_equal_str(type_string, "Mifare Plus EV1")) {
        *data = MfPlusTypeEV1;
    } else if(furi_string_equal_str(type_string, "Mifare Plus EV2")) {
        *data = MfPlusTypeEV2;
    } else {
        *data = MfPlusTypeUnknown;
    }

    furi_string_free(type_string);
    return true;
}

bool mf_plus_size_load(MfPlusSize* data, FlipperFormat* ff) {
    FuriString* size_string = furi_string_alloc();
    flipper_format_read_string(ff, MF_PLUS_FFF_MEMORY_SIZE_KEY, size_string);

    if(furi_string_equal_str(size_string, "1K")) {
        *data = MfPlusSize1K;
    } else if(furi_string_equal_str(size_string, "2K")) {
        *data = MfPlusSize2K;
    } else if(furi_string_equal_str(size_string, "4K")) {
        *data = MfPlusSize4K;
    } else {
        *data = MfPlusSizeUnknown;
    }

    furi_string_free(size_string);
    return true;
}

bool mf_plus_version_save(const MfPlusVersion* data, FlipperFormat* ff) {
    return flipper_format_write_hex(
        ff, MF_PLUS_FFF_VERSION_KEY, (const uint8_t*)data, sizeof(MfPlusVersion));
}

bool mf_plus_security_level_save(const MfPlusSecurityLevel* data, FlipperFormat* ff) {
    FuriString* security_level_string = furi_string_alloc();

    switch(*data) {
    case MfPlusSecurityLevel0:
        furi_string_cat(security_level_string, "SL0");
        break;
    case MfPlusSecurityLevel1:
        furi_string_cat(security_level_string, "SL1");
        break;
    case MfPlusSecurityLevel2:
        furi_string_cat(security_level_string, "SL2");
        break;
    case MfPlusSecurityLevel3:
        furi_string_cat(security_level_string, "SL3");
        break;
    default:
        furi_string_cat(security_level_string, "Unknown");
        break;
    }

    bool success =
        flipper_format_write_string(ff, MF_PLUS_FFF_SECURITY_LEVEL_KEY, security_level_string);
    furi_string_free(security_level_string);

    return success;
}

bool mf_plus_type_save(const MfPlusType* data, FlipperFormat* ff) {
    FuriString* type_string = furi_string_alloc();

    switch(*data) {
    case MfPlusTypePlus:
        furi_string_cat(type_string, "Mifare Plus");
        break;
    case MfPlusTypeX:
        furi_string_cat(type_string, "Mifare Plus X");
        break;
    case MfPlusTypeS:
        furi_string_cat(type_string, "Mifare Plus S");
        break;
    case MfPlusTypeSE:
        furi_string_cat(type_string, "Mifare Plus SE");
        break;
    case MfPlusTypeEV1:
        furi_string_cat(type_string, "Mifare Plus EV1");
        break;
    case MfPlusTypeEV2:
        furi_string_cat(type_string, "Mifare Plus EV2");
        break;
    default:
        furi_string_cat(type_string, "Unknown");
        break;
    }

    bool success = flipper_format_write_string(ff, MF_PLUS_FFF_CARD_TYPE_KEY, type_string);
    furi_string_free(type_string);

    return success;
}

bool mf_plus_size_save(const MfPlusSize* data, FlipperFormat* ff) {
    FuriString* size_string = furi_string_alloc();

    switch(*data) {
    case MfPlusSize1K:
        furi_string_cat(size_string, "1K");
        break;
    case MfPlusSize2K:
        furi_string_cat(size_string, "2K");
        break;
    case MfPlusSize4K:
        furi_string_cat(size_string, "4K");
        break;
    default:
        furi_string_cat(size_string, "Unknown");
        break;
    }

    bool success = flipper_format_write_string(ff, MF_PLUS_FFF_MEMORY_SIZE_KEY, size_string);
    furi_string_free(size_string);

    return success;
}

/* ---- SL3 payload: geometry, block-read bitmap, and (de)serialization ---- */

#define MF_PLUS_FFF_DATA_FORMAT_VERSION_KEY "Data format version"
#define MF_PLUS_FFF_SIGNATURE_KEY           "Signature"

static const uint32_t mf_plus_data_format_version = 1;

static const char* const mf_plus_admin_key_names[MfPlusAdminKeyNum] = {
    [MfPlusAdminKeyCardMaster] = "Card Master Key",
    [MfPlusAdminKeyCardConfig] = "Card Config Key",
    [MfPlusAdminKeyL3Switch] = "L3 Switch Key",
    [MfPlusAdminKeySL1CardAuth] = "SL1 Card Auth Key",
};

// Lock the mask-fits-domain invariants at compile time.
_Static_assert(MfPlusAdminKeyNum <= 8, "admin_key_mask (uint8_t) too small");
_Static_assert(MF_PLUS_CONFIG_BLOCK_NUM <= 8, "config_read_mask (uint8_t) too small");
_Static_assert(MF_PLUS_MAX_SECTORS <= 64, "key masks (uint64_t) too small");
_Static_assert(
    MF_PLUS_MAX_BLOCKS == 32 * MF_PLUS_BLOCK_READ_MASK_SIZE,
    "block_read_mask size mismatch");

uint16_t mf_plus_get_block_count(MfPlusSize size) {
    switch(size) {
    case MfPlusSize1K:
        return 64;
    case MfPlusSize2K:
        return 128;
    case MfPlusSize4K:
        return 256;
    default:
        return 0;
    }
}

uint8_t mf_plus_get_sector_count(MfPlusSize size) {
    switch(size) {
    case MfPlusSize1K:
        return 16;
    case MfPlusSize2K:
        return 32;
    case MfPlusSize4K:
        return 40;
    default:
        return 0;
    }
}

bool mf_plus_is_block_read(const MfPlusData* data, uint16_t block_num) {
    furi_check(block_num < MF_PLUS_MAX_BLOCKS);
    return (data->block_read_mask[block_num / 32] >> (block_num % 32)) & 1U;
}

void mf_plus_set_block_read(MfPlusData* data, uint16_t block_num) {
    furi_check(block_num < MF_PLUS_MAX_BLOCKS);
    data->block_read_mask[block_num / 32] |= (1UL << (block_num % 32));
}

// Render `len` bytes as "AA BB .." when known, or an equal run of "??" when not.
static void mf_plus_bytes_to_str(FuriString* out, const uint8_t* data, size_t len, bool known) {
    furi_string_reset(out);
    for(size_t i = 0; i < len; i++) {
        if(known) {
            furi_string_cat_printf(out, "%02X ", data[i]);
        } else {
            furi_string_cat_printf(out, "?? ");
        }
    }
    furi_string_trim(out);
}

// Parse `len` space-separated bytes into `out`. Returns true only if every byte is valid hex,
// and leaves `out` untouched on failure (parses into a local, commits on success). The paired
// writer emits all-hex (known) or an all-"??" run (unknown), so a leading '?' is a legitimately
// unknown field (returns false quietly); a too-short line or a non-hex byte in a hex field is
// treated as unknown and logged. The length guard also prevents an out-of-bounds string read
// (firmware builds NDEBUG, so FuriString's index assert is compiled out).
static bool mf_plus_str_to_bytes(FuriString* str, uint8_t* out, size_t len) {
    furi_check(len <= MF_PLUS_SIGNATURE_SIZE);
    furi_string_trim(str);
    if(furi_string_size(str) < (3 * len - 1)) return false; // truncated / too short
    if(furi_string_get_char(str, 0) == '?') return false; // legitimate unknown ("?? ..")

    uint8_t tmp[MF_PLUS_SIGNATURE_SIZE];
    for(size_t i = 0; i < len; i++) {
        char hi = furi_string_get_char(str, 3 * i);
        char lo = furi_string_get_char(str, 3 * i + 1);
        if(!hex_char_to_uint8(hi, lo, &tmp[i])) {
            FURI_LOG_W(TAG, "Corrupt MFP hex field, treating as unknown");
            return false;
        }
    }
    memcpy(out, tmp, len);
    return true;
}

bool mf_plus_sl3_data_save(const MfPlusData* data, FlipperFormat* ff) {
    bool saved = false;
    FuriString* key = furi_string_alloc();
    FuriString* val = furi_string_alloc();

    do {
        if(!flipper_format_write_uint32(
               ff, MF_PLUS_FFF_DATA_FORMAT_VERSION_KEY, &mf_plus_data_format_version, 1))
            break;

        // Signature is written densely (hex-or-"??") so the payload stays sequentially
        // readable regardless of presence.
        mf_plus_bytes_to_str(
            val, data->signature, MF_PLUS_SIGNATURE_SIZE, data->signature_present);
        if(!flipper_format_write_string(ff, MF_PLUS_FFF_SIGNATURE_KEY, val)) break;

        // Blocks / keys / config exist only for a fully-AES (SL3) card.
        if(data->security_level != MfPlusSecurityLevel3) {
            saved = true;
            break;
        }

        if(!flipper_format_write_comment_cstr(ff, "SL3 blocks and keys, \'??\' means unknown"))
            break;

        const uint16_t blocks = mf_plus_get_block_count(data->size);
        const uint8_t sectors = mf_plus_get_sector_count(data->size);
        bool ok = true;

        for(uint16_t i = 0; ok && i < blocks; i++) {
            mf_plus_bytes_to_str(
                val, data->block[i].data, MF_PLUS_BLOCK_SIZE, mf_plus_is_block_read(data, i));
            furi_string_printf(key, "Block %u", i);
            ok = flipper_format_write_string(ff, furi_string_get_cstr(key), val);
        }

        for(uint8_t s = 0; ok && s < sectors; s++) {
            mf_plus_bytes_to_str(
                val, data->key_a[s].data, MF_PLUS_KEY_SIZE, (data->key_a_mask >> s) & 1U);
            furi_string_printf(key, "Key A %u", s);
            ok = flipper_format_write_string(ff, furi_string_get_cstr(key), val);
            if(!ok) break;

            mf_plus_bytes_to_str(
                val, data->key_b[s].data, MF_PLUS_KEY_SIZE, (data->key_b_mask >> s) & 1U);
            furi_string_printf(key, "Key B %u", s);
            ok = flipper_format_write_string(ff, furi_string_get_cstr(key), val);
        }

        for(uint8_t a = 0; ok && a < MfPlusAdminKeyNum; a++) {
            mf_plus_bytes_to_str(
                val, data->admin_key[a].data, MF_PLUS_KEY_SIZE, (data->admin_key_mask >> a) & 1U);
            ok = flipper_format_write_string(ff, mf_plus_admin_key_names[a], val);
        }

        for(uint8_t c = 0; ok && c < MF_PLUS_CONFIG_BLOCK_NUM; c++) {
            mf_plus_bytes_to_str(
                val,
                data->config_block[c].data,
                MF_PLUS_BLOCK_SIZE,
                (data->config_read_mask >> c) & 1U);
            furi_string_printf(key, "Config Block %u", c);
            ok = flipper_format_write_string(ff, furi_string_get_cstr(key), val);
        }

        saved = ok;
    } while(false);

    furi_string_free(key);
    furi_string_free(val);
    return saved;
}

bool mf_plus_sl3_data_load(MfPlusData* data, FlipperFormat* ff) {
    // Legacy metadata-only dumps have no "Data format version" -> nothing more to read.
    uint32_t dfv = 0;
    if(!flipper_format_read_uint32(ff, MF_PLUS_FFF_DATA_FORMAT_VERSION_KEY, &dfv, 1)) {
        return true;
    }
    // Refuse a payload written by a newer, unknown layout rather than misparsing it as v1.
    if(dfv > mf_plus_data_format_version) {
        FURI_LOG_W(TAG, "Unsupported MFP data format version %lu", (unsigned long)dfv);
        return false;
    }

    bool loaded = false;
    FuriString* key = furi_string_alloc();
    FuriString* val = furi_string_alloc();

    do {
        if(!flipper_format_read_string(ff, MF_PLUS_FFF_SIGNATURE_KEY, val)) break;
        data->signature_present =
            mf_plus_str_to_bytes(val, data->signature, MF_PLUS_SIGNATURE_SIZE);

        if(data->security_level != MfPlusSecurityLevel3) {
            loaded = true;
            break;
        }

        const uint16_t blocks = mf_plus_get_block_count(data->size);
        const uint8_t sectors = mf_plus_get_sector_count(data->size);
        bool ok = true;

        for(uint16_t i = 0; ok && i < blocks; i++) {
            furi_string_printf(key, "Block %u", i);
            ok = flipper_format_read_string(ff, furi_string_get_cstr(key), val);
            if(ok && mf_plus_str_to_bytes(val, data->block[i].data, MF_PLUS_BLOCK_SIZE)) {
                mf_plus_set_block_read(data, i);
            }
        }

        for(uint8_t s = 0; ok && s < sectors; s++) {
            furi_string_printf(key, "Key A %u", s);
            ok = flipper_format_read_string(ff, furi_string_get_cstr(key), val);
            if(ok && mf_plus_str_to_bytes(val, data->key_a[s].data, MF_PLUS_KEY_SIZE)) {
                data->key_a_mask |= (1ULL << s);
            }
            if(!ok) break;

            furi_string_printf(key, "Key B %u", s);
            ok = flipper_format_read_string(ff, furi_string_get_cstr(key), val);
            if(ok && mf_plus_str_to_bytes(val, data->key_b[s].data, MF_PLUS_KEY_SIZE)) {
                data->key_b_mask |= (1ULL << s);
            }
        }

        for(uint8_t a = 0; ok && a < MfPlusAdminKeyNum; a++) {
            ok = flipper_format_read_string(ff, mf_plus_admin_key_names[a], val);
            if(ok && mf_plus_str_to_bytes(val, data->admin_key[a].data, MF_PLUS_KEY_SIZE)) {
                data->admin_key_mask |= (1U << a);
            }
        }

        for(uint8_t c = 0; ok && c < MF_PLUS_CONFIG_BLOCK_NUM; c++) {
            furi_string_printf(key, "Config Block %u", c);
            ok = flipper_format_read_string(ff, furi_string_get_cstr(key), val);
            if(ok && mf_plus_str_to_bytes(val, data->config_block[c].data, MF_PLUS_BLOCK_SIZE)) {
                data->config_read_mask |= (1U << c);
            }
        }

        loaded = ok;
    } while(false);

    furi_string_free(key);
    furi_string_free(val);
    return loaded;
}
