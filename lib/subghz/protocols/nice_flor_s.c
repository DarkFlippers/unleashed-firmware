#include "nice_flor_s.h"
#include "public_api.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

/** Size of the Nice Flor-S rainbow table, in bytes. */
#define SUBGHZ_NICE_FLOR_S_RAINBOW_TABLE_SIZE_BYTES 32

/* The Flor-S cipher with a 16 bit installer code (Nice O-Code); ic ==
 * NICE_FLOR_S_IC_PLAIN gives plain Nice Flor-S. */
static uint64_t
    subghz_protocol_nice_flor_s_encrypt_ic(uint64_t data, uint16_t ic, const char* file_name);

/** The rainbow table, read from the SD card on first use and then kept. */
static const uint8_t* subghz_protocol_nice_flor_s_table(const char* file_name);

/*
 * https://phreakerclub.com/1615
 * https://phreakerclub.com/forum/showthread.php?t=2360
 * https://vrtp.ru/index.php?showtopic=27867
 */

#define TAG "SubGhzProtocolNiceFlorS"

static uint8_t nice_flors_counter_mode = 0;

#define NICE_ONE_COUNT_BIT        72
#define NICE_ONE_NAME             "Nice One"
#define NICE_O_NAME               "Nice O-Code"
/* The 52 bit body followed by 60 more bits. Only O-Code sends this length. The body
 * splits into generic.data as usual, the rest into data_2, saved as the hex key
 * "Data2" since the uint32 "Data" of a 72 bit frame cannot hold it. */
#define NICE_O_LONG_COUNT_BIT     112
/* Nice O-Code: the final permutation of the Flor-S cipher is XORed with a per-remote
 * 16 bit installer code instead of being inverted. IC == 0xFFFF is plain Nice Flor-S. */
#define NICE_FLOR_S_IC_PLAIN      0xFFFFu
/* Frames in a row matching nothing either side, with no repeat between, before O-Code
 * is called. Each step costs one O-Code parcel and buys margin against a press arriving
 * as a single parcel. Held in 2 bits, so 3 is the most. */
#define NICE_O_FRESH_REPORTS      1u
#define NICE_O_MIN_SAMPLES        4
#define NICE_O_MAX_SAMPLES        16
/* The parcel index a generated remote's stored frame carries. The encoder sends all 16
 * anyway, so this only decides which one the saved file looks like a capture of. */
#define NICE_FLOR_S_CREATE_PARCEL 0x3
/* Presses that still carry the certificate, counted in the file by "Cert_Cnt". Nice
 * documents twenty; after that the remote is enrolled and drops to 72 bit frames. */
#define NICE_O_CERT_SENDS         20
/* A 112 bit parcel costs 228 durations against 148 for a 72 bit one, so ten of them fit
 * the existing buffer where sixteen would not. Real presses run anywhere from 9 to 26
 * parcels, so a short sweep is nothing unusual on air. */
#define NICE_O_CERT_PARCELS       10
/* The last two parcels of a press carry the release marker, on every press in every
 * capture whatever its length. */
#define NICE_O_RELEASE_PARCELS    2
//variable used to bypass CounterMode settings if user just change Counter or Button
static bool bypass = false;

static const SubGhzBlockConst subghz_protocol_nice_flor_s_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 300,
    .min_count_bit_for_found = 52,
};

struct SubGhzProtocolDecoderNiceFlorS {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    const char* nice_flor_s_rainbow_table_file_name;

    /* Set only from a loaded file's IC key. Kept per instance so that opening an
     * O-Code signal cannot make every later Nice Flor-S decode look like O-Code. */
    uint16_t ic;
    uint8_t held_count_bit;
    /* One bit each, so the flags share a byte. o_code_no_ic is O-Code with the code
     * unknown: no serial or counter to recover, so it is shown raw and never sent. */
    bool o_code       : 1;
    bool o_code_no_ic : 1;
    bool skip_o_code  : 1;
    bool held_repeat  : 1;
    bool has_held     : 1;
    uint8_t fresh_run : 2;
    /* From the file's Cert / Cert_Cnt keys, for a remote still enrolling itself. A long
     * frame carries its own and does not need these. */
    uint8_t cert_cnt;
    uint32_t cert;

    /* The frame waiting to be judged, 52 and 72 bit only. Split in halves because a
     * uint64 here would cost eight bytes of alignment padding. */
    uint32_t held_lo;
    uint32_t held_hi;
    uint32_t held_trail;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderNiceFlorS);

struct SubGhzProtocolEncoderNiceFlorS {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    const char* nice_flor_s_rainbow_table_file_name;

    /* The receiver certificate, sent in the long frame while the remote is still being
     * enrolled. 0 for a remote that has none. */
    uint32_t cert;
    uint16_t ic;
    bool o_code;
    /* Set for this transmission when a certificate is present and its twenty presses
     * are not used up. */
    bool cert_on;
};
SUBGHZ_ASSERT_ENCODER_COMMON_LAYOUT(SubGhzProtocolEncoderNiceFlorS);

typedef enum {
    NiceFlorSDecoderStepReset = 0,
    NiceFlorSDecoderStepCheckHeader,
    NiceFlorSDecoderStepFoundHeader,
    NiceFlorSDecoderStepSaveDuration,
    NiceFlorSDecoderStepCheckDuration,
} NiceFlorSDecoderStep;

/* Nice O-Code is a distinct variant of the protocol, not a counter mode: the "IC" key in
 * the .sub file is the type marker, and FFFF in it means O-Code whose installer code is
 * not known. CounterMode keeps its normal meaning for both. */

static uint8_t subghz_protocol_decoder_nice_flor_s_get_hash_data(void* context);

const SubGhzProtocolDecoder subghz_protocol_nice_flor_s_decoder = {
    .alloc = subghz_protocol_decoder_nice_flor_s_alloc,
    .free = subghz_protocol_decoder_nice_flor_s_free,

    .feed = subghz_protocol_decoder_nice_flor_s_feed,
    .reset = subghz_protocol_decoder_nice_flor_s_reset,

    .get_hash_data = subghz_protocol_decoder_nice_flor_s_get_hash_data,
    .serialize = subghz_protocol_decoder_nice_flor_s_serialize,
    .deserialize = subghz_protocol_decoder_nice_flor_s_deserialize,
    .get_string = subghz_protocol_decoder_nice_flor_s_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nice_flor_s_encoder = {
    .alloc = subghz_protocol_encoder_nice_flor_s_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_nice_flor_s_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_nice_flor_s = {
    .name = SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send | SubGhzProtocolFlag_NiceFlorS,

    .decoder = &subghz_protocol_nice_flor_s_decoder,
    .encoder = &subghz_protocol_nice_flor_s_encoder,
};

static void subghz_protocol_nice_flor_s_remote_controller(
    SubGhzBlockGeneric* instance,
    const char* file_name,
    uint16_t ic,
    bool o_code);

void* subghz_protocol_encoder_nice_flor_s_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderNiceFlorS* instance = malloc(sizeof(SubGhzProtocolEncoderNiceFlorS));

    instance->ic = NICE_FLOR_S_IC_PLAIN;
    instance->cert = 0;
    instance->o_code = false;
    instance->base.protocol = &subghz_protocol_nice_flor_s;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->nice_flor_s_rainbow_table_file_name =
        subghz_environment_get_nice_flor_s_rainbow_table_file_name(environment);
    if(instance->nice_flor_s_rainbow_table_file_name) {
        FURI_LOG_D(
            TAG, "Loading rainbow table from %s", instance->nice_flor_s_rainbow_table_file_name);
    }
    instance->encoder.repeat = 1;
    instance->encoder.size_upload = 2400; // 2368 for Nice ONE
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

static void subghz_protocol_nice_one_get_data(uint8_t* p, uint8_t num_parcel, uint8_t flags);

static void
    subghz_protocol_nice_o_cert_data(uint8_t* p, uint8_t index, uint32_t cert, uint64_t body);

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x1 | 0x2 | 0x4 | 0x8 |
 * @return Button code
 */
static uint8_t subghz_protocol_nice_flor_s_get_btn_code(void);

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 * @return true On success
 */
static void subghz_protocol_encoder_nice_flor_s_get_upload(
    SubGhzProtocolEncoderNiceFlorS* instance,
    uint8_t btn,
    const char* file_name) {
    furi_assert(instance);
    size_t index = 0;
    btn = instance->generic.btn;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(btn);
    }

    btn = subghz_protocol_nice_flor_s_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&btn)) {
        bypass = true;
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", btn);
    }

    size_t size_upload = ((instance->generic.data_count_bit * 2) + ((37 + 2 + 2) * 2) * 16);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
    } else {
        instance->encoder.size_upload = size_upload;
    }

    // if we change counter/button in SignalSettings menu then we must bypass counter_modes, just gen and save signal file.
    if(subghz_block_generic_global.cnt_need_override) bypass = true;

    if(nice_flors_counter_mode == 0 || bypass) {
        // Check for OFEX (overflow experimental) mode
        if(furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF || bypass) {
            bypass = false;
            // standart counter mode. PULL data from subghz_block_generic_global variables
            if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
                // if counter_override_get return FALSE then counter was not changed and we increase counter by standart mult value
                if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
                    instance->generic.cnt = 0;
                } else {
                    instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
                }
            }
        } else {
            if((instance->generic.cnt + 0x1) > 0xFFFF) {
                instance->generic.cnt = 0;
            } else if(instance->generic.cnt >= 0x1 && instance->generic.cnt != 0xFFFE) {
                instance->generic.cnt = 0xFFFE;
            } else {
                instance->generic.cnt++;
            }
        }
    } else if(nice_flors_counter_mode == 1) {
        // Mode 1 (floxi2r)
        // 0001 / FFFE
        if(instance->generic.cnt == 0xFFFE) {
            instance->generic.cnt = 0x0001;
        } else {
            instance->generic.cnt = 0xFFFE;
        }
    } else {
        // Mode 2 (ox2)
        // 0x0000 / 0x0001
        if(instance->generic.cnt >= 0x0001) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt++;
        }
    }

    const bool o_code = instance->o_code;
    uint64_t decrypt = ((uint64_t)instance->generic.serial << 16) | instance->generic.cnt;
    uint64_t enc_part = subghz_protocol_nice_flor_s_encrypt(decrypt, file_name);

    /* A certificate sweep is shorter so that its longer parcels still fit the buffer. */
    const uint8_t parcels = instance->cert_on ? NICE_O_CERT_PARCELS : 16;

    for(int i = 0; i < parcels; i++) {
        static const uint64_t loops[16] = {
            0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

        if(o_code) {
            // both the serial mask and the cipher key change with the parcel index
            instance->generic.data = subghz_protocol_nice_o_make_key(
                instance->generic.serial,
                instance->generic.cnt,
                btn,
                (uint8_t)loops[i],
                instance->ic,
                file_name);
        } else {
            uint8_t byte = btn << 4 | (0xF ^ btn ^ loops[i]);
            instance->generic.data = (uint64_t)byte << 44 | enc_part;
        }

        //Send header
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_nice_flor_s_const.te_short * 37);
        //Send start bit
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nice_flor_s_const.te_short * 3);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_nice_flor_s_const.te_short * 3);

        //Send key data
        for(uint8_t j = 52; j > 0; j--) {
            if(bit_read(instance->generic.data, j - 1)) {
                //send bit 1
                instance->encoder.upload[index++] =
                    level_duration_make(true, (uint32_t)subghz_protocol_nice_flor_s_const.te_long);
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_nice_flor_s_const.te_short);
            } else {
                //send bit 0
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_nice_flor_s_const.te_short);
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_nice_flor_s_const.te_long);
            }
        }
        if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
            uint8_t add_data[15] = {0};
            for(size_t i = 0; i < 7; i++) {
                add_data[i] = (instance->generic.data >> (48 - i * 8)) & 0xFF;
            }
            /* An O-Code remote marks the last two parcels of every press, whatever its
             * length, so a press of ours ends the same way. The one Nice One remote
             * captured instead has the bit on every parcel it sent - parcel 0 was not
             * among them - so that side keeps the behaviour it shipped with. */
            uint8_t flags = 0;
            if(o_code) {
                flags = 0x4;
                if(i + NICE_O_RELEASE_PARCELS >= parcels) flags |= 0x1;
            } else if(loops[i]) {
                flags = 0x1;
            }
            if(instance->cert_on) flags |= 0x2;
            subghz_protocol_nice_one_get_data(add_data, loops[i], flags);
            const size_t tail = instance->cert_on ? 8 : 3;
            if(instance->cert_on) {
                /* The index is parcel + 1 xored with a value a real remote holds for a
                 * whole press and changes for the next one. Taking it from the counter
                 * gives the same per press behaviour for nothing. The stored certificate
                 * is the receiver's own, so it goes out masked with this remote's serial. */
                subghz_protocol_nice_o_cert_data(
                    add_data,
                    (uint8_t)(((loops[i] + 1) ^ instance->generic.cnt) & 0x0F),
                    instance->cert ^ (instance->generic.serial & 0xFFFFFF),
                    instance->generic.data);
            }
            instance->generic.data_2 = 0;
            for(size_t j = 7; j < 7 + tail; j++) {
                instance->generic.data_2 <<= 8;
                instance->generic.data_2 += add_data[j];
            }

            //Send key data
            for(uint8_t j = (uint8_t)(tail * 8); j > 4; j--) {
                if(bit_read(instance->generic.data_2, j - 1)) {
                    //send bit 1
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_nice_flor_s_const.te_long);
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_nice_flor_s_const.te_short);
                } else {
                    //send bit 0
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_nice_flor_s_const.te_short);
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_nice_flor_s_const.te_long);
                }
            }
        }
        //Send stop bit
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nice_flor_s_const.te_short * 3);
        //instance->encoder.upload[index++] =
        //level_duration_make(false, (uint32_t)subghz_protocol_nice_flor_s_const.te_short * 3);
    }
    instance->encoder.size_upload = index;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderNiceFlorS* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);
        // flipper_format_read_uint32(
        // flipper_format, "Data", (uint32_t*)&instance->generic.data_2, 1);
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint32_t tmp_counter_mode;
        if(flipper_format_read_uint32(flipper_format, "CounterMode", &tmp_counter_mode, 1)) {
            nice_flors_counter_mode = (uint8_t)tmp_counter_mode;
        } else {
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t ic_data[2] = {0xFF, 0xFF};
        if(flipper_format_read_hex(flipper_format, "IC", ic_data, sizeof(ic_data))) {
            instance->ic = (uint16_t)ic_data[0] << 8 | ic_data[1];
            instance->o_code = true;
        } else {
            instance->ic = NICE_FLOR_S_IC_PLAIN;
            instance->o_code = false;
        }

        instance->cert = 0;
        instance->cert_on = false;
        uint32_t cert_cnt = 0;
        if(instance->o_code && flipper_format_rewind(flipper_format)) {
            uint8_t cert_data[3] = {0};
            if(flipper_format_read_hex(flipper_format, "Cert", cert_data, sizeof(cert_data))) {
                instance->cert = (uint32_t)cert_data[0] << 16 | (uint32_t)cert_data[1] << 8 |
                                 cert_data[2];
            }
            if(flipper_format_rewind(flipper_format)) {
                flipper_format_read_uint32(flipper_format, "Cert_Cnt", &cert_cnt, 1);
            }
            instance->cert_on = instance->cert && (cert_cnt < NICE_O_CERT_SENDS) &&
                                (instance->generic.data_count_bit == NICE_ONE_COUNT_BIT);
        }

        subghz_protocol_nice_flor_s_remote_controller(
            &instance->generic,
            instance->nice_flor_s_rainbow_table_file_name,
            instance->ic,
            instance->o_code);
        subghz_protocol_encoder_nice_flor_s_get_upload(
            instance, instance->generic.btn, instance->nice_flor_s_rainbow_table_file_name);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }

        if(instance->cert_on) {
            /* data_2 holds the 60 bit tail here, not the 20 bit one the file stores, so
             * leave "Data" alone and spend one of the twenty presses instead. */
            cert_cnt++;
            if(!flipper_format_rewind(flipper_format) ||
               !flipper_format_insert_or_update_uint32(flipper_format, "Cert_Cnt", &cert_cnt, 1)) {
                FURI_LOG_E(TAG, "Unable to update Cert_Cnt");
            }
        } else if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
            if(!flipper_format_rewind(flipper_format)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            uint32_t temp = (instance->generic.data_2 >> 4) & 0xFFFFF;
            if(!flipper_format_update_uint32(flipper_format, "Data", &temp, 1)) {
                FURI_LOG_E(TAG, "Unable to update Data");
            }
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

/**
 * Read bytes from rainbow table
 * @param p array[10]  P0-P1|P2-P3-P4-P5-P6-P7-P8-P9-P10
 * @return crc
 */
static uint32_t subghz_protocol_nice_one_crc(uint8_t* p, uint8_t start, uint8_t end) {
    uint8_t crc = 0;
    uint8_t crc_data = 0xff;
    for(uint8_t i = start; i < end; i++) {
        if(subghz_protocol_blocks_get_bit_array(p, i)) {
            crc = crc_data ^ 1;
        } else {
            crc = crc_data;
        }
        crc_data >>= 1;
        if((crc & 0x01)) {
            crc_data ^= 0x97;
        }
    }
    crc = 0;
    for(uint8_t i = 0; i < 8; i++) {
        crc <<= 1;
        if((crc_data >> i) & 0x01) crc = crc | 1;
    }
    return crc;
}

/**
 * Read bytes from rainbow table
 * @param p array[10]  P0-P1|P2-P3-P4-P5-P6-P7-XX-XX-XX
 * @param num_parcel  parcel number 0..15
 * @param hold_bit  0 - the button was only pressed, 1 - the button was held down
 */
/* An O-Code remote's k departs from the Nice One formula at parcels 8..B. Read off one
 * remote, the same in 72 and 112 bit frames across two captures; a real Nice One remote in
 * the same captures follows the formula, so the table is kept to O-Code. */
static const uint8_t nice_o_k[16] =
    {0x8F, 0x8E, 0x8D, 0x8C, 0x84, 0x85, 0x86, 0x87, 0x8F, 0x8C, 0x89, 0x8D, 0x8C, 0x8D, 0x8E, 0x8F};

/* flags: bit 0 is the release marker the last parcels of a press carry, bit 1 says this is
 * a certificate frame, bit 2 picks the O-Code table over the Nice One formula. Only the
 * low two reach the frame. */
static void subghz_protocol_nice_one_get_data(uint8_t* p, uint8_t num_parcel, uint8_t flags) {
    uint8_t k = 0;
    uint8_t crc = 0;
    num_parcel &= 0x0F;
    p[1] = (p[1] & 0x0f) | ((0x0f ^ (p[0] & 0x0F) ^ num_parcel) << 4);
    if(flags & 0x4) {
        k = nice_o_k[num_parcel];
    } else {
        k = num_parcel ^ (num_parcel < 4 ? 0x8f : 0x80);
    }
    /* a certificate frame is the same k with bit 6 set */
    if(flags & 0x2) k |= 0x40;
    p[7] = k;
    p[8] = (uint8_t)(((flags & 0x3) ^ k) << 4);

    crc = subghz_protocol_nice_one_crc(p, 4, 68);

    p[8] |= crc >> 4;
    p[9] = crc << 4;
}

/* The 40 bits a certificate frame adds after the 72 bit one: an 0xE marker, a 4 bit index,
 * the certificate masked with the low body bits, and a CRC over the join. p must hold 15
 * bytes. */
static void
    subghz_protocol_nice_o_cert_data(uint8_t* p, uint8_t index, uint32_t cert, uint64_t body) {
    const uint32_t v = (cert ^ (uint32_t)body) & 0xFFFFFF;
    p[9] |= 0x0E;
    p[10] = (uint8_t)(index << 4) | (uint8_t)(v >> 20);
    p[11] = (uint8_t)(v >> 12);
    p[12] = (uint8_t)(v >> 4);
    p[13] = (uint8_t)(v << 4);
    const uint8_t crc = (uint8_t)subghz_protocol_nice_one_crc(p, 68, 108);
    p[13] |= crc >> 4;
    p[14] = (uint8_t)(crc << 4);
}

/** 
 * Read bytes from buffer array with rainbow table
 * @param buffer pointer to decrypted rainbow table 
 * @param address Byte address in file
 * @return data
 */
static uint8_t
    subghz_protocol_nice_flor_s_get_byte_from_buffer(const uint8_t* buffer, uint8_t address) {
    return buffer[address];
}

static inline void subghz_protocol_decoder_nice_flor_s_magic_xor(uint8_t* p, uint8_t k) {
    for(uint8_t i = 1; i < 6; i++) {
        p[i] ^= k;
    }
}

static uint64_t
    subghz_protocol_nice_flor_s_encrypt_ic(uint64_t data, uint16_t ic, const char* file_name) {
    const uint8_t* buffer_ptr = subghz_protocol_nice_flor_s_table(file_name);
    if(!buffer_ptr) return SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE;

    uint8_t* p = (uint8_t*)&data;

    uint8_t k = 0;
    for(uint8_t y = 0; y < 2; y++) {
        k = subghz_protocol_nice_flor_s_get_byte_from_buffer(buffer_ptr, p[0] & 0x1f);
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0xe0;
        k = subghz_protocol_nice_flor_s_get_byte_from_buffer(buffer_ptr, p[0] >> 3) + 0x25;
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0x7;
        if(y == 0) {
            k = p[0];
            p[0] = p[1];
            p[1] = k;
        }
    }

    // p[3] and p[4] are XORed with the installer code; ic == 0xFFFF gives the plain
    // Nice Flor-S inversion.
    p[5] = ~p[5] & 0x0f;
    k = ~p[4];
    p[4] = p[0] ^ (uint8_t)(ic >> 8);
    p[0] = ~p[2];
    p[2] = k;
    k = p[1] ^ (uint8_t)ic;
    p[1] = ~p[3];
    p[3] = k;

    return data;
}

uint64_t subghz_protocol_nice_flor_s_encrypt(uint64_t data, const char* file_name) {
    return subghz_protocol_nice_flor_s_encrypt_ic(data, NICE_FLOR_S_IC_PLAIN, file_name);
}

static uint64_t subghz_protocol_nice_flor_s_decrypt_ic_buffer(
    uint64_t data,
    uint16_t ic,
    const uint8_t* buffer_ptr);

/* As subghz_protocol_nice_flor_s_decrypt_ic(), against a table already in RAM, for
 * anything that decrypts in a loop. */
static uint64_t subghz_protocol_nice_flor_s_decrypt_ic_buffer(
    uint64_t data,
    uint16_t ic,
    const uint8_t* buffer_ptr) {
    uint8_t* p = (uint8_t*)&data;

    uint8_t k = 0;

    k = p[4] ^ (uint8_t)(ic >> 8);
    p[5] = ~p[5];
    p[4] = ~p[2];
    p[2] = ~p[0];
    p[0] = k;
    k = p[3] ^ (uint8_t)ic;
    p[3] = ~p[1];
    p[1] = k;

    for(uint8_t y = 0; y < 2; y++) {
        k = subghz_protocol_nice_flor_s_get_byte_from_buffer(buffer_ptr, p[0] >> 3) + 0x25;
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0x7;
        k = subghz_protocol_nice_flor_s_get_byte_from_buffer(buffer_ptr, p[0] & 0x1f);
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0xe0;

        if(y == 0) {
            k = p[0];
            p[0] = p[1];
            p[1] = k;
        }
    }

    return data;
}

/* Reading the rainbow table means AES decrypting a file on the SD card, so keep the
 * last one: a sweep of all 65536 installer codes becomes a RAM only loop. Only results
 * leave this file, never the table. */
static uint8_t nice_flor_s_table[SUBGHZ_NICE_FLOR_S_RAINBOW_TABLE_SIZE_BYTES];
static char nice_flor_s_table_path[64];
static bool nice_flor_s_table_valid = false;

static const uint8_t* subghz_protocol_nice_flor_s_table(const char* file_name) {
    if(!file_name) return NULL;
    if(nice_flor_s_table_valid && (strcmp(nice_flor_s_table_path, file_name) == 0)) {
        return nice_flor_s_table;
    }

    nice_flor_s_table_valid = false;
    if(!subghz_keystore_raw_get_data(
           file_name, 0, nice_flor_s_table, SUBGHZ_NICE_FLOR_S_RAINBOW_TABLE_SIZE_BYTES)) {
        return NULL;
    }
    /* a path too long to remember is still usable, it just reloads every time */
    if(strlen(file_name) < sizeof(nice_flor_s_table_path)) {
        strcpy(nice_flor_s_table_path, file_name);
        nice_flor_s_table_valid = true;
    }
    return nice_flor_s_table;
}

uint64_t
    subghz_protocol_nice_flor_s_decrypt_ic(uint64_t data, uint16_t ic, const char* file_name) {
    const uint8_t* buffer = subghz_protocol_nice_flor_s_table(file_name);
    if(!buffer) return SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE;
    return subghz_protocol_nice_flor_s_decrypt_ic_buffer(data, ic, buffer);
}

static uint64_t
    subghz_protocol_nice_flor_s_decrypt(SubGhzBlockGeneric* instance, const char* file_name) {
    furi_assert(instance);
    return subghz_protocol_nice_flor_s_decrypt_ic(instance->data, NICE_FLOR_S_IC_PLAIN, file_name);
}

/**
 * Per-parcel obfuscation mask used by Nice O-Code.
 * @param n Parcel index, 0..15, recovered from P0 ^ P1 ^ 0xF
 * @param mask_32 Mask applied to the 28 bit serial
 * @param mask_16 Mask applied to the installer code before decryption
 */
void subghz_protocol_nice_o_mask(uint8_t n, uint32_t* mask_32, uint16_t* mask_16) {
    const uint32_t a = (uint32_t)(0x0Eu - n) & 0x0Fu;
    const uint32_t b = (uint32_t)(n + 2u);
    const uint32_t odd = (uint32_t)n & 1u;
    const uint32_t c = b + (odd << 1);
    const uint32_t bn = b & 0x0Fu;
    const uint32_t cn = c & 0x0Fu;

    uint32_t m = (a << 8) | (bn << 4) | (bn << 20) | (a << 24) | cn | (cn << 16);
    m |= (((((b ^ (uint32_t)(0x0Eu - n) ^ c) & 0x0Fu) + (odd << 2)) << 12) & 0xFFFFu);

    if(mask_32) *mask_32 = m & 0x0FFFFFFFu;
    // REV16: swap the two bytes inside each halfword, then keep the low halfword
    if(mask_16) *mask_16 = (uint16_t)(((m & 0x00FFu) << 8) | ((m >> 8) & 0x00FFu));
}

uint8_t subghz_protocol_nice_o_get_parcel(uint64_t data) {
    /* P0, the button, is bits 48..51 and P1 is bits 44..47, where P1 = 0xF ^ P0 ^ n.
     * Everything below bit 44 is ciphertext. */
    return (uint8_t)((((data >> 48) & 0x0Fu) ^ ((data >> 44) & 0x0Fu)) ^ 0x0Fu);
}

static bool subghz_protocol_nice_o_decrypt(
    uint64_t data,
    uint16_t ic,
    const char* file_name,
    uint32_t* serial,
    uint16_t* cnt,
    uint8_t* btn) {
    uint32_t mask_32 = 0;
    uint16_t mask_16 = 0;
    subghz_protocol_nice_o_mask(subghz_protocol_nice_o_get_parcel(data), &mask_32, &mask_16);

    uint64_t decrypted =
        subghz_protocol_nice_flor_s_decrypt_ic(data, (uint16_t)(ic ^ mask_16), file_name);
    if(decrypted == SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE) return false;

    if(cnt) *cnt = (uint16_t)decrypted;
    if(serial) *serial = (uint32_t)((decrypted >> 16) ^ mask_32) & 0x0FFFFFFFu;
    if(btn) *btn = (uint8_t)((data >> 48) & 0x0Fu);
    return true;
}

uint64_t subghz_protocol_nice_o_make_key(
    uint32_t serial,
    uint16_t cnt,
    uint8_t btn,
    uint8_t parcel,
    uint16_t ic,
    const char* file_name) {
    parcel &= 0x0Fu;
    uint32_t mask_32 = 0;
    uint16_t mask_16 = 0;
    subghz_protocol_nice_o_mask(parcel, &mask_32, &mask_16);

    const uint64_t plain = ((uint64_t)((serial ^ mask_32) & 0x0FFFFFFFu) << 16) | cnt;
    const uint64_t enc_part =
        subghz_protocol_nice_flor_s_encrypt_ic(plain, (uint16_t)(ic ^ mask_16), file_name);
    if(enc_part == SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE) return 0;

    const uint8_t byte = (uint8_t)((btn & 0x0Fu) << 4 | (0x0Fu ^ (btn & 0x0Fu) ^ parcel));
    return (uint64_t)byte << 44 | enc_part;
}

static bool subghz_protocol_nice_flor_s_create_data_ic(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset,
    bool nice_one,
    bool o_code,
    uint16_t ic) {
    furi_assert(context);
    SubGhzProtocolEncoderNiceFlorS* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    /* O-Code comes in both lengths: the 20 extra bits are the Nice One tail built over
     * whatever body precedes them, so they ride on an O-Code body just as well. */
    if(nice_one) {
        instance->generic.data_count_bit = NICE_ONE_COUNT_BIT;
    } else {
        instance->generic.data_count_bit = 52;
    }
    instance->o_code = o_code;
    instance->ic = o_code ? ic : NICE_FLOR_S_IC_PLAIN;

    if(o_code) {
        instance->generic.data = subghz_protocol_nice_o_make_key(
            serial,
            cnt,
            btn,
            NICE_FLOR_S_CREATE_PARCEL,
            ic,
            instance->nice_flor_s_rainbow_table_file_name);
        if(instance->generic.data == 0) return false;
    } else {
        uint64_t decrypt = ((uint64_t)instance->generic.serial << 16) | instance->generic.cnt;
        uint64_t enc_part = subghz_protocol_nice_flor_s_encrypt(
            decrypt, instance->nice_flor_s_rainbow_table_file_name);
        uint8_t byte = btn << 4 | (0xF ^ btn ^ NICE_FLOR_S_CREATE_PARCEL);
        instance->generic.data = (uint64_t)byte << 44 | enc_part;
    }

    if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
        uint8_t add_data[10] = {0};
        for(size_t i = 0; i < 7; i++) {
            add_data[i] = (instance->generic.data >> (48 - i * 8)) & 0xFF;
        }
        subghz_protocol_nice_one_get_data(add_data, NICE_FLOR_S_CREATE_PARCEL, o_code ? 0x4 : 0x0);
        instance->generic.data_2 = 0;
        for(size_t j = 7; j < 10; j++) {
            instance->generic.data_2 <<= 8;
            instance->generic.data_2 += add_data[j];
        }
    }

    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((res == SubGhzProtocolStatusOk) && o_code) {
        uint8_t ic_data[2] = {(uint8_t)(ic >> 8), (uint8_t)ic};
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_insert_or_update_hex(flipper_format, "IC", ic_data, sizeof(ic_data))) {
            FURI_LOG_E(TAG, "Unable to add IC");
            res = SubGhzProtocolStatusErrorParserOthers;
        }
    }

    return res == SubGhzProtocolStatusOk;
}

bool subghz_protocol_nice_flor_s_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset,
    bool nice_one) {
    return subghz_protocol_nice_flor_s_create_data_ic(
        context, flipper_format, serial, btn, cnt, preset, nice_one, false, 0);
}

bool subghz_protocol_nice_o_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint16_t ic,
    SubGhzRadioPreset* preset) {
    return subghz_protocol_nice_flor_s_create_data_ic(
        context, flipper_format, serial, btn, cnt, preset, false, true, ic);
}

/* O-Code in the 72 bit Nice One frame, for a gate taught by a remote that sends it. */
bool subghz_protocol_nice_o_one_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint16_t ic,
    SubGhzRadioPreset* preset) {
    return subghz_protocol_nice_flor_s_create_data_ic(
        context, flipper_format, serial, btn, cnt, preset, true, true, ic);
}

void* subghz_protocol_decoder_nice_flor_s_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderNiceFlorS* instance = subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderNiceFlorS), &subghz_protocol_nice_flor_s);
    instance->ic = NICE_FLOR_S_IC_PLAIN;
    instance->cert = 0;
    instance->cert_cnt = 0;
    instance->o_code = false;
    instance->o_code_no_ic = false;
    instance->has_held = false;
    instance->held_repeat = false;
    instance->fresh_run = 0;
    instance->skip_o_code = true;
    instance->nice_flor_s_rainbow_table_file_name =
        subghz_environment_get_nice_flor_s_rainbow_table_file_name(environment);
    if(instance->nice_flor_s_rainbow_table_file_name) {
        FURI_LOG_D(
            TAG, "Loading rainbow table from %s", instance->nice_flor_s_rainbow_table_file_name);
    }
    return instance;
}

/* Drops what a loaded O-Code file left behind, since live signals carry no installer
 * code. Read calls this per frame, not per receive, so the held frame is left alone
 * and cleared where a run really ends: alloc, a loaded file, or the app. */
void subghz_protocol_decoder_nice_flor_s_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    instance->ic = NICE_FLOR_S_IC_PLAIN;
    instance->cert = 0;
    instance->cert_cnt = 0;
    instance->o_code = false;
    instance->o_code_no_ic = false;
    subghz_protocol_decoder_common_reset(context);
}

/* True when the last decoded frame was O-Code with no installer code known, so it has
 * no recoverable serial or counter and must not be transmitted. */
bool subghz_protocol_decoder_nice_flor_s_is_o_code_no_ic(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    return instance->o_code_no_ic;
}

void subghz_protocol_decoder_nice_flor_s_set_skip_o_code(void* context, bool skip) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    instance->skip_o_code = skip;
    instance->has_held = false;
    instance->held_repeat = false;
    instance->fresh_run = 0;
}

/* Hash the body without the parcel number at bits 44..51, and from generic rather than
 * the decoder, since the frame handed on is the held one. */
static uint8_t subghz_protocol_decoder_nice_flor_s_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    uint32_t id = (uint32_t)instance->generic.data ^
                  (uint32_t)((instance->generic.data >> 32) & 0xFFF);
    return (uint8_t)((id * 2654435761u) >> 24);
}

void subghz_protocol_decoder_nice_flor_s_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    instance->nice_flor_s_rainbow_table_file_name = NULL;
    free(instance);
}

void subghz_protocol_decoder_nice_flor_s_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;

    switch(instance->decoder.parser_step) {
    case NiceFlorSDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 38) <
                        subghz_protocol_nice_flor_s_const.te_delta * 38)) {
            //Found start header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepCheckHeader;
        }
        break;
    case NiceFlorSDecoderStepCheckHeader:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
                       subghz_protocol_nice_flor_s_const.te_delta * 3)) {
            //Found next header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepFoundHeader;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        break;
    case NiceFlorSDecoderStepFoundHeader:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
                        subghz_protocol_nice_flor_s_const.te_delta * 3)) {
            //Found header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        break;
    case NiceFlorSDecoderStepSaveDuration:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
               subghz_protocol_nice_flor_s_const.te_delta) {
                //Found STOP bit
                instance->decoder.parser_step = NiceFlorSDecoderStepReset;
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_nice_flor_s_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit == NICE_ONE_COUNT_BIT) ||
                   (instance->decoder.decode_count_bit == NICE_O_LONG_COUNT_BIT)) {
                    instance->generic.data = instance->generic.data_2;
                    instance->generic.data_2 = instance->decoder.decode_data;
                    instance->decoder.decode_data = instance->generic.data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    const bool long_o =
                        (instance->generic.data_count_bit == NICE_O_LONG_COUNT_BIT);
                    if(long_o) {
                        /* Proof on its own, so it goes out as it comes in and vouches
                         * for the shorter parcels sent with it. */
                        instance->fresh_run = NICE_O_FRESH_REPORTS;
                        instance->o_code = true;
                        instance->o_code_no_ic = (instance->ic == NICE_FLOR_S_IC_PLAIN);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                        break;
                    }

                    /* One press reuses its body for all 16 parcels, rotating only the
                     * P0/P1 byte above bit 43, while O-Code re-encrypts every parcel. A
                     * frame alone cannot say which it is, so one is held back and judged
                     * against both neighbours. Only the frame opening a press goes on,
                     * the rest repeat its counter, and a frame still undecided is held
                     * back rather than guessed at. */
                    const uint32_t body = (uint32_t)instance->generic.data;

                    if(instance->has_held && !instance->held_repeat) {
                        const bool fresh = (instance->held_lo != body);
                        const bool is_o = fresh && (instance->fresh_run >= NICE_O_FRESH_REPORTS);

                        if(!fresh) {
                            instance->fresh_run = 0;
                        } else if(instance->fresh_run < NICE_O_FRESH_REPORTS) {
                            instance->fresh_run++;
                        }

                        if(!fresh || is_o) {
                            const uint64_t data = instance->generic.data;
                            const uint64_t data_2 = instance->generic.data_2;
                            const uint16_t count_bit = instance->generic.data_count_bit;
                            instance->generic.data = (uint64_t)instance->held_hi << 32 |
                                                     instance->held_lo;
                            instance->generic.data_2 = instance->held_trail;
                            instance->generic.data_count_bit = instance->held_count_bit;
                            instance->o_code = is_o;
                            instance->o_code_no_ic = is_o &&
                                                     (instance->ic == NICE_FLOR_S_IC_PLAIN);
                            if(instance->base.callback)
                                instance->base.callback(&instance->base, instance->base.context);
                            instance->generic.data = data;
                            instance->generic.data_2 = data_2;
                            instance->generic.data_count_bit = count_bit;
                        }
                    } else if(instance->has_held) {
                        instance->fresh_run = 0;
                    }
                    instance->held_repeat = instance->has_held && (instance->held_lo == body);
                    instance->held_lo = body;
                    instance->held_hi = (uint32_t)(instance->generic.data >> 32);
                    instance->held_trail = (uint32_t)instance->generic.data_2;
                    instance->held_count_bit = (uint8_t)instance->generic.data_count_bit;
                    instance->has_held = true;
                }
                break;
            } else {
                //save interval
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = NiceFlorSDecoderStepCheckDuration;
            }
        }
        break;
    case NiceFlorSDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nice_flor_s_const.te_short) <
                subghz_protocol_nice_flor_s_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_long) <
                subghz_protocol_nice_flor_s_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nice_flor_s_const.te_long) <
                 subghz_protocol_nice_flor_s_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short) <
                 subghz_protocol_nice_flor_s_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        if(instance->decoder.decode_count_bit ==
           subghz_protocol_nice_flor_s_const.min_count_bit_for_found) {
            instance->generic.data_2 = instance->decoder.decode_data;
            instance->decoder.decode_data = 0;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file 
 */
static void subghz_protocol_nice_flor_s_remote_controller(
    SubGhzBlockGeneric* instance,
    const char* file_name,
    uint16_t ic,
    bool o_code) {
    /*
    * Protocol Nice Flor-S
    * Packet format Nice Flor-s: START-P0-P1-P2-P3-P4-P5-P6-P7-STOP
    * P0 (4-bit)    - button positional code - 1:0x1, 2:0x2, 3:0x4, 4:0x8;
    * P1 (4-bit)    - batch repetition number, calculated by the formula:
    * P1 = 0xF ^ P0 ^ n; where n changes from 1 to 15, then 0, and then in a circle
    * key 1: {0xE,0xF,0xC,0xD,0xA,0xB,0x8,0x9,0x6,0x7,0x4,0x5,0x2,0x3,0x0,0x1};
    * key 2: {0xD,0xC,0xF,0xE,0x9,0x8,0xB,0xA,0x5,0x4,0x7,0x6,0x1,0x0,0x3,0x2};
    * key 3: {0xB,0xA,0x9,0x8,0xF,0xE,0xD,0xC,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4};
    * key 4: {0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0,0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8};
    * P2 (4-bit)    - part of the serial number, P2 = (K ^ S3) & 0xF;
    * P3 (byte)     - the major part of the encrypted index
    * P4 (byte)     - the low-order part of the encrypted index
    * P5 (byte)     - part of the serial number, P5 = K ^ S2;
    * P6 (byte)     - part of the serial number, P6 = K ^ S1;
    * P7 (byte)     - part of the serial number, P7 = K ^ S0;
    * K (byte)      - depends on P3 and P4, K = Fk(P3, P4);
    * S3,S2,S1,S0   - serial number of the console 28 bit.
    *
    * data    => 0x1c5783607f7b3     key  serial  cnt
    * decrypt => 0x10436c6820444 => 0x1  0436c682 0444
    * 
    * Protocol Nice One
    * Generally repeats the Nice Flor-S protocol, but there are a few changes
    * Packet format first 52 bytes repeat Nice Flor-S protocol
    * The additional 20 bytes contain the code of the pressed button,
    *    the button hold bit and the CRC of the entire message.
    *       START-P0-P1-P2-P3-P4-P5-P6-P7-P8-P9-P10-STOP
    * P7 (byte)     - if (n<4) k=0x8f : k=0x80; P7= k^n;
    * P8 (byte)     - if (hold bit) b=0x00 : b=0x10; P8= b^(k<<4) | 4 hi bit crc
    * P10 (4-bit)   - 4 lo bit crc 
    *                            key+b crc  
    * data    => 0x1724A7D9A522F  899  D6 hold bit = 0 - just pressed the button
    * data    => 0x1424A7D9A522F  8AB  03 hold bit = 1 - button hold
    * 
    * A small button hold counter (0..15) is stored between each press,
    *  i.e. if 1 press of the button stops counter 6, then the next press 
    *  of the button will start from the value 7 (hold bit = 0), 8 (hold bit = 1)...
    *  further up to 15 with overflow
    * 
    */
    if(!file_name) {
        instance->cnt = 0;
        instance->serial = 0;
        instance->btn = 0;
    } else if(o_code) {
        // Nice O-Code: the serial is additionally masked per parcel and the cipher is
        // keyed with the remote's installer code.
        uint32_t serial = 0;
        uint16_t cnt = 0;
        uint8_t btn = 0;
        if(subghz_protocol_nice_o_decrypt(instance->data, ic, file_name, &serial, &cnt, &btn)) {
            instance->serial = serial;
            instance->cnt = cnt;
            instance->btn = btn;
        } else {
            instance->cnt = 0;
            instance->serial = 0;
            instance->btn = 0;
        }
    } else {
        uint64_t decrypt = subghz_protocol_nice_flor_s_decrypt(instance, file_name);
        instance->cnt = decrypt & 0xFFFF;
        instance->serial = (decrypt >> 16) & 0xFFFFFFF;
        instance->btn = (decrypt >> 48) & 0xF;
    }

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(4);
}

/* The long frame repeats the low body bits XORed with the certificate the remote was
 * loaded with, which is the receiver's own XORed with the remote's serial, so both come
 * back out together. Needs a decoded serial, so a frame with no installer code has none. */
static uint32_t subghz_protocol_nice_flor_s_cert(SubGhzBlockGeneric* generic) {
    return (uint32_t)((generic->data_2 >> 8) ^ generic->data ^ generic->serial) & 0xFFFFFF;
}

SubGhzProtocolStatus subghz_protocol_decoder_nice_flor_s_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((ret == SubGhzProtocolStatusOk) && instance->o_code) {
        uint8_t ic_data[2] = {(uint8_t)(instance->ic >> 8), (uint8_t)instance->ic};
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_insert_or_update_hex(flipper_format, "IC", ic_data, sizeof(ic_data))) {
            FURI_LOG_E(TAG, "Unable to add IC");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
    }
    if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
        if((ret == SubGhzProtocolStatusOk) &&
           !flipper_format_insert_or_update_uint32(
               flipper_format, "Data", (uint32_t*)&instance->generic.data_2, 1)) {
            FURI_LOG_E(TAG, "Unable to add Data");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
    } else if(instance->generic.data_count_bit == NICE_O_LONG_COUNT_BIT) {
        uint8_t d2[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            d2[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> i * 8) & 0xFF;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
        if((ret == SubGhzProtocolStatusOk) &&
           !flipper_format_insert_or_update_hex(flipper_format, "Data2", d2, sizeof(d2))) {
            FURI_LOG_E(TAG, "Unable to add Data2");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
        if((ret == SubGhzProtocolStatusOk) && !instance->o_code_no_ic) {
            subghz_protocol_nice_flor_s_remote_controller(
                &instance->generic,
                instance->nice_flor_s_rainbow_table_file_name,
                instance->ic,
                instance->o_code);
            const uint32_t cert = subghz_protocol_nice_flor_s_cert(&instance->generic);
            uint8_t cert_data[3] = {(uint8_t)(cert >> 16), (uint8_t)(cert >> 8), (uint8_t)cert};
            if(!flipper_format_rewind(flipper_format) ||
               !flipper_format_insert_or_update_hex(
                   flipper_format, "Cert", cert_data, sizeof(cert_data))) {
                FURI_LOG_E(TAG, "Unable to add Cert");
                ret = SubGhzProtocolStatusErrorParserOthers;
            }
        }
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_nice_flor_s_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit != NICE_ONE_COUNT_BIT) &&
           (instance->generic.data_count_bit != NICE_O_LONG_COUNT_BIT)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            ret = SubGhzProtocolStatusErrorValueBitCount;
            break;
        }
        if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
            if(!flipper_format_rewind(flipper_format)) {
                FURI_LOG_E(TAG, "Rewind error");
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }
            uint32_t temp = 0;
            if(!flipper_format_read_uint32(flipper_format, "Data", (uint32_t*)&temp, 1)) {
                FURI_LOG_E(TAG, "Missing Data");
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }
            instance->generic.data_2 = (uint64_t)temp;
        } else if(instance->generic.data_count_bit == NICE_O_LONG_COUNT_BIT) {
            if(!flipper_format_rewind(flipper_format)) {
                FURI_LOG_E(TAG, "Rewind error");
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }
            uint8_t d2[sizeof(uint64_t)] = {0};
            if(!flipper_format_read_hex(flipper_format, "Data2", d2, sizeof(d2))) {
                FURI_LOG_E(TAG, "Missing Data2");
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }
            instance->generic.data_2 = 0;
            for(size_t i = 0; i < sizeof(uint64_t); i++) {
                instance->generic.data_2 = instance->generic.data_2 << 8 | d2[i];
            }
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint32_t tmp_counter_mode;
        if(flipper_format_read_uint32(flipper_format, "CounterMode", &tmp_counter_mode, 1)) {
            nice_flors_counter_mode = (uint8_t)tmp_counter_mode;
        } else {
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t ic_data[2] = {0xFF, 0xFF};
        if(flipper_format_read_hex(flipper_format, "IC", ic_data, sizeof(ic_data))) {
            instance->ic = (uint16_t)ic_data[0] << 8 | ic_data[1];
            instance->o_code = true;
        } else {
            instance->ic = NICE_FLOR_S_IC_PLAIN;
            instance->o_code = false;
        }
        instance->cert = 0;
        instance->cert_cnt = 0;
        if(instance->o_code && flipper_format_rewind(flipper_format)) {
            uint8_t cert_data[3] = {0};
            if(flipper_format_read_hex(flipper_format, "Cert", cert_data, sizeof(cert_data))) {
                instance->cert = (uint32_t)cert_data[0] << 16 | (uint32_t)cert_data[1] << 8 |
                                 cert_data[2];
            }
            uint32_t used = 0;
            if(flipper_format_rewind(flipper_format) &&
               flipper_format_read_uint32(flipper_format, "Cert_Cnt", &used, 1)) {
                instance->cert_cnt = (used > 0xFF) ? 0xFF : (uint8_t)used;
            }
        }

        /* A file carries the code it was written with, and FFFF is a real one - a system
         * nobody personalised. Only a frame off the air has a code we do not know. */
        instance->o_code_no_ic = false;
        instance->has_held = false;
        instance->held_repeat = false;
        instance->fresh_run = 0;
    } while(false);
    return ret;
}

static uint8_t subghz_protocol_nice_flor_s_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x2;
            break;
        case 0x2:
            btn = 0x1;
            break;
        case 0x4:
            btn = 0x1;
            break;
        case 0x8:
            btn = 0x1;
            break;
        case 0x3:
            btn = 0x1;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x4;
            break;
        case 0x2:
            btn = 0x4;
            break;
        case 0x4:
            btn = 0x2;
            break;
        case 0x8:
            btn = 0x4;
            break;
        case 0x3:
            btn = 0x4;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x8;
            break;
        case 0x2:
            btn = 0x8;
            break;
        case 0x4:
            btn = 0x8;
            break;
        case 0x8:
            btn = 0x2;
            break;
        case 0x3:
            btn = 0x8;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_RIGHT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x3;
            break;
        case 0x2:
            btn = 0x3;
            break;
        case 0x4:
            btn = 0x3;
            break;
        case 0x8:
            btn = 0x3;
            break;
        case 0x3:
            btn = 0x2;
            break;

        default:
            break;
        }
    }

    return btn;
}

void subghz_protocol_decoder_nice_flor_s_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;

    subghz_protocol_nice_flor_s_remote_controller(
        &instance->generic,
        instance->nice_flor_s_rainbow_table_file_name,
        instance->ic,
        instance->o_code);

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 16;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 4;
    //

    if(instance->o_code_no_ic) {
        /* No installer code, so a serial and counter would be random. Show the raw key
         * only, and say where it can be decoded. */
        subghz_block_generic_global.cnt_is_available = false;
        subghz_block_generic_global.btn_is_available = false;
        furi_string_cat_printf(
            output,
            "%s %db\r\n"
            "Key:%013llX\r\n"
            "No installer code\r\n"
            "Use Nice O-Code app\r\n"
            "to receive and decode",
            NICE_O_NAME,
            instance->generic.data_count_bit,
            instance->generic.data);
    } else if(instance->o_code) {
        /* O-Code comes in both lengths, so the installer code decides the name before
         * the bit count does; the longer frame shows its tail like Nice One. */
        furi_string_cat_printf(
            output, "%s %db\r\n", NICE_O_NAME, instance->generic.data_count_bit);
        if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
            furi_string_cat_printf(
                output, "Key:%013llX%llX\r\n", instance->generic.data, instance->generic.data_2);
        } else {
            furi_string_cat_printf(output, "Key:%013llX\r\n", instance->generic.data);
        }
        furi_string_cat_printf(
            output,
            "Sn:%05lX IC:%04X\r\n"
            "Cnt:%04lX Btn:%02X\r\n",
            instance->generic.serial,
            instance->ic,
            instance->generic.cnt,
            instance->generic.btn);
        /* A long frame carries the certificate itself; a saved remote carries it as a key
         * and stops sending it once the twenty presses are gone, so it is only worth
         * showing while there are some left. */
        if(instance->generic.data_count_bit == NICE_O_LONG_COUNT_BIT) {
            furi_string_cat_printf(
                output, "Cert:%06lX\r\n", subghz_protocol_nice_flor_s_cert(&instance->generic));
        } else if(instance->cert && (instance->cert_cnt < NICE_O_CERT_SENDS)) {
            furi_string_cat_printf(
                output,
                "Cert:%06lX %u left\r\n",
                instance->cert,
                NICE_O_CERT_SENDS - instance->cert_cnt);
        }
    } else if(instance->generic.data_count_bit == NICE_ONE_COUNT_BIT) {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:%013llX%llX\r\n"
            "Sn:%05lX\r\n"
            "Cnt:%04lX Btn:%02X\r\n",
            NICE_ONE_NAME,
            instance->generic.data_count_bit,
            instance->generic.data,
            instance->generic.data_2,
            instance->generic.serial,
            instance->generic.cnt,
            instance->generic.btn);
    } else {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:0x%013llX\r\n"
            "Sn:%05lX\r\n"
            "Cnt:%04lX Btn:%02X\r\n",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            instance->generic.data,
            instance->generic.serial,
            instance->generic.cnt,
            instance->generic.btn);
    }
}
