#include <limits.h>
#include <mbedtls/sha1.h>
#include "mifare_ultralight.h"
#include "nfc_util.h"
#include <furi.h>
#include <furi_hal_nfc.h>

#define TAG "MfUltralight"

// Algorithms from: https://github.com/RfidResearchGroup/proxmark3/blob/0f6061c16f072372b7d4d381911f1542afbc3a69/common/generator.c#L110
uint32_t mf_ul_pwdgen_xiaomi(FuriHalNfcDevData* data) {
    uint8_t hash[20];
    mbedtls_sha1(data->uid, data->uid_len, hash);

    uint32_t pwd = 0;
    pwd |= (hash[hash[0] % 20]) << 24;
    pwd |= (hash[(hash[0] + 5) % 20]) << 16;
    pwd |= (hash[(hash[0] + 13) % 20]) << 8;
    pwd |= (hash[(hash[0] + 17) % 20]);

    return pwd;
}

uint32_t mf_ul_pwdgen_amiibo(FuriHalNfcDevData* data) {
    uint8_t* uid = data->uid;

    uint32_t pwd = 0;
    pwd |= (uid[1] ^ uid[3] ^ 0xAA) << 24;
    pwd |= (uid[2] ^ uid[4] ^ 0x55) << 16;
    pwd |= (uid[3] ^ uid[5] ^ 0xAA) << 8;
    pwd |= uid[4] ^ uid[6] ^ 0x55;

    return pwd;
}

bool mf_ul_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    if((ATQA0 == 0x44) && (ATQA1 == 0x00) && (SAK == 0x00)) {
        return true;
    }
    return false;
}

void mf_ul_reset(MfUltralightData* data) {
    furi_assert(data);
    data->type = MfUltralightTypeUnknown;
    memset(&data->version, 0, sizeof(MfUltralightVersion));
    memset(data->signature, 0, sizeof(data->signature));
    memset(data->counter, 0, sizeof(data->counter));
    memset(data->tearing, 0, sizeof(data->tearing));
    memset(data->data, 0, sizeof(data->data));
    data->data_size = 0;
    data->data_read = 0;
    data->curr_authlim = 0;
    data->auth_success = false;
}

static MfUltralightFeatures mf_ul_get_features(MfUltralightType type) {
    switch(type) {
    case MfUltralightTypeUL11:
    case MfUltralightTypeUL21:
        return MfUltralightSupportFastRead | MfUltralightSupportCompatWrite |
               MfUltralightSupportReadCounter | MfUltralightSupportIncrCounter |
               MfUltralightSupportAuth | MfUltralightSupportSignature |
               MfUltralightSupportTearingFlags | MfUltralightSupportVcsl;
    case MfUltralightTypeNTAG213:
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
        return MfUltralightSupportFastRead | MfUltralightSupportCompatWrite |
               MfUltralightSupportReadCounter | MfUltralightSupportAuth |
               MfUltralightSupportSignature | MfUltralightSupportSingleCounter |
               MfUltralightSupportAsciiMirror;
    case MfUltralightTypeNTAGI2C1K:
    case MfUltralightTypeNTAGI2C2K:
        return MfUltralightSupportFastRead | MfUltralightSupportSectorSelect;
    case MfUltralightTypeNTAGI2CPlus1K:
    case MfUltralightTypeNTAGI2CPlus2K:
        return MfUltralightSupportFastRead | MfUltralightSupportAuth |
               MfUltralightSupportFastWrite | MfUltralightSupportSignature |
               MfUltralightSupportSectorSelect;
    case MfUltralightTypeNTAG203:
        return MfUltralightSupportCompatWrite | MfUltralightSupportCounterInMemory;
    default:
        // Assumed original MFUL 512-bit
        return MfUltralightSupportCompatWrite;
    }
}

static void mf_ul_set_default_version(MfUltralightReader* reader, MfUltralightData* data) {
    data->type = MfUltralightTypeUnknown;
    reader->pages_to_read = 16;
}

static void mf_ul_set_version_ntag203(MfUltralightReader* reader, MfUltralightData* data) {
    data->type = MfUltralightTypeNTAG203;
    reader->pages_to_read = 42;
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
        if(!furi_hal_nfc_tx_rx(tx_rx, 50) || tx_rx->rx_bits != 64) {
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
        } else if(version->storage_size == 0x0E) {
            data->type = MfUltralightTypeUL21;
            reader->pages_to_read = 41;
        } else if(version->storage_size == 0x0F) {
            data->type = MfUltralightTypeNTAG213;
            reader->pages_to_read = 45;
        } else if(version->storage_size == 0x11) {
            data->type = MfUltralightTypeNTAG215;
            reader->pages_to_read = 135;
        } else if(version->prod_subtype == 5 && version->prod_ver_major == 2) {
            // NTAG I2C
            bool known = false;
            if(version->prod_ver_minor == 1) {
                if(version->storage_size == 0x13) {
                    data->type = MfUltralightTypeNTAGI2C1K;
                    reader->pages_to_read = 231;
                    known = true;
                } else if(version->storage_size == 0x15) {
                    data->type = MfUltralightTypeNTAGI2C2K;
                    reader->pages_to_read = 485;
                    known = true;
                }
            } else if(version->prod_ver_minor == 2) {
                if(version->storage_size == 0x13) {
                    data->type = MfUltralightTypeNTAGI2CPlus1K;
                    reader->pages_to_read = 236;
                    known = true;
                } else if(version->storage_size == 0x15) {
                    data->type = MfUltralightTypeNTAGI2CPlus2K;
                    reader->pages_to_read = 492;
                    known = true;
                }
            }

            if(!known) {
                mf_ul_set_default_version(reader, data);
            }
        } else if(version->storage_size == 0x13) {
            data->type = MfUltralightTypeNTAG216;
            reader->pages_to_read = 231;
        } else {
            mf_ul_set_default_version(reader, data);
            break;
        }
        version_read = true;
    } while(false);

    reader->supported_features = mf_ul_get_features(data->type);
    return version_read;
}

bool mf_ultralight_authenticate(FuriHalNfcTxRxContext* tx_rx, uint32_t key, uint16_t* pack) {
    furi_assert(pack);
    bool authenticated = false;

    do {
        FURI_LOG_D(TAG, "Authenticating");
        tx_rx->tx_data[0] = MF_UL_AUTH;
        nfc_util_num2bytes(key, 4, &tx_rx->tx_data[1]);
        tx_rx->tx_bits = 40;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        if(!furi_hal_nfc_tx_rx(tx_rx, 50)) {
            FURI_LOG_D(TAG, "Tag did not respond to authentication");
            break;
        }

        // PACK
        if(tx_rx->rx_bits < 2 * 8) {
            FURI_LOG_D(TAG, "Authentication failed");
            break;
        }

        *pack = (tx_rx->rx_data[1] << 8) | tx_rx->rx_data[0];

        FURI_LOG_I(TAG, "Auth success. Password: %08lX. PACK: %04X", key, *pack);
        authenticated = true;
    } while(false);

    return authenticated;
}

static int16_t mf_ultralight_page_addr_to_tag_addr(uint8_t sector, uint8_t page) {
    return sector * 256 + page;
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

MfUltralightConfigPages* mf_ultralight_get_config_pages(MfUltralightData* data) {
    if(data->type >= MfUltralightTypeUL11 && data->type <= MfUltralightTypeNTAG216) {
        return (MfUltralightConfigPages*)&data->data[data->data_size - 4 * 4];
    } else if(
        data->type >= MfUltralightTypeNTAGI2CPlus1K &&
        data->type <= MfUltralightTypeNTAGI2CPlus2K) {
        return (MfUltralightConfigPages*)&data->data[0xe3 * 4]; //-V641
    } else {
        return NULL;
    }
}

static uint16_t mf_ultralight_calc_auth_count(MfUltralightData* data) {
    if(mf_ul_get_features(data->type) & MfUltralightSupportAuth) {
        MfUltralightConfigPages* config = mf_ultralight_get_config_pages(data);
        uint16_t scaled_authlim = config->access.authlim;
        // NTAG I2C Plus uses 2^AUTHLIM attempts rather than the direct number
        if(scaled_authlim > 0 && data->type >= MfUltralightTypeNTAGI2CPlus1K &&
           data->type <= MfUltralightTypeNTAGI2CPlus2K) {
            scaled_authlim = 1 << scaled_authlim;
        }
        return scaled_authlim;
    }

    return 0;
}

// NTAG21x will NAK if NFC_CNT_EN unset, so preempt
static bool mf_ultralight_should_read_counters(MfUltralightData* data) {
    if(data->type < MfUltralightTypeNTAG213 || data->type > MfUltralightTypeNTAG216) return true;

    MfUltralightConfigPages* config = mf_ultralight_get_config_pages(data);
    return config->access.nfc_cnt_en;
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

bool mf_ultralight_read_pages_direct(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t start_index,
    uint8_t* data) {
    FURI_LOG_D(TAG, "Reading pages %d - %d", start_index, start_index + 3);
    tx_rx->tx_data[0] = MF_UL_READ_CMD;
    tx_rx->tx_data[1] = start_index;
    tx_rx->tx_bits = 16;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
    if(!furi_hal_nfc_tx_rx(tx_rx, 50) || tx_rx->rx_bits < 16 * 8) {
        FURI_LOG_D(TAG, "Failed to read pages %d - %d", start_index, start_index + 3);
        return false;
    }
    memcpy(data, tx_rx->rx_data, 16); //-V1086
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

        FURI_LOG_D(
            TAG, "Reading pages %zu - %zu", i, i + (valid_pages > 4 ? 4 : valid_pages) - 1U);
        tx_rx->tx_data[0] = MF_UL_READ_CMD;
        tx_rx->tx_data[1] = tag_page;
        tx_rx->tx_bits = 16;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

        if(!furi_hal_nfc_tx_rx(tx_rx, 50) || tx_rx->rx_bits < 16 * 8) {
            FURI_LOG_D(
                TAG,
                "Failed to read pages %zu - %zu",
                i,
                i + (valid_pages > 4 ? 4 : valid_pages) - 1U);
            break;
        }

        if(valid_pages > 4) {
            pages_read_cnt = 4;
        } else {
            pages_read_cnt = valid_pages;
        }
        reader->pages_read += pages_read_cnt;
        memcpy(&data->data[i * 4], tx_rx->rx_data, pages_read_cnt * 4);
    }
    data->data_size = reader->pages_to_read * 4;
    data->data_read = reader->pages_read * 4;

    return reader->pages_read > 0;
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
    bool is_single_counter = (mf_ul_get_features(data->type) & MfUltralightSupportSingleCounter) !=
                             0;
    for(size_t i = is_single_counter ? 2 : 0; i < 3; i++) {
        tx_rx->tx_data[0] = MF_UL_READ_CNT;
        tx_rx->tx_data[1] = i;
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

    return counter_read == (is_single_counter ? 1 : 3);
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
        if(reader->supported_features & MfUltralightSupportSignature) {
            // Read Signature
            mf_ultralight_read_signature(tx_rx, data);
        }
    } else {
        // No GET_VERSION command, check for NTAG203 by reading last page (41)
        uint8_t dummy[16];
        if(mf_ultralight_read_pages_direct(tx_rx, 41, dummy)) {
            mf_ul_set_version_ntag203(reader, data);
            reader->supported_features = mf_ul_get_features(data->type);
        } else {
            // We're really an original Mifare Ultralight, reset tag for safety
            furi_hal_nfc_sleep();
            furi_hal_nfc_activate_nfca(300, NULL);
        }
    }

    card_read = mf_ultralight_read_pages(tx_rx, reader, data);

    if(card_read) {
        if(reader->supported_features & MfUltralightSupportReadCounter &&
           mf_ultralight_should_read_counters(data)) {
            mf_ultralight_read_counters(tx_rx, data);
        }
        if(reader->supported_features & MfUltralightSupportTearingFlags) {
            mf_ultralight_read_tearing_flags(tx_rx, data);
        }
        data->curr_authlim = 0;

        if(reader->pages_read == reader->pages_to_read &&
           reader->supported_features & MfUltralightSupportAuth && !data->auth_success) {
            MfUltralightConfigPages* config = mf_ultralight_get_config_pages(data);
            if(config->access.authlim == 0) {
                // Attempt to auth with default PWD
                uint16_t pack;
                data->auth_success = mf_ultralight_authenticate(tx_rx, MF_UL_DEFAULT_PWD, &pack);
                if(data->auth_success) {
                    config->auth_data.pwd.value = MF_UL_DEFAULT_PWD;
                    config->auth_data.pack.value = pack;
                } else {
                    furi_hal_nfc_sleep();
                    furi_hal_nfc_activate_nfca(300, NULL);
                }
            }
        }
    }

    if(reader->pages_read != reader->pages_to_read) {
        if(reader->supported_features & MfUltralightSupportAuth) {
            // Probably password protected, fix AUTH0 and PROT so before AUTH0
            // can be written and since AUTH0 won't be readable, like on the
            // original card
            MfUltralightConfigPages* config = mf_ultralight_get_config_pages(data);
            config->auth0 = reader->pages_read;
            config->access.prot = true;
        }
    }

    return card_read;
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
            if(emulator->config_cache.access.prot) {
                uint8_t auth0 = emulator->config_cache.auth0;
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
        while(tx_page_offset < 0 && page_length > 0) { //-V614
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

static bool mf_ul_check_auth(MfUltralightEmulator* emulator, uint8_t start_page, bool is_write) {
    if(!emulator->auth_success) {
        if(start_page >= emulator->config_cache.auth0 &&
           (emulator->config_cache.access.prot || is_write))
            return false;
    }

    if(is_write && emulator->config_cache.access.cfglck) {
        uint16_t config_start_page = emulator->page_num - 4;
        if(start_page == config_start_page || start_page == config_start_page + 1) return false;
    }

    return true;
}

static bool mf_ul_ntag_i2c_plus_check_auth(
    MfUltralightEmulator* emulator,
    uint8_t start_page,
    bool is_write) {
    if(!emulator->auth_success) {
        // Check NFC_PROT
        if(emulator->curr_sector == 0 && (emulator->config_cache.access.prot || is_write)) {
            if(start_page >= emulator->config_cache.auth0) return false;
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
        if(emulator->config_cache.access.nfc_dis_sec1) return false;
    }

    return true;
}

static int16_t mf_ul_get_dynamic_lock_page_addr(MfUltralightData* data) {
    switch(data->type) {
    case MfUltralightTypeNTAG203:
        return 0x28;
    case MfUltralightTypeUL21:
    case MfUltralightTypeNTAG213:
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
        return data->data_size / 4 - 5;
    case MfUltralightTypeNTAGI2C1K:
    case MfUltralightTypeNTAGI2CPlus1K:
    case MfUltralightTypeNTAGI2CPlus2K:
        return 0xe2;
    case MfUltralightTypeNTAGI2C2K:
        return 0x1e0;
    default:
        return -1; // No dynamic lock bytes
    }
}

// Returns true if page not locked
// write_page is tag address
static bool mf_ul_check_lock(MfUltralightEmulator* emulator, int16_t write_page) {
    if(write_page < 2) return false; // Page 0-1 is always locked
    if(write_page == 2) return true; // Page 2 does not have a lock flag

    // Check static lock bytes
    if(write_page <= 15) {
        uint16_t static_lock_bytes = emulator->data.data[10] | (emulator->data.data[11] << 8);
        return (static_lock_bytes & (1 << write_page)) == 0;
    }

    // Check dynamic lock bytes

    // Check max page
    switch(emulator->data.type) {
    case MfUltralightTypeNTAG203:
        // Counter page can be locked and is after dynamic locks
        if(write_page == 40) return true;
        break;
    case MfUltralightTypeUL21:
    case MfUltralightTypeNTAG213:
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
        if(write_page >= emulator->page_num - 5) return true;
        break;
    case MfUltralightTypeNTAGI2C1K:
    case MfUltralightTypeNTAGI2CPlus1K:
        if(write_page > 225) return true;
        break;
    case MfUltralightTypeNTAGI2C2K:
        if(write_page > 479) return true;
        break;
    case MfUltralightTypeNTAGI2CPlus2K:
        if(write_page >= 226 && write_page <= 255) return true;
        if(write_page >= 512) return true;
        break;
    default:
        furi_crash("Unknown MFUL");
        return true;
    }

    int16_t dynamic_lock_index = mf_ul_get_dynamic_lock_page_addr(&emulator->data);
    if(dynamic_lock_index == -1) return true;
    // Run address through converter because NTAG I2C 2K is special
    uint16_t valid_pages; // unused
    dynamic_lock_index =
        mf_ultralight_ntag_i2c_addr_tag_to_lin(
            &emulator->data, dynamic_lock_index & 0xff, dynamic_lock_index >> 8, &valid_pages) *
        4;

    uint16_t dynamic_lock_bytes = emulator->data.data[dynamic_lock_index] |
                                  (emulator->data.data[dynamic_lock_index + 1] << 8);
    uint8_t shift;

    switch(emulator->data.type) {
    // low byte LSB range, MSB range
    case MfUltralightTypeNTAG203:
        if(write_page >= 16 && write_page <= 27) //-V560
            shift = (write_page - 16) / 4 + 1;
        else if(write_page >= 28 && write_page <= 39) //-V560
            shift = (write_page - 28) / 4 + 5;
        else if(write_page == 41)
            shift = 12;
        else {
            furi_crash("Unknown MFUL");
        }

        break;
    case MfUltralightTypeUL21:
    case MfUltralightTypeNTAG213:
        // 16-17, 30-31
        shift = (write_page - 16) / 2;
        break;
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
    case MfUltralightTypeNTAGI2C1K:
    case MfUltralightTypeNTAGI2CPlus1K:
        // 16-31, 128-129
        // 16-31, 128-143
        shift = (write_page - 16) / 16;
        break;
    case MfUltralightTypeNTAGI2C2K:
        // 16-47, 240-271
        shift = (write_page - 16) / 32;
        break;
    case MfUltralightTypeNTAGI2CPlus2K:
        // 16-47, 256-271
        if(write_page >= 208 && write_page <= 225)
            shift = 6;
        else if(write_page >= 256 && write_page <= 271)
            shift = 7;
        else
            shift = (write_page - 16) / 32;
        break;
    default:
        furi_crash("Unknown MFUL");
        break;
    }

    return (dynamic_lock_bytes & (1 << shift)) == 0;
}

static void mf_ul_make_ascii_mirror(MfUltralightEmulator* emulator, FuriString* str) {
    // Locals to improve readability
    uint8_t mirror_page = emulator->config->mirror_page;
    uint8_t mirror_byte = emulator->config->mirror.mirror_byte;
    MfUltralightMirrorConf mirror_conf = emulator->config_cache.mirror.mirror_conf;
    uint16_t last_user_page_index = emulator->page_num - 6;
    bool uid_printed = false;

    if(mirror_conf == MfUltralightMirrorUid || mirror_conf == MfUltralightMirrorUidCounter) {
        // UID range check
        if(mirror_page < 4 || mirror_page > last_user_page_index - 3 ||
           (mirror_page == last_user_page_index - 3 && mirror_byte > 2)) {
            if(mirror_conf == MfUltralightMirrorUid) return;
            // NTAG21x has the peculiar behavior when UID+counter selected, if UID does not fit but
            // counter will fit, it will actually mirror the counter
            furi_string_cat(str, "              ");
        } else {
            for(int i = 0; i < 3; ++i) {
                furi_string_cat_printf(str, "%02X", emulator->data.data[i]);
            }
            // Skip BCC0
            for(int i = 4; i < 8; ++i) {
                furi_string_cat_printf(str, "%02X", emulator->data.data[i]);
            }
            uid_printed = true;
        }

        uint16_t next_byte_offset = mirror_page * 4 + mirror_byte + 14;
        if(mirror_conf == MfUltralightMirrorUidCounter) ++next_byte_offset;
        mirror_page = next_byte_offset / 4;
        mirror_byte = next_byte_offset % 4;
    }

    if(mirror_conf == MfUltralightMirrorCounter || mirror_conf == MfUltralightMirrorUidCounter) {
        // Counter is only printed if counter enabled
        if(emulator->config_cache.access.nfc_cnt_en) {
            // Counter protection check
            if(emulator->config_cache.access.nfc_cnt_pwd_prot && !emulator->auth_success) return;
            // Counter range check
            if(mirror_page < 4) return;
            if(mirror_page > last_user_page_index - 1) return;
            if(mirror_page == last_user_page_index - 1 && mirror_byte > 2) return;

            if(mirror_conf == MfUltralightMirrorUidCounter)
                furi_string_cat(str, uid_printed ? "x" : " ");

            furi_string_cat_printf(str, "%06lX", emulator->data.counter[2]);
        }
    }
}

static void mf_ul_increment_single_counter(MfUltralightEmulator* emulator) {
    if(!emulator->read_counter_incremented && emulator->config_cache.access.nfc_cnt_en) {
        if(emulator->data.counter[2] < 0xFFFFFF) {
            ++emulator->data.counter[2];
            emulator->data_changed = true;
        }
        emulator->read_counter_incremented = true;
    }
}

static bool
    mf_ul_emulate_ntag203_counter_write(MfUltralightEmulator* emulator, uint8_t* page_buff) {
    // We'll reuse the existing counters for other NTAGs as staging
    // Counter 0 stores original value, data is new value
    uint32_t counter_value;
    if(emulator->data.tearing[0] == MF_UL_TEARING_FLAG_DEFAULT) {
        counter_value = emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4] |
                        (emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4 + 1] << 8);
    } else {
        // We've had a reset here, so load from original value
        counter_value = emulator->data.counter[0];
    }
    // Although the datasheet says increment by 0 is always possible, this is not the case on
    // an actual tag. If the counter is at 0xFFFF, any writes are locked out.
    if(counter_value == 0xFFFF) return false;
    uint32_t increment = page_buff[0] | (page_buff[1] << 8);
    if(counter_value == 0) {
        counter_value = increment;
    } else {
        // Per datasheet specifying > 0x000F is supposed to NAK, but actual tag doesn't
        increment &= 0x000F;
        if(counter_value + increment > 0xFFFF) return false;
        counter_value += increment;
    }
    // Commit to new value counter
    emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4] = (uint8_t)counter_value;
    emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4 + 1] = (uint8_t)(counter_value >> 8);
    emulator->data.tearing[0] = MF_UL_TEARING_FLAG_DEFAULT;
    if(counter_value == 0xFFFF) {
        // Tag will lock out counter if final number is 0xFFFF, even if you try to roll it back
        emulator->data.counter[1] = 0xFFFF;
    }
    emulator->data_changed = true;
    return true;
}

static void mf_ul_emulate_write(
    MfUltralightEmulator* emulator,
    int16_t tag_addr,
    int16_t write_page,
    uint8_t* page_buff) {
    // Assumption: all access checks have been completed

    if(tag_addr == 2) {
        // Handle static locks
        uint16_t orig_static_locks = emulator->data.data[write_page * 4 + 2] |
                                     (emulator->data.data[write_page * 4 + 3] << 8);
        uint16_t new_static_locks = page_buff[2] | (page_buff[3] << 8);
        if(orig_static_locks & 1) new_static_locks &= ~0x08;
        if(orig_static_locks & 2) new_static_locks &= ~0xF0;
        if(orig_static_locks & 4) new_static_locks &= 0xFF;
        new_static_locks |= orig_static_locks;
        page_buff[0] = emulator->data.data[write_page * 4];
        page_buff[1] = emulator->data.data[write_page * 4 + 1];
        page_buff[2] = new_static_locks & 0xff;
        page_buff[3] = new_static_locks >> 8;
    } else if(tag_addr == 3) {
        // Handle OTP/capability container
        *(uint32_t*)page_buff |= *(uint32_t*)&emulator->data.data[write_page * 4];
    } else if(tag_addr == mf_ul_get_dynamic_lock_page_addr(&emulator->data)) {
        // Handle dynamic locks
        if(emulator->data.type == MfUltralightTypeNTAG203) {
            // NTAG203 lock bytes are a bit different from the others
            uint8_t orig_page_lock_byte = emulator->data.data[write_page * 4];
            uint8_t orig_cnt_lock_byte = emulator->data.data[write_page * 4 + 1];
            uint8_t new_page_lock_byte = page_buff[0];
            uint8_t new_cnt_lock_byte = page_buff[1];

            if(orig_page_lock_byte & 0x01) // Block lock bits 1-3
                new_page_lock_byte &= ~0x0E;
            if(orig_page_lock_byte & 0x10) // Block lock bits 5-7
                new_page_lock_byte &= ~0xE0;
            for(uint8_t i = 0; i < 4; ++i) {
                if(orig_cnt_lock_byte & (1 << i)) // Block lock counter bit
                    new_cnt_lock_byte &= ~(1 << (4 + i));
            }

            new_page_lock_byte |= orig_page_lock_byte;
            new_cnt_lock_byte |= orig_cnt_lock_byte;
            page_buff[0] = new_page_lock_byte;
            page_buff[1] = new_cnt_lock_byte;
        } else {
            uint16_t orig_locks = emulator->data.data[write_page * 4] |
                                  (emulator->data.data[write_page * 4 + 1] << 8);
            uint8_t orig_block_locks = emulator->data.data[write_page * 4 + 2];
            uint16_t new_locks = page_buff[0] | (page_buff[1] << 8);
            uint8_t new_block_locks = page_buff[2];

            int block_lock_count;
            switch(emulator->data.type) {
            case MfUltralightTypeUL21:
                block_lock_count = 5;
                break;
            case MfUltralightTypeNTAG213:
                block_lock_count = 6;
                break;
            case MfUltralightTypeNTAG215:
                block_lock_count = 4;
                break;
            case MfUltralightTypeNTAG216:
            case MfUltralightTypeNTAGI2C1K:
            case MfUltralightTypeNTAGI2CPlus1K:
                block_lock_count = 7;
                break;
            case MfUltralightTypeNTAGI2C2K:
            case MfUltralightTypeNTAGI2CPlus2K:
                block_lock_count = 8;
                break;
            default:
                furi_crash("Unknown MFUL");
                break;
            }

            for(int i = 0; i < block_lock_count; ++i) {
                if(orig_block_locks & (1 << i)) new_locks &= ~(3 << (2 * i));
            }

            new_locks |= orig_locks;
            new_block_locks |= orig_block_locks;

            page_buff[0] = new_locks & 0xff;
            page_buff[1] = new_locks >> 8;
            page_buff[2] = new_block_locks;
            if(emulator->data.type >= MfUltralightTypeUL21 && //-V1016
               emulator->data.type <= MfUltralightTypeNTAG216)
                page_buff[3] = MF_UL_TEARING_FLAG_DEFAULT;
            else
                page_buff[3] = 0;
        }
    }

    memcpy(&emulator->data.data[write_page * 4], page_buff, 4);
    emulator->data_changed = true;
}

void mf_ul_reset_emulation(MfUltralightEmulator* emulator, bool is_power_cycle) {
    emulator->comp_write_cmd_started = false;
    emulator->sector_select_cmd_started = false;
    emulator->curr_sector = 0;
    emulator->ntag_i2c_plus_sector3_lockout = false;
    emulator->auth_success = false;
    if(is_power_cycle) {
        if(emulator->config != NULL) emulator->config_cache = *emulator->config;

        if(emulator->supported_features & MfUltralightSupportSingleCounter) {
            emulator->read_counter_incremented = false;
        }

        if(emulator->data.type == MfUltralightTypeNTAG203) {
            // Apply lockout if counter ever reached 0xFFFF
            if(emulator->data.counter[1] == 0xFFFF) {
                emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4] = 0xFF;
                emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4 + 1] = 0xFF;
            }
            // Copy original counter value from data
            emulator->data.counter[0] =
                emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4] |
                (emulator->data.data[MF_UL_NTAG203_COUNTER_PAGE * 4 + 1] << 8);
        }
    } else {
        if(emulator->config != NULL) {
            // ACCESS (less CFGLCK) and AUTH0 are updated when reactivated
            // MIRROR_CONF is not; don't know about STRG_MOD_EN, but we're not using that anyway
            emulator->config_cache.access.value = (emulator->config->access.value & 0xBF) |
                                                  (emulator->config_cache.access.value & 0x40);
            emulator->config_cache.auth0 = emulator->config->auth0;
        }
    }
    if(emulator->data.type == MfUltralightTypeNTAG203) {
        // Mark counter as dirty
        emulator->data.tearing[0] = 0;
    }
}

void mf_ul_prepare_emulation(MfUltralightEmulator* emulator, MfUltralightData* data) {
    FURI_LOG_D(TAG, "Prepare emulation");
    emulator->data = *data;
    emulator->supported_features = mf_ul_get_features(data->type);
    emulator->config = mf_ultralight_get_config_pages(&emulator->data);
    emulator->page_num = emulator->data.data_size / 4;
    emulator->data_changed = false;
    memset(&emulator->auth_attempt, 0, sizeof(MfUltralightAuth));
    mf_ul_reset_emulation(emulator, true);
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
    uint16_t tx_bytes = 0;
    uint16_t tx_bits = 0;
    bool command_parsed = false;
    bool send_ack = false;
    bool respond_nothing = false;
    bool reset_idle = false;

#ifdef FURI_DEBUG
    FuriString* debug_buf;
    debug_buf = furi_string_alloc();
    for(int i = 0; i < (buff_rx_len + 7) / 8; ++i) {
        furi_string_cat_printf(debug_buf, "%02x ", buff_rx[i]);
    }
    furi_string_trim(debug_buf);
    FURI_LOG_T(TAG, "Emu RX (%d): %s", buff_rx_len, furi_string_get_cstr(debug_buf));
    furi_string_reset(debug_buf);
#endif

    // Check composite commands
    if(emulator->comp_write_cmd_started) {
        if(buff_rx_len == 16 * 8) {
            if(emulator->data.type == MfUltralightTypeNTAG203 &&
               emulator->comp_write_page_addr == MF_UL_NTAG203_COUNTER_PAGE) {
                send_ack = mf_ul_emulate_ntag203_counter_write(emulator, buff_rx);
                command_parsed = send_ack;
            } else {
                mf_ul_emulate_write(
                    emulator,
                    emulator->comp_write_page_addr,
                    emulator->comp_write_page_addr,
                    buff_rx);
                send_ack = true;
                command_parsed = true;
            }
        }
        emulator->comp_write_cmd_started = false;
    } else if(emulator->sector_select_cmd_started) {
        if(buff_rx_len == 4 * 8) {
            if(buff_rx[0] <= 0xFE) {
                emulator->curr_sector = buff_rx[0] > 3 ? 0 : buff_rx[0];
                emulator->ntag_i2c_plus_sector3_lockout = false;
                command_parsed = true;
                respond_nothing = true;
                FURI_LOG_D(TAG, "Changing sector to %d", emulator->curr_sector);
            }
        }
        emulator->sector_select_cmd_started = false;
    } else if(buff_rx_len >= 8) {
        uint8_t cmd = buff_rx[0];
        if(cmd == MF_UL_GET_VERSION_CMD) {
            if(emulator->data.type >= MfUltralightTypeUL11) {
                if(buff_rx_len == 1 * 8) {
                    tx_bytes = sizeof(emulator->data.version);
                    memcpy(buff_tx, &emulator->data.version, tx_bytes);
                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                    command_parsed = true;
                }
            }
        } else if(cmd == MF_UL_READ_CMD) {
            if(buff_rx_len == (1 + 1) * 8) {
                int16_t start_page = buff_rx[1];
                tx_bytes = 16;
                if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
                    if(start_page < emulator->page_num) {
                        do {
                            uint8_t copied_pages = 0;
                            uint8_t src_page = start_page;
                            uint8_t last_page_plus_one = start_page + 4;
                            uint8_t pwd_page = emulator->page_num - 2;
                            FuriString* ascii_mirror = NULL;
                            size_t ascii_mirror_len = 0;
                            const char* ascii_mirror_cptr = NULL;
                            uint8_t ascii_mirror_curr_page = 0;
                            uint8_t ascii_mirror_curr_byte = 0;
                            if(last_page_plus_one > emulator->page_num)
                                last_page_plus_one = emulator->page_num;
                            if(emulator->supported_features & MfUltralightSupportAuth) {
                                if(!mf_ul_check_auth(emulator, start_page, false)) break;
                                if(!emulator->auth_success && emulator->config_cache.access.prot &&
                                   emulator->config_cache.auth0 < last_page_plus_one)
                                    last_page_plus_one = emulator->config_cache.auth0;
                            }
                            if(emulator->supported_features & MfUltralightSupportSingleCounter)
                                mf_ul_increment_single_counter(emulator);
                            if(emulator->supported_features & MfUltralightSupportAsciiMirror &&
                               emulator->config_cache.mirror.mirror_conf !=
                                   MfUltralightMirrorNone) {
                                ascii_mirror_curr_byte = emulator->config->mirror.mirror_byte;
                                ascii_mirror_curr_page = emulator->config->mirror_page;
                                // Try to avoid wasting time making mirror if we won't copy it
                                // Conservatively check with UID+counter mirror size
                                if(last_page_plus_one > ascii_mirror_curr_page &&
                                   start_page + 3 >= ascii_mirror_curr_page &&
                                   start_page <= ascii_mirror_curr_page + 6) {
                                    ascii_mirror = furi_string_alloc();
                                    mf_ul_make_ascii_mirror(emulator, ascii_mirror);
                                    ascii_mirror_len = furi_string_utf8_length(ascii_mirror);
                                    ascii_mirror_cptr = furi_string_get_cstr(ascii_mirror);
                                    // Move pointer to where it should be to start copying
                                    if(ascii_mirror_len > 0 &&
                                       ascii_mirror_curr_page < start_page &&
                                       ascii_mirror_curr_byte != 0) {
                                        uint8_t diff = 4 - ascii_mirror_curr_byte;
                                        ascii_mirror_len -= diff;
                                        ascii_mirror_cptr += diff;
                                        ascii_mirror_curr_byte = 0;
                                        ++ascii_mirror_curr_page;
                                    }
                                    while(ascii_mirror_len > 0 &&
                                          ascii_mirror_curr_page < start_page) {
                                        uint8_t diff = ascii_mirror_len > 4 ? 4 : ascii_mirror_len;
                                        ascii_mirror_len -= diff;
                                        ascii_mirror_cptr += diff;
                                        ++ascii_mirror_curr_page;
                                    }
                                }
                            }

                            uint8_t* dest_ptr = buff_tx;
                            while(copied_pages < 4) {
                                // Copy page
                                memcpy(dest_ptr, &emulator->data.data[src_page * 4], 4);

                                // Note: don't have to worry about roll-over with ASCII mirror because
                                // lowest valid page for it is 4, while roll-over will at best read
                                // pages 0-2
                                if(ascii_mirror_len > 0 && src_page == ascii_mirror_curr_page) {
                                    // Copy ASCII mirror
                                    size_t copy_len = 4 - ascii_mirror_curr_byte;
                                    if(copy_len > ascii_mirror_len) copy_len = ascii_mirror_len;
                                    for(size_t i = 0; i < copy_len; ++i) {
                                        if(*ascii_mirror_cptr != ' ')
                                            dest_ptr[ascii_mirror_curr_byte] =
                                                (uint8_t)*ascii_mirror_cptr;
                                        ++ascii_mirror_curr_byte;
                                        ++ascii_mirror_cptr;
                                    }
                                    ascii_mirror_len -= copy_len;
                                    // Don't care if this is inaccurate after ascii_mirror_len = 0
                                    ascii_mirror_curr_byte = 0;
                                    ++ascii_mirror_curr_page;
                                }

                                if(emulator->supported_features & MfUltralightSupportAuth) {
                                    if(src_page == pwd_page || src_page == pwd_page + 1) {
                                        // Blank out PWD and PACK pages
                                        memset(dest_ptr, 0, 4);
                                    }
                                }

                                dest_ptr += 4;
                                ++copied_pages;
                                ++src_page;
                                if(src_page >= last_page_plus_one) src_page = 0;
                            }
                            if(ascii_mirror != NULL) {
                                furi_string_free(ascii_mirror);
                            }
                            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                            command_parsed = true;
                        } while(false);
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
                                memcpy(
                                    &buff_tx[12], &emulator->data.data[(start_page + 1) * 4], 4);
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
                           emulator->curr_sector == 3 &&
                           !emulator->ntag_i2c_plus_sector3_lockout) {
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
            }
        } else if(cmd == MF_UL_FAST_READ_CMD) {
            if(emulator->supported_features & MfUltralightSupportFastRead) {
                if(buff_rx_len == (1 + 2) * 8) {
                    int16_t start_page = buff_rx[1];
                    uint8_t end_page = buff_rx[2];
                    if(start_page <= end_page) {
                        tx_bytes = ((end_page + 1) - start_page) * 4;
                        if(emulator->data.type < MfUltralightTypeNTAGI2C1K) {
                            if((start_page < emulator->page_num) &&
                               (end_page < emulator->page_num)) {
                                do {
                                    if(emulator->supported_features & MfUltralightSupportAuth) {
                                        // NAK if not authenticated and requested pages cross over AUTH0
                                        if(!emulator->auth_success &&
                                           emulator->config_cache.access.prot &&
                                           (start_page >= emulator->config_cache.auth0 ||
                                            end_page >= emulator->config_cache.auth0))
                                            break;
                                    }
                                    if(emulator->supported_features &
                                       MfUltralightSupportSingleCounter)
                                        mf_ul_increment_single_counter(emulator);

                                    // Copy requested pages
                                    memcpy(
                                        buff_tx, &emulator->data.data[start_page * 4], tx_bytes);

                                    if(emulator->supported_features &
                                           MfUltralightSupportAsciiMirror &&
                                       emulator->config_cache.mirror.mirror_conf !=
                                           MfUltralightMirrorNone) {
                                        // Copy ASCII mirror
                                        // Less stringent check here, because expecting FAST_READ to
                                        // only be issued once rather than repeatedly
                                        FuriString* ascii_mirror;
                                        ascii_mirror = furi_string_alloc();
                                        mf_ul_make_ascii_mirror(emulator, ascii_mirror);
                                        size_t ascii_mirror_len =
                                            furi_string_utf8_length(ascii_mirror);
                                        const char* ascii_mirror_cptr =
                                            furi_string_get_cstr(ascii_mirror);
                                        int16_t mirror_start_offset =
                                            (emulator->config->mirror_page - start_page) * 4 +
                                            emulator->config->mirror.mirror_byte;
                                        if(mirror_start_offset < 0) {
                                            if(mirror_start_offset < -(int16_t)ascii_mirror_len) {
                                                // Past ASCII mirror, don't copy
                                                ascii_mirror_len = 0;
                                            } else {
                                                ascii_mirror_cptr += -mirror_start_offset;
                                                ascii_mirror_len -= -mirror_start_offset;
                                                mirror_start_offset = 0;
                                            }
                                        }
                                        if(ascii_mirror_len > 0) {
                                            int16_t mirror_end_offset =
                                                mirror_start_offset + ascii_mirror_len;
                                            if(mirror_end_offset > (end_page + 1) * 4) {
                                                mirror_end_offset = (end_page + 1) * 4;
                                                ascii_mirror_len =
                                                    mirror_end_offset - mirror_start_offset;
                                            }
                                            for(size_t i = 0; i < ascii_mirror_len; ++i) {
                                                if(*ascii_mirror_cptr != ' ')
                                                    buff_tx[mirror_start_offset] =
                                                        (uint8_t)*ascii_mirror_cptr;
                                                ++mirror_start_offset;
                                                ++ascii_mirror_cptr;
                                            }
                                        }
                                        furi_string_free(ascii_mirror);
                                    }

                                    if(emulator->supported_features & MfUltralightSupportAuth) {
                                        // Clear PWD and PACK pages
                                        uint8_t pwd_page = emulator->page_num - 2;
                                        int16_t pwd_page_offset = pwd_page - start_page;
                                        // PWD page
                                        if(pwd_page_offset >= 0 && pwd_page <= end_page) {
                                            memset(&buff_tx[pwd_page_offset * 4], 0, 4);
                                            // PACK page
                                            if(pwd_page + 1 <= end_page)
                                                memset(&buff_tx[(pwd_page_offset + 1) * 4], 0, 4);
                                        }
                                    }
                                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                                    command_parsed = true;
                                } while(false);
                            }
                        } else {
                            uint16_t valid_pages;
                            start_page = mf_ultralight_ntag_i2c_addr_tag_to_lin(
                                &emulator->data, start_page, emulator->curr_sector, &valid_pages);
                            if(start_page != -1) {
                                if(emulator->data.type < MfUltralightTypeNTAGI2CPlus1K ||
                                   mf_ul_ntag_i2c_plus_check_auth(emulator, buff_rx[1], false)) {
                                    uint16_t copy_count = tx_bytes;
                                    if(copy_count > valid_pages * 4) copy_count = valid_pages * 4;
                                    memcpy(
                                        buff_tx, &emulator->data.data[start_page * 4], copy_count);
                                    if(copy_count < tx_bytes)
                                        memset(&buff_tx[copy_count], 0, tx_bytes - copy_count);
                                    mf_ul_ntag_i2c_fill_cross_area_read(
                                        buff_tx, buff_rx[1], buff_rx[2], emulator);
                                    mf_ul_protect_auth_data_on_read_command_i2c(
                                        buff_tx,
                                        start_page,
                                        start_page + copy_count / 4 - 1,
                                        emulator);
                                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                                    command_parsed = true;
                                }
                            }
                        }
                        if(!command_parsed) tx_bytes = 0;
                    }
                }
            }
        } else if(cmd == MF_UL_WRITE) {
            if(buff_rx_len == (1 + 5) * 8) {
                do {
                    uint8_t orig_write_page = buff_rx[1];
                    int16_t write_page = orig_write_page;
                    uint16_t valid_pages; // unused
                    write_page = mf_ultralight_ntag_i2c_addr_tag_to_lin(
                        &emulator->data, write_page, emulator->curr_sector, &valid_pages);
                    if(write_page == -1) // NTAG I2C range check
                        break;
                    else if(write_page < 2 || write_page >= emulator->page_num) // Other MFUL/NTAG range check
                        break;

                    if(emulator->supported_features & MfUltralightSupportAuth) {
                        if(emulator->data.type >= MfUltralightTypeNTAGI2CPlus1K) {
                            if(!mf_ul_ntag_i2c_plus_check_auth(emulator, orig_write_page, true))
                                break;
                        } else {
                            if(!mf_ul_check_auth(emulator, orig_write_page, true)) break;
                        }
                    }
                    int16_t tag_addr = mf_ultralight_page_addr_to_tag_addr(
                        emulator->curr_sector, orig_write_page);
                    if(!mf_ul_check_lock(emulator, tag_addr)) break;
                    if(emulator->data.type == MfUltralightTypeNTAG203 &&
                       orig_write_page == MF_UL_NTAG203_COUNTER_PAGE) {
                        send_ack = mf_ul_emulate_ntag203_counter_write(emulator, &buff_rx[2]);
                        command_parsed = send_ack;
                    } else {
                        mf_ul_emulate_write(emulator, tag_addr, write_page, &buff_rx[2]);
                        send_ack = true;
                        command_parsed = true;
                    }
                } while(false);
            }
        } else if(cmd == MF_UL_FAST_WRITE) {
            if(emulator->supported_features & MfUltralightSupportFastWrite) {
                if(buff_rx_len == (1 + 66) * 8) {
                    if(buff_rx[1] == 0xF0 && buff_rx[2] == 0xFF) {
                        // TODO: update when SRAM emulation implemented
                        send_ack = true;
                        command_parsed = true;
                    }
                }
            }
        } else if(cmd == MF_UL_COMP_WRITE) {
            if(emulator->supported_features & MfUltralightSupportCompatWrite) {
                if(buff_rx_len == (1 + 1) * 8) {
                    uint8_t write_page = buff_rx[1];
                    do {
                        if(write_page < 2 || write_page >= emulator->page_num) break;
                        if(emulator->supported_features & MfUltralightSupportAuth &&
                           !mf_ul_check_auth(emulator, write_page, true))
                            break;
                        // Note we don't convert to tag addr here because there's only one sector
                        if(!mf_ul_check_lock(emulator, write_page)) break;

                        emulator->comp_write_cmd_started = true;
                        emulator->comp_write_page_addr = write_page;
                        send_ack = true;
                        command_parsed = true;
                    } while(false);
                }
            }
        } else if(cmd == MF_UL_READ_CNT) {
            if(emulator->supported_features & MfUltralightSupportReadCounter) {
                if(buff_rx_len == (1 + 1) * 8) {
                    do {
                        uint8_t cnt_num = buff_rx[1];

                        // NTAG21x checks
                        if(emulator->supported_features & MfUltralightSupportSingleCounter) {
                            if(cnt_num != 2) break; // Only counter 2 is available
                            if(!emulator->config_cache.access.nfc_cnt_en)
                                break; // NAK if counter not enabled
                            if(emulator->config_cache.access.nfc_cnt_pwd_prot &&
                               !emulator->auth_success)
                                break;
                        }

                        if(cnt_num < 3) {
                            buff_tx[0] = emulator->data.counter[cnt_num] & 0xFF;
                            buff_tx[1] = (emulator->data.counter[cnt_num] >> 8) & 0xFF;
                            buff_tx[2] = (emulator->data.counter[cnt_num] >> 16) & 0xFF;
                            tx_bytes = 3;
                            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                            command_parsed = true;
                        }
                    } while(false);
                }
            }
        } else if(cmd == MF_UL_INC_CNT) {
            if(emulator->supported_features & MfUltralightSupportIncrCounter) {
                if(buff_rx_len == (1 + 5) * 8) {
                    uint8_t cnt_num = buff_rx[1];
                    uint32_t inc = (buff_rx[2] | (buff_rx[3] << 8) | (buff_rx[4] << 16));
                    // TODO: can you increment by 0 when counter is at 0xffffff?
                    if((cnt_num < 3) && (emulator->data.counter[cnt_num] != 0x00FFFFFF) &&
                       (emulator->data.counter[cnt_num] + inc <= 0x00FFFFFF)) {
                        emulator->data.counter[cnt_num] += inc;
                        // We're RAM-backed, so tearing never happens
                        emulator->data.tearing[cnt_num] = MF_UL_TEARING_FLAG_DEFAULT;
                        emulator->data_changed = true;
                        send_ack = true;
                        command_parsed = true;
                    }
                }
            }
        } else if(cmd == MF_UL_AUTH) {
            if(emulator->supported_features & MfUltralightSupportAuth) {
                if(buff_rx_len == (1 + 4) * 8) {
                    // Record password sent by PCD
                    memcpy(
                        emulator->auth_attempt.pwd.raw,
                        &buff_rx[1],
                        sizeof(emulator->auth_attempt.pwd.raw));
                    emulator->auth_attempted = true;
                    if(emulator->auth_received_callback) {
                        emulator->auth_received_callback(
                            emulator->auth_attempt, emulator->context);
                    }

                    uint16_t scaled_authlim = mf_ultralight_calc_auth_count(&emulator->data);
                    if(scaled_authlim != 0 && emulator->data.curr_authlim >= scaled_authlim) {
                        if(emulator->data.curr_authlim != UINT16_MAX) {
                            // Handle case where AUTHLIM has been lowered or changed from 0
                            emulator->data.curr_authlim = UINT16_MAX;
                            emulator->data_changed = true;
                        }
                        // AUTHLIM reached, always fail
                        buff_tx[0] = MF_UL_NAK_AUTHLIM_REACHED;
                        tx_bits = 4;
                        *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
                        mf_ul_reset_emulation(emulator, false);
                        command_parsed = true;
                    } else {
                        if(memcmp(&buff_rx[1], emulator->config->auth_data.pwd.raw, 4) == 0) {
                            // Correct password
                            buff_tx[0] = emulator->config->auth_data.pack.raw[0];
                            buff_tx[1] = emulator->config->auth_data.pack.raw[1];
                            tx_bytes = 2;
                            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                            emulator->auth_success = true;
                            command_parsed = true;
                            if(emulator->data.curr_authlim != 0) {
                                // Reset current AUTHLIM
                                emulator->data.curr_authlim = 0;
                                emulator->data_changed = true;
                            }
                        } else if(!emulator->config->auth_data.pwd.value) {
                            // Unknown password, pretend to be an Amiibo
                            buff_tx[0] = 0x80;
                            buff_tx[1] = 0x80;
                            tx_bytes = 2;
                            *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                            emulator->auth_success = true;
                            command_parsed = true;
                        } else {
                            // Wrong password, increase negative verification count
                            if(emulator->data.curr_authlim < UINT16_MAX) {
                                ++emulator->data.curr_authlim;
                                emulator->data_changed = true;
                            }
                            if(scaled_authlim != 0 &&
                               emulator->data.curr_authlim >= scaled_authlim) {
                                emulator->data.curr_authlim = UINT16_MAX;
                                buff_tx[0] = MF_UL_NAK_AUTHLIM_REACHED;
                                tx_bits = 4;
                                *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
                                mf_ul_reset_emulation(emulator, false);
                                command_parsed = true;
                            } else {
                                // Should delay here to slow brute forcing
                            }
                        }
                    }
                }
            }
        } else if(cmd == MF_UL_READ_SIG) {
            if(emulator->supported_features & MfUltralightSupportSignature) {
                // Check 2nd byte = 0x00 - RFU
                if(buff_rx_len == (1 + 1) * 8 && buff_rx[1] == 0x00) {
                    tx_bytes = sizeof(emulator->data.signature);
                    memcpy(buff_tx, emulator->data.signature, tx_bytes);
                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                    command_parsed = true;
                }
            }
        } else if(cmd == MF_UL_CHECK_TEARING) {
            if(emulator->supported_features & MfUltralightSupportTearingFlags) {
                if(buff_rx_len == (1 + 1) * 8) {
                    uint8_t cnt_num = buff_rx[1];
                    if(cnt_num < 3) {
                        buff_tx[0] = emulator->data.tearing[cnt_num];
                        tx_bytes = 1;
                        *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                        command_parsed = true;
                    }
                }
            }
        } else if(cmd == MF_UL_HALT_START) {
            reset_idle = true;
            FURI_LOG_D(TAG, "Received HLTA");
        } else if(cmd == MF_UL_SECTOR_SELECT) {
            if(emulator->supported_features & MfUltralightSupportSectorSelect) {
                if(buff_rx_len == (1 + 1) * 8 && buff_rx[1] == 0xFF) {
                    // Send ACK
                    emulator->sector_select_cmd_started = true;
                    send_ack = true;
                    command_parsed = true;
                }
            }
        } else if(cmd == MF_UL_READ_VCSL) {
            if(emulator->supported_features & MfUltralightSupportVcsl) {
                if(buff_rx_len == (1 + 20) * 8) {
                    buff_tx[0] = emulator->config_cache.vctid;
                    tx_bytes = 1;
                    *data_type = FURI_HAL_NFC_TXRX_DEFAULT;
                    command_parsed = true;
                }
            }
        } else {
            // NTAG203 appears to NAK instead of just falling off on invalid commands
            if(emulator->data.type != MfUltralightTypeNTAG203) reset_idle = true;
            FURI_LOG_D(TAG, "Received invalid command");
        }
    } else {
        reset_idle = true;
        FURI_LOG_D(TAG, "Received invalid buffer less than 8 bits in length");
    }

    if(reset_idle) {
        mf_ul_reset_emulation(emulator, false);
        tx_bits = 0;
        command_parsed = true;
    }

    if(!command_parsed) {
        // Send NACK
        buff_tx[0] = MF_UL_NAK_INVALID_ARGUMENT;
        tx_bits = 4;
        *data_type = FURI_HAL_NFC_TX_RAW_RX_DEFAULT;
        // Every NAK should cause reset to IDLE
        mf_ul_reset_emulation(emulator, false);
    } else if(send_ack) {
        buff_tx[0] = MF_UL_ACK;
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
            furi_string_cat_printf(debug_buf, "%02x ", buff_tx[i]);
        }
        furi_string_trim(debug_buf);
        FURI_LOG_T(TAG, "Emu TX (%d): %s", *buff_tx_len, furi_string_get_cstr(debug_buf));
        furi_string_free(debug_buf);
    } else {
        FURI_LOG_T(TAG, "Emu TX: HALT");
    }
#endif

    return tx_bits > 0;
}

bool mf_ul_is_full_capture(MfUltralightData* data) {
    if(data->data_read != data->data_size) return false;

    // Having read all the pages doesn't mean that we've got everything.
    // By default PWD is 0xFFFFFFFF, but if read back it is always 0x00000000,
    // so a default read on an auth-supported NTAG is never complete.
    if(!(mf_ul_get_features(data->type) & MfUltralightSupportAuth)) return true;
    MfUltralightConfigPages* config = mf_ultralight_get_config_pages(data);
    return config->auth_data.pwd.value != 0 || config->auth_data.pack.value != 0;
}
