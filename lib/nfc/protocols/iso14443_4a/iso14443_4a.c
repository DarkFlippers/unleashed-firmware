#include "iso14443_4a_i.h"

#include <furi.h>

#define ISO14443_4A_PROTOCOL_NAME "ISO14443-4A"
#define ISO14443_4A_DEVICE_NAME   "ISO14443-4A (Unknown)"

#define ISO14443_4A_T0_KEY    "T0"
#define ISO14443_4A_TA1_KEY   "TA(1)"
#define ISO14443_4A_TB1_KEY   "TB(1)"
#define ISO14443_4A_TC1_KEY   "TC(1)"
#define ISO14443_4A_T1_TK_KEY "T1...Tk"

#define ISO14443_4A_FDT_DEFAULT_FC ISO14443_3A_FDT_POLL_FC

typedef enum {
    Iso14443_4aInterfaceByteTA1,
    Iso14443_4aInterfaceByteTB1,
    Iso14443_4aInterfaceByteTC1,
} Iso14443_4aInterfaceByte;

const NfcDeviceBase nfc_device_iso14443_4a = {
    .protocol_name = ISO14443_4A_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)iso14443_4a_alloc,
    .free = (NfcDeviceFree)iso14443_4a_free,
    .reset = (NfcDeviceReset)iso14443_4a_reset,
    .copy = (NfcDeviceCopy)iso14443_4a_copy,
    .verify = (NfcDeviceVerify)iso14443_4a_verify,
    .load = (NfcDeviceLoad)iso14443_4a_load,
    .save = (NfcDeviceSave)iso14443_4a_save,
    .is_equal = (NfcDeviceEqual)iso14443_4a_is_equal,
    .get_name = (NfcDeviceGetName)iso14443_4a_get_device_name,
    .get_uid = (NfcDeviceGetUid)iso14443_4a_get_uid,
    .set_uid = (NfcDeviceSetUid)iso14443_4a_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)iso14443_4a_get_base_data,
};

Iso14443_4aData* iso14443_4a_alloc(void) {
    Iso14443_4aData* data = malloc(sizeof(Iso14443_4aData));

    data->iso14443_3a_data = iso14443_3a_alloc();
    data->ats_data.t1_tk = simple_array_alloc(&simple_array_config_uint8_t);

    return data;
}

void iso14443_4a_free(Iso14443_4aData* data) {
    furi_check(data);

    simple_array_free(data->ats_data.t1_tk);
    iso14443_3a_free(data->iso14443_3a_data);

    free(data);
}

void iso14443_4a_reset(Iso14443_4aData* data) {
    furi_check(data);

    iso14443_3a_reset(data->iso14443_3a_data);

    data->ats_data.tl = 1;
    data->ats_data.t0 = 0;
    data->ats_data.ta_1 = 0;
    data->ats_data.tb_1 = 0;
    data->ats_data.tc_1 = 0;

    simple_array_reset(data->ats_data.t1_tk);
}

void iso14443_4a_copy(Iso14443_4aData* data, const Iso14443_4aData* other) {
    furi_check(data);
    furi_check(other);

    iso14443_3a_copy(data->iso14443_3a_data, other->iso14443_3a_data);

    data->ats_data.tl = other->ats_data.tl;
    data->ats_data.t0 = other->ats_data.t0;
    data->ats_data.ta_1 = other->ats_data.ta_1;
    data->ats_data.tb_1 = other->ats_data.tb_1;
    data->ats_data.tc_1 = other->ats_data.tc_1;

    simple_array_copy(data->ats_data.t1_tk, other->ats_data.t1_tk);
}

bool iso14443_4a_verify(Iso14443_4aData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    // Empty, unified file format only
    return false;
}

bool iso14443_4a_load(Iso14443_4aData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;

    do {
        if(!iso14443_3a_load(data->iso14443_3a_data, ff, version)) break;

        Iso14443_4aAtsData* ats_data = &data->ats_data;

        ats_data->tl = 1;

        if(flipper_format_key_exist(ff, ISO14443_4A_T0_KEY)) {
            if(!flipper_format_read_hex(ff, ISO14443_4A_T0_KEY, &ats_data->t0, 1)) break;
            ++ats_data->tl;
        }

        if(ats_data->t0 & ISO14443_4A_ATS_T0_TA1) {
            if(!flipper_format_key_exist(ff, ISO14443_4A_TA1_KEY)) break;
            if(!flipper_format_read_hex(ff, ISO14443_4A_TA1_KEY, &ats_data->ta_1, 1)) break;
            ++ats_data->tl;
        }
        if(ats_data->t0 & ISO14443_4A_ATS_T0_TB1) {
            if(!flipper_format_key_exist(ff, ISO14443_4A_TB1_KEY)) break;
            if(!flipper_format_read_hex(ff, ISO14443_4A_TB1_KEY, &ats_data->tb_1, 1)) break;
            ++ats_data->tl;
        }
        if(ats_data->t0 & ISO14443_4A_ATS_T0_TC1) {
            if(!flipper_format_key_exist(ff, ISO14443_4A_TC1_KEY)) break;
            if(!flipper_format_read_hex(ff, ISO14443_4A_TC1_KEY, &ats_data->tc_1, 1)) break;
            ++ats_data->tl;
        }

        if(flipper_format_key_exist(ff, ISO14443_4A_T1_TK_KEY)) {
            uint32_t t1_tk_size;
            if(!flipper_format_get_value_count(ff, ISO14443_4A_T1_TK_KEY, &t1_tk_size)) break;

            if(t1_tk_size > 0) {
                simple_array_init(ats_data->t1_tk, t1_tk_size);
                if(!flipper_format_read_hex(
                       ff,
                       ISO14443_4A_T1_TK_KEY,
                       simple_array_get_data(ats_data->t1_tk),
                       t1_tk_size))
                    break;
                ats_data->tl += t1_tk_size;
            }
        }
        parsed = true;
    } while(false);

    return parsed;
}

bool iso14443_4a_save(const Iso14443_4aData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        if(!iso14443_3a_save(data->iso14443_3a_data, ff)) break;
        if(!flipper_format_write_comment_cstr(ff, ISO14443_4A_PROTOCOL_NAME " specific data"))
            break;

        const Iso14443_4aAtsData* ats_data = &data->ats_data;

        if(ats_data->tl > 1) {
            if(!flipper_format_write_hex(ff, ISO14443_4A_T0_KEY, &ats_data->t0, 1)) break;

            if(ats_data->t0 & ISO14443_4A_ATS_T0_TA1) {
                if(!flipper_format_write_hex(ff, ISO14443_4A_TA1_KEY, &ats_data->ta_1, 1)) break;
            }
            if(ats_data->t0 & ISO14443_4A_ATS_T0_TB1) {
                if(!flipper_format_write_hex(ff, ISO14443_4A_TB1_KEY, &ats_data->tb_1, 1)) break;
            }
            if(ats_data->t0 & ISO14443_4A_ATS_T0_TC1) {
                if(!flipper_format_write_hex(ff, ISO14443_4A_TC1_KEY, &ats_data->tc_1, 1)) break;
            }

            const uint32_t t1_tk_size = simple_array_get_count(ats_data->t1_tk);
            if(t1_tk_size > 0) {
                if(!flipper_format_write_hex(
                       ff,
                       ISO14443_4A_T1_TK_KEY,
                       simple_array_cget_data(ats_data->t1_tk),
                       t1_tk_size))
                    break;
            }
        }
        saved = true;
    } while(false);

    return saved;
}

bool iso14443_4a_is_equal(const Iso14443_4aData* data, const Iso14443_4aData* other) {
    furi_check(data);
    furi_check(other);

    return iso14443_3a_is_equal(data->iso14443_3a_data, other->iso14443_3a_data);
}

const char* iso14443_4a_get_device_name(const Iso14443_4aData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return ISO14443_4A_DEVICE_NAME;
}

const uint8_t* iso14443_4a_get_uid(const Iso14443_4aData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    return iso14443_3a_get_uid(data->iso14443_3a_data, uid_len);
}

bool iso14443_4a_set_uid(Iso14443_4aData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso14443_3a_set_uid(data->iso14443_3a_data, uid, uid_len);
}

Iso14443_3aData* iso14443_4a_get_base_data(const Iso14443_4aData* data) {
    furi_check(data);

    return data->iso14443_3a_data;
}

uint16_t iso14443_4a_get_frame_size_max(const Iso14443_4aData* data) {
    furi_check(data);

    const uint8_t fsci = data->ats_data.t0 & 0x0F;

    if(fsci < 5) {
        return fsci * 8 + 16;
    } else if(fsci == 5) {
        return 64;
    } else if(fsci == 6) {
        return 96;
    } else if(fsci < 13) {
        return 128U << (fsci - 7);
    } else {
        return 0;
    }
}

uint32_t iso14443_4a_get_fwt_fc_max(const Iso14443_4aData* data) {
    furi_check(data);

    uint32_t fwt_fc_max = ISO14443_4A_FDT_DEFAULT_FC;

    do {
        if(!(data->ats_data.tl > 1)) break;
        if(!(data->ats_data.t0 & ISO14443_4A_ATS_T0_TB1)) break;

        const uint8_t fwi = data->ats_data.tb_1 >> 4;
        if(fwi == 0x0F) break;

        fwt_fc_max = 4096UL << fwi;
    } while(false);

    return fwt_fc_max;
}

const uint8_t* iso14443_4a_get_historical_bytes(const Iso14443_4aData* data, uint32_t* count) {
    furi_check(data);
    furi_check(count);

    *count = simple_array_get_count(data->ats_data.t1_tk);
    const uint8_t* hist_bytes = NULL;
    if(*count > 0) {
        hist_bytes = simple_array_cget_data(data->ats_data.t1_tk);
    }

    return hist_bytes;
}

bool iso14443_4a_supports_bit_rate(const Iso14443_4aData* data, Iso14443_4aBitRate bit_rate) {
    furi_check(data);

    if(!(data->ats_data.t0 & ISO14443_4A_ATS_T0_TA1))
        return bit_rate == Iso14443_4aBitRateBoth106Kbit;

    const uint8_t ta_1 = data->ats_data.ta_1;

    switch(bit_rate) {
    case Iso14443_4aBitRateBoth106Kbit:
        return ta_1 == ISO14443_4A_ATS_TA1_BOTH_SAME_COMPULSORY;
    case Iso14443_4aBitRatePiccToPcd212Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PCD_TO_PICC_212KBIT;
    case Iso14443_4aBitRatePiccToPcd424Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PCD_TO_PICC_424KBIT;
    case Iso14443_4aBitRatePiccToPcd848Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PCD_TO_PICC_848KBIT;
    case Iso14443_4aBitRatePcdToPicc212Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PICC_TO_PCD_212KBIT;
    case Iso14443_4aBitRatePcdToPicc424Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PICC_TO_PCD_424KBIT;
    case Iso14443_4aBitRatePcdToPicc848Kbit:
        return ta_1 & ISO14443_4A_ATS_TA1_PICC_TO_PCD_848KBIT;
    default:
        return false;
    }
}

bool iso14443_4a_supports_frame_option(const Iso14443_4aData* data, Iso14443_4aFrameOption option) {
    furi_check(data);

    const Iso14443_4aAtsData* ats_data = &data->ats_data;
    if(!(ats_data->t0 & ISO14443_4A_ATS_T0_TC1)) return false;

    switch(option) {
    case Iso14443_4aFrameOptionNad:
        return ats_data->tc_1 & ISO14443_4A_ATS_TC1_NAD;
    case Iso14443_4aFrameOptionCid:
        return ats_data->tc_1 & ISO14443_4A_ATS_TC1_CID;
    default:
        return false;
    }
}
