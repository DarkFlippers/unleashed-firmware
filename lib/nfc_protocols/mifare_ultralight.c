#include "mifare_ultralight.h"

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    if((ATQA0 == 0x44) && (ATQA1 == 0x00) && (SAK == 0x00)) {
        return true;
    }
    return false;
}

uint16_t mf_ul_prepare_get_version(uint8_t* dest) {
    dest[0] = MF_UL_GET_VERSION_CMD;
    return 1;
}

void mf_ul_parse_get_version_response(uint8_t* buff, MfUltralightRead* mf_ul_read) {
    MfUltralightVersion* version = (MfUltralightVersion*) buff;
    if(version->storage_size == 0x0B || version->storage_size == 0x00) {
        mf_ul_read->type = MfUltralightTypeUL11;
        mf_ul_read->pages_to_read = 20;
        mf_ul_read->support_fast_read = true;
    } else if(version->storage_size == 0x0E) {
        mf_ul_read->type = MfUltralightTypeUL21;
        mf_ul_read->pages_to_read = 41;
        mf_ul_read->support_fast_read = true;
    } else if(version->storage_size == 0x0F) {
        mf_ul_read->type = MfUltralightTypeNTAG213;
        mf_ul_read->pages_to_read = 45;
        mf_ul_read->support_fast_read = false;
    } else if(version->storage_size == 0x11) {
        mf_ul_read->type = MfUltralightTypeNTAG215;
        mf_ul_read->pages_to_read = 135;
        mf_ul_read->support_fast_read = false;
    } else if(version->storage_size == 0x13) {
        mf_ul_read->type = MfUltralightTypeNTAG216;
        mf_ul_read->pages_to_read = 231;
        mf_ul_read->support_fast_read = false;
    } else {
        mf_ul_set_default_version(mf_ul_read);
    }
}

void mf_ul_set_default_version(MfUltralightRead* mf_ul_read) {
    mf_ul_read->type = MfUltralightTypeUnknown;
    mf_ul_read->pages_to_read = 20;
    mf_ul_read->support_fast_read = false;
}

uint16_t mf_ul_prepare_read(uint8_t* dest, uint8_t start_page) {
    dest[0] = MF_UL_READ_CMD;
    dest[1] = start_page;
    return 2;
}

uint16_t mf_ul_prepare_fast_read(uint8_t* dest, uint8_t start_page, uint8_t end_page) {
    dest[0] = MF_UL_FAST_READ_CMD;
    dest[1] = start_page;
    dest[2] = end_page;
    return 3;
}
