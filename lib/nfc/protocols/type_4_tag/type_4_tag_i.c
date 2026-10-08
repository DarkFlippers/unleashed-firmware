#include "type_4_tag_i.h"

#include <bit_lib/bit_lib.h>

#define TAG "Type4Tag"

#define TYPE_4_TAG_FFF_NDEF_DATA_SIZE_KEY "NDEF Data Size"
#define TYPE_4_TAG_FFF_NDEF_DATA_KEY      "NDEF Data"

#define TYPE_4_TAG_FFF_NDEF_DATA_PER_LINE (16U)

const uint8_t type_4_tag_iso_mf_name[TYPE_4_TAG_ISO_NAME_LEN] = {TYPE_4_TAG_ISO_MF_NAME};
const uint8_t type_4_tag_iso_df_name[TYPE_4_TAG_ISO_NAME_LEN] = {TYPE_4_TAG_ISO_DF_NAME};

Type4TagError type_4_tag_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return Type4TagErrorNone;
    case Iso14443_4aErrorNotPresent:
        return Type4TagErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return Type4TagErrorTimeout;
    default:
        return Type4TagErrorProtocol;
    }
}

void type_4_tag_cc_dump(const Type4TagData* data, uint8_t* buf, size_t len) {
    furi_check(len >= TYPE_4_TAG_T4T_CC_MIN_SIZE);
    Type4TagCc* cc = (Type4TagCc*)buf;

    bit_lib_num_to_bytes_be(TYPE_4_TAG_T4T_CC_MIN_SIZE, sizeof(cc->len), (void*)&cc->len);
    cc->t4t_vno = TYPE_4_TAG_T4T_CC_VNO;
    bit_lib_num_to_bytes_be(
        data->is_tag_specific ? MIN(data->chunk_max_read, TYPE_4_TAG_CHUNK_LEN) :
                                TYPE_4_TAG_CHUNK_LEN,
        sizeof(cc->mle),
        (void*)&cc->mle);
    bit_lib_num_to_bytes_be(
        data->is_tag_specific ? MIN(data->chunk_max_write, TYPE_4_TAG_CHUNK_LEN) :
                                TYPE_4_TAG_CHUNK_LEN,
        sizeof(cc->mlc),
        (void*)&cc->mlc);

    cc->tlv[0].type = Type4TagCcTlvTypeNdefFileCtrl;
    cc->tlv[0].len = sizeof(cc->tlv[0].value.ndef_file_ctrl);

    bit_lib_num_to_bytes_be(
        data->is_tag_specific ? data->ndef_file_id : TYPE_4_TAG_T4T_NDEF_EF_ID,
        sizeof(cc->tlv[0].value.ndef_file_ctrl.file_id),
        (void*)&cc->tlv[0].value.ndef_file_ctrl.file_id);
    bit_lib_num_to_bytes_be(
        sizeof(uint16_t) +
            (data->is_tag_specific ? data->ndef_max_len : TYPE_4_TAG_DEFAULT_NDEF_SIZE),
        sizeof(cc->tlv[0].value.ndef_file_ctrl.max_len),
        (void*)&cc->tlv[0].value.ndef_file_ctrl.max_len);
    cc->tlv[0].value.ndef_file_ctrl.read_perm =
        data->is_tag_specific ? data->ndef_read_lock : TYPE_4_TAG_T4T_CC_RW_LOCK_NONE;
    cc->tlv[0].value.ndef_file_ctrl.write_perm =
        data->is_tag_specific ? data->ndef_write_lock : TYPE_4_TAG_T4T_CC_RW_LOCK_NONE;
}

Type4TagError type_4_tag_cc_parse(Type4TagData* data, const uint8_t* buf, size_t len) {
    if(len < TYPE_4_TAG_T4T_CC_MIN_SIZE) {
        FURI_LOG_E(TAG, "Unsupported T4T version");
        return Type4TagErrorWrongFormat;
    }

    const Type4TagCc* cc = (const Type4TagCc*)buf;
    if(cc->t4t_vno != TYPE_4_TAG_T4T_CC_VNO) {
        FURI_LOG_E(TAG, "Unsupported T4T version");
        return Type4TagErrorNotSupported;
    }

    const Type4TagCcTlv* tlv = cc->tlv;
    const Type4TagCcTlvNdefFileCtrl* ndef_file_ctrl = NULL;
    const void* end = MIN((void*)cc + cc->len, (void*)cc + len);
    while((void*)tlv < end) {
        if(tlv->type == Type4TagCcTlvTypeNdefFileCtrl) {
            ndef_file_ctrl = &tlv->value.ndef_file_ctrl;
            break;
        }

        if(tlv->len < 0xFF) {
            tlv = (void*)&tlv->value + tlv->len;
        } else {
            uint16_t len = bit_lib_bytes_to_num_be((void*)&tlv->len + 1, sizeof(uint16_t));
            tlv = (void*)&tlv->value + sizeof(len) + len;
        }
    }
    if(!ndef_file_ctrl) {
        FURI_LOG_E(TAG, "No NDEF file ctrl TLV");
        return Type4TagErrorWrongFormat;
    }

    data->t4t_version.value = cc->t4t_vno;
    data->chunk_max_read = bit_lib_bytes_to_num_be((void*)&cc->mle, sizeof(cc->mle));
    data->chunk_max_write = bit_lib_bytes_to_num_be((void*)&cc->mlc, sizeof(cc->mlc));
    data->ndef_file_id =
        bit_lib_bytes_to_num_be((void*)&ndef_file_ctrl->file_id, sizeof(ndef_file_ctrl->file_id));
    data->ndef_max_len =
        bit_lib_bytes_to_num_be((void*)&ndef_file_ctrl->max_len, sizeof(ndef_file_ctrl->max_len)) -
        sizeof(uint16_t);
    data->ndef_read_lock = ndef_file_ctrl->read_perm;
    data->ndef_write_lock = ndef_file_ctrl->write_perm;

    return Type4TagErrorNone;
}

bool type_4_tag_ndef_data_load(Type4TagData* data, FlipperFormat* ff) {
    uint32_t ndef_data_size;
    if(!flipper_format_read_uint32(ff, TYPE_4_TAG_FFF_NDEF_DATA_SIZE_KEY, &ndef_data_size, 1)) {
        return false;
    }
    if(ndef_data_size == 0) {
        return true;
    }

    simple_array_init(data->ndef_data, ndef_data_size);

    uint32_t ndef_data_pos = 0;
    uint8_t* ndef_data = simple_array_get_data(data->ndef_data);
    while(ndef_data_size > 0) {
        uint8_t ndef_line_size = MIN(ndef_data_size, TYPE_4_TAG_FFF_NDEF_DATA_PER_LINE);

        if(!flipper_format_read_hex(
               ff, TYPE_4_TAG_FFF_NDEF_DATA_KEY, &ndef_data[ndef_data_pos], ndef_line_size)) {
            simple_array_reset(data->ndef_data);
            return false;
        }

        ndef_data_pos += ndef_line_size;
        ndef_data_size -= ndef_line_size;
    }

    return true;
}

bool type_4_tag_ndef_data_save(const Type4TagData* data, FlipperFormat* ff) {
    uint32_t ndef_data_size = simple_array_get_count(data->ndef_data);
    if(!flipper_format_write_uint32(ff, TYPE_4_TAG_FFF_NDEF_DATA_SIZE_KEY, &ndef_data_size, 1)) {
        return false;
    }
    if(ndef_data_size == 0) {
        return true;
    }

    uint32_t ndef_data_pos = 0;
    uint8_t* ndef_data = simple_array_get_data(data->ndef_data);
    while(ndef_data_size > 0) {
        uint8_t ndef_line_size = MIN(ndef_data_size, TYPE_4_TAG_FFF_NDEF_DATA_PER_LINE);

        if(!flipper_format_write_hex(
               ff, TYPE_4_TAG_FFF_NDEF_DATA_KEY, &ndef_data[ndef_data_pos], ndef_line_size)) {
            return false;
        }

        ndef_data_pos += ndef_line_size;
        ndef_data_size -= ndef_line_size;
    }

    return true;
}
