#include "lfrfid_manual_format.h"
#include "lfrfid_casi_format.h"

#include <bit_lib/bit_lib.h>

// Called with zeroed data of the protocol's size and values checked against their range
typedef void (*LfRfidManualFormatEncode)(const uint64_t* values, uint8_t* data);

typedef struct {
    const LfRfidManualFormatField* fields;
    size_t fields_count;
    size_t data_size; // the protocol's, the encoders write fixed offsets
    LfRfidManualFormatEncode encode;
} LfRfidManualFormatDescriptor;

// 26-bit Wiegand: 8-bit facility code, 16-bit card number
static const LfRfidManualFormatField lfrfid_manual_format_fields_wiegand26[] = {
    {"Facility Code", 0, 0xFF},
    {"Card Number", 0, 0xFFFF},
};

static const LfRfidManualFormatField lfrfid_manual_format_fields_io_prox[] = {
    {"Facility Code", 0, 0xFF},
    {"Card Number", 0, 0xFFFF},
    {"Version", 0, 0xFF},
};

// The maxima are what the Gallagher encoder packs into a frame
static const LfRfidManualFormatField lfrfid_manual_format_fields_gallagher[] = {
    {"Facility Code", 0, 0xFFFF},
    {"Card Number", 0, 0xFFFFFF},
    {"Region Code", 0, 0xF},
    {"Issue Level", 0, 0xF},
};

// Casi-Rusco: the twelve-digit badge id as the access system prints it
static const LfRfidManualFormatField lfrfid_manual_format_fields_casi[] = {
    {"Credential", LFRFID_CASI_CREDENTIAL_MIN, LFRFID_CASI_CREDENTIAL_MAX},
    {"Card Number", 0, LFRFID_CASI_CARD_MAX},
};

// The scene keeps one value per field, so no format may ask for more
_Static_assert(
    COUNT_OF(lfrfid_manual_format_fields_wiegand26) <= LFRFID_MANUAL_FORMAT_FIELDS_MAX,
    "LFRFID_MANUAL_FORMAT_FIELDS_MAX does not cover the Wiegand fields");
_Static_assert(
    COUNT_OF(lfrfid_manual_format_fields_io_prox) <= LFRFID_MANUAL_FORMAT_FIELDS_MAX,
    "LFRFID_MANUAL_FORMAT_FIELDS_MAX does not cover the ioProx fields");
_Static_assert(
    COUNT_OF(lfrfid_manual_format_fields_gallagher) <= LFRFID_MANUAL_FORMAT_FIELDS_MAX,
    "LFRFID_MANUAL_FORMAT_FIELDS_MAX does not cover the Gallagher fields");
_Static_assert(
    COUNT_OF(lfrfid_manual_format_fields_casi) <= LFRFID_MANUAL_FORMAT_FIELDS_MAX,
    "LFRFID_MANUAL_FORMAT_FIELDS_MAX does not cover the Casi-Rusco fields");

static void lfrfid_manual_format_encode_em4100(const uint64_t* values, uint8_t* data) {
    // the id bytes render_data reads as fc and card, the two before them stay zero
    data[2] = values[0];
    bit_lib_num_to_bytes_be(values[1], 2, &data[3]);
}

static void lfrfid_manual_format_encode_h10301(const uint64_t* values, uint8_t* data) {
    // wiegand parity is added by the encoder
    data[0] = values[0];
    bit_lib_num_to_bytes_be(values[1], 2, &data[1]);
}

static void lfrfid_manual_format_encode_io_prox_xsf(const uint64_t* values, uint8_t* data) {
    // checksum is added by the encoder
    data[0] = values[0];
    data[1] = values[2];
    bit_lib_num_to_bytes_be(values[1], 2, &data[2]);
}

static void lfrfid_manual_format_encode_awid(const uint64_t* values, uint8_t* data) {
    // format length, then the 26 wiegand bits as stored: even parity, fc, card, odd parity
    const uint32_t fc_and_card = values[0] << 16 | values[1];
    data[0] = 26;
    bit_lib_set_bit(data, 8, bit_lib_test_parity_32(fc_and_card >> 12, BitLibParityEven));
    bit_lib_set_bits(data, 9, values[0], 8);
    bit_lib_set_bits(data, 17, values[1] >> 8, 8);
    bit_lib_set_bits(data, 25, values[1], 8);
    bit_lib_set_bit(data, 33, bit_lib_test_parity_32(fc_and_card & 0xFFF, BitLibParityOdd));
}

static void lfrfid_manual_format_encode_pyramid(const uint64_t* values, uint8_t* data) {
    // format length, fc, card; wiegand parity and crc are added by the encoder
    data[0] = 26;
    data[1] = values[0];
    bit_lib_num_to_bytes_be(values[1], 2, &data[2]);
}

static void lfrfid_manual_format_encode_gallagher(const uint64_t* values, uint8_t* data) {
    // region code, issue level, fc in 24 bits, card in 32 bits
    bit_lib_set_bits(data, 0, values[2], 4);
    bit_lib_set_bits(data, 4, values[3], 4);
    bit_lib_num_to_bytes_be(values[0], 3, &data[1]);
    bit_lib_num_to_bytes_be(values[1], 4, &data[4]);
}

static void lfrfid_manual_format_encode_casi(const uint64_t* values, uint8_t* data) {
    lfrfid_casi_format_encode(values[0], values[1], data);
}

// Indexed by protocol. The protocols left out are hex-only: no facility code / card number
// layout, or the scrambling and checksums are not exported by the protocol library
static const LfRfidManualFormatDescriptor lfrfid_manual_format_descriptors[LFRFIDProtocolMax] = {
    [LFRFIDProtocolEM4100] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 5,
            .encode = lfrfid_manual_format_encode_em4100,
        },
    [LFRFIDProtocolEM4100_32] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 5,
            .encode = lfrfid_manual_format_encode_em4100,
        },
    [LFRFIDProtocolEM4100_16] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 5,
            .encode = lfrfid_manual_format_encode_em4100,
        },
    [LFRFIDProtocolH10301] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 3,
            .encode = lfrfid_manual_format_encode_h10301,
        },
    [LFRFIDProtocolIOProxXSF] =
        {
            .fields = lfrfid_manual_format_fields_io_prox,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_io_prox),
            .data_size = 4,
            .encode = lfrfid_manual_format_encode_io_prox_xsf,
        },
    [LFRFIDProtocolAwid] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 9,
            .encode = lfrfid_manual_format_encode_awid,
        },
    [LFRFIDProtocolPyramid] =
        {
            .fields = lfrfid_manual_format_fields_wiegand26,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_wiegand26),
            .data_size = 4,
            .encode = lfrfid_manual_format_encode_pyramid,
        },
    [LFRFIDProtocolGallagher] =
        {
            .fields = lfrfid_manual_format_fields_gallagher,
            .fields_count = COUNT_OF(lfrfid_manual_format_fields_gallagher),
            .data_size = 8,
            .encode = lfrfid_manual_format_encode_gallagher,
        },
};

// The Casi-Rusco badge is saved as EM4100
static const LfRfidManualFormatDescriptor lfrfid_manual_format_descriptor_casi = {
    .fields = lfrfid_manual_format_fields_casi,
    .fields_count = COUNT_OF(lfrfid_manual_format_fields_casi),
    .data_size = 5,
    .encode = lfrfid_manual_format_encode_casi,
};

// The HID Proximity formats are entered by facility code (if the format has one) and card number
static const LfRfidHidFormat* lfrfid_manual_format_hid(uint32_t format) {
    if(format < LFRFID_MANUAL_FORMAT_HID) {
        return NULL;
    }
    return lfrfid_hid_format_get(format - LFRFID_MANUAL_FORMAT_HID);
}

static size_t lfrfid_manual_format_hid_fields_count(const LfRfidHidFormat* hid_format) {
    return lfrfid_hid_format_has_facility_code(hid_format) ? 2 : 1;
}

static bool lfrfid_manual_format_hid_field(
    const LfRfidHidFormat* hid_format,
    size_t index,
    LfRfidManualFormatField* field) {
    if(index >= lfrfid_manual_format_hid_fields_count(hid_format)) {
        return false;
    }

    field->min = 0;
    if(lfrfid_hid_format_has_facility_code(hid_format) && index == 0) {
        field->name = "Facility Code";
        field->max = lfrfid_hid_format_get_facility_code_max(hid_format);
    } else {
        field->name = "Card Number";
        field->max = lfrfid_hid_format_get_card_number_max(hid_format);
    }
    return true;
}

static const LfRfidManualFormatDescriptor* lfrfid_manual_format_descriptor_get(uint32_t format) {
    const LfRfidManualFormatDescriptor* desc = NULL;
    ProtocolId protocol_id = lfrfid_manual_format_protocol(format);

    if(format == LFRFID_MANUAL_FORMAT_CASI) {
        desc = &lfrfid_manual_format_descriptor_casi;
    } else if(format < LFRFIDProtocolMax && lfrfid_manual_format_descriptors[format].fields) {
        desc = &lfrfid_manual_format_descriptors[format];
    }

    if(desc) {
        // the encoders write fixed offsets into a buffer of the protocol's size
        furi_check(desc->data_size == lfrfid_protocols[protocol_id]->data_size);
    }

    return desc;
}

void lfrfid_manual_format_get_label(uint32_t format, FuriString* label) {
    furi_check(label);

    const LfRfidHidFormat* hid_format = lfrfid_manual_format_hid(format);
    if(format < LFRFIDProtocolMax) {
        // manufacturer and name, unless the two would repeat each other
        const char* manufacturer = lfrfid_protocols[format]->manufacturer;
        const char* name = lfrfid_protocols[format]->name;
        if(strcmp(manufacturer, name) != 0 && strcmp(manufacturer, "N/A") != 0) {
            furi_string_printf(label, "%s %s", manufacturer, name);
        } else {
            furi_string_set(label, name);
        }
    } else if(format == LFRFID_MANUAL_FORMAT_CASI) {
        furi_string_set(label, "Casi-Rusco C10106");
    } else if(hid_format) {
        furi_string_printf(label, "HID %s", lfrfid_hid_format_get_name(hid_format));
    } else {
        furi_string_reset(label);
    }
}

ProtocolId lfrfid_manual_format_protocol(uint32_t format) {
    if(format < LFRFIDProtocolMax) {
        return format;
    }
    if(format == LFRFID_MANUAL_FORMAT_CASI) {
        // the clock real badges decode at
        return LFRFIDProtocolEM4100_32;
    }
    return lfrfid_manual_format_hid(format) ? LFRFIDProtocolHidGeneric : PROTOCOL_NO;
}

size_t lfrfid_manual_format_fields_count(uint32_t format) {
    const LfRfidHidFormat* hid_format = lfrfid_manual_format_hid(format);
    if(hid_format) {
        return lfrfid_manual_format_hid_fields_count(hid_format);
    }

    const LfRfidManualFormatDescriptor* desc = lfrfid_manual_format_descriptor_get(format);
    return desc ? desc->fields_count : 0;
}

bool lfrfid_manual_format_field(uint32_t format, size_t index, LfRfidManualFormatField* field) {
    furi_check(field);

    const LfRfidHidFormat* hid_format = lfrfid_manual_format_hid(format);
    if(hid_format) {
        return lfrfid_manual_format_hid_field(hid_format, index, field);
    }

    const LfRfidManualFormatDescriptor* desc = lfrfid_manual_format_descriptor_get(format);
    if(!desc || index >= desc->fields_count) {
        return false;
    }
    *field = desc->fields[index];
    return true;
}

bool lfrfid_manual_format_encode(
    uint32_t format,
    const uint64_t* values,
    size_t values_count,
    uint8_t* data,
    size_t data_size) {
    furi_check(values);
    furi_check(data);

    const size_t count = lfrfid_manual_format_fields_count(format);
    if(count == 0 || values_count != count) {
        return false;
    }

    for(size_t i = 0; i < count; i++) {
        LfRfidManualFormatField field;
        if(!lfrfid_manual_format_field(format, i, &field) || values[i] < field.min ||
           values[i] > field.max) {
            return false;
        }
    }

    const LfRfidHidFormat* hid_format = lfrfid_manual_format_hid(format);
    if(hid_format) {
        if(data_size < lfrfid_protocols[LFRFIDProtocolHidGeneric]->data_size) {
            return false;
        }
        // the card number is the only field of a format without a facility code
        const bool has_fc = lfrfid_hid_format_has_facility_code(hid_format);
        lfrfid_hid_format_encode(hid_format, has_fc ? values[0] : 0, values[has_fc ? 1 : 0], data);
        return true;
    }

    const LfRfidManualFormatDescriptor* desc = lfrfid_manual_format_descriptor_get(format);
    if(data_size < desc->data_size) {
        return false;
    }

    memset(data, 0, desc->data_size);
    desc->encode(values, data);

    return true;
}

void lfrfid_manual_format_render(ProtocolId protocol_id, const uint8_t* data, FuriString* result) {
    furi_check(data);
    furi_check(result);

    if(protocol_id == LFRFIDProtocolHidGeneric) {
        lfrfid_hid_format_render(data, result);
    } else if(
        protocol_id == LFRFIDProtocolEM4100 || protocol_id == LFRFIDProtocolEM4100_32 ||
        protocol_id == LFRFIDProtocolEM4100_16) {
        // a Casi-Rusco badge is an EM4100 frame (RF/32 on every badge seen, but the same
        // bits at any clock), so its reading goes alongside the EM4100 one
        lfrfid_casi_format_render(data, result);
    }
}
