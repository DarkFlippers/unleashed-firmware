#include "mifare_ultralight.h"
#include <furi.h>

#define TAG "MfUltralight"

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    if((ATQA0 == 0x44) && (ATQA1 == 0x00) && (SAK == 0x00)) {
        return true;
    }
    return false;
}

static void mf_ul_set_default_version(MfUltralightReader* reader, MfUltralightData* data) {
    data->type = MfUltralightTypeUnknown;
    reader->pages_to_read = 16;
    reader->support_fast_read = false;
    reader->support_tearing_flags = false;
    reader->support_counters = false;
}

bool mf_ultralight_read_version(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data) {
    bool version_read = false;

    do {
        FURI_LOG_D(TAG, "Reading version");
        tx_rx->tx_data[0] = MF_UL_GET_VERSION_CMD;
        tx_rx->tx_bits = 8;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(TAG, "Failed reading version");
            mf_ul_set_default_version(reader, data);
            furi_hal_nfc_sleep();
            furi_hal_nfc_activate_nfca(300, NULL);
            break;
        }
        MfUltralightVersion* version = (MfUltralightVersion*)tx_rx->rx_data;
        data->version = *version;
        if(version->storage_size == 0x0B || version->storage_size == 0x00) {
            data->type = MfUltralightTypeUL11;
            reader->pages_to_read = 20;
            reader->support_fast_read = true;
            reader->support_tearing_flags = true;
            reader->support_counters = true;
        } else if(version->storage_size == 0x0E) {
            data->type = MfUltralightTypeUL21;
            reader->pages_to_read = 41;
            reader->support_fast_read = true;
            reader->support_tearing_flags = true;
            reader->support_counters = true;
        } else if(version->storage_size == 0x0F) {
            data->type = MfUltralightTypeNTAG213;
            reader->pages_to_read = 45;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
        } else if(version->storage_size == 0x11) {
            data->type = MfUltralightTypeNTAG215;
            reader->pages_to_read = 135;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
        } else if(version->storage_size == 0x13) {
            data->type = MfUltralightTypeNTAG216;
            reader->pages_to_read = 231;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
        } else {
            mf_ul_set_default_version(reader, data);
            break;
        }
        version_read = true;
    } while(false);

    return version_read;
}

bool mf_ultralight_read_pages(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data) {
    uint8_t pages_read_cnt = 0;

    for(size_t i = 0; i < reader->pages_to_read; i += 4) {
        FURI_LOG_D(TAG, "Reading pages %d - %d", i, i + 3);
        tx_rx->tx_data[0] = MF_UL_READ_CMD;
        tx_rx->tx_data[1] = i;
        tx_rx->tx_bits = 16;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(TAG, "Failed to read pages %d - %d", i, i + 3);
            break;
        }
        if(i + 4 <= reader->pages_to_read) {
            pages_read_cnt = 4;
        } else {
            pages_read_cnt = reader->pages_to_read - reader->pages_read;
        }
        reader->pages_read += pages_read_cnt;
        data->data_size = reader->pages_read * 4;
        memcpy(&data->data[i * 4], tx_rx->rx_data, pages_read_cnt * 4);
    }

    return reader->pages_read == reader->pages_to_read;
}

bool mf_ultralight_fast_read_pages(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data) {
    FURI_LOG_D(TAG, "Reading pages 0 - %d", reader->pages_to_read);
    tx_rx->tx_data[0] = MF_UL_FAST_READ_CMD;
    tx_rx->tx_data[1] = 0;
    tx_rx->tx_data[2] = reader->pages_to_read - 1;
    tx_rx->tx_bits = 24;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
    if(furi_hal_nfc_tx_rx(tx_rx, 50)) {
        reader->pages_read = reader->pages_to_read;
        data->data_size = reader->pages_read * 4;
        memcpy(data->data, tx_rx->rx_data, data->data_size);
    } else {
        FURI_LOG_D(TAG, "Failed to read pages 0 - %d", reader->pages_to_read);
    }

    return reader->pages_read == reader->pages_to_read;
}

bool mf_ultralight_read_signature(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data) {
    bool signature_read = false;

    FURI_LOG_D(TAG, "Reading signature");
    tx_rx->tx_data[0] = MF_UL_READ_SIG;
    tx_rx->tx_data[1] = 0;
    tx_rx->tx_bits = 16;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
    if(furi_hal_nfc_tx_rx(tx_rx, 50)) {
        memcpy(data->signature, tx_rx->rx_data, sizeof(data->signature));
        signature_read = true;
    } else {
        FURI_LOG_D(TAG, "Failed redaing signature");
    }

    return signature_read;
}

bool mf_ultralight_read_counters(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data) {
    uint8_t counter_read = 0;

    FURI_LOG_D(TAG, "Reading counters");
    for(size_t i = 0; i < 3; i++) {
        tx_rx->tx_data[0] = MF_UL_READ_CNT;
        tx_rx->rx_data[1] = i;
        tx_rx->tx_bits = 16;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(TAG, "Failed to read %d counter", i);
            break;
        }
        data->counter[i] = (tx_rx->rx_data[2] << 16) | (tx_rx->rx_data[1] << 8) |
                           tx_rx->rx_data[0];
        counter_read++;
    }

    return counter_read == 2;
}

bool mf_ultralight_read_tearing_flags(FuriHalNfcTxRxContext* tx_rx, MfUltralightData* data) {
    uint8_t flag_read = 0;

    FURI_LOG_D(TAG, "Reading tearing flags");
    for(size_t i = 0; i < 3; i++) {
        tx_rx->tx_data[0] = MF_UL_CHECK_TEARING;
        tx_rx->rx_data[1] = i;
        tx_rx->tx_bits = 16;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(TAG, "Failed to read %d tearing flag", i);
            break;
        }
        data->tearing[i] = tx_rx->rx_data[0];
        flag_read++;
    }

    return flag_read == 2;
}

bool mf_ul_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data) {
    furi_assert(tx_rx);
    furi_assert(reader);
    furi_assert(data);

    bool card_read = false;

    // Read Mifare Ultralight version
    if(mf_ultralight_read_version(tx_rx, reader, data)) {
        // Read Signature
        mf_ultralight_read_signature(tx_rx, data);
    }
    if(reader->support_counters) {
        mf_ultralight_read_counters(tx_rx, data);
    }
    if(reader->support_tearing_flags) {
        mf_ultralight_read_tearing_flags(tx_rx, data);
    }
    card_read = mf_ultralight_read_pages(tx_rx, reader, data);

    return card_read;
}

// TODO rework
static void mf_ul_protect_auth_data_on_read_command(
    uint8_t* tx_buff,
    uint8_t start_page,
    uint8_t end_page,
    MfUltralightEmulator* emulator) {
    if(emulator->data.type >= MfUltralightTypeNTAG213) {
        uint8_t pwd_page = (emulator->data.data_size / 4) - 2;
        uint8_t pack_page = pwd_page + 1;
        if((start_page <= pwd_page) && (end_page >= pwd_page)) {
            memset(&tx_buff[(pwd_page - start_page) * 4], 0, 4);
        }
        if((start_page <= pack_page) && (end_page >= pack_page)) {
            memset(&tx_buff[(pack_page - start_page) * 4], 0, 2);
        }
    }
}

void mf_ul_prepare_emulation(MfUltralightEmulator* emulator, MfUltralightData* data) {
    emulator->data = *data;
    emulator->auth_data = NULL;
    emulator->data_changed = false;
    emulator->comp_write_cmd_started = false;
    if(data->type == MfUltralightTypeUnknown) {
        emulator->support_fast_read = false;
    } else if(data->type == MfUltralightTypeUL11) {
        emulator->support_fast_read = true;
    } else if(data->type == MfUltralightTypeUL21) {
        emulator->support_fast_read = true;
    } else if(data->type == MfUltralightTypeNTAG213) {
        emulator->support_fast_read = true;
    } else if(data->type == MfUltralightTypeNTAG215) {
        emulator->support_fast_read = true;
    } else if(data->type == MfUltralightTypeNTAG216) {
        emulator->support_fast_read = true;
    }

    if(data->type >= MfUltralightTypeNTAG213) {
        uint16_t pwd_page = (data->data_size / 4) - 2;
        emulator->auth_data = (MfUltralightAuth*)&data->data[pwd_page * 4];
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
    MfUltralightEmulator* emulator = context;
    uint8_t cmd = buff_rx[0];
    uint16_t page_num = emulator->data.data_size / 4;
    uint16_t tx_bytes = 0;
    uint16_t tx_bits = 0;
    bool command_parsed = false;

    // Check composite commands
    if(emulator->comp_write_cmd_started) {
        // Compatibility write is the only one composit command
        if(buff_rx_len == 16) {
            memcpy(&emulator->data.data[emulator->comp_write_page_addr * 4], buff_rx, 4);
            emulator->data_changed = true;
            // Send ACK message
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
        emulator->comp_write_cmd_started = false;
    } else if(cmd == MF_UL_GET_VERSION_CMD) {
        if(emulator->data.type != MfUltralightTypeUnknown) {
            tx_bytes = sizeof(emulator->data.version);
            memcpy(buff_tx, &emulator->data.version, tx_bytes);
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
                memcpy(buff_tx, &emulator->data.data[start_page * 4], end_pages_num * 4);
                memcpy(&buff_tx[end_pages_num * 4], emulator->data.data, (4 - end_pages_num) * 4);
            } else {
                memcpy(buff_tx, &emulator->data.data[start_page * 4], tx_bytes);
            }
            mf_ul_protect_auth_data_on_read_command(
                buff_tx, start_page, (start_page + 4), emulator);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_FAST_READ_CMD) {
        if(emulator->support_fast_read) {
            uint8_t start_page = buff_rx[1];
            uint8_t end_page = buff_rx[2];
            if((start_page < page_num) && (end_page < page_num) && (start_page < (end_page + 1))) {
                tx_bytes = ((end_page + 1) - start_page) * 4;
                memcpy(buff_tx, &emulator->data.data[start_page * 4], tx_bytes);
                mf_ul_protect_auth_data_on_read_command(buff_tx, start_page, end_page, emulator);
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_WRITE) {
        uint8_t write_page = buff_rx[1];
        if((write_page > 1) && (write_page < page_num - 2)) {
            memcpy(&emulator->data.data[write_page * 4], &buff_rx[2], 4);
            emulator->data_changed = true;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_COMP_WRITE) {
        uint8_t write_page = buff_rx[1];
        if((write_page > 1) && (write_page < page_num - 2)) {
            emulator->comp_write_cmd_started = true;
            emulator->comp_write_page_addr = write_page;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_READ_CNT) {
        uint8_t cnt_num = buff_rx[1];
        if(cnt_num < 3) {
            buff_tx[0] = emulator->data.counter[cnt_num] >> 16;
            buff_tx[1] = emulator->data.counter[cnt_num] >> 8;
            buff_tx[2] = emulator->data.counter[cnt_num];
            tx_bytes = 3;
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_INC_CNT) {
        uint8_t cnt_num = buff_rx[1];
        uint32_t inc = (buff_rx[2] | (buff_rx[3] << 8) | (buff_rx[4] << 16));
        if((cnt_num < 3) && (emulator->data.counter[cnt_num] + inc < 0x00FFFFFF)) {
            emulator->data.counter[cnt_num] += inc;
            emulator->data_changed = true;
            // ACK
            buff_tx[0] = 0x0A;
            tx_bits = 4;
            *data_type = FURI_HAL_NFC_TXRX_RAW;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_AUTH) {
        if(emulator->data.type >= MfUltralightTypeNTAG213) {
            if(memcmp(&buff_rx[1], emulator->auth_data->pwd, 4) == 0) {
                buff_tx[0] = emulator->auth_data->pack.raw[0];
                buff_tx[1] = emulator->auth_data->pack.raw[1];
                tx_bytes = 2;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            } else if(!emulator->auth_data->pack.value) {
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
            tx_bytes = sizeof(emulator->data.signature);
            memcpy(buff_tx, emulator->data.signature, tx_bytes);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_CHECK_TEARING) {
        uint8_t cnt_num = buff_rx[1];
        if(cnt_num < 3) {
            buff_tx[0] = emulator->data.tearing[cnt_num];
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
