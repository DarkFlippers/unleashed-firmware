#include "mf_plus_i.h"

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
            mf_plus_data->size = MfPlusSizeUnknown;
            FURI_LOG_D(TAG, "Unknown storage size");
            break;
        }

        // Security level from the SAK bit map (AN10833): 0x20 -> SL3, 0x10/0x11 -> SL2,
        // 0x08/0x18 -> SL1. GetVersion already gave the authoritative type/size, so the SAK here
        // only refines SL; an unrecognized SAK leaves it Unknown rather than guessing SL1 (which
        // previously collapsed every non-0x20 card, mislabelling genuine SL2 as SL1).
        const uint8_t sak = iso14443_4a_data->iso14443_3a_data->sak;
        switch(sak) {
        case 0x20:
            // SL0 and SL3 are indistinguishable from SAK/ATS; the poller's active probe resolves
            // them and passes the result in. Fall back to SL3 (the prior behaviour) if it could not.
            mf_plus_data->security_level = (probed_security_level != MfPlusSecurityLevelUnknown) ?
                                               probed_security_level :
                                               MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus EV1/2 SL0/SL3 (probe-resolved)");
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
        mf_plus_data->security_level = probed_security_level;
        mf_plus_data->size = MfPlusSizeUnknown;
        if(ats_matchable &&
           memcmp(historical_bytes, mf_plus_ats_t1_tk_values[0], historical_bytes_len) == 0) {
            mf_plus_data->type = MfPlusTypeS;
        } else if(
            ats_matchable &&
            memcmp(historical_bytes, mf_plus_ats_t1_tk_values[1], historical_bytes_len) == 0) {
            mf_plus_data->type = MfPlusTypeX;
        } else if(
            ats_matchable &&
            (memcmp(historical_bytes, mf_plus_ats_t1_tk_values[2], historical_bytes_len) == 0 ||
             memcmp(historical_bytes, mf_plus_ats_t1_tk_values[3], historical_bytes_len) == 0)) {
            mf_plus_data->type = MfPlusTypeSE; // SE is 1K regardless of security level
            mf_plus_data->size = MfPlusSize1K;
        } else {
            mf_plus_data->type = MfPlusTypePlus;
        }
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
    case 0x20:
        // Reached only when the probe gave no usable result (Unknown): keep the strict ATS-table
        // gate so DESFire (also SAK 0x20) is not hijacked. 2K vs 4K needs the AN10833-forbidden
        // ATQA nibble, so size stays Unknown.
        if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[0], historical_bytes_len) == 0) {
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSizeUnknown;
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus S SL3");
            error = MfPlusErrorNone;
        } else if(memcmp(historical_bytes, mf_plus_ats_t1_tk_values[1], historical_bytes_len) == 0) {
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSizeUnknown;
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus X SL3");
            error = MfPlusErrorNone;
        } else if(
            memcmp(historical_bytes, mf_plus_ats_t1_tk_values[2], historical_bytes_len) == 0 ||
            memcmp(historical_bytes, mf_plus_ats_t1_tk_values[3], historical_bytes_len) == 0) {
            // SE is 1K regardless of security level.
            mf_plus_data->type = MfPlusTypeSE;
            mf_plus_data->size = MfPlusSize1K;
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus SE 1K SL3");
            error = MfPlusErrorNone;
        } else {
            FURI_LOG_D(TAG, "Sak 20 but no known Mifare Plus type");
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
