#include <limits.h>
#include "mifare_ultralight.h"
#include <furi.h>
#include <m-string.h>

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
    reader->support_signature = false;
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
            reader->support_signature = true;
        } else if(version->storage_size == 0x0E) {
            data->type = MfUltralightTypeUL21;
            reader->pages_to_read = 41;
            reader->support_fast_read = true;
            reader->support_tearing_flags = true;
            reader->support_counters = true;
            reader->support_signature = true;
        } else if(version->storage_size == 0x0F) {
            data->type = MfUltralightTypeNTAG213;
            reader->pages_to_read = 45;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
            reader->support_signature = true;
        } else if(version->storage_size == 0x11) {
            data->type = MfUltralightTypeNTAG215;
            reader->pages_to_read = 135;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
            reader->support_signature = true;
        } else if(version->prod_subtype == 5 && version->prod_ver_major == 2) {
            // NTAG I2C
            bool known = false;
            if(version->prod_ver_minor == 1) {
                if(version->storage_size == 0x13) {
                    data->type = MfUltralightTypeNTAGI2C1K;
                    reader->pages_to_read = 231;
                    reader->support_signature = false;
                    known = true;
                } else if(version->storage_size == 0x15) {
                    data->type = MfUltralightTypeNTAGI2C2K;
                    reader->pages_to_read = 485;
                    reader->support_signature = false;
                    known = true;
                }
            } else if(version->prod_ver_minor == 2) {
                if(version->storage_size == 0x13) {
                    data->type = MfUltralightTypeNTAGI2CPlus1K;
                    reader->pages_to_read = 236;
                    reader->support_signature = true;
                    known = true;
                } else if(version->storage_size == 0x15) {
                    data->type = MfUltralightTypeNTAGI2CPlus2K;
                    reader->pages_to_read = 492;
                    reader->support_signature = true;
                    known = true;
                }
            }

            if(known) {
                reader->support_fast_read = true;
                reader->support_tearing_flags = false;
                reader->support_counters = false;
            } else {
                mf_ul_set_default_version(reader, data);
            }
        } else if(version->storage_size == 0x13) {
            data->type = MfUltralightTypeNTAG216;
            reader->pages_to_read = 231;
            reader->support_fast_read = true;
            reader->support_tearing_flags = false;
            reader->support_counters = false;
            reader->support_signature = true;
        } else {
            mf_ul_set_default_version(reader, data);
            break;
        }
        version_read = true;
    } while(false);

    return version_read;
}

static int16_t mf_ultralight_ntag_i2c_addr_lin_to_tag_1k(
    int16_t linear_address,
    uint8_t* sector,
    int16_t* valid_pages) {
    // 0 - 226: sector 0
    // 227 - 228: config registers
    // 229 - 230: session registers

    if(linear_address > 230) {
        *valid_pages = 0;
        return -1;
    } else if(linear_address >= 229) {
        *sector = 3;
        *valid_pages = 2 - (linear_address - 229);
        return linear_address - 229 + 248;
    } else if(linear_address >= 227) {
        *sector = 0;
        *valid_pages = 2 - (linear_address - 227);
        return linear_address - 227 + 232;
    } else {
        *sector = 0;
        *valid_pages = 227 - linear_address;
        return linear_address;
    }
}

static int16_t mf_ultralight_ntag_i2c_addr_lin_to_tag_2k(
    int16_t linear_address,
    uint8_t* sector,
    int16_t* valid_pages) {
    // 0 - 255: sector 0
    // 256 - 480: sector 1
    // 481 - 482: config registers
    // 483 - 484: session registers

    if(linear_address > 484) {
        *valid_pages = 0;
        return -1;
    } else if(linear_address >= 483) {
        *sector = 3;
        *valid_pages = 2 - (linear_address - 483);
        return linear_address - 483 + 248;
    } else if(linear_address >= 481) {
        *sector = 1;
        *valid_pages = 2 - (linear_address - 481);
        return linear_address - 481 + 232;
    } else if(linear_address >= 256) {
        *sector = 1;
        *valid_pages = 225 - (linear_address - 256);
        return linear_address - 256;
    } else {
        *sector = 0;
        *valid_pages = 256 - linear_address;
        return linear_address;
    }
}

static int16_t mf_ultralight_ntag_i2c_addr_lin_to_tag_plus_1k(
    int16_t linear_address,
    uint8_t* sector,
    int16_t* valid_pages) {
    // 0 - 233: sector 0 + registers
    // 234 - 235: session registers

    if(linear_address > 235) {
        *valid_pages = 0;
        return -1;
    } else if(linear_address >= 234) {
        *sector = 0;
        *valid_pages = 2 - (linear_address - 234);
        return linear_address - 234 + 236;
    } else {
        *sector = 0;
        *valid_pages = 234 - linear_address;
        return linear_address;
    }
}

static int16_t mf_ultralight_ntag_i2c_addr_lin_to_tag_plus_2k(
    int16_t linear_address,
    uint8_t* sector,
    int16_t* valid_pages) {
    // 0 - 233: sector 0 + registers
    // 234 - 235: session registers
    // 236 - 491: sector 1

    if(linear_address > 491) {
        *valid_pages = 0;
        return -1;
    } else if(linear_address >= 236) {
        *sector = 1;
        *valid_pages = 256 - (linear_address - 236);
        return linear_address - 236;
    } else if(linear_address >= 234) {
        *sector = 0;
        *valid_pages = 2 - (linear_address - 234);
        return linear_address - 234 + 236;
    } else {
        *sector = 0;
        *valid_pages = 234 - linear_address;
        return linear_address;
    }
}

static int16_t mf_ultralight_ntag_i2c_addr_lin_to_tag(
    MfUltralightData* data,
    MfUltralightReader* reader,
    int16_t linear_address,
    uint8_t* sector,
    int16_t* valid_pages) {
    switch(data->type) {
    case MfUltralightTypeNTAGI2C1K:
        return mf_ultralight_ntag_i2c_addr_lin_to_tag_1k(linear_address, sector, valid_pages);

    case MfUltralightTypeNTAGI2C2K:
        return mf_ultralight_ntag_i2c_addr_lin_to_tag_2k(linear_address, sector, valid_pages);

    case MfUltralightTypeNTAGI2CPlus1K:
        return mf_ultralight_ntag_i2c_addr_lin_to_tag_plus_1k(linear_address, sector, valid_pages);

    case MfUltralightTypeNTAGI2CPlus2K:
        return mf_ultralight_ntag_i2c_addr_lin_to_tag_plus_2k(linear_address, sector, valid_pages);

    default:
        *sector = 0xff;
        *valid_pages = reader->pages_to_read - linear_address;
        return linear_address;
    }
}

static int16_t
    mf_ultralight_ntag_i2c_addr_tag_to_lin_1k(uint8_t page, uint8_t sector, uint16_t* valid_pages) {
    bool valid = false;
    int16_t translated_page;
    if(sector == 0) {
        if(page <= 226) {
            *valid_pages = 227 - page;
            translated_page = page;
            valid = true;
        } else if(page >= 232 && page <= 233) {
            *valid_pages = 2 - (page - 232);
            translated_page = page - 232 + 227;
            valid = true;
        }
    } else if(sector == 3) {
        if(page >= 248 && page <= 249) {
            *valid_pages = 2 - (page - 248);
            translated_page = page - 248 + 229;
            valid = true;
        }
    }

    if(!valid) {
        *valid_pages = 0;
        translated_page = -1;
    }
    return translated_page;
}

static int16_t
    mf_ultralight_ntag_i2c_addr_tag_to_lin_2k(uint8_t page, uint8_t sector, uint16_t* valid_pages) {
    bool valid = false;
    int16_t translated_page;
    if(sector == 0) {
        *valid_pages = 256 - page;
        translated_page = page;
        valid = true;
    } else if(sector == 1) {
        if(page <= 224) {
            *valid_pages = 225 - page;
            translated_page = 256 + page;
            valid = true;
        } else if(page >= 232 && page <= 233) {
            *valid_pages = 2 - (page - 232);
            translated_page = page - 232 + 481;
            valid = true;
        }
    } else if(sector == 3) {
        if(page >= 248 && page <= 249) {
            *valid_pages = 2 - (page - 248);
            translated_page = page - 248 + 483;
            valid = true;
        }
    }

    if(!valid) {
        *valid_pages = 0;
        translated_page = -1;
    }
    return translated_page;
}

static int16_t mf_ultralight_ntag_i2c_addr_tag_to_lin_plus_1k(
    uint8_t page,
    uint8_t sector,
    uint16_t* valid_pages) {
    bool valid = false;
    int16_t translated_page;
    if(sector == 0) {
        if(page <= 233) {
            *valid_pages = 234 - page;
            translated_page = page;
            valid = true;
        } else if(page >= 236 && page <= 237) {
            *valid_pages = 2 - (page - 236);
            translated_page = page - 236 + 234;
            valid = true;
        }
    } else if(sector == 3) {
        if(page >= 248 && page <= 249) {
            *valid_pages = 2 - (page - 248);
            translated_page = page - 248 + 234;
            valid = true;
        }
    }

    if(!valid) {
        *valid_pages = 0;
        translated_page = -1;
    }
    return translated_page;
}

static int16_t mf_ultralight_ntag_i2c_addr_tag_to_lin_plus_2k(
    uint8_t page,
    uint8_t sector,
    uint16_t* valid_pages) {
    bool valid = false;
    int16_t translated_page;
    if(sector == 0) {
        if(page <= 233) {
            *valid_pages = 234 - page;
            translated_page = page;
            valid = true;
        } else if(page >= 236 && page <= 237) {
            *valid_pages = 2 - (page - 236);
            translated_page = page - 236 + 234;
            valid = true;
        }
    } else if(sector == 1) {
        *valid_pages = 256 - page;
        translated_page = page + 236;
        valid = true;
    } else if(sector == 3) {
        if(page >= 248 && page <= 249) {
            *valid_pages = 2 - (page - 248);
            translated_page = page - 248 + 234;
            valid = true;
        }
    }

    if(!valid) {
        *valid_pages = 0;
        translated_page = -1;
    }
    return translated_page;
}

static int16_t mf_ultralight_ntag_i2c_addr_tag_to_lin(
    MfUltralightData* data,
    uint8_t page,
    uint8_t sector,
    uint16_t* valid_pages) {
    switch(data->type) {
    case MfUltralightTypeNTAGI2C1K:
        return mf_ultralight_ntag_i2c_addr_tag_to_lin_1k(page, sector, valid_pages);

    case MfUltralightTypeNTAGI2C2K:
        return mf_ultralight_ntag_i2c_addr_tag_to_lin_2k(page, sector, valid_pages);

    case MfUltralightTypeNTAGI2CPlus1K:
        return mf_ultralight_ntag_i2c_addr_tag_to_lin_plus_1k(page, sector, valid_pages);

    case MfUltralightTypeNTAGI2CPlus2K:
        return mf_ultralight_ntag_i2c_addr_tag_to_lin_plus_2k(page, sector, valid_pages);

    default:
        *valid_pages = data->data_size / 4 - page;
        return page;
    }
}

static bool mf_ultralight_sector_select(FuriHalNfcTxRxContext* tx_rx, uint8_t sector) {
    FURI_LOG_D(TAG, "Selecting sector %u", sector);
    tx_rx->tx_data[0] = MF_UL_SECTOR_SELECT;
    tx_rx->tx_data[1] = 0xff;
    tx_rx->tx_bits = 16;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
    if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
        FURI_LOG_D(TAG, "Failed to issue sector select command");
        return false;
    }

    tx_rx->tx_data[0] = sector;
    tx_rx->tx_data[1] = 0x00;
    tx_rx->tx_data[2] = 0x00;
    tx_rx->tx_data[3] = 0x00;
    tx_rx->tx_bits = 32;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
    // This is NOT a typo! The tag ACKs by not sending a response within 1ms.
    if(furi_hal_nfc_tx_rx(tx_rx, 20)) {
        // TODO: what gets returned when an actual NAK is received?
        FURI_LOG_D(TAG, "Sector %u select NAK'd", sector);
        return false;
    }

    return true;
}

bool mf_ultralight_read_pages(
    FuriHalNfcTxRxContext* tx_rx,
    MfUltralightReader* reader,
    MfUltralightData* data) {
    uint8_t pages_read_cnt = 0;
    uint8_t curr_sector_index = 0xff;
    reader->pages_read = 0;
    for(size_t i = 0; i < reader->pages_to_read; i += pages_read_cnt) {
        uint8_t tag_sector;
        int16_t valid_pages;
        int16_t tag_page = mf_ultralight_ntag_i2c_addr_lin_to_tag(
            data, reader, (int16_t)i, &tag_sector, &valid_pages);

        furi_assert(tag_page != -1);
        if(curr_sector_index != tag_sector) {
            if(!mf_ultralight_sector_select(tx_rx, tag_sector)) return false;
            curr_sector_index = tag_sector;
        }

        FURI_LOG_D(TAG, "Reading pages %d - %d", i, i + (valid_pages > 4 ? 4 : valid_pages) - 1);
        tx_rx->tx_data[0] = MF_UL_READ_CMD;
        tx_rx->tx_data[1] = tag_page;
        tx_rx->tx_bits = 16;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(
                TAG,
                "Failed to read pages %d - %d",
                i,
                i + (valid_pages > 4 ? 4 : valid_pages) - 1);
            break;
        }
        if(valid_pages > 4) {
            pages_read_cnt = 4;
        } else {
            pages_read_cnt = valid_pages;
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
    uint8_t curr_sector_index = 0xff;
    reader->pages_read = 0;
    while(reader->pages_read < reader->pages_to_read) {
        uint8_t tag_sector;
        int16_t valid_pages;
        int16_t tag_page = mf_ultralight_ntag_i2c_addr_lin_to_tag(
            data, reader, reader->pages_read, &tag_sector, &valid_pages);

        furi_assert(tag_page != -1);
        if(curr_sector_index != tag_sector) {
            if(!mf_ultralight_sector_select(tx_rx, tag_sector)) return false;
            curr_sector_index = tag_sector;
        }

        FURI_LOG_D(
            TAG, "Reading pages %d - %d", reader->pages_read, reader->pages_read + valid_pages - 1);
        tx_rx->tx_data[0] = MF_UL_FAST_READ_CMD;
        tx_rx->tx_data[1] = tag_page;
        tx_rx->tx_data[2] = valid_pages - 1;
        tx_rx->tx_bits = 24;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(furi_hal_nfc_tx_rx(tx_rx, 50)) {
            memcpy(&data->data[reader->pages_read * 4], tx_rx->rx_data, valid_pages * 4);
            reader->pages_read += valid_pages;
            data->data_size = reader->pages_read * 4;
        } else {
            FURI_LOG_D(
                TAG,
                "Failed to read pages %d - %d",
                reader->pages_read,
                reader->pages_read + valid_pages - 1);
            break;
        }
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
        if(reader->support_signature) {
            // Read Signature
            mf_ultralight_read_signature(tx_rx, data);
        }
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

static void mf_ul_protect_auth_data_on_read_command_i2c(
    uint8_t* tx_buff,
    uint8_t start_page,
    uint8_t end_page,
    MfUltralightEmulator* emulator) {
    if(emulator->data.type >= MfUltralightTypeNTAGI2CPlus1K) {
        // Blank out PWD and PACK
        if(start_page <= 229 && end_page >= 229) {
            uint16_t offset = (229 - start_page) * 4;
            uint8_t count = 4;
            if(end_page >= 230) count += 2;
            memset(&tx_buff[offset], 0, count);
        }

        // Handle AUTH0 for sector 0
        if(!emulator->auth_success) {
            uint8_t access = emulator->data.data[228 * 4];
            if(access & 0x80) {
                uint8_t auth0 = emulator->data.data[227 * 4 + 3];
                if(auth0 < end_page) {
                    // start_page is always < auth0; otherwise is NAK'd already
                    uint8_t page_offset = auth0 - start_page;
                    uint8_t page_count = end_page - auth0;
                    memset(&tx_buff[page_offset * 4], 0, page_count * 4);
                }
            }
        }
    }
}

static void mf_ul_ntag_i2c_fill_cross_area_read(
    uint8_t* tx_buff,
    uint8_t start_page,
    uint8_t end_page,
    MfUltralightEmulator* emulator) {
    // For copying config or session registers in fast read
    int16_t tx_page_offset;
    int16_t data_page_offset;
    uint8_t page_length;
    bool apply = false;
    MfUltralightType type = emulator->data.type;
    if(emulator->curr_sector == 0) {
        if(type == MfUltralightTypeNTAGI2C1K) {
            if(start_page <= 233 && end_page >= 232) {
                tx_page_offset = start_page - 232;
                data_page_offset = 227;
                page_length = 2;
                apply = true;
            }
        } else if(type == MfUltralightTypeNTAGI2CPlus1K || type == MfUltralightTypeNTAGI2CPlus2K) {
            if(start_page <= 237 && end_page >= 236) {
                tx_page_offset = start_page - 236;
                data_page_offset = 234;
                page_length = 2;
                apply = true;
            }
        }
    } else if(emulator->curr_sector == 1) {
        if(type == MfUltralightTypeNTAGI2C2K) {
            if(start_page <= 233 && end_page >= 232) {
                tx_page_offset = start_page - 232;
                data_page_offset = 483;
                page_length = 2;
                apply = true;
            }
        }
    }

    if(apply) {
        while(tx_page_offset < 0 && page_length > 0) {
            ++tx_page_offset;
            ++data_page_offset;
            --page_length;
        }
        memcpy(
            &tx_buff[tx_page_offset * 4],
            &emulator->data.data[data_page_offset * 4],
            page_length * 4);
    }
}

static bool mf_ul_ntag_i2c_plus_check_auth(
    MfUltralightEmulator* emulator,
    uint8_t start_page,
    bool is_write) {
    if(!emulator->auth_success) {
        uint8_t access = emulator->data.data[228 * 4];
        // Check NFC_PROT
        if(emulator->curr_sector == 0 && ((access & 0x80) || is_write)) {
            uint8_t auth0 = emulator->data.data[227 * 4 + 3];
            if(start_page >= auth0) return false;
        } else if(emulator->curr_sector == 1) {
            // We don't have to specifically check for type because this is done
            // by address translator
            uint8_t pt_i2c = emulator->data.data[231 * 4];
            // Check 2K_PROT
            if(pt_i2c & 0x08) return false;
        }
    }

    if(emulator->curr_sector == 1) {
        // Check NFC_DIS_SEC1
        uint8_t access = emulator->data.data[228 * 4];
        if(access & 0x20) return false;
    }

    return true;
}

void mf_ul_prepare_emulation(MfUltralightEmulator* emulator, MfUltralightData* data) {
    FURI_LOG_D(TAG, "Prepare emulation");
    emulator->data = *data;
    emulator->auth_data = NULL;
    emulator->data_changed = false;
    emulator->comp_write_cmd_started = false;
    emulator->sector_select_cmd_started = false;
    emulator->ntag_i2c_plus_sector3_lockout = false;
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
    } else if(data->type >= MfUltralightTypeNTAGI2C1K) {
        emulator->support_fast_read = true;
    }

    if(data->type >= MfUltralightTypeNTAG213 && data->type < MfUltralightTypeNTAGI2C1K) {
        uint16_t pwd_page = (data->data_size / 4) - 2;
        emulator->auth_data = (MfUltralightAuth*)&data->data[pwd_page * 4];
    } else if(data->type >= MfUltralightTypeNTAGI2CPlus1K) {
        emulator->auth_data = (MfUltralightAuth*)&data->data[229 * 4];
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
    bool send_ack = false;
    bool respond_nothing = false;

#ifdef FURI_DEBUG
    string_t debug_buf;
    string_init(debug_buf);
    for(int i = 0; i < (buff_rx_len + 7) / 8; ++i) {
        string_cat_printf(debug_buf, "%02x ", buff_rx[i]);
    }
    string_strim(debug_buf);
    FURI_LOG_T(TAG, "Emu RX (%d): %s", buff_rx_len, string_get_cstr(debug_buf));
    string_reset(debug_buf);
#endif

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
    } else if(emulator->sector_select_cmd_started) {
        if(buff_rx[0] <= 0xFE) {
            emulator->curr_sector = buff_rx[0] > 3 ? 0 : buff_rx[0];
            emulator->ntag_i2c_plus_sector3_lockout = false;
            command_parsed = true;
            respond_nothing = true;
            FURI_LOG_D(TAG, "Changing sector to %d", emulator->curr_sector);
        }
        emulator->sector_select_cmd_started = false;
    } else if(cmd == MF_UL_GET_VERSION_CMD) {
        if(emulator->data.type != MfUltralightTypeUnknown) {
            tx_bytes = sizeof(emulator->data.version);
            memcpy(buff_tx, &emulator->data.version, tx_bytes);
            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
            command_parsed = true;
        }
    } else if(cmd == MF_UL_READ_CMD) {
        int16_t start_page = buff_rx[1];
        tx_bytes = 16;
        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
            if(start_page < page_num) {
                if(start_page + 4 > page_num) {
                    // Handle roll-over mechanism
                    uint8_t end_pages_num = page_num - start_page;
                    memcpy(buff_tx, &emulator->data.data[start_page * 4], end_pages_num * 4);
                    memcpy(
                        &buff_tx[end_pages_num * 4], emulator->data.data, (4 - end_pages_num) * 4);
                } else {
                    memcpy(buff_tx, &emulator->data.data[start_page * 4], tx_bytes);
                }
                mf_ul_protect_auth_data_on_read_command(
                    buff_tx, start_page, (start_page + 4), emulator);
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        } else {
            uint16_t valid_pages;
            start_page = mf_ultralight_ntag_i2c_addr_tag_to_lin(
                &emulator->data, start_page, emulator->curr_sector, &valid_pages);
            if(start_page != -1) {
                if(emulator->data.type < MfUltralightTypeNTAGI2CPlus1K ||
                   mf_ul_ntag_i2c_plus_check_auth(emulator, buff_rx[1], false)) {
                    if(emulator->data.type >= MfUltralightTypeNTAGI2CPlus1K &&
                       emulator->curr_sector == 3 && valid_pages == 1) {
                        // Rewind back a sector to match behavior on a real tag
                        --start_page;
                        ++valid_pages;
                    }

                    uint16_t copy_count = (valid_pages > 4 ? 4 : valid_pages) * 4;
                    FURI_LOG_D(
                        TAG,
                        "NTAG I2C Emu: page valid, %02x:%02x -> %d, %d",
                        emulator->curr_sector,
                        buff_rx[1],
                        start_page,
                        valid_pages);
                    memcpy(buff_tx, &emulator->data.data[start_page * 4], copy_count);
                    // For NTAG I2C, there's no roll-over; remainder is filled by null bytes
                    if(copy_count < tx_bytes)
                        memset(&buff_tx[copy_count], 0, tx_bytes - copy_count);
                    // Special case: NTAG I2C Plus sector 0 page 233 read crosses into page 236
                    if(start_page == 233)
                        memcpy(&buff_tx[12], &emulator->data.data[(start_page + 1) * 4], 4);
                    mf_ul_protect_auth_data_on_read_command_i2c(
                        buff_tx, start_page, start_page + copy_count / 4 - 1, emulator);
                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                    command_parsed = true;
                }
            } else {
                FURI_LOG_D(
                    TAG,
                    "NTAG I2C Emu: page invalid, %02x:%02x",
                    emulator->curr_sector,
                    buff_rx[1]);
                if(emulator->data.type >= MfUltralightTypeNTAGI2CPlus1K &&
                   emulator->curr_sector == 3 && !emulator->ntag_i2c_plus_sector3_lockout) {
                    // NTAG I2C Plus has a weird behavior where if you read sector 3
                    // at an invalid address, it responds with zeroes then locks
                    // the read out, while if you read the mirrored session registers,
                    // it returns both session registers on either pages
                    memset(buff_tx, 0, tx_bytes);
                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                    command_parsed = true;
                    emulator->ntag_i2c_plus_sector3_lockout = true;
                }
            }
        }
        if(!command_parsed) tx_bytes = 0;
    } else if(cmd == MF_UL_FAST_READ_CMD) {
        if(emulator->support_fast_read) {
            int16_t start_page = buff_rx[1];
            uint8_t end_page = buff_rx[2];
            if(start_page <= end_page) {
                tx_bytes = ((end_page + 1) - start_page) * 4;
                if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
                    if((start_page < page_num) && (end_page < page_num)) {
                        memcpy(buff_tx, &emulator->data.data[start_page * 4], tx_bytes);
                        mf_ul_protect_auth_data_on_read_command(
                            buff_tx, start_page, end_page, emulator);
                        *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                        command_parsed = true;
                    }
                } else {
                    uint16_t valid_pages;
                    start_page = mf_ultralight_ntag_i2c_addr_tag_to_lin(
                        &emulator->data, start_page, emulator->curr_sector, &valid_pages);
                    if(start_page != -1) {
                        if(emulator->data.type < MfUltralightTypeNTAGI2CPlus1K ||
                           mf_ul_ntag_i2c_plus_check_auth(emulator, buff_rx[1], false)) {
                            uint16_t copy_count = (valid_pages > 4 ? 4 : valid_pages) * 4;
                            memcpy(buff_tx, &emulator->data.data[start_page * 4], copy_count);
                            if(copy_count < tx_bytes)
                                memset(&buff_tx[copy_count], 0, tx_bytes - copy_count);
                            mf_ul_ntag_i2c_fill_cross_area_read(
                                buff_tx, buff_rx[1], buff_rx[2], emulator);
                            mf_ul_protect_auth_data_on_read_command_i2c(
                                buff_tx, start_page, start_page + copy_count / 4 - 1, emulator);
                            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                            command_parsed = true;
                        }
                    }
                }
                if(!command_parsed) tx_bytes = 0;
            }
        }
    } else if(cmd == MF_UL_WRITE) {
        int16_t write_page = buff_rx[1];
        if(write_page > 1) {
            uint16_t valid_pages;
            write_page = mf_ultralight_ntag_i2c_addr_tag_to_lin(
                &emulator->data, write_page, emulator->curr_sector, &valid_pages);
            if(write_page != -1 &&
               (emulator->data.type >= MfUltralightTypeNTAGI2C1K || (write_page < page_num - 2))) {
                if(emulator->data.type < MfUltralightTypeNTAGI2CPlus1K ||
                   mf_ul_ntag_i2c_plus_check_auth(emulator, buff_rx[1], true)) {
                    memcpy(&emulator->data.data[write_page * 4], &buff_rx[2], 4);
                    emulator->data_changed = true;
                    send_ack = true;
                    command_parsed = true;
                }
            }
        }
    } else if(cmd == MF_UL_FAST_WRITE) {
        if(emulator->data.type >= MfUltralightTypeNTAGI2CPlus1K) {
            if(buff_rx[1] == 0xF0 && buff_rx[2] == 0xFF) {
                // TODO: update when SRAM emulation implemented
                send_ack = true;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_COMP_WRITE) {
        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
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
        }
    } else if(cmd == MF_UL_READ_CNT) {
        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
            uint8_t cnt_num = buff_rx[1];
            if(cnt_num < 3) {
                buff_tx[0] = emulator->data.counter[cnt_num] >> 16;
                buff_tx[1] = emulator->data.counter[cnt_num] >> 8;
                buff_tx[2] = emulator->data.counter[cnt_num];
                tx_bytes = 3;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_INC_CNT) {
        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
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
        }
    } else if(cmd == MF_UL_AUTH) {
        if(emulator->data.type >= MfUltralightTypeNTAG213 &&
           emulator->data.type != MfUltralightTypeNTAGI2C1K &&
           emulator->data.type != MfUltralightTypeNTAGI2C2K) {
            if(memcmp(&buff_rx[1], emulator->auth_data->pwd, 4) == 0) {
                buff_tx[0] = emulator->auth_data->pack.raw[0];
                buff_tx[1] = emulator->auth_data->pack.raw[1];
                tx_bytes = 2;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                emulator->auth_success = true;
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
        if(emulator->data.type != MfUltralightTypeNTAGI2C1K &&
           emulator->data.type != MfUltralightTypeNTAGI2C2K) {
            // Check 2nd byte = 0x00 - RFU
            if(buff_rx[1] == 0x00) {
                tx_bytes = sizeof(emulator->data.signature);
                memcpy(buff_tx, emulator->data.signature, tx_bytes);
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_CHECK_TEARING) {
        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
            uint8_t cnt_num = buff_rx[1];
            if(cnt_num < 3) {
                buff_tx[0] = emulator->data.tearing[cnt_num];
                tx_bytes = 1;
                *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                command_parsed = true;
            }
        }
    } else if(cmd == MF_UL_HALT_START) {
        tx_bits = 0;
        emulator->curr_sector = 0;
        emulator->ntag_i2c_plus_sector3_lockout = false;
        emulator->auth_success = false;
        command_parsed = true;
        FURI_LOG_D(TAG, "Received HLTA");
    } else if(cmd == MF_UL_SECTOR_SELECT) {
        if(emulator->data.type >= MfUltralightTypeNTAGI2C1K) {
            if(buff_rx[1] == 0xFF) {
                // Send ACK
                emulator->sector_select_cmd_started = true;
                send_ack = true;
                command_parsed = true;
            }
        }
    }

    if(!command_parsed) {
        // Send NACK
        buff_tx[0] = 0x00;
        tx_bits = 4;
        *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
    } else if(send_ack) {
        buff_tx[0] = 0x0A;
        tx_bits = 4;
        *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
    }

    if(respond_nothing) {
        *buff_tx_len = UINT16_MAX;
        *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
    } else {
        // Return tx buffer size in bits
        if(tx_bytes) {
            tx_bits = tx_bytes * 8;
        }
        *buff_tx_len = tx_bits;
    }

#ifdef FURI_DEBUG
    if(*buff_tx_len == UINT16_MAX) {
        FURI_LOG_T(TAG, "Emu TX: no reply");
    } else if(*buff_tx_len > 0) {
        int count = (*buff_tx_len + 7) / 8;
        for(int i = 0; i < count; ++i) {
            string_cat_printf(debug_buf, "%02x ", buff_tx[i]);
        }
        string_strim(debug_buf);
        FURI_LOG_T(TAG, "Emu TX (%d): %s", *buff_tx_len, string_get_cstr(debug_buf));
        string_clear(debug_buf);
    } else {
        FURI_LOG_T(TAG, "Emu TX: HALT");
    }
#endif

    return tx_bits > 0;
}
