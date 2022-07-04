#include <furi_hal_random.h>
#include "nfc_generators.h"

#define NXP_MANUFACTURER_ID (0x04)

static const uint8_t version_bytes_mf0ulx1[] = {0x00, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x03};
static const uint8_t version_bytes_ntag21x[] = {0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x03};
static const uint8_t version_bytes_ntag_i2c[] = {0x00, 0x04, 0x04, 0x05, 0x02, 0x00, 0x00, 0x03};
static const uint8_t default_data_ntag213[] = {0x01, 0x03, 0xA0, 0x0C, 0x34, 0x03, 0x00, 0xFE};
static const uint8_t default_data_ntag215_216[] = {0x03, 0x00, 0xFE};
static const uint8_t default_data_ntag_i2c[] = {0xE1, 0x10, 0x00, 0x00, 0x03, 0x00, 0xFE};
static const uint8_t default_config_ntag_i2c[] = {0x01, 0x00, 0xF8, 0x48, 0x08, 0x01, 0x00, 0x00};

static void nfc_generate_common_start(NfcDeviceData* data) {
    nfc_device_data_clear(data);
}

static void nfc_generate_mf_ul_uid(uint8_t* uid) {
    uid[0] = NXP_MANUFACTURER_ID;
    furi_hal_random_fill_buf(&uid[1], 6);
    // I'm not sure how this is generated, but the upper nybble always seems to be 8
    uid[6] &= 0x0F;
    uid[6] |= 0x80;
}

static void nfc_generate_mf_ul_common(NfcDeviceData* data) {
    data->nfc_data.type = FuriHalNfcTypeA;
    data->nfc_data.interface = FuriHalNfcInterfaceRf;
    data->nfc_data.uid_len = 7;
    nfc_generate_mf_ul_uid(data->nfc_data.uid);
    data->nfc_data.atqa[0] = 0x44;
    data->nfc_data.atqa[1] = 0x00;
    data->nfc_data.sak = 0x00;
    data->protocol = NfcDeviceProtocolMifareUl;
}

static void nfc_generate_calc_bcc(uint8_t* uid, uint8_t* bcc0, uint8_t* bcc1) {
    *bcc0 = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
    *bcc1 = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];
}

static void nfc_generate_mf_ul_copy_uid_with_bcc(NfcDeviceData* data) {
    MfUltralightData* mful = &data->mf_ul_data;
    memcpy(mful->data, data->nfc_data.uid, 3);
    memcpy(&mful->data[4], &data->nfc_data.uid[3], 4);
    nfc_generate_calc_bcc(data->nfc_data.uid, &mful->data[3], &mful->data[8]);
}

static void nfc_generate_mf_ul_orig(NfcDeviceData* data) {
    nfc_generate_common_start(data);
    nfc_generate_mf_ul_common(data);

    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeUnknown;
    mful->data_size = 16 * 4;
    nfc_generate_mf_ul_copy_uid_with_bcc(data);
    // TODO: what's internal byte on page 2?
    memset(&mful->data[4 * 4], 0xFF, 4);
}

static void nfc_generate_mf_ul_with_config_common(NfcDeviceData* data, uint8_t num_pages) {
    nfc_generate_common_start(data);
    nfc_generate_mf_ul_common(data);

    MfUltralightData* mful = &data->mf_ul_data;
    mful->data_size = num_pages * 4;
    nfc_generate_mf_ul_copy_uid_with_bcc(data);
    uint16_t config_index = (num_pages - 4) * 4;
    mful->data[config_index] = 0x04; // STRG_MOD_EN
    mful->data[config_index + 3] = 0xFF; // AUTH0
    mful->data[config_index + 5] = 0x05; // VCTID
    memset(&mful->data[config_index + 8], 0xFF, 4); // Default PWD
    if(num_pages > 20) mful->data[config_index - 1] = MF_UL_TEARING_FLAG_DEFAULT;
}

static void nfc_generate_mf_ul_ev1_common(NfcDeviceData* data, uint8_t num_pages) {
    nfc_generate_mf_ul_with_config_common(data, num_pages);
    MfUltralightData* mful = &data->mf_ul_data;
    memcpy(&mful->version, version_bytes_mf0ulx1, sizeof(version_bytes_mf0ulx1));
    for(size_t i = 0; i < 3; ++i) {
        mful->tearing[i] = MF_UL_TEARING_FLAG_DEFAULT;
    }
    // TODO: what's internal byte on page 2?
}

static void nfc_generate_mf_ul_11(NfcDeviceData* data) {
    nfc_generate_mf_ul_ev1_common(data, 20);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeUL11;
    mful->version.prod_subtype = 0x01;
    mful->version.storage_size = 0x0B;
    mful->data[16 * 4] = 0x00; // Low capacitance version does not have STRG_MOD_EN
}

static void nfc_generate_mf_ul_h11(NfcDeviceData* data) {
    nfc_generate_mf_ul_ev1_common(data, 20);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeUL11;
    mful->version.prod_subtype = 0x02;
    mful->version.storage_size = 0x0B;
}

static void nfc_generate_mf_ul_21(NfcDeviceData* data) {
    nfc_generate_mf_ul_ev1_common(data, 41);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeUL21;
    mful->version.prod_subtype = 0x01;
    mful->version.storage_size = 0x0E;
    mful->data[37 * 4] = 0x00; // Low capacitance version does not have STRG_MOD_EN
}

static void nfc_generate_mf_ul_h21(NfcDeviceData* data) {
    nfc_generate_mf_ul_ev1_common(data, 41);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeUL21;
    mful->version.prod_subtype = 0x02;
    mful->version.storage_size = 0x0E;
}

static void nfc_generate_ntag21x_common(NfcDeviceData* data, uint8_t num_pages) {
    nfc_generate_mf_ul_with_config_common(data, num_pages);
    MfUltralightData* mful = &data->mf_ul_data;
    memcpy(&mful->version, version_bytes_ntag21x, sizeof(version_bytes_mf0ulx1));
    mful->data[9] = 0x48; // Internal byte
    // Capability container
    mful->data[12] = 0xE1;
    mful->data[13] = 0x10;
}

static void nfc_generate_ntag213(NfcDeviceData* data) {
    nfc_generate_ntag21x_common(data, 45);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeNTAG213;
    mful->version.storage_size = 0x0F;
    mful->data[14] = 0x12;
    // Default contents
    memcpy(&mful->data[16], default_data_ntag213, sizeof(default_data_ntag213));
}

static void nfc_generate_ntag215(NfcDeviceData* data) {
    nfc_generate_ntag21x_common(data, 135);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeNTAG215;
    mful->version.storage_size = 0x11;
    mful->data[14] = 0x3E;
    // Default contents
    memcpy(&mful->data[16], default_data_ntag215_216, sizeof(default_data_ntag215_216));
}

static void nfc_generate_ntag216(NfcDeviceData* data) {
    nfc_generate_ntag21x_common(data, 231);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = MfUltralightTypeNTAG216;
    mful->version.storage_size = 0x13;
    mful->data[14] = 0x6D;
    // Default contents
    memcpy(&mful->data[16], default_data_ntag215_216, sizeof(default_data_ntag215_216));
}

static void
    nfc_generate_ntag_i2c_common(NfcDeviceData* data, MfUltralightType type, uint16_t num_pages) {
    nfc_generate_common_start(data);
    nfc_generate_mf_ul_common(data);

    MfUltralightData* mful = &data->mf_ul_data;
    mful->type = type;
    memcpy(&mful->version, version_bytes_ntag_i2c, sizeof(version_bytes_ntag_i2c));
    mful->data_size = num_pages * 4;
    memcpy(mful->data, data->nfc_data.uid, data->nfc_data.uid_len);
    mful->data[7] = data->nfc_data.sak;
    mful->data[8] = data->nfc_data.atqa[0];
    mful->data[9] = data->nfc_data.atqa[1];

    uint16_t config_register_page;
    uint16_t session_register_page;

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
        furi_assert(false);
        break;
    }

    memcpy(
        &mful->data[config_register_page * 4],
        default_config_ntag_i2c,
        sizeof(default_config_ntag_i2c));
    memcpy(
        &mful->data[session_register_page * 4],
        default_config_ntag_i2c,
        sizeof(default_config_ntag_i2c));
}

static void nfc_generate_ntag_i2c_1k(NfcDeviceData* data) {
    nfc_generate_ntag_i2c_common(data, MfUltralightTypeNTAGI2C1K, 231);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->version.prod_ver_minor = 0x01;
    mful->version.storage_size = 0x13;

    memcpy(&mful->data[12], default_data_ntag_i2c, sizeof(default_data_ntag_i2c));
    mful->data[14] = 0x6D; // Size of tag in CC
}

static void nfc_generate_ntag_i2c_2k(NfcDeviceData* data) {
    nfc_generate_ntag_i2c_common(data, MfUltralightTypeNTAGI2C2K, 485);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->version.prod_ver_minor = 0x01;
    mful->version.storage_size = 0x15;

    memcpy(&mful->data[12], default_data_ntag_i2c, sizeof(default_data_ntag_i2c));
    mful->data[14] = 0xEA; // Size of tag in CC
}

static void nfc_generate_ntag_i2c_plus_common(
    NfcDeviceData* data,
    MfUltralightType type,
    uint16_t num_pages) {
    nfc_generate_ntag_i2c_common(data, type, num_pages);

    MfUltralightData* mful = &data->mf_ul_data;
    uint16_t config_index = 227 * 4;
    mful->data[config_index + 3] = 0xFF; // AUTH0
    memset(&mful->data[config_index + 8], 0xFF, 4); // Default PWD
}

static void nfc_generate_ntag_i2c_plus_1k(NfcDeviceData* data) {
    nfc_generate_ntag_i2c_plus_common(data, MfUltralightTypeNTAGI2CPlus1K, 236);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->version.prod_ver_minor = 0x02;
    mful->version.storage_size = 0x13;
}

static void nfc_generate_ntag_i2c_plus_2k(NfcDeviceData* data) {
    nfc_generate_ntag_i2c_plus_common(data, MfUltralightTypeNTAGI2CPlus2K, 492);
    MfUltralightData* mful = &data->mf_ul_data;
    mful->version.prod_ver_minor = 0x02;
    mful->version.storage_size = 0x15;
}

static const NfcGenerator mf_ul_generator = {
    .name = "Mifare Ultralight",
    .generator_func = nfc_generate_mf_ul_orig,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator mf_ul_11_generator = {
    .name = "Mifare Ultralight EV1 11",
    .generator_func = nfc_generate_mf_ul_11,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator mf_ul_h11_generator = {
    .name = "Mifare Ultralight EV1 H11",
    .generator_func = nfc_generate_mf_ul_h11,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator mf_ul_21_generator = {
    .name = "Mifare Ultralight EV1 21",
    .generator_func = nfc_generate_mf_ul_21,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator mf_ul_h21_generator = {
    .name = "Mifare Ultralight EV1 H21",
    .generator_func = nfc_generate_mf_ul_h21,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag213_generator = {
    .name = "NTAG213",
    .generator_func = nfc_generate_ntag213,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag215_generator = {
    .name = "NTAG215",
    .generator_func = nfc_generate_ntag215,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag216_generator = {
    .name = "NTAG216",
    .generator_func = nfc_generate_ntag216,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag_i2c_1k_generator = {
    .name = "NTAG I2C 1k",
    .generator_func = nfc_generate_ntag_i2c_1k,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag_i2c_2k_generator = {
    .name = "NTAG I2C 2k",
    .generator_func = nfc_generate_ntag_i2c_2k,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag_i2c_plus_1k_generator = {
    .name = "NTAG I2C Plus 1k",
    .generator_func = nfc_generate_ntag_i2c_plus_1k,
    .next_scene = NfcSceneMifareUlMenu};

static const NfcGenerator ntag_i2c_plus_2k_generator = {
    .name = "NTAG I2C Plus 2k",
    .generator_func = nfc_generate_ntag_i2c_plus_2k,
    .next_scene = NfcSceneMifareUlMenu};

const NfcGenerator* const nfc_generators[] = {
    &mf_ul_generator,
    &mf_ul_11_generator,
    &mf_ul_h11_generator,
    &mf_ul_21_generator,
    &mf_ul_h21_generator,
    &ntag213_generator,
    &ntag215_generator,
    &ntag216_generator,
    &ntag_i2c_1k_generator,
    &ntag_i2c_2k_generator,
    &ntag_i2c_plus_1k_generator,
    &ntag_i2c_plus_2k_generator,
    NULL,
};
