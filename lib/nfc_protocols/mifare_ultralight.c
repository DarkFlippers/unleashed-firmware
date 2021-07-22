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

void mf_ul_parse_get_version_response(uint8_t* buff, MifareUlDevice* mf_ul_read) {
    MfUltralightVersion* version = (MfUltralightVersion*) buff;
    memcpy(&mf_ul_read->data.version, version, sizeof(MfUltralightVersion));
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

void mf_ul_set_default_version(MifareUlDevice* mf_ul_read) {
    mf_ul_read->type = MfUltralightTypeUnknown;
    mf_ul_read->pages_to_read = 16;
    mf_ul_read->support_fast_read = false;
}

uint16_t mf_ul_prepare_read(uint8_t* dest, uint8_t start_page) {
    dest[0] = MF_UL_READ_CMD;
    dest[1] = start_page;
    return 2;
}

void mf_ul_parse_read_response(uint8_t* buff, uint16_t page_addr, MifareUlDevice* mf_ul_read) {
    mf_ul_read->pages_readed += 4;
    mf_ul_read->data.data_size = mf_ul_read->pages_readed * 4;
    memcpy(&mf_ul_read->data.data[page_addr * 4], buff, 16);
}

uint16_t mf_ul_prepare_fast_read(uint8_t* dest, uint8_t start_page, uint8_t end_page) {
    dest[0] = MF_UL_FAST_READ_CMD;
    dest[1] = start_page;
    dest[2] = end_page;
    return 3;
}

void mf_ul_parse_fast_read_response(uint8_t* buff, uint8_t start_page, uint8_t end_page, MifareUlDevice* mf_ul_read) {
    mf_ul_read->pages_readed = end_page - start_page + 1;
    mf_ul_read->data.data_size = mf_ul_read->pages_readed * 4;
    memcpy(mf_ul_read->data.data, buff, mf_ul_read->data.data_size);
}

uint16_t mf_ul_prepare_read_signature(uint8_t* dest) {
    dest[0] = MF_UL_CHECK_TEARING;
    dest[1] = 0;
    return 2;
}

void mf_ul_parse_read_signature_response(uint8_t* buff, MifareUlDevice* mf_ul_read) {
    memcpy(mf_ul_read->data.signature, buff, sizeof(mf_ul_read->data.signature));
}

uint16_t mf_ul_prepare_read_cnt(uint8_t* dest, uint8_t cnt_index) {
    if(cnt_index > 2) {
        return 0;
    }
    dest[0] = MF_UL_READ_CNT;
    dest[1] = cnt_index;
    return 2;
}

void mf_ul_parse_read_cnt_response(uint8_t* buff, uint8_t cnt_index, MifareUlDevice* mf_ul_read) {
    // Reverse LSB sequence
    if(cnt_index < 3) {
        mf_ul_read->data.counter[cnt_index] = (buff[2] << 16) | (buff[1] << 8) | (buff[0]);
    }
}

uint16_t mf_ul_prepare_inc_cnt(uint8_t* dest, uint8_t cnt_index, uint32_t value) {
    if(cnt_index > 2) {
        return 0;
    }
    dest[0] = MF_UL_INC_CNT;
    dest[1] = cnt_index;
    dest[2] = (uint8_t) value;
    dest[3] = (uint8_t) (value >> 8);
    dest[4] = (uint8_t) (value >> 16);
    dest[5] = 0;
    return 6;
}

uint16_t mf_ul_prepare_check_tearing(uint8_t* dest, uint8_t cnt_index) {
    if(cnt_index > 2) {
        return 0;
    }
    dest[0] = MF_UL_CHECK_TEARING;
    dest[1] = cnt_index;
    return 2;
}

void mf_ul_parse_check_tearing_response(uint8_t* buff, uint8_t cnt_index, MifareUlDevice* mf_ul_read) {
    if(cnt_index < 2) {
        mf_ul_read->data.tearing[cnt_index] = buff[0];
    }
}

uint16_t mf_ul_prepare_write(uint8_t* dest, uint16_t page_addr, uint32_t data) {
    if(page_addr < 2) {
        return 0;
    }
    dest[0] = MF_UL_WRITE;
    dest[1] = page_addr;
    dest[2] = (uint8_t) (data >> 24);
    dest[3] = (uint8_t) (data >> 16);
    dest[4] = (uint8_t) (data >> 8);
    dest[5] = (uint8_t) data;
    return 6;
}
