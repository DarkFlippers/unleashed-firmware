#include "nfc_data_generator.h"

#include <furi/furi.h>
#include <furi_hal_random.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <nfc/protocols/mf_plus/mf_plus.h>
#include <nfc/protocols/mf_plus/mf_plus_i.h> // set_* accessors for the blank-format fill
#include <toolbox/simple_array.h>

#define NXP_MANUFACTURER_ID (0x04)

// MIFARE Plus ATS historical bytes (the "C1 05 <type/size> <gen> <caps> <crc16>" block). 7 bytes.
#define MF_PLUS_ATS_HIST_LEN (7)

typedef void (*NfcDataGeneratorHandler)(NfcDevice* nfc_device);

typedef struct {
    const char* name;
    NfcDataGeneratorHandler handler;
} NfcDataGenerator;

static const uint8_t version_bytes_mf0ulx1[] = {0x00, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x03};
static const uint8_t version_bytes_mf0aes20[] = {0x00, 0x04, 0x03, 0x01, 0x04, 0x00, 0x0F, 0x03};
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

static void nfc_generate_mf_ultralight_aes(NfcDevice* nfc_device) {
    MfUltralightData* mfu_data = mf_ultralight_alloc();
    nfc_generate_mf_ul_common(mfu_data);

    mfu_data->type = MfUltralightTypeUltralightAES;
    mfu_data->pages_total = 60; // MF0AES20: 60 pages, 0x00-0x3B
    mfu_data->pages_read = 60;
    memcpy(&mfu_data->version, version_bytes_mf0aes20, sizeof(MfUltralightVersion));
    mfu_data->page[2].data[1] = 0x48; // Internal byte (MF0AES20 default)

    // Factory-default config (matches a blank MF0AES20): AUTH0 protects nothing (open); and, left at
    // their all-zero defaults, AUTH_LIM unlimited, Random ID / secure messaging off, key locks off,
    // and the DataProtKey (0x30-0x33) / UIDRetrKey (0x34-0x37).
    mfu_data->page[MF_ULTRALIGHT_AES_CFG_PAGE].data[3] = 0x3C; // AUTH0 > 0x3B => disabled
    mfu_data->page[MF_ULTRALIGHT_AES_ACCESS_PAGE].data[0] = MF_ULTRALIGHT_AES_ACCESS_PROT |
                                                            MF_ULTRALIGHT_AES_ACCESS_CNT_INC_EN |
                                                            MF_ULTRALIGHT_AES_ACCESS_CNT_RD_EN;
    mfu_data->page[MF_ULTRALIGHT_AES_ACCESS_PAGE].data[1] = 0x05; // VCTID

    // Placeholder originality signature so the emulated card answers READ_SIG; random, so it won't
    // verify against NXP's key - expected for a fabricated card.
    furi_hal_random_fill_buf(mfu_data->aes_signature, MF_ULTRALIGHT_AES_SIGNATURE_SIZE);
    mfu_data->aes_signature_present = true;

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

    // Set every block to 0x00
    uint16_t block_num = mf_classic_get_total_block_num(type);
    for(uint16_t i = 1; i < block_num; i++) {
        if(mf_classic_is_sector_trailer(i)) {
            nfc_generate_mf_classic_sector_trailer(mfc_data, i);
        } else {
            memset(&mfc_data->block[i].data, 0x00, MF_CLASSIC_BLOCK_SIZE);
        }
        mf_classic_set_block_read(mfc_data, i, &mfc_data->block[i]);
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

// MIFARE Plus ATS historical bytes per product. Only the type/size nibbles are load-bearing for
// detection (see mf_plus_type_from_ats): S/X mask their size with 2F 2F and differ by the caps
// byte's SVC bit; SE reveals 21 30. The trailing CRC16 floats between configs -- the SE value is a
// real capture (77 C1), the S/X values are AN10833. EV1/EV2 identify via GetVersion, not the ATS, so
// they reuse the byte-identical S block.
static const uint8_t mf_plus_ats_hist_se[MF_PLUS_ATS_HIST_LEN] =
    {0xC1, 0x05, 0x21, 0x30, 0x00, 0x77, 0xC1};
static const uint8_t mf_plus_ats_hist_s[MF_PLUS_ATS_HIST_LEN] =
    {0xC1, 0x05, 0x2F, 0x2F, 0x00, 0x35, 0xC7};
static const uint8_t mf_plus_ats_hist_x[MF_PLUS_ATS_HIST_LEN] =
    {0xC1, 0x05, 0x2F, 0x2F, 0x01, 0xBC, 0xD6};

// EV1/EV2 answer GetVersion; fill a plausible response. hw_type low nibble 0x02 = Plus, hw_major
// 0x11/0x22 = EV1/EV2, hw_storage 0x16/0x18 = 2K/4K (matches mf_plus_get_type_from_version). The
// 7-byte manufacturing UID mirrors the anticollision UID for a 7-byte card, else a fresh random NXP
// UID (a 4-byte-UID card still reports a full 7-byte UID from GetVersion).
static void nfc_generate_mf_plus_version(
    MfPlusVersion* version,
    uint8_t hw_major,
    uint8_t hw_storage,
    const uint8_t* card_uid,
    uint8_t uid_len) {
    // Field values match a real MIFARE Plus EV1 GetVersion capture (hw_proto 0x04; the software
    // version reports its own major/minor 0x01/0x01, independent of the hardware major).
    version->hw_vendor = NXP_MANUFACTURER_ID;
    version->hw_type = 0x02;
    version->hw_subtype = 0x01;
    version->hw_major = hw_major;
    version->hw_minor = 0x00;
    version->hw_storage = hw_storage;
    version->hw_proto = 0x04;

    version->sw_vendor = NXP_MANUFACTURER_ID;
    version->sw_type = 0x02;
    version->sw_subtype = 0x01;
    version->sw_major = 0x01;
    version->sw_minor = 0x01;
    version->sw_storage = hw_storage;
    version->sw_proto = 0x04;

    if(uid_len == 7) {
        memcpy(version->uid, card_uid, 7);
    } else {
        version->uid[0] = NXP_MANUFACTURER_ID;
        furi_hal_random_fill_buf(&version->uid[1], 6);
    }
    furi_hal_random_fill_buf(version->batch, sizeof(version->batch));
    version->prod_week = 0x18;
    version->prod_year = 0x18;
}

// Fill a manually-added SL3 card with a blank-formatted factory state -- the MIFARE Plus analogue of
// the MIFARE Classic generator's 0x00 blocks + FFFFFF.. keys. Without it a manual card reads as all
// "??" (nothing recovered), which is useless to emulate or inspect. Data blocks are zeroed and marked
// read; every sector and admin AES key is the all-FF default and marked found; config blocks zeroed.
static void nfc_generate_mf_plus_default_content(MfPlusData* data) {
    MfPlusKey default_key;
    memset(default_key.data, 0xFF, MF_PLUS_KEY_SIZE);

    MfPlusBlock zero_block;
    memset(zero_block.data, 0x00, MF_PLUS_BLOCK_SIZE);

    const uint16_t block_count = mf_plus_get_block_count(data->size);
    for(uint16_t b = 0; b < block_count; b++) {
        mf_plus_set_block_read(data, b, &zero_block);
    }

    const uint8_t sector_count = mf_plus_get_sector_count(data->size);
    for(uint8_t s = 0; s < sector_count; s++) {
        mf_plus_set_key_found(data, s, MfPlusKeyTypeA, &default_key);
        mf_plus_set_key_found(data, s, MfPlusKeyTypeB, &default_key);
    }

    for(uint8_t a = 0; a < MfPlusAdminKeyNum; a++) {
        mf_plus_set_admin_key_found(data, (MfPlusAdminKeyType)a, &default_key);
    }

    for(uint8_t c = 0; c < MF_PLUS_CONFIG_BLOCK_NUM; c++) {
        mf_plus_set_config_block_read(data, c, &zero_block);
    }
}

// Block 0 is the read-only manufacturer block: UID + (BCC, on 4-byte UIDs) + SAK + ATQA +
// manufacturer bytes, same layout as MIFARE Classic. A real card's block 0 is never all-zero, so
// overwrite the blank-format zero fill for it.
static void nfc_generate_mf_plus_block_0(
    MfPlusData* data,
    const uint8_t* uid,
    uint8_t uid_len,
    uint8_t sak,
    uint8_t atqa0,
    uint8_t atqa1) {
    MfPlusBlock block0;
    memset(block0.data, 0xFF, MF_PLUS_BLOCK_SIZE); // manufacturer bytes
    memcpy(block0.data, uid, uid_len);

    uint8_t offset = uid_len;
    if(uid_len == 4) {
        block0.data[4] = uid[0] ^ uid[1] ^ uid[2] ^
                         uid[3]; // BCC (7-byte UIDs carry no block-0 BCC)
        offset = 5;
    }
    block0.data[offset] = sak;
    block0.data[offset + 1] = atqa0;
    block0.data[offset + 2] = atqa1;

    mf_plus_set_block_read(data, 0, &block0);
}

// Build a manually-added MIFARE Plus card at SL3 (the native ISO14443-4 presentation this app fully
// supports): a random UID plus the product's identity (ATQA/SAK/ATS/type/size), a blank-formatted
// factory memory (default keys + zeroed blocks), and for EV1/EV2 a GetVersion response and a
// placeholder originality signature.
static void nfc_generate_mf_plus(
    NfcDevice* nfc_device,
    uint8_t uid_len,
    MfPlusType type,
    MfPlusSize size,
    const uint8_t* ats_hist) {
    furi_assert(uid_len == 4 || uid_len == 7);

    MfPlusData* data = mf_plus_alloc();

    uint8_t uid[MF_PLUS_UID_SIZE_MAX];
    uid[0] = NXP_MANUFACTURER_ID;
    furi_hal_random_fill_buf(&uid[1], uid_len - 1);
    uid[3] |=
        0x01; // avoid the forbidden 0x88 cascade tag as the first byte of CL2 in a 7-byte UID
    mf_plus_set_uid(data, uid, uid_len);

    // SL3 presents SAK 0x20 (ISO14443-4). ATQA size bit: 4K = 0x02, 1K/2K = 0x04; +0x40 for a
    // 7-byte UID. SE's size comes from its ATS, not the ATQA, so it shares the 1K/2K coding.
    Iso14443_3aData* iso3 = iso14443_4a_get_base_data(data->iso14443_4a_data);
    iso3->atqa[0] = (size == MfPlusSize4K) ? 0x02 : 0x04;
    if(uid_len == 7) {
        iso3->atqa[0] |= 0x40;
    }
    iso3->atqa[1] = 0x00;
    iso3->sak = 0x20;

    // Standard MIFARE Plus ATS: TL T0 TA TB TC + 7 historical bytes (FSCI 5 = 64-byte frame, TA/TB/TC
    // present). tl is the full ATS length so iso14443_4a_save round-trips it.
    Iso14443_4aAtsData* ats = &data->iso14443_4a_data->ats_data;
    ats->t0 = 0x75;
    ats->ta_1 = 0x77;
    ats->tb_1 = 0x80;
    ats->tc_1 = 0x02;
    simple_array_init(ats->t1_tk, MF_PLUS_ATS_HIST_LEN);
    memcpy(simple_array_get_data(ats->t1_tk), ats_hist, MF_PLUS_ATS_HIST_LEN);
    ats->tl = 5 + MF_PLUS_ATS_HIST_LEN;

    data->type = type;
    data->size = size;
    data->security_level = MfPlusSecurityLevel3;

    nfc_generate_mf_plus_default_content(data);
    nfc_generate_mf_plus_block_0(data, uid, uid_len, iso3->sak, iso3->atqa[0], iso3->atqa[1]);

    // EV1/EV2 answer GetVersion and carry an originality signature; the EV0 products (SE/S/X) do
    // neither, so their version stays zeroed (shown as "no GetVersion") and signature_present false.
    if(type == MfPlusTypeEV1 || type == MfPlusTypeEV2) {
        const uint8_t hw_major = (type == MfPlusTypeEV2) ? 0x22 : 0x11;
        const uint8_t hw_storage = (size == MfPlusSize4K) ? 0x18 : 0x16;
        nfc_generate_mf_plus_version(&data->version, hw_major, hw_storage, uid, uid_len);
        furi_hal_random_fill_buf(data->signature, MF_PLUS_SIGNATURE_SIZE);
        data->signature_present = true;
    }

    nfc_device_set_data(nfc_device, NfcProtocolMfPlus, data);
    mf_plus_free(data);
}

// The 18 MIFARE Plus variants differ only by parameters (UID length, product type, memory size and
// ATS historical bytes), so they are data-driven from this table rather than a thunk each. Indexed
// by (type - NfcDataGeneratorTypeMfPlusSE_4b), so the row order must track the enum. EV1/EV2 reuse
// the S ATS block (byte-identical; those products are identified by GetVersion, not the ATS).
typedef struct {
    const char* name;
    uint8_t uid_len;
    uint8_t type; // MfPlusType
    uint8_t size; // MfPlusSize
    const uint8_t* ats_hist;
} MfPlusGeneratorConfig;

static const MfPlusGeneratorConfig mf_plus_generator_configs[] = {
    {"Mifare Plus SE 4byte UID", 4, MfPlusTypeSE, MfPlusSize1K, mf_plus_ats_hist_se},
    {"Mifare Plus SE 7byte UID", 7, MfPlusTypeSE, MfPlusSize1K, mf_plus_ats_hist_se},
    {"Mifare Plus S 2K 4byte UID", 4, MfPlusTypeS, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus S 2K 7byte UID", 7, MfPlusTypeS, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus S 4K 4byte UID", 4, MfPlusTypeS, MfPlusSize4K, mf_plus_ats_hist_s},
    {"Mifare Plus S 4K 7byte UID", 7, MfPlusTypeS, MfPlusSize4K, mf_plus_ats_hist_s},
    {"Mifare Plus X 2K 4byte UID", 4, MfPlusTypeX, MfPlusSize2K, mf_plus_ats_hist_x},
    {"Mifare Plus X 2K 7byte UID", 7, MfPlusTypeX, MfPlusSize2K, mf_plus_ats_hist_x},
    {"Mifare Plus X 4K 4byte UID", 4, MfPlusTypeX, MfPlusSize4K, mf_plus_ats_hist_x},
    {"Mifare Plus X 4K 7byte UID", 7, MfPlusTypeX, MfPlusSize4K, mf_plus_ats_hist_x},
    {"Mifare Plus EV1 2K 4byte UID", 4, MfPlusTypeEV1, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus EV1 2K 7byte UID", 7, MfPlusTypeEV1, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus EV1 4K 4byte UID", 4, MfPlusTypeEV1, MfPlusSize4K, mf_plus_ats_hist_s},
    {"Mifare Plus EV1 4K 7byte UID", 7, MfPlusTypeEV1, MfPlusSize4K, mf_plus_ats_hist_s},
    {"Mifare Plus EV2 2K 4byte UID", 4, MfPlusTypeEV2, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus EV2 2K 7byte UID", 7, MfPlusTypeEV2, MfPlusSize2K, mf_plus_ats_hist_s},
    {"Mifare Plus EV2 4K 4byte UID", 4, MfPlusTypeEV2, MfPlusSize4K, mf_plus_ats_hist_s},
    {"Mifare Plus EV2 4K 7byte UID", 7, MfPlusTypeEV2, MfPlusSize4K, mf_plus_ats_hist_s},
};

_Static_assert(
    COUNT_OF(mf_plus_generator_configs) ==
        (size_t)(NfcDataGeneratorTypeMfPlusEV2_4k_7b - NfcDataGeneratorTypeMfPlusSE_4b + 1),
    "mf_plus_generator_configs must cover every MIFARE Plus generator type");

static bool nfc_data_generator_type_is_mf_plus(NfcDataGeneratorType type) {
    return type >= NfcDataGeneratorTypeMfPlusSE_4b && type <= NfcDataGeneratorTypeMfPlusEV2_4k_7b;
}

// Handler-based table for the Ultralight/NTAG and Classic types, whose per-variant layouts are
// bespoke. The MIFARE Plus types are parametric and dispatched from mf_plus_generator_configs, so
// this table intentionally stops before them.
static const NfcDataGenerator nfc_data_generator[NfcDataGeneratorTypeMfPlusSE_4b] = {
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
    [NfcDataGeneratorTypeMfUltralightAES] =
        {
            .name = "Mifare Ultralight AES",
            .handler = nfc_generate_mf_ultralight_aes,
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

    if(nfc_data_generator_type_is_mf_plus(type)) {
        return mf_plus_generator_configs[type - NfcDataGeneratorTypeMfPlusSE_4b].name;
    }

    return nfc_data_generator[type].name;
}

void nfc_data_generator_fill_data(NfcDataGeneratorType type, NfcDevice* nfc_device) {
    furi_check(type < NfcDataGeneratorTypeNum);
    furi_check(nfc_device);

    if(nfc_data_generator_type_is_mf_plus(type)) {
        const MfPlusGeneratorConfig* config =
            &mf_plus_generator_configs[type - NfcDataGeneratorTypeMfPlusSE_4b];
        nfc_generate_mf_plus(
            nfc_device,
            config->uid_len,
            (MfPlusType)config->type,
            (MfPlusSize)config->size,
            config->ats_hist);
        return;
    }

    nfc_data_generator[type].handler(nfc_device);
}
