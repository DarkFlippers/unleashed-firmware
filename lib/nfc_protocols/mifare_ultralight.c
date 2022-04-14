#include "mifare_ultralight.h"
#include <furi.h>
#include <furi_hal_nfc.h>

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
    MfUltralightVersion* version = (MfUltralightVersion*)buff;
    memcpy(&mf_ul_read->data.version, version, sizeof(MfUltralightVersion));
    if(version->storage_size == 0x0B || version->storage_size == 0x00) {
        mf_ul_read->data.type = MfUltralightTypeUL11;
        mf_ul_read->pages_to_read = 20;
        mf_ul_read->support_fast_read = true;
    } else if(version->storage_size == 0x0E) {
        mf_ul_read->data.type = MfUltralightTypeUL21;
        mf_ul_read->pages_to_read = 41;
        mf_ul_read->support_fast_read = true;
    } else if(version->storage_size == 0x0F) {
        mf_ul_read->data.type = MfUltralightTypeNTAG213;
        mf_ul_read->pages_to_read = 45;
        mf_ul_read->support_fast_read = false;
    } else if(version->storage_size == 0x11) {
        mf_ul_read->data.type = MfUltralightTypeNTAG215;
        mf_ul_read->pages_to_read = 135;
        mf_ul_read->support_fast_read = false;
    } else if(version->storage_size == 0x13) {
        mf_ul_read->data.type = MfUltralightTypeNTAG216;
        mf_ul_read->pages_to_read = 231;
        mf_ul_read->support_fast_read = false;
    } else {
        mf_ul_set_default_version(mf_ul_read);
    }
}

void mf_ul_set_default_version(MifareUlDevice* mf_ul_read) {
    mf_ul_read->data.type = MfUltralightTypeUnknown;
    mf_ul_read->pages_to_read = 16;
    mf_ul_read->support_fast_read = false;
}

uint16_t mf_ul_prepare_read(uint8_t* dest, uint8_t start_page) {
    dest[0] = MF_UL_READ_CMD;
    dest[1] = start_page;
    return 2;
}

void mf_ul_parse_read_response(uint8_t* buff, uint16_t page_addr, MifareUlDevice* mf_ul_read) {
    uint8_t pages_read = 4;
    uint8_t page_read_count = mf_ul_read->pages_read + pages_read;
    if(page_read_count > mf_ul_read->pages_to_read) {
        pages_read -= page_read_count - mf_ul_read->pages_to_read;
    }
    mf_ul_read->pages_read += pages_read;
    mf_ul_read->data.data_size = mf_ul_read->pages_read * 4;
    memcpy(&mf_ul_read->data.data[page_addr * 4], buff, pages_read * 4);
}

uint16_t mf_ul_prepare_fast_read(uint8_t* dest, uint8_t start_page, uint8_t end_page) {
    dest[0] = MF_UL_FAST_READ_CMD;
    dest[1] = start_page;
    dest[2] = end_page;
    return 3;
}

void mf_ul_parse_fast_read_response(
    uint8_t* buff,
    uint8_t start_page,
    uint8_t end_page,
    MifareUlDevice* mf_ul_read) {
    mf_ul_read->pages_read = end_page - start_page + 1;
    mf_ul_read->data.data_size = mf_ul_read->pages_read * 4;
    memcpy(mf_ul_read->data.data, buff, mf_ul_read->data.data_size);
}

uint16_t mf_ul_prepare_read_signature(uint8_t* dest) {
    dest[0] = MF_UL_READ_SIG;
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
    dest[2] = (uint8_t)value;
    dest[3] = (uint8_t)(value >> 8);
    dest[4] = (uint8_t)(value >> 16);
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

void mf_ul_parse_check_tearing_response(
    uint8_t* buff,
    uint8_t cnt_index,
    MifareUlDevice* mf_ul_read) {
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
    dest[2] = (uint8_t)(data >> 24);
    dest[3] = (uint8_t)(data >> 16);
    dest[4] = (uint8_t)(data >> 8);
    dest[5] = (uint8_t)data;
    return 6;
}

void mf_ul_prepare_emulation(MifareUlDevice* mf_ul_emulate, MifareUlData* data) {
    mf_ul_emulate->data = *data;
    mf_ul_emulate->auth_data = NULL;
    mf_ul_emulate->data_changed = false;
    mf_ul_emulate->comp_write_cmd_started = false;
    if(data->version.storage_size == 0) {
        mf_ul_emulate->data.type = MfUltralightTypeUnknown;
        mf_ul_emulate->support_fast_read = false;
    } else if(data->version.storage_size == 0x0B) {
        mf_ul_emulate->data.type = MfUltralightTypeUL11;
        mf_ul_emulate->support_fast_read = true;
    } else if(data->version.storage_size == 0x0E) {
        mf_ul_emulate->data.type = MfUltralightTypeUL21;
        mf_ul_emulate->support_fast_read = true;
    } else if(data->version.storage_size == 0x0F) {
        mf_ul_emulate->data.type = MfUltralightTypeNTAG213;
        mf_ul_emulate->support_fast_read = true;
    } else if(data->version.storage_size == 0x11) {
        mf_ul_emulate->data.type = MfUltralightTypeNTAG215;
        mf_ul_emulate->support_fast_read = true;
    } else if(data->version.storage_size == 0x13) {
        mf_ul_emulate->data.type = MfUltralightTypeNTAG216;
        mf_ul_emulate->support_fast_read = true;
    }

    if(mf_ul_emulate->data.type >= MfUltralightTypeNTAG213) {
        uint16_t pwd_page = (data->data_size / 4) - 2;
        mf_ul_emulate->auth_data = (MifareUlAuthData*)&data->data[pwd_page * 4];
    }
}

void mf_ul_protect_auth_data_on_read_command(
    uint8_t* tx_buff,
    uint8_t start_page,
    uint8_t end_page,
    MifareUlDevice* mf_ul_emulate) {
    if(mf_ul_emulate->data.type >= MfUltralightTypeNTAG213) {
        uint8_t pwd_page = (mf_ul_emulate->data.data_size / 4) - 2;
        uint8_t pack_page = pwd_page + 1;
        if((start_page <= pwd_page) && (end_page >= pwd_page)) {
            memset(&tx_buff[(pwd_page - start_page) * 4], 0, 4);
        }
        if((start_page <= pack_page) && (end_page >= pack_page)) {
            memset(&tx_buff[(pack_page - start_page) * 4], 0, 2);
        }
    }
}

bool mf_ul_prepare_emulation_response(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* data_type,
    void* context) {
    furi_assert(context);
    MifareUlDevice* mf_ul_emulate = context;
    uint8_t cmd = buff_rx[0];
    uint16_t page_num = mf_ul_emulate->data.data_size / 4;
    uint16_t tx_bytes = 0;
    uint16_t tx_bits = 0;
    bool command_parsed = false;

    // Check composite commands
    if(mf_ul_emulate->comp_write_cmd_started) {
        // Compatibility write is the only one composit command
        if(buff_rx_len == 16) {
            memcpy(&mf_ul_emulate->data.data[mf_ul_emulate->comp_write_page_addr * 4], buff_rx, 4);
            mf_ul_emulate->data_changed = true;
            // Send ACK message
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
        mf_ul_emulate->comp_write_cmd_started = false;
    } else if(cmd == MF_UL_GET_VERSION_CMD) {
        if(mf_ul_emulate->data.type != MfUltralightTypeUnknown) {
            tx_bytes = sizeof(mf_ul_emulate->data.version);
            memcpy(buff_tx, &mf_ul_emulate->data.version, tx_bytes);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_READ_CMD) {
        uint8_t start_page = buff_rx[1];
        if(start_page < page_num) {
            tx_bytes = 16;
            if(start_page + 4 > page_num) {
                // Handle roll-over mechanism
                uint8_t end_pages_num = page_num - start_page;
                memcpy(buff_tx, &mf_ul_emulate->data.data[start_page * 4], end_pages_num * 4);
                memcpy(
                    &buff_tx[end_pages_num * 4],
                    mf_ul_emulate->data.data,
                    (4 - end_pages_num) * 4);
            } else {
                memcpy(buff_tx, &mf_ul_emulate->data.data[start_page * 4], tx_bytes);
            }
            mf_ul_protect_auth_data_on_read_command(
                buff_tx, start_page, (start_page + 4), mf_ul_emulate);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_FAST_READ_CMD) {
        if(mf_ul_emulate->support_fast_read) {
            uint8_t start_page = buff_rx[1];
            uint8_t end_page = buff_rx[2];
            if((start_page < page_num) && (end_page < page_num) && (start_page < (end_page + 1))) {
                tx_bytes = ((end_page + 1) - start_page) * 4;
                memcpy(buff_tx, &mf_ul_emulate->data.data[start_page * 4], tx_bytes);
                mf_ul_protect_auth_data_on_read_command(
                    buff_tx, start_page, end_page, mf_ul_emulate);
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_WRITE) {
        uint8_t write_page = buff_rx[1];
        if((write_page > 1) && (write_page < page_num - 2)) {
            memcpy(&mf_ul_emulate->data.data[write_page * 4], &buff_rx[2], 4);
            mf_ul_emulate->data_changed = true;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_COMP_WRITE) {
        uint8_t write_page = buff_rx[1];
        if((write_page > 1) && (write_page < page_num - 2)) {
            mf_ul_emulate->comp_write_cmd_started = true;
            mf_ul_emulate->comp_write_page_addr = write_page;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_READ_CNT) {
        uint8_t cnt_num = buff_rx[1];
        if(cnt_num < 3) {
            buff_tx[0] = mf_ul_emulate->data.counter[cnt_num] >> 16;
            buff_tx[1] = mf_ul_emulate->data.counter[cnt_num] >> 8;
            buff_tx[2] = mf_ul_emulate->data.counter[cnt_num];
            tx_bytes = 3;
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_INC_CNT) {
        uint8_t cnt_num = buff_rx[1];
        uint32_t inc = (buff_rx[2] | (buff_rx[3] << 8) | (buff_rx[4] << 16));
        if((cnt_num < 3) && (mf_ul_emulate->data.counter[cnt_num] + inc < 0x00FFFFFF)) {
            mf_ul_emulate->data.counter[cnt_num] += inc;
            mf_ul_emulate->data_changed = true;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_AUTH) {
        if(mf_ul_emulate->data.type >= MfUltralightTypeNTAG213) {
            if(memcmp(&buff_rx[1], mf_ul_emulate->auth_data->pwd, 4) == 0) {
                buff_tx[0] = mf_ul_emulate->auth_data->pack.raw[0];
                buff_tx[1] = mf_ul_emulate->auth_data->pack.raw[1];
                tx_bytes = 2;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            } else if(!mf_ul_emulate->auth_data->pack.value) {
                buff_tx[0] = 0x80;
                buff_tx[1] = 0x80;
                tx_bytes = 2;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_READ_SIG) {
        // Check 2nd byte = 0x00 - RFU
        if(buff_rx[1] == 0x00) {
            tx_bytes = sizeof(mf_ul_emulate->data.signature);
            memcpy(buff_tx, mf_ul_emulate->data.signature, tx_bytes);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_CHECK_TEARING) {
        uint8_t cnt_num = buff_rx[1];
        if(cnt_num < 3) {
            buff_tx[0] = mf_ul_emulate->data.tearing[cnt_num];
            tx_bytes = 1;
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_HALT_START) {
        tx_bits = 0;
        command_parsed = true;
    }

    if(!command_parsed) {
        // Send NACK
        buff_tx[0] = 0x00;
        tx_bits = 4;
        *data_type = FURI_HAL_NFC_TXRX_RAW;
    }
    // Return tx buffer size in bits
    if(tx_bytes) {
        tx_bits = tx_bytes * 8;
    }
    *buff_tx_len = tx_bits;
    return tx_bits > 0;
}
