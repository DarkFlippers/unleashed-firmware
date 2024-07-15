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
    {0xC1, 0x05, 0x2F, 0x2F, 0x00, 0xF6, 0xD1}, // Mifare Plus SE
    {0xC1, 0x05, 0x2F, 0x2F, 0x01, 0xF6, 0xD1}, // Mifare Plus SE
};

MfPlusError mf_plus_get_type_from_version(
    const Iso14443_4aData* iso14443_4a_data,
    MfPlusData* mf_plus_data) {
    furi_assert(iso14443_4a_data);
    furi_assert(mf_plus_data);

    MfPlusError error = MfPlusErrorProtocol;

    if(mf_plus_data->version.hw_major == 0x02 || mf_plus_data->version.hw_major == 0x82) {
        error = MfPlusErrorNone;
        if(iso14443_4a_data->iso14443_3a_data->sak == 0x10) {
            // Mifare Plus 2K SL2
            mf_plus_data->type = MfPlusTypePlus;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel2;
            FURI_LOG_D(TAG, "Mifare Plus 2K SL2");
        } else if(iso14443_4a_data->iso14443_3a_data->sak == 0x11) {
            // Mifare Plus 4K SL3
            mf_plus_data->type = MfPlusTypePlus;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus 4K SL3");
        } else {
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

            // Security level
            if(iso14443_4a_data->iso14443_3a_data->sak == 0x20) {
                // Mifare Plus EV1/2 SL3
                mf_plus_data->security_level = MfPlusSecurityLevel3;
                FURI_LOG_D(TAG, "Miare Plus EV1/2 SL3");
            } else {
                // Mifare Plus EV1/2 SL1
                mf_plus_data->security_level = MfPlusSecurityLevel1;
                FURI_LOG_D(TAG, "Miare Plus EV1/2 SL1");
            }
        }
    }

    return error;
}

MfPlusError
    mf_plus_get_type_from_iso4(const Iso14443_4aData* iso4_data, MfPlusData* mf_plus_data) {
    furi_assert(iso4_data);
    furi_assert(mf_plus_data);

    MfPlusError error = MfPlusErrorProtocol;

    if(simple_array_get_count(iso4_data->ats_data.t1_tk) != MF_PLUS_T1_TK_VALUE_LEN) {
        return MfPlusErrorProtocol;
    }

    switch(iso4_data->iso14443_3a_data->sak) {
    case 0x08:
        if(memcmp(
               simple_array_get_data(iso4_data->ats_data.t1_tk),
               mf_plus_ats_t1_tk_values[0],
               simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
            // Mifare Plus S 2K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus S 2K SL1");
            error = MfPlusErrorNone;
        } else if(
            memcmp(
                simple_array_get_data(iso4_data->ats_data.t1_tk),
                mf_plus_ats_t1_tk_values[1],
                simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
            // Mifare Plus X 2K SL1
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus X 2K SL1");
            error = MfPlusErrorNone;
        } else if(
            memcmp(
                simple_array_get_data(iso4_data->ats_data.t1_tk),
                mf_plus_ats_t1_tk_values[2],
                simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0 ||
            memcmp(
                simple_array_get_data(iso4_data->ats_data.t1_tk),
                mf_plus_ats_t1_tk_values[3],
                simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
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
    case 0x18:
        if(memcmp(
               simple_array_get_data(iso4_data->ats_data.t1_tk),
               mf_plus_ats_t1_tk_values[0],
               simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
            // Mifare Plus S 4K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;

            FURI_LOG_D(TAG, "Mifare Plus S 4K SL1");
            error = MfPlusErrorNone;
        } else if(
            memcmp(
                simple_array_get_data(iso4_data->ats_data.t1_tk),
                mf_plus_ats_t1_tk_values[1],
                simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
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
        if(memcmp(
               simple_array_get_data(iso4_data->ats_data.t1_tk),
               mf_plus_ats_t1_tk_values[0],
               simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
            // Mifare Plus S 2/4K SL3
            FURI_LOG_D(TAG, "Mifare Plus S SL3");
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->security_level = MfPlusSecurityLevel3;

            if((iso4_data->iso14443_3a_data->atqa[0] & 0x0F) == 0x04) {
                // Mifare Plus S 2K SL3
                mf_plus_data->size = MfPlusSize2K;

                FURI_LOG_D(TAG, "Mifare Plus S 2K SL3");
                error = MfPlusErrorNone;
            } else if((iso4_data->iso14443_3a_data->atqa[0] & 0x0F) == 0x02) {
                // Mifare Plus S 4K SL3
                mf_plus_data->size = MfPlusSize4K;

                FURI_LOG_D(TAG, "Mifare Plus S 4K SL3");
                error = MfPlusErrorNone;
            } else {
                FURI_LOG_D(TAG, "Sak 20 but no known Mifare Plus type (S)");
            }
        } else if(
            memcmp(
                simple_array_get_data(iso4_data->ats_data.t1_tk),
                mf_plus_ats_t1_tk_values[1],
                simple_array_get_count(iso4_data->ats_data.t1_tk)) == 0) {
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->security_level = MfPlusSecurityLevel3;
            FURI_LOG_D(TAG, "Mifare Plus X SL3");

            if((iso4_data->iso14443_3a_data->atqa[0] & 0x0F) == 0x04) {
                mf_plus_data->size = MfPlusSize2K;

                FURI_LOG_D(TAG, "Mifare Plus X 2K SL3");
                error = MfPlusErrorNone;
            } else if((iso4_data->iso14443_3a_data->atqa[0] & 0x0F) == 0x02) {
                mf_plus_data->size = MfPlusSize4K;

                FURI_LOG_D(TAG, "Mifare Plus X 4K SL3");
                error = MfPlusErrorNone;
            } else {
                FURI_LOG_D(TAG, "Sak 20 but no known Mifare Plus type (X)");
            }
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
