#include "mf_classic.h"

#include <furi/furi.h>
#include <toolbox/hex.h>

#include <lib/bit_lib/bit_lib.h>

#define MF_CLASSIC_PROTOCOL_NAME "Mifare Classic"

typedef struct {
    uint8_t sectors_total;
    uint16_t blocks_total;
    const char* full_name;
    const char* type_name;
} MfClassicFeatures;

static const uint32_t mf_classic_data_format_version = 2;

static const MfClassicFeatures mf_classic_features[MfClassicTypeNum] = {
    [MfClassicTypeMini] =
        {
            .sectors_total = 5,
            .blocks_total = 20,
            .full_name = "Mifare Classic Mini 0.3K",
            .type_name = "MINI",
        },
    [MfClassicType1k] =
        {
            .sectors_total = 16,
            .blocks_total = 64,
            .full_name = "Mifare Classic 1K",
            .type_name = "1K",
        },
    [MfClassicType4k] =
        {
            .sectors_total = 40,
            .blocks_total = 256,
            .full_name = "Mifare Classic 4K",
            .type_name = "4K",
        },
};

const NfcDeviceBase nfc_device_mf_classic = {
    .protocol_name = MF_CLASSIC_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)mf_classic_alloc,
    .free = (NfcDeviceFree)mf_classic_free,
    .reset = (NfcDeviceReset)mf_classic_reset,
    .copy = (NfcDeviceCopy)mf_classic_copy,
    .verify = (NfcDeviceVerify)mf_classic_verify,
    .load = (NfcDeviceLoad)mf_classic_load,
    .save = (NfcDeviceSave)mf_classic_save,
    .is_equal = (NfcDeviceEqual)mf_classic_is_equal,
    .get_name = (NfcDeviceGetName)mf_classic_get_device_name,
    .get_uid = (NfcDeviceGetUid)mf_classic_get_uid,
    .set_uid = (NfcDeviceSetUid)mf_classic_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)mf_classic_get_base_data,
};

MfClassicData* mf_classic_alloc(void) {
    MfClassicData* data = malloc(sizeof(MfClassicData));
    data->iso14443_3a_data = iso14443_3a_alloc();
    return data;
}

void mf_classic_free(MfClassicData* data) {
    furi_check(data);

    iso14443_3a_free(data->iso14443_3a_data);
    free(data);
}

void mf_classic_reset(MfClassicData* data) {
    furi_check(data);

    iso14443_3a_reset(data->iso14443_3a_data);
}

void mf_classic_copy(MfClassicData* data, const MfClassicData* other) {
    furi_check(data);
    furi_check(other);

    iso14443_3a_copy(data->iso14443_3a_data, other->iso14443_3a_data);
    for(size_t i = 0; i < COUNT_OF(data->block); i++) {
        data->block[i] = other->block[i];
    }
    for(size_t i = 0; i < COUNT_OF(data->block_read_mask); i++) {
        data->block_read_mask[i] = other->block_read_mask[i];
    }
    data->type = other->type;
    data->key_a_mask = other->key_a_mask;
    data->key_b_mask = other->key_b_mask;
}

bool mf_classic_verify(MfClassicData* data, const FuriString* device_type) {
    furi_check(device_type);
    UNUSED(data);

    return furi_string_equal_str(device_type, "Mifare Classic");
}

static void mf_classic_parse_block(FuriString* block_str, MfClassicData* data, uint8_t block_num) {
    furi_string_trim(block_str);
    MfClassicBlock block_tmp = {};
    bool is_sector_trailer = mf_classic_is_sector_trailer(block_num);
    uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
    uint16_t block_unknown_bytes_mask = 0;

    furi_string_trim(block_str);
    for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i++) {
        char hi = furi_string_get_char(block_str, 3 * i);
        char low = furi_string_get_char(block_str, 3 * i + 1);
        uint8_t byte = 0;
        if(hex_char_to_uint8(hi, low, &byte)) {
            block_tmp.data[i] = byte;
        } else {
            FURI_BIT_SET(block_unknown_bytes_mask, i);
        }
    }

    if(block_unknown_bytes_mask != 0xffff) {
        if(is_sector_trailer) {
            MfClassicSectorTrailer* sec_tr_tmp = (MfClassicSectorTrailer*)&block_tmp;
            // Load Key A
            // Key A mask 0b0000000000111111 = 0x003f
            if((block_unknown_bytes_mask & 0x003f) == 0) {
                uint64_t key =
                    bit_lib_bytes_to_num_be(sec_tr_tmp->key_a.data, sizeof(MfClassicKey));
                mf_classic_set_key_found(data, sector_num, MfClassicKeyTypeA, key);
            }
            // Load Access Bits
            // Access bits mask 0b0000001111000000 = 0x03c0
            if((block_unknown_bytes_mask & 0x03c0) == 0) {
                mf_classic_set_block_read(data, block_num, &block_tmp);
            }
            // Load Key B
            // Key B mask 0b1111110000000000 = 0xfc00
            if((block_unknown_bytes_mask & 0xfc00) == 0) {
                uint64_t key =
                    bit_lib_bytes_to_num_be(sec_tr_tmp->key_b.data, sizeof(MfClassicKey));
                mf_classic_set_key_found(data, sector_num, MfClassicKeyTypeB, key);
            }
        } else {
            if(block_unknown_bytes_mask == 0) {
                mf_classic_set_block_read(data, block_num, &block_tmp);
            }
        }
    }
}

bool mf_classic_load(MfClassicData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    FuriString* temp_str = furi_string_alloc();
    bool parsed = false;

    do {
        // Read ISO14443_3A data
        if(!iso14443_3a_load(data->iso14443_3a_data, ff, version)) break;

        // Read Mifare Classic type
        if(!flipper_format_read_string(ff, "Mifare Classic type", temp_str)) break;
        bool type_parsed = false;
        for(size_t i = 0; i < MfClassicTypeNum; i++) {
            if(furi_string_equal_str(temp_str, mf_classic_features[i].type_name)) {
                data->type = i;
                type_parsed = true;
            }
        }
        if(!type_parsed) break;

        // Read format version
        uint32_t data_format_version = 0;
        bool old_format = false;
        // Read Mifare Classic format version
        if(!flipper_format_read_uint32(ff, "Data format version", &data_format_version, 1)) {
            // Load unread sectors with zero keys access for backward compatibility
            if(!flipper_format_rewind(ff)) break;
            old_format = true;
        } else {
            if(data_format_version < mf_classic_data_format_version) {
                old_format = true;
            }
        }

        // Read Mifare Classic blocks
        bool block_read = true;
        FuriString* block_str = furi_string_alloc();
        uint16_t blocks_total = mf_classic_get_total_block_num(data->type);
        for(size_t i = 0; i < blocks_total; i++) {
            furi_string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_string(ff, furi_string_get_cstr(temp_str), block_str)) {
                block_read = false;
                break;
            }
            mf_classic_parse_block(block_str, data, i);
        }
        furi_string_free(block_str);
        if(!block_read) break;

        // Set keys and blocks as unknown for backward compatibility
        if(old_format) {
            data->key_a_mask = 0ULL;
            data->key_b_mask = 0ULL;
            memset(data->block_read_mask, 0, sizeof(data->block_read_mask));
        }

        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

static void
    mf_classic_set_block_str(FuriString* block_str, const MfClassicData* data, uint8_t block_num) {
    furi_string_reset(block_str);
    bool is_sec_trailer = mf_classic_is_sector_trailer(block_num);
    if(is_sec_trailer) {
        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sector_num);
        // Write key A
        for(size_t i = 0; i < sizeof(sec_tr->key_a); i++) {
            if(mf_classic_is_key_found(data, sector_num, MfClassicKeyTypeA)) {
                furi_string_cat_printf(block_str, "%02X ", sec_tr->key_a.data[i]);
            } else {
                furi_string_cat_printf(block_str, "?? ");
            }
        }
        // Write Access bytes
        for(size_t i = 0; i < MF_CLASSIC_ACCESS_BYTES_SIZE; i++) {
            if(mf_classic_is_block_read(data, block_num)) {
                furi_string_cat_printf(block_str, "%02X ", sec_tr->access_bits.data[i]);
            } else {
                furi_string_cat_printf(block_str, "?? ");
            }
        }
        // Write key B
        for(size_t i = 0; i < sizeof(sec_tr->key_b); i++) {
            if(mf_classic_is_key_found(data, sector_num, MfClassicKeyTypeB)) {
                furi_string_cat_printf(block_str, "%02X ", sec_tr->key_b.data[i]);
            } else {
                furi_string_cat_printf(block_str, "?? ");
            }
        }
    } else {
        // Write data block
        for(size_t i = 0; i < MF_CLASSIC_BLOCK_SIZE; i++) {
            if(mf_classic_is_block_read(data, block_num)) {
                furi_string_cat_printf(block_str, "%02X ", data->block[block_num].data[i]);
            } else {
                furi_string_cat_printf(block_str, "?? ");
            }
        }
    }
    furi_string_trim(block_str);
}

bool mf_classic_save(const MfClassicData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* temp_str = furi_string_alloc();
    bool saved = false;

    do {
        if(!iso14443_3a_save(data->iso14443_3a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, "Mifare Classic specific data")) break;
        if(!flipper_format_write_string_cstr(
               ff, "Mifare Classic type", mf_classic_features[data->type].type_name))
            break;
        if(!flipper_format_write_uint32(
               ff, "Data format version", &mf_classic_data_format_version, 1))
            break;
        if(!flipper_format_write_comment_cstr(
               ff, "Mifare Classic blocks, \'??\' means unknown data"))
            break;

        uint16_t blocks_total = mf_classic_get_total_block_num(data->type);
        FuriString* block_str = furi_string_alloc();
        bool block_saved = true;
        for(size_t i = 0; i < blocks_total; i++) {
            furi_string_printf(temp_str, "Block %d", i);
            mf_classic_set_block_str(block_str, data, i);
            if(!flipper_format_write_string(ff, furi_string_get_cstr(temp_str), block_str)) {
                block_saved = false;
                break;
            }
        }
        furi_string_free(block_str);
        if(!block_saved) break;

        saved = true;
    } while(false);

    furi_string_free(temp_str);

    return saved;
}

bool mf_classic_is_equal(const MfClassicData* data, const MfClassicData* other) {
    furi_check(data);
    furi_check(other);

    bool is_equal = false;
    bool data_array_is_equal = true;

    do {
        if(!iso14443_3a_is_equal(data->iso14443_3a_data, other->iso14443_3a_data)) break;
        if(data->type != other->type) break;
        if(data->key_a_mask != other->key_a_mask) break;
        if(data->key_b_mask != other->key_b_mask) break;

        for(size_t i = 0; i < COUNT_OF(data->block_read_mask); i++) {
            if(data->block_read_mask[i] != other->block_read_mask[i]) {
                data_array_is_equal = false;
                break;
            }
        }
        if(!data_array_is_equal) break;

        for(size_t i = 0; i < COUNT_OF(data->block); i++) {
            if(memcmp(&data->block[i], &other->block[i], sizeof(data->block[i])) != 0) {
                data_array_is_equal = false;
                break;
            }
        }
        if(!data_array_is_equal) break;

        is_equal = true;
    } while(false);

    return is_equal;
}

const char* mf_classic_get_device_name(const MfClassicData* data, NfcDeviceNameType name_type) {
    furi_check(data);
    furi_check(data->type < MfClassicTypeNum);

    if(name_type == NfcDeviceNameTypeFull) {
        return mf_classic_features[data->type].full_name;
    } else {
        return mf_classic_features[data->type].type_name;
    }
}

const uint8_t* mf_classic_get_uid(const MfClassicData* data, size_t* uid_len) {
    furi_check(data);

    return iso14443_3a_get_uid(data->iso14443_3a_data, uid_len);
}

bool mf_classic_set_uid(MfClassicData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    bool uid_valid = iso14443_3a_set_uid(data->iso14443_3a_data, uid, uid_len);

    if(uid_valid) {
        uint8_t* block = data->block[0].data;

        // Copy UID to block 0
        memcpy(block, data->iso14443_3a_data->uid, uid_len);

        if(uid_len == 4) {
            // Calculate BCC byte
            block[uid_len] = 0;

            for(size_t i = 0; i < uid_len; i++) {
                block[uid_len] ^= block[i];
            }
        }
    }

    return uid_valid;
}

Iso14443_3aData* mf_classic_get_base_data(const MfClassicData* data) {
    furi_check(data);

    return data->iso14443_3a_data;
}

uint8_t mf_classic_get_total_sectors_num(MfClassicType type) {
    furi_check(type < MfClassicTypeNum);
    return mf_classic_features[type].sectors_total;
}

uint16_t mf_classic_get_total_block_num(MfClassicType type) {
    furi_check(type < MfClassicTypeNum);
    return mf_classic_features[type].blocks_total;
}

uint8_t mf_classic_get_sector_trailer_num_by_sector(uint8_t sector) {
    uint8_t block_num = 0;

    if(sector < 32) {
        block_num = sector * 4 + 3;
    } else if(sector < 40) {
        block_num = 32 * 4 + (sector - 32) * 16 + 15;
    } else {
        furi_crash("Wrong sector num");
    }

    return block_num;
}

uint8_t mf_classic_get_sector_trailer_num_by_block(uint8_t block) {
    uint8_t sec_tr_block_num = 0;

    if(block < 128) {
        sec_tr_block_num = block | 0x03;
    } else {
        sec_tr_block_num = block | 0x0f;
    }

    return sec_tr_block_num;
}

MfClassicSectorTrailer*
    mf_classic_get_sector_trailer_by_sector(const MfClassicData* data, uint8_t sector_num) {
    furi_check(data);

    uint8_t sec_tr_block = mf_classic_get_sector_trailer_num_by_sector(sector_num);
    MfClassicSectorTrailer* sec_trailer = (MfClassicSectorTrailer*)&data->block[sec_tr_block];

    return sec_trailer;
}

bool mf_classic_is_sector_trailer(uint8_t block) {
    return block == mf_classic_get_sector_trailer_num_by_block(block);
}

void mf_classic_set_sector_trailer_read(
    MfClassicData* data,
    uint8_t block_num,
    MfClassicSectorTrailer* sec_tr) {
    furi_check(data);
    furi_check(sec_tr);
    furi_check(mf_classic_is_sector_trailer(block_num));

    uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
    MfClassicSectorTrailer* sec_trailer =
        mf_classic_get_sector_trailer_by_sector(data, sector_num);
    memcpy(sec_trailer, sec_tr, sizeof(MfClassicSectorTrailer));
    FURI_BIT_SET(data->block_read_mask[block_num / 32], block_num % 32);
    FURI_BIT_SET(data->key_a_mask, sector_num);
    FURI_BIT_SET(data->key_b_mask, sector_num);
}

uint8_t mf_classic_get_sector_by_block(uint8_t block) {
    uint8_t sector = 0;

    if(block < 128) {
        sector = (block | 0x03) / 4;
    } else {
        sector = 32 + ((block | 0x0f) - 32 * 4) / 16;
    }

    return sector;
}

bool mf_classic_block_to_value(const MfClassicBlock* block, int32_t* value, uint8_t* addr) {
    furi_check(block);
    furi_check(value);

    uint32_t v = *(uint32_t*)&block->data[0];
    uint32_t v_inv = *(uint32_t*)&block->data[sizeof(uint32_t)];
    uint32_t v1 = *(uint32_t*)&block->data[sizeof(uint32_t) * 2];

    bool val_checks =
        ((v == v1) && (v == ~v_inv) && (block->data[12] == (~block->data[13] & 0xFF)) &&
         (block->data[14] == (~block->data[15] & 0xFF)) && (block->data[12] == block->data[14]));
    *value = (int32_t)v;
    if(addr) {
        *addr = block->data[12];
    }
    return val_checks;
}

void mf_classic_value_to_block(int32_t value, uint8_t addr, MfClassicBlock* block) {
    furi_check(block);

    uint32_t v_inv = ~((uint32_t)value);

    memcpy(&block->data[0], &value, 4); //-V1086
    memcpy(&block->data[4], &v_inv, 4); //-V1086
    memcpy(&block->data[8], &value, 4); //-V1086

    block->data[12] = addr;
    block->data[13] = ~addr & 0xFF;
    block->data[14] = addr;
    block->data[15] = ~addr & 0xFF;
}

bool mf_classic_is_key_found(
    const MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type) {
    furi_check(data);

    bool key_found = false;
    if(key_type == MfClassicKeyTypeA) {
        key_found = (FURI_BIT(data->key_a_mask, sector_num) == 1);
    } else if(key_type == MfClassicKeyTypeB) {
        key_found = (FURI_BIT(data->key_b_mask, sector_num) == 1);
    }

    return key_found;
}

void mf_classic_set_key_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type,
    uint64_t key) {
    furi_check(data);

    uint8_t key_arr[6] = {};
    MfClassicSectorTrailer* sec_trailer =
        mf_classic_get_sector_trailer_by_sector(data, sector_num);
    bit_lib_num_to_bytes_be(key, 6, key_arr);
    if(key_type == MfClassicKeyTypeA) {
        memcpy(sec_trailer->key_a.data, key_arr, sizeof(MfClassicKey));
        FURI_BIT_SET(data->key_a_mask, sector_num);
    } else if(key_type == MfClassicKeyTypeB) {
        memcpy(sec_trailer->key_b.data, key_arr, sizeof(MfClassicKey));
        FURI_BIT_SET(data->key_b_mask, sector_num);
    }
}

void mf_classic_set_key_not_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type) {
    furi_check(data);

    if(key_type == MfClassicKeyTypeA) {
        FURI_BIT_CLEAR(data->key_a_mask, sector_num);
    } else if(key_type == MfClassicKeyTypeB) {
        FURI_BIT_CLEAR(data->key_b_mask, sector_num);
    }
}

MfClassicKey
    mf_classic_get_key(const MfClassicData* data, uint8_t sector_num, MfClassicKeyType key_type) {
    furi_check(data);
    furi_check(sector_num < mf_classic_get_total_sectors_num(data->type));
    furi_check(key_type == MfClassicKeyTypeA || key_type == MfClassicKeyTypeB);

    const MfClassicSectorTrailer* sector_trailer =
        mf_classic_get_sector_trailer_by_sector(data, sector_num);

    if(key_type == MfClassicKeyTypeA) {
        return sector_trailer->key_a;
    } else {
        return sector_trailer->key_b;
    }
}

bool mf_classic_is_block_read(const MfClassicData* data, uint8_t block_num) {
    furi_check(data);

    return FURI_BIT(data->block_read_mask[block_num / 32], block_num % 32) == 1;
}

void mf_classic_set_block_read(MfClassicData* data, uint8_t block_num, MfClassicBlock* block_data) {
    furi_check(data);
    furi_check(block_data);

    if(mf_classic_is_sector_trailer(block_num)) {
        memcpy(&data->block[block_num].data[6], &block_data->data[6], 4);
    } else {
        memcpy(data->block[block_num].data, block_data->data, MF_CLASSIC_BLOCK_SIZE);
    }
    FURI_BIT_SET(data->block_read_mask[block_num / 32], block_num % 32);
}

uint8_t mf_classic_get_first_block_num_of_sector(uint8_t sector) {
    furi_check(sector < 40);

    uint8_t block = 0;
    if(sector < 32) {
        block = sector * 4;
    } else {
        block = 32 * 4 + (sector - 32) * 16;
    }

    return block;
}

uint8_t mf_classic_get_blocks_num_in_sector(uint8_t sector) {
    furi_check(sector < 40);

    return sector < 32 ? 4 : 16;
}

void mf_classic_get_read_sectors_and_keys(
    const MfClassicData* data,
    uint8_t* sectors_read,
    uint8_t* keys_found) {
    furi_check(data);
    furi_check(sectors_read);
    furi_check(keys_found);

    *sectors_read = 0;
    *keys_found = 0;
    uint8_t sectors_total = mf_classic_get_total_sectors_num(data->type);
    for(size_t i = 0; i < sectors_total; i++) {
        if(mf_classic_is_key_found(data, i, MfClassicKeyTypeA)) {
            *keys_found += 1;
        }
        if(mf_classic_is_key_found(data, i, MfClassicKeyTypeB)) {
            *keys_found += 1;
        }
        uint8_t first_block = mf_classic_get_first_block_num_of_sector(i);
        uint8_t total_blocks_in_sec = mf_classic_get_blocks_num_in_sector(i);
        bool blocks_read = true;
        for(size_t j = first_block; j < first_block + total_blocks_in_sec; j++) {
            blocks_read = mf_classic_is_block_read(data, j);
            if(!blocks_read) break;
        }
        if(blocks_read) {
            *sectors_read += 1;
        }
    }
}

bool mf_classic_is_card_read(const MfClassicData* data) {
    furi_check(data);

    uint8_t sectors_total = mf_classic_get_total_sectors_num(data->type);
    uint8_t sectors_read = 0;
    uint8_t keys_found = 0;
    mf_classic_get_read_sectors_and_keys(data, &sectors_read, &keys_found);
    bool card_read = (sectors_read == sectors_total) && (keys_found == sectors_total * 2);

    return card_read;
}

bool mf_classic_is_sector_read(const MfClassicData* data, uint8_t sector_num) {
    furi_check(data);

    bool sector_read = false;
    do {
        if(!mf_classic_is_key_found(data, sector_num, MfClassicKeyTypeA)) break;
        if(!mf_classic_is_key_found(data, sector_num, MfClassicKeyTypeB)) break;
        uint8_t start_block = mf_classic_get_first_block_num_of_sector(sector_num);
        uint8_t total_blocks = mf_classic_get_blocks_num_in_sector(sector_num);
        uint8_t block_read = true;
        for(size_t i = start_block; i < start_block + total_blocks; i++) {
            block_read = mf_classic_is_block_read(data, i);
            if(!block_read) break;
        }
        sector_read = block_read;
    } while(false);

    return sector_read;
}

static bool mf_classic_is_allowed_access_sector_trailer(
    MfClassicData* data,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicAction action) {
    uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sector_num);
    uint8_t* access_bits_arr = sec_tr->access_bits.data;
    uint8_t AC = ((access_bits_arr[1] >> 5) & 0x04) | ((access_bits_arr[2] >> 2) & 0x02) |
                 ((access_bits_arr[2] >> 7) & 0x01);
    FURI_LOG_T("NFC", "AC: %02X", AC);

    switch(action) {
    case MfClassicActionKeyARead: {
        return false;
    }
    case MfClassicActionKeyAWrite:
    case MfClassicActionKeyBWrite: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x00 || AC == 0x01)) ||
               (key_type == MfClassicKeyTypeB &&
                (AC == 0x00 || AC == 0x04 || AC == 0x03 || AC == 0x01));
    }
    case MfClassicActionKeyBRead: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x00 || AC == 0x02 || AC == 0x01)) ||
               (key_type == MfClassicKeyTypeB && (AC == 0x00 || AC == 0x02 || AC == 0x01));
    }
    case MfClassicActionACRead: {
        return (key_type == MfClassicKeyTypeA) || (key_type == MfClassicKeyTypeB);
    }
    case MfClassicActionACWrite: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x01)) ||
               (key_type == MfClassicKeyTypeB && (AC == 0x01 || AC == 0x03 || AC == 0x05));
    }
    default:
        return false;
    }
    return true;
}

bool mf_classic_is_allowed_access_data_block(
    MfClassicSectorTrailer* sec_tr,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicAction action) {
    furi_check(sec_tr);

    uint8_t* access_bits_arr = sec_tr->access_bits.data;

    if(block_num == 0 && action == MfClassicActionDataWrite) {
        return false;
    }

    uint8_t sector_block = 0;
    if(block_num <= 128) {
        sector_block = block_num & 0x03;
    } else {
        sector_block = (block_num & 0x0f) / 5;
    }

    uint8_t AC;
    switch(sector_block) {
    case 0x00: {
        AC = ((access_bits_arr[1] >> 2) & 0x04) | ((access_bits_arr[2] << 1) & 0x02) |
             ((access_bits_arr[2] >> 4) & 0x01);
        break;
    }
    case 0x01: {
        AC = ((access_bits_arr[1] >> 3) & 0x04) | ((access_bits_arr[2] >> 0) & 0x02) |
             ((access_bits_arr[2] >> 5) & 0x01);
        break;
    }
    case 0x02: {
        AC = ((access_bits_arr[1] >> 4) & 0x04) | ((access_bits_arr[2] >> 1) & 0x02) |
             ((access_bits_arr[2] >> 6) & 0x01);
        break;
    }
    default:
        return false;
    }

    switch(action) {
    case MfClassicActionDataRead: {
        return (key_type == MfClassicKeyTypeA && !(AC == 0x03 || AC == 0x05 || AC == 0x07)) ||
               (key_type == MfClassicKeyTypeB && !(AC == 0x07));
    }
    case MfClassicActionDataWrite: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x00)) ||
               (key_type == MfClassicKeyTypeB &&
                (AC == 0x00 || AC == 0x04 || AC == 0x06 || AC == 0x03));
    }
    case MfClassicActionDataInc: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x00)) ||
               (key_type == MfClassicKeyTypeB && (AC == 0x00 || AC == 0x06));
    }
    case MfClassicActionDataDec: {
        return (key_type == MfClassicKeyTypeA && (AC == 0x00 || AC == 0x06 || AC == 0x01)) ||
               (key_type == MfClassicKeyTypeB && (AC == 0x00 || AC == 0x06 || AC == 0x01));
    }
    default:
        return false;
    }

    return false;
}

bool mf_classic_is_allowed_access(
    MfClassicData* data,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicAction action) {
    furi_check(data);

    bool access_allowed = false;
    if(mf_classic_is_sector_trailer(block_num)) {
        access_allowed =
            mf_classic_is_allowed_access_sector_trailer(data, block_num, key_type, action);
    } else {
        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sector_num);
        access_allowed =
            mf_classic_is_allowed_access_data_block(sec_tr, block_num, key_type, action);
    }

    return access_allowed;
}

bool mf_classic_is_value_block(MfClassicSectorTrailer* sec_tr, uint8_t block_num) {
    furi_check(sec_tr);

    // Check if key A can write, if it can, it's transport configuration, not data block
    return !mf_classic_is_allowed_access_data_block(
               sec_tr, block_num, MfClassicKeyTypeA, MfClassicActionDataWrite) &&
           (mf_classic_is_allowed_access_data_block(
                sec_tr, block_num, MfClassicKeyTypeB, MfClassicActionDataInc) ||
            mf_classic_is_allowed_access_data_block(
                sec_tr, block_num, MfClassicKeyTypeB, MfClassicActionDataDec));
}
