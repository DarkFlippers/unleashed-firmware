/*
 * bambu.c - Parser for Bambu Lab filament spool RFID tags (MIFARE Classic 1K).
 *
 * Ported into the firmware from the standalone flipper-bambu plugin:
 *   https://github.com/uzyn/flipper-bambu
 *
 * Copyright U-Zyn Chua <https://uzyn.com>
 *
 * Filament database sourced from https://github.com/queengooborg/Bambu-Lab-RFID-Library
 * Tag format research from https://github.com/Bambu-Research-Group/RFID-Tag-Guide
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <furi.h>
#include <string.h>

#include "bambu_filaments.h"
#include "bambu_parser.h"

static const uint8_t bambu_required_blocks[] = {
    BLOCK_MATERIAL_IDS,
    BLOCK_FILAMENT_TYPE,
    BLOCK_DETAILED_TYPE,
    BLOCK_COLOR_WEIGHT,
    BLOCK_TEMPERATURES,
    BLOCK_NOZZLE,
    BLOCK_SPOOL_WIDTH,
    BLOCK_PRODUCTION_DATE,
    BLOCK_FILAMENT_LENGTH,
};

typedef struct {
    uint8_t data[64];
    uint32_t data_len;
    uint64_t bit_len;
    uint32_t state[8];
} BambuSha256Context;

static inline uint32_t bambu_sha256_rotr(uint32_t value, uint32_t bits) {
    return (value >> bits) | (value << (32 - bits));
}

static inline uint32_t bambu_sha256_ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

static inline uint32_t bambu_sha256_maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint32_t bambu_sha256_ep0(uint32_t x) {
    return bambu_sha256_rotr(x, 2) ^ bambu_sha256_rotr(x, 13) ^ bambu_sha256_rotr(x, 22);
}

static inline uint32_t bambu_sha256_ep1(uint32_t x) {
    return bambu_sha256_rotr(x, 6) ^ bambu_sha256_rotr(x, 11) ^ bambu_sha256_rotr(x, 25);
}

static inline uint32_t bambu_sha256_sig0(uint32_t x) {
    return bambu_sha256_rotr(x, 7) ^ bambu_sha256_rotr(x, 18) ^ (x >> 3);
}

static inline uint32_t bambu_sha256_sig1(uint32_t x) {
    return bambu_sha256_rotr(x, 17) ^ bambu_sha256_rotr(x, 19) ^ (x >> 10);
}

static void bambu_sha256_transform(BambuSha256Context* context, const uint8_t data[64]) {
    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
        0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
        0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
        0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
        0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
        0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
        0xc67178f2,
    };

    uint32_t message[64];
    for(size_t i = 0, j = 0; i < 16; i++, j += 4) {
        message[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) |
                     ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
    }

    for(size_t i = 16; i < 64; i++) {
        message[i] = bambu_sha256_sig1(message[i - 2]) + message[i - 7] +
                     bambu_sha256_sig0(message[i - 15]) + message[i - 16];
    }

    uint32_t a = context->state[0];
    uint32_t b = context->state[1];
    uint32_t c = context->state[2];
    uint32_t d = context->state[3];
    uint32_t e = context->state[4];
    uint32_t f = context->state[5];
    uint32_t g = context->state[6];
    uint32_t h = context->state[7];

    for(size_t i = 0; i < 64; i++) {
        uint32_t t1 = h + bambu_sha256_ep1(e) + bambu_sha256_ch(e, f, g) + k[i] + message[i];
        uint32_t t2 = bambu_sha256_ep0(a) + bambu_sha256_maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
    context->state[5] += f;
    context->state[6] += g;
    context->state[7] += h;
}

static void bambu_sha256_init(BambuSha256Context* context) {
    context->data_len = 0;
    context->bit_len = 0;
    context->state[0] = 0x6a09e667;
    context->state[1] = 0xbb67ae85;
    context->state[2] = 0x3c6ef372;
    context->state[3] = 0xa54ff53a;
    context->state[4] = 0x510e527f;
    context->state[5] = 0x9b05688c;
    context->state[6] = 0x1f83d9ab;
    context->state[7] = 0x5be0cd19;
}

static void bambu_sha256_update(BambuSha256Context* context, const uint8_t* data, size_t len) {
    for(size_t i = 0; i < len; i++) {
        context->data[context->data_len++] = data[i];
        if(context->data_len == 64) {
            bambu_sha256_transform(context, context->data);
            context->bit_len += 512;
            context->data_len = 0;
        }
    }
}

static void bambu_sha256_final(BambuSha256Context* context, uint8_t hash[32]) {
    uint32_t i = context->data_len;

    context->data[i++] = 0x80;
    if(i > 56) {
        while(i < 64) {
            context->data[i++] = 0x00;
        }
        bambu_sha256_transform(context, context->data);
        i = 0;
    }

    while(i < 56) {
        context->data[i++] = 0x00;
    }

    context->bit_len += (uint64_t)context->data_len * 8;
    context->data[56] = (uint8_t)(context->bit_len >> 56);
    context->data[57] = (uint8_t)(context->bit_len >> 48);
    context->data[58] = (uint8_t)(context->bit_len >> 40);
    context->data[59] = (uint8_t)(context->bit_len >> 32);
    context->data[60] = (uint8_t)(context->bit_len >> 24);
    context->data[61] = (uint8_t)(context->bit_len >> 16);
    context->data[62] = (uint8_t)(context->bit_len >> 8);
    context->data[63] = (uint8_t)(context->bit_len);
    bambu_sha256_transform(context, context->data);

    for(i = 0; i < 4; i++) {
        hash[i] = (uint8_t)((context->state[0] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 4] = (uint8_t)((context->state[1] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 8] = (uint8_t)((context->state[2] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 12] = (uint8_t)((context->state[3] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 16] = (uint8_t)((context->state[4] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 20] = (uint8_t)((context->state[5] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 24] = (uint8_t)((context->state[6] >> (24 - i * 8)) & 0x000000ff);
        hash[i + 28] = (uint8_t)((context->state[7] >> (24 - i * 8)) & 0x000000ff);
    }
}

static void bambu_hmac_sha256(
    const uint8_t* key,
    size_t key_len,
    const uint8_t* data,
    size_t data_len,
    uint8_t out[32]) {
    uint8_t key_block[64] = {0};
    if(key_len > sizeof(key_block)) {
        BambuSha256Context key_hash_ctx;
        bambu_sha256_init(&key_hash_ctx);
        bambu_sha256_update(&key_hash_ctx, key, key_len);
        bambu_sha256_final(&key_hash_ctx, key_block);
    } else {
        memcpy(key_block, key, key_len);
    }

    uint8_t ipad[64];
    uint8_t opad[64];
    for(size_t i = 0; i < 64; i++) {
        ipad[i] = key_block[i] ^ 0x36;
        opad[i] = key_block[i] ^ 0x5C;
    }

    uint8_t inner_hash[32];
    BambuSha256Context inner_ctx;
    bambu_sha256_init(&inner_ctx);
    bambu_sha256_update(&inner_ctx, ipad, sizeof(ipad));
    bambu_sha256_update(&inner_ctx, data, data_len);
    bambu_sha256_final(&inner_ctx, inner_hash);

    BambuSha256Context outer_ctx;
    bambu_sha256_init(&outer_ctx);
    bambu_sha256_update(&outer_ctx, opad, sizeof(opad));
    bambu_sha256_update(&outer_ctx, inner_hash, sizeof(inner_hash));
    bambu_sha256_final(&outer_ctx, out);
}

static void bambu_derive_keys_from_uid(const uint8_t* uid, size_t uid_len, MfClassicDeviceKeys* keys) {
    static const uint8_t master_key[16] = {
        0x9A,
        0x75,
        0x9C,
        0xF2,
        0xC4,
        0xF7,
        0xCA,
        0xFF,
        0x22,
        0x2C,
        0xB9,
        0x76,
        0x9B,
        0x41,
        0xBC,
        0x96,
    };
    static const uint8_t hkdf_context[] = {'R', 'F', 'I', 'D', '-', 'A', '\0'};
    static const size_t sector_count = 16;
    static const size_t key_size = sizeof(MfClassicKey);
    uint8_t prk[32];
    bambu_hmac_sha256(master_key, sizeof(master_key), uid, uid_len, prk);

    uint8_t key_material[16 * sizeof(MfClassicKey)] = {0};
    uint8_t t[32] = {0};
    size_t t_len = 0;
    uint8_t hmac_input[32 + sizeof(hkdf_context) + 1];
    size_t generated = 0;
    uint8_t counter = 1;

    while(generated < sizeof(key_material)) {
        size_t input_len = 0;
        if(t_len > 0) {
            memcpy(hmac_input, t, t_len);
            input_len = t_len;
        }
        memcpy(&hmac_input[input_len], hkdf_context, sizeof(hkdf_context));
        input_len += sizeof(hkdf_context);
        hmac_input[input_len++] = counter++;

        bambu_hmac_sha256(prk, sizeof(prk), hmac_input, input_len, t);
        t_len = sizeof(t);

        size_t chunk_len = sizeof(t);
        if(chunk_len > sizeof(key_material) - generated) {
            chunk_len = sizeof(key_material) - generated;
        }
        memcpy(&key_material[generated], t, chunk_len);
        generated += chunk_len;
    }

    for(size_t sector = 0; sector < sector_count; sector++) {
        const uint8_t* sector_key = &key_material[sector * key_size];
        memcpy(keys->key_a[sector].data, sector_key, key_size);
        FURI_BIT_SET(keys->key_a_mask, sector);
        memcpy(keys->key_b[sector].data, sector_key, key_size);
        FURI_BIT_SET(keys->key_b_mask, sector);
    }
}

static bool bambu_has_required_blocks(const MfClassicData* data) {
    for(size_t i = 0; i < COUNT_OF(bambu_required_blocks); i++) {
        if(!mf_classic_is_block_read(data, bambu_required_blocks[i])) {
            return false;
        }
    }
    return true;
}

static bool bambu_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;
    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone || type != MfClassicType1k) {
            break;
        }

        size_t uid_len = 0;
        const uint8_t* uid = nfc_device_get_uid(device, &uid_len);
        if(uid == NULL || uid_len == 0) {
            break;
        }

        data->type = type;

        MfClassicDeviceKeys keys = {};
        bambu_derive_keys_from_uid(uid, uid_len, &keys);

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone && error != MfClassicErrorPartialRead) {
            break;
        }

        if(!bambu_has_required_blocks(data)) {
            break;
        }

        if(!bambu_tag_is_valid(data)) {
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);
        is_read = true;
    } while(false);

    mf_classic_free(data);
    return is_read;
}

// Main parse function: Extract and format all Bambu spool data
static bool bambu_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    // Quick type check
    if(data->type != MfClassicType1k) {
        return false;
    }

    // Verify this is a Bambu tag using our detection logic
    if(!bambu_tag_is_valid(data)) {
        return false;
    }

    // Parse Material ID and Variant ID from Block 1
    const uint8_t* block1 = data->block[BLOCK_MATERIAL_IDS].data;
    char material_id[7] = {0};  // "GFxxx" + null
    char variant_id[8] = {0};   // "xxx-Rx" + null
    bambu_copy_ascii_string(material_id, &block1[8], 6);
    bambu_copy_ascii_string(variant_id, &block1[0], 7);

    // Parse Detailed Type from Block 4
    const uint8_t* block4 = data->block[BLOCK_DETAILED_TYPE].data;
    char detailed_type[17] = {0};
    bambu_copy_ascii_string(detailed_type, block4, 16);

    // Parse Color, Weight, Diameter from Block 5
    const uint8_t* block5 = data->block[BLOCK_COLOR_WEIGHT].data;
    uint8_t color_r = block5[0];
    uint8_t color_g = block5[1];
    uint8_t color_b = block5[2];
    uint8_t color_a = block5[3];
    uint16_t weight_grams = bambu_read_le16(&block5[4]);
    float diameter_mm = bambu_read_le_float(&block5[8]);

    // Parse Temperatures from Block 6
    const uint8_t* block6 = data->block[BLOCK_TEMPERATURES].data;
    uint16_t drying_temp = bambu_read_le16(&block6[0]);
    uint16_t drying_hours = bambu_read_le16(&block6[2]);
    uint16_t hotend_max = bambu_read_le16(&block6[8]);
    uint16_t hotend_min = bambu_read_le16(&block6[10]);

    // Parse Nozzle Diameter from Block 8 (float at bytes 12-15)
    const uint8_t* block8 = data->block[BLOCK_NOZZLE].data;
    float nozzle_diameter = bambu_read_le_float(&block8[12]);

    // Parse Spool Width from Block 10 (uint16 at bytes 4-5, value in mm*100)
    const uint8_t* block10 = data->block[BLOCK_SPOOL_WIDTH].data;
    uint16_t spool_width_raw = bambu_read_le16(&block10[4]);
    float spool_width_mm = (float)spool_width_raw / 100.0f;

    // Parse Production Date from Block 12 (ASCII YYYY_MM_DD_HH_MM)
    const uint8_t* block12 = data->block[BLOCK_PRODUCTION_DATE].data;
    char production_date[17] = {0};
    bambu_copy_ascii_string(production_date, block12, 16);

    // Format production date from "YYYY_MM_DD_HH_MM" to "YYYY-MM-DD HH:MM"
    // Only format if underscores are present at expected positions
    if(production_date[4] == '_' &&
       production_date[7] == '_' &&
       production_date[10] == '_' &&
       production_date[13] == '_') {
        production_date[4] = '-';
        production_date[7] = '-';
        production_date[10] = ' ';
        production_date[13] = ':';
    }

    // Parse Filament Length from Block 14 (uint16 at bytes 4-5, meters)
    const uint8_t* block14 = data->block[BLOCK_FILAMENT_LENGTH].data;
    uint16_t filament_length = bambu_read_le16(&block14[4]);

    // Look up filament info from variant_id
    const BambuFilamentInfo* filament_info = bambu_lookup_filament(variant_id);

    // Build formatted output
    furi_string_cat_printf(parsed_data, "\e#Bambu Lab Filament\n");
    furi_string_cat_printf(parsed_data, "Type: %s\n", detailed_type);

    // Display color: show name with hex if available, otherwise just hex
    // For hex code: show 6-digit if fully opaque, otherwise show "#RRGGBB @ XX%"
    if(filament_info != NULL) {
        if(color_a == 0xFF) {
            furi_string_cat_printf(parsed_data, "Color: %s (#%02X%02X%02X)\n",
                                  filament_info->color_name, color_r, color_g, color_b);
        } else {
            uint8_t alpha_percent = (color_a * 100) / 255;
            furi_string_cat_printf(parsed_data, "Color: %s (#%02X%02X%02X @ %u%%)\n",
                                  filament_info->color_name, color_r, color_g, color_b, alpha_percent);
        }
    } else {
        if(color_a == 0xFF) {
            furi_string_cat_printf(parsed_data, "Color: #%02X%02X%02X\n",
                                  color_r, color_g, color_b);
        } else {
            uint8_t alpha_percent = (color_a * 100) / 255;
            furi_string_cat_printf(parsed_data, "Color: #%02X%02X%02X @ %u%%\n",
                                  color_r, color_g, color_b, alpha_percent);
        }
    }

    if(filament_info != NULL) {
        furi_string_cat_printf(parsed_data, "Filament Code: %s\n", filament_info->filament_code);
    } else {
        furi_string_cat_printf(parsed_data, "Filament Code: Unknown (update lookup table)\n");
        furi_string_cat_printf(parsed_data, "Material ID: %s\n", material_id);
        furi_string_cat_printf(parsed_data, "Variant: %s\n", variant_id);
    }

    furi_string_cat_printf(parsed_data, "Prod: %s\n", production_date);


    furi_string_cat_printf(parsed_data, "\n\e#Configurations\n");
    furi_string_cat_printf(parsed_data, "Hotend: %u-%u C\n", hotend_min, hotend_max);
    furi_string_cat_printf(parsed_data, "Drying: %u C for %uh\n", drying_temp, drying_hours);
    furi_string_cat_printf(parsed_data, "Nozzle: >= %.2fmm\n", (double)nozzle_diameter);

    furi_string_cat_printf(parsed_data, "\n\e#Specifications\n");
    furi_string_cat_printf(parsed_data, "Weight: %ug\n", weight_grams);
    furi_string_cat_printf(parsed_data, "Diameter: %.2fmm\n", (double)diameter_mm);
    furi_string_cat_printf(parsed_data, "Spool Width: %.2fmm\n", (double)spool_width_mm);
    if(filament_length > 0) {
        furi_string_cat_printf(parsed_data, "Length: %um\n", filament_length);
    }

    return true;
}

static const NfcSupportedCardsPlugin bambu_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,  // No early verify - validation done in parse()
    .read = bambu_read,
    .parse = bambu_parse,
};

static const FlipperAppPluginDescriptor bambu_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &bambu_plugin,
};

const FlipperAppPluginDescriptor* bambu_plugin_ep(void) {
    return &bambu_plugin_descriptor;
}
