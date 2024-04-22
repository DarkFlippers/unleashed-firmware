#include "mf_plus_i.h"

#define MF_PLUS_FFF_VERSION_KEY \
    MF_PLUS_FFF_PICC_PREFIX " " \
                            "Version"

#define MF_PLUS_FFF_SECURITY_LEVEL_KEY "Security Level"
#define MF_PLUS_FFF_CARD_TYPE_KEY "Card Type"
#define MF_PLUS_FFF_MEMORY_SIZE_KEY "Memory Size"

bool mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusVersion);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusVersion));
    }

    return can_parse;
}

bool mf_plus_security_level_parse(MfPlusSecurityLevel* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusSecurityLevel);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusSecurityLevel));
    }

    return can_parse;
}

bool mf_plus_type_parse(MfPlusType* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusType);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusType));
    }

    return can_parse;
}

bool mf_plus_size_parse(MfPlusSize* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusSize);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusSize));
    }

    return can_parse;
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

    flipper_format_write_string(ff, MF_PLUS_FFF_SECURITY_LEVEL_KEY, security_level_string);
    furi_string_free(security_level_string);

    return true;
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

    flipper_format_write_string(ff, MF_PLUS_FFF_CARD_TYPE_KEY, type_string);
    furi_string_free(type_string);

    return true;
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

    flipper_format_write_string(ff, MF_PLUS_FFF_MEMORY_SIZE_KEY, size_string);
    furi_string_free(size_string);

    return true;
}