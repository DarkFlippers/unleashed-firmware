#include "nfc_data_generator.h"

#include <furi/furi.h>
#include <furi_hal_random.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#define NXP_MANUFACTURER_ID (0x04)

typedef void (*NfcDataGeneratorHandler)(NfcDevice* nfc_device);

typedef struct {
    const char* name;
    NfcDataGeneratorHandler handler;
} NfcDataGenerator;

static const uint8_t version_bytes_mf0ulx1[] = {0x00, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x03};
static const uint8_t version_bytes_ntag21x[] = {0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x03};
static const uint8_t version_bytes_ntag_i2c[] = {0x00, 0x04, 0x04, 0x05, 0x02, 0x00, 0x00, 0x03};
static const uint8_t default_data_ntag203[] =
    {0xE1, 0x10, 0x12, 0x00, 0x01, 0x03, 0xA0, 0x10, 0x44, 0x03, 0x00, 0xFE};
static const uint8_t default_data_ntag213[] = {0x01, 0x03, 0xA0, 0x0C, 0x34, 0x03, 0x00, 0xFE};
static const uint8_t default_data_ntag215_216[] = {0x03, 0x00, 0xFE};
static const uint8_t default_data_ntag_i2c[] = {0xE1, 0x10, 0x00, 0x00, 0x03, 0x00, 0xFE};
static const uint8_t default_config_ntag_i2c[] = {0x01, 0x00, 0xF8, 0x48, 0x08, 0x01, 0x00, 0x00};

static void nfc_generate_mf_ul_uid(uint8_t* uid) {
    uid[0] = NXP_MANUFACTURER_ID;
    furi_hal_random_fill_buf(&uid[1], 6);
    uid[3] |= 0x01; // To avoid forbidden 0x88 value
    // I'm not sure how this is generated, but the upper nybble always seems to be 8
    uid[6] &= 0x0F;
    uid[6] |= 0x80;
}

static void nfc_generate_mf_ul_common(MfUltralightData* mfu_data) {
    uint8_t uid[7];
    mfu_data->iso14443_3a_data->uid_len = 7;
    nfc_generate_mf_ul_uid(uid);
    mf_ultralight_set_uid(mfu_data, uid, 7);

    mfu_data->iso14443_3a_data->atqa[0] = 0x44;
    mfu_data->iso14443_3a_data->atqa[1] = 0x00;
    mfu_data->iso14443_3a_data->sak = 0x00;
}

static void nfc_generate_mf_ul_orig(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();
    nfc_generate_mf_ul_common(mfu_data);

    mfu_data->type = MfUltralightTypeOrigin;
    mfu_data->pages_total = 16;
    mfu_data->pages_read = 16;
    memset(&mfu_data->page[4], 0xff, sizeof(MfUltralightPage));

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_mf_ul_with_config_common(MfUltralightData* mfu_data, uint8_t num_pages) {
    nfc_generate_mf_ul_common(mfu_data);

    mfu_data->pages_total = num_pages;
    mfu_data->pages_read = num_pages;

    uint16_t config_index = (num_pages - 4);
    mfu_data->page[config_index].data[0] = 0x04; // STRG_MOD_EN
    mfu_data->page[config_index].data[3] = 0xff; // AUTH0
    mfu_data->page[config_index + 1].data[1] = 0x05; // VCTID
    memset(&mfu_data->page[config_index + 2], 0xff, sizeof(MfUltralightPage)); // Default PWD
    if(num_pages > 20) {
        mfu_data->page[config_index - 1].data[3] = MF_ULTRALIGHT_TEARING_FLAG_DEFAULT;
    }
}

static void nfc_generate_mf_ul_ev1_common(MfUltralightData* mfu_data, uint8_t num_pages) {
    nfc_generate_mf_ul_with_config_common(mfu_data, num_pages);
    memcpy(&mfu_data->version, version_bytes_mf0ulx1, sizeof(MfUltralightVersion));
    for(size_t i = 0; i < 3; ++i) {
        mfu_data->tearing_flag[i].data = MF_ULTRALIGHT_TEARING_FLAG_DEFAULT;
    }
}

static void nfc_generate_mf_ul_11(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_mf_ul_ev1_common(mfu_data, 20);
    mfu_data->type = MfUltralightTypeUL11;
    mfu_data->version.prod_subtype = 0x01;
    mfu_data->version.storage_size = 0x0B;
    mfu_data->page[16].data[0] = 0x00; // Low capacitance version does not have STRG_MOD_EN

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_mf_ul_h11(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_mf_ul_ev1_common(mfu_data, 20);
    mfu_data->type = MfUltralightTypeUL11;
    mfu_data->version.prod_subtype = 0x02;
    mfu_data->version.storage_size = 0x0B;

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_mf_ul_21(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_mf_ul_ev1_common(mfu_data, 41);
    mfu_data->type = MfUltralightTypeUL21;
    mfu_data->version.prod_subtype = 0x01;
    mfu_data->version.storage_size = 0x0E;
    mfu_data->page[37].data[0] = 0x00; // Low capacitance version does not have STRG_MOD_EN

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_mf_ul_h21(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_mf_ul_ev1_common(mfu_data, 41);
    mfu_data->type = MfUltralightTypeUL21;
    mfu_data->version.prod_subtype = 0x02;
    mfu_data->version.storage_size = 0x0E;

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag203(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_mf_ul_common(mfu_data);
    mfu_data->type = MfUltralightTypeNTAG203;
    mfu_data->pages_total = 42;
    mfu_data->pages_read = 42;
    mfu_data->page[2].data[1] = 0x48; // Internal byte
    memcpy(&mfu_data->page[3], default_data_ntag203, sizeof(MfUltralightPage)); //-V1086

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag21x_common(MfUltralightData* mfu_data, uint8_t num_pages) {
    nfc_generate_mf_ul_with_config_common(mfu_data, num_pages);
    memcpy(&mfu_data->version, version_bytes_ntag21x, sizeof(MfUltralightVersion));
    mfu_data->page[2].data[1] = 0x48; // Internal byte
    // Capability container
    mfu_data->page[3].data[0] = 0xE1;
    mfu_data->page[3].data[1] = 0x10;
}

static void nfc_generate_ntag213(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag21x_common(mfu_data, 45);
    mfu_data->type = MfUltralightTypeNTAG213;
    mfu_data->version.storage_size = 0x0F;
    mfu_data->page[3].data[2] = 0x12;
    // Default contents
    memcpy(&mfu_data->page[4], default_data_ntag213, sizeof(default_data_ntag213));

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag215(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag21x_common(mfu_data, 135);
    mfu_data->type = MfUltralightTypeNTAG215;
    mfu_data->version.storage_size = 0x11;
    mfu_data->page[3].data[2] = 0x3E;
    // Default contents
    memcpy(&mfu_data->page[4], default_data_ntag215_216, sizeof(default_data_ntag215_216));

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag216(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag21x_common(mfu_data, 231);
    mfu_data->type = MfUltralightTypeNTAG216;
    mfu_data->version.storage_size = 0x13;
    mfu_data->page[3].data[2] = 0x6D;
    // Default contents
    memcpy(&mfu_data->page[4], default_data_ntag215_216, sizeof(default_data_ntag215_216));

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag_i2c_common(
    MfUltralightData* mfu_data,
    MfUltralightType type,
    uint16_t num_pages) {
    nfc_generate_mf_ul_common(mfu_data);

    mfu_data->type = type;
    memcpy(&mfu_data->version, version_bytes_ntag_i2c, sizeof(version_bytes_ntag_i2c));
    mfu_data->pages_total = num_pages;
    mfu_data->pages_read = num_pages;
    memcpy(
        mfu_data->page[0].data,
        mfu_data->iso14443_3a_data->uid,
        mfu_data->iso14443_3a_data->uid_len);
    mfu_data->page[1].data[3] = mfu_data->iso14443_3a_data->sak;
    mfu_data->page[2].data[0] = mfu_data->iso14443_3a_data->atqa[0];
    mfu_data->page[2].data[1] = mfu_data->iso14443_3a_data->atqa[1];

    uint16_t config_register_page = 0;
    uint16_t session_register_page = 0;

    // Sync with mifare_ultralight.c
    switch(type) {
    case MfUltralightTypeNTAGI2C1K:
        config_register_page = 227;
        session_register_page = 229;
        break;
    case MfUltralightTypeNTAGI2C2K:
        config_register_page = 481;
        session_register_page = 483;
        break;
    case MfUltralightTypeNTAGI2CPlus1K:
    case MfUltralightTypeNTAGI2CPlus2K:
        config_register_page = 232;
        session_register_page = 234;
        break;
    default:
        furi_crash("Unknown MFUL");
        break;
    }

    memcpy(
        &mfu_data->page[config_register_page],
        default_config_ntag_i2c,
        sizeof(default_config_ntag_i2c));
    memcpy(
        &mfu_data->page[session_register_page],
        default_config_ntag_i2c,
        sizeof(default_config_ntag_i2c));
}

static void nfc_generate_ntag_i2c_1k(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag_i2c_common(mfu_data, MfUltralightTypeNTAGI2C1K, 231);
    mfu_data->version.prod_ver_minor = 0x01;
    mfu_data->version.storage_size = 0x13;
    memcpy(&mfu_data->page[3], default_data_ntag_i2c, sizeof(default_data_ntag_i2c));
    mfu_data->page[3].data[2] = 0x6D; // Size of tag in CC

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag_i2c_2k(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag_i2c_common(mfu_data, MfUltralightTypeNTAGI2C2K, 485);
    mfu_data->version.prod_ver_minor = 0x01;
    mfu_data->version.storage_size = 0x15;
    memcpy(&mfu_data->page[3], default_data_ntag_i2c, sizeof(default_data_ntag_i2c));
    mfu_data->page[3].data[2] = 0xEA; // Size of tag in CC

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag_i2c_plus_common(
    MfUltralightData* mfu_data,
    MfUltralightType type,
    uint16_t num_pages) {
    nfc_generate_ntag_i2c_common(mfu_data, type, num_pages);

    uint16_t config_index = 227;
    mfu_data->page[config_index].data[3] = 0xff; // AUTH0

    memset(&mfu_data->page[config_index + 2], 0xFF, sizeof(MfUltralightPage)); // Default PWD
}

static void nfc_generate_ntag_i2c_plus_1k(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag_i2c_plus_common(mfu_data, MfUltralightTypeNTAGI2CPlus1K, 236);
    mfu_data->version.prod_ver_minor = 0x02;
    mfu_data->version.storage_size = 0x13;

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_ntag_i2c_plus_2k(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();

    nfc_generate_ntag_i2c_plus_common(mfu_data, MfUltralightTypeNTAGI2CPlus2K, 492);
    mfu_data->version.prod_ver_minor = 0x02;
    mfu_data->version.storage_size = 0x15;

    nfc_device_set_data(nfc_device, NfcProtocolMfUltralight, mfu_data);
    mf_ultralight_free(mfu_data);
}

static void nfc_generate_mf_classic_uid(uint8_t* uid, uint8_t length) {
    uid[0] = NXP_MANUFACTURER_ID;
    furi_hal_random_fill_buf(&uid[1], length - 1);
    uid[3] |= 0x01; // To avoid forbidden 0x88 value
}

static void
    nfc_generate_mf_classic_common(MfClassicData* data, uint8_t uid_len, MfClassicType type) {
    data->iso14443_3a_data->uid_len = uid_len;
    data->iso14443_3a_data->atqa[0] = 0x00;
    data->iso14443_3a_data->atqa[1] = 0x00;
    data->iso14443_3a_data->sak = 0x00;
    // Calculate the proper ATQA and SAK
    if(uid_len == 7) {
        data->iso14443_3a_data->atqa[0] |= 0x40;
    }
    if(type == MfClassicType1k) {
        data->iso14443_3a_data->atqa[0] |= 0x04;
        data->iso14443_3a_data->sak = 0x08;
    } else if(type == MfClassicType4k) {
        data->iso14443_3a_data->atqa[0] |= 0x02;
        data->iso14443_3a_data->sak = 0x18;
    } else if(type == MfClassicTypeMini) {
        data->iso14443_3a_data->atqa[0] |= 0x08;
        data->iso14443_3a_data->sak = 0x09;
    }
    data->type = type;
}

static void nfc_generate_mf_classic_sector_trailer(MfClassicData* data, uint8_t block) {
    // All keys are set to FFFF FFFF FFFFh at chip delivery and the bytes 6, 7 and 8 are set to FF0780h.
    MfClassicSectorTrailer* sec_tr = (MfClassicSectorTrailer*)data->block[block].data;
    sec_tr->access_bits.data[0] = 0xFF;
    sec_tr->access_bits.data[1] = 0x07;
    sec_tr->access_bits.data[2] = 0x80;
    sec_tr->access_bits.data[3] = 0x69; // Nice

    for(int i = 0; i < 6; i++) {
        sec_tr->key_a.data[i] = 0xFF;
        sec_tr->key_b.data[i] = 0xFF;
    }

    mf_classic_set_block_read(data, block, &data->block[block]);
    mf_classic_set_key_found(
        data, mf_classic_get_sector_by_block(block), MfClassicKeyTypeA, 0xFFFFFFFFFFFF);
    mf_classic_set_key_found(
        data, mf_classic_get_sector_by_block(block), MfClassicKeyTypeB, 0xFFFFFFFFFFFF);
}

static void nfc_generate_mf_classic_block_0(
    uint8_t* block,
    uint8_t uid_len,
    uint8_t sak,
    uint8_t atqa0,
    uint8_t atqa1) {
    // Block length is always 16 bytes, and the UID can be either 4 or 7 bytes
    furi_assert(uid_len == 4 || uid_len == 7);
    furi_assert(block);

    if(uid_len == 7) {
        uid_len -= 1;
    }

    block[uid_len + 1] = sak;
    block[uid_len + 2] = atqa0;
    block[uid_len + 3] = atqa1;

    for(int i = uid_len + 4; i < 16; i++) {
        block[i] = 0xFF;
    }
}

static void nfc_generate_mf_classic(NfcDevice* nfc_device, uint8_t uid_len, MfClassicType type) {
    MfClassicData* mfc_data = mf_classic_alloc();

    uint8_t uid[ISO14443_3A_MAX_UID_SIZE];

    nfc_generate_mf_classic_uid(uid, uid_len);
    mf_classic_set_uid(mfc_data, uid, uid_len);

    nfc_generate_mf_classic_common(mfc_data, uid_len, type);

    mf_classic_set_block_read(mfc_data, 0, &mfc_data->block[0]);

    uint16_t block_num = mf_classic_get_total_block_num(type);
    if(type == MfClassicType4k) {
        // Set every block to 0x00
        for(uint16_t i = 1; i < block_num; i++) {
            if(mf_classic_is_sector_trailer(i)) {
                nfc_generate_mf_classic_sector_trailer(mfc_data, i);
            } else {
                memset(&mfc_data->block[i].data, 0x00, 16);
            }
            mf_classic_set_block_read(mfc_data, i, &mfc_data->block[i]);
        }
    } else if(type == MfClassicType1k) {
        // Set every block to 0x00
        for(uint16_t i = 1; i < block_num; i++) {
            if(mf_classic_is_sector_trailer(i)) {
                nfc_generate_mf_classic_sector_trailer(mfc_data, i);
            } else {
                memset(&mfc_data->block[i].data, 0x00, 16);
            }
            mf_classic_set_block_read(mfc_data, i, &mfc_data->block[i]);
        }
    } else if(type == MfClassicTypeMini) {
        // Set every block to 0x00
        for(uint16_t i = 1; i < block_num; i++) {
            if(mf_classic_is_sector_trailer(i)) {
                nfc_generate_mf_classic_sector_trailer(mfc_data, i);
            } else {
                memset(&mfc_data->block[i].data, 0x00, 16);
            }
            mf_classic_set_block_read(mfc_data, i, &mfc_data->block[i]);
        }
    }

    nfc_generate_mf_classic_block_0(
        mfc_data->block[0].data,
        uid_len,
        mfc_data->iso14443_3a_data->sak,
        mfc_data->iso14443_3a_data->atqa[0],
        mfc_data->iso14443_3a_data->atqa[1]);

    mfc_data->type = type;

    nfc_device_set_data(nfc_device, NfcProtocolMfClassic, mfc_data);
    mf_classic_free(mfc_data);
}

static void nfc_generate_mf_classic_mini(NfcDevice* nfc_device) {
    nfc_generate_mf_classic(nfc_device, 4, MfClassicTypeMini);
}

static void nfc_generate_mf_classic_1k_4b_uid(NfcDevice* nfc_device) {
    nfc_generate_mf_classic(nfc_device, 4, MfClassicType1k);
}

static void nfc_generate_mf_classic_1k_7b_uid(NfcDevice* nfc_device) {
    nfc_generate_mf_classic(nfc_device, 7, MfClassicType1k);
}

static void nfc_generate_mf_classic_4k_4b_uid(NfcDevice* nfc_device) {
    nfc_generate_mf_classic(nfc_device, 4, MfClassicType4k);
}

static void nfc_generate_mf_classic_4k_7b_uid(NfcDevice* nfc_device) {
    nfc_generate_mf_classic(nfc_device, 7, MfClassicType4k);
}

static const NfcDataGenerator nfc_data_generator[NfcDataGeneratorTypeNum] = {
    [NfcDataGeneratorTypeMfUltralight] =
        {
            .name = "Mifare Ultralight",
            .handler = nfc_generate_mf_ul_orig,
        },
    [NfcDataGeneratorTypeMfUltralightEV1_11] =
        {
            .name = "Mifare Ultralight EV1 11",
            .handler = nfc_generate_mf_ul_11,
        },
    [NfcDataGeneratorTypeMfUltralightEV1_H11] =
        {
            .name = "Mifare Ultralight EV1 H11",
            .handler = nfc_generate_mf_ul_h11,
        },
    [NfcDataGeneratorTypeMfUltralightEV1_21] =
        {
            .name = "Mifare Ultralight EV1 21",
            .handler = nfc_generate_mf_ul_21,
        },
    [NfcDataGeneratorTypeMfUltralightEV1_H21] =
        {
            .name = "Mifare Ultralight EV1 H21",
            .handler = nfc_generate_mf_ul_h21,
        },
    [NfcDataGeneratorTypeNTAG203] =
        {
            .name = "NTAG203",
            .handler = nfc_generate_ntag203,
        },
    [NfcDataGeneratorTypeNTAG213] =
        {
            .name = "NTAG213",
            .handler = nfc_generate_ntag213,
        },
    [NfcDataGeneratorTypeNTAG215] =
        {
            .name = "NTAG215",
            .handler = nfc_generate_ntag215,
        },
    [NfcDataGeneratorTypeNTAG216] =
        {
            .name = "NTAG216",
            .handler = nfc_generate_ntag216,
        },
    [NfcDataGeneratorTypeNTAGI2C1k] =
        {
            .name = "NTAG I2C 1k",
            .handler = nfc_generate_ntag_i2c_1k,
        },
    [NfcDataGeneratorTypeNTAGI2C2k] =
        {
            .name = "NTAG I2C 2k",
            .handler = nfc_generate_ntag_i2c_2k,
        },
    [NfcDataGeneratorTypeNTAGI2CPlus1k] =
        {
            .name = "NTAG I2C Plus 1k",
            .handler = nfc_generate_ntag_i2c_plus_1k,
        },
    [NfcDataGeneratorTypeNTAGI2CPlus2k] =
        {
            .name = "NTAG I2C Plus 2k",
            .handler = nfc_generate_ntag_i2c_plus_2k,
        },
    [NfcDataGeneratorTypeMfClassicMini] =
        {
            .name = "Mifare Mini",
            .handler = nfc_generate_mf_classic_mini,
        },
    [NfcDataGeneratorTypeMfClassic1k_4b] =
        {
            .name = "Mifare Classic 1k 4byte UID",
            .handler = nfc_generate_mf_classic_1k_4b_uid,
        },
    [NfcDataGeneratorTypeMfClassic1k_7b] =
        {
            .name = "Mifare Classic 1k 7byte UID",
            .handler = nfc_generate_mf_classic_1k_7b_uid,
        },
    [NfcDataGeneratorTypeMfClassic4k_4b] =
        {
            .name = "Mifare Classic 4k 4byte UID",
            .handler = nfc_generate_mf_classic_4k_4b_uid,
        },
    [NfcDataGeneratorTypeMfClassic4k_7b] =
        {
            .name = "Mifare Classic 4k 7byte UID",
            .handler = nfc_generate_mf_classic_4k_7b_uid,
        },
};

const char* nfc_data_generator_get_name(NfcDataGeneratorType type) {
    furi_check(type < NfcDataGeneratorTypeNum);

    return nfc_data_generator[type].name;
}

void nfc_data_generator_fill_data(NfcDataGeneratorType type, NfcDevice* nfc_device) {
    furi_check(type < NfcDataGeneratorTypeNum);
    furi_check(nfc_device);

    nfc_data_generator[type].handler(nfc_device);
}
