#include "subghz_protocol_keeloq.h"
#include "subghz_protocol_keeloq_common.h"

#include "../subghz_keystore.h"

#include <furi.h>

#include <m-string.h>

struct SubGhzProtocolKeeloq {
    SubGhzProtocolCommon common;
    SubGhzKeystore* keystore;
    const char* manufacture_name;
};

SubGhzProtocolKeeloq* subghz_protocol_keeloq_alloc(SubGhzKeystore* keystore) {
    SubGhzProtocolKeeloq* instance = furi_alloc(sizeof(SubGhzProtocolKeeloq));

    instance->keystore = keystore;

    instance->common.name = "KeeLoq";
    instance->common.code_min_count_bit_for_found = 64;
    instance->common.te_short = 400;
    instance->common.te_long = 800;
    instance->common.te_delta = 140;
    instance->common.type_protocol = TYPE_PROTOCOL_DYNAMIC;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_keeloq_to_str;
    instance->common.to_save_string =
        (SubGhzProtocolCommonGetStrSave)subghz_protocol_keeloq_to_save_str;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_keeloq_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_keeloq_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_keeloq_send_key;

    return instance;
}

void subghz_protocol_keeloq_free(SubGhzProtocolKeeloq* instance) {
    furi_assert(instance);
    free(instance);
}

/** Checking the accepted code against the database manafacture key
 * 
 * @param instance SubGhzProtocolKeeloq instance
 * @param fix fix part of the parcel
 * @param hop hop encrypted part of the parcel
 * @return true on successful search
 */
uint8_t subghz_protocol_keeloq_check_remote_controller_selector(
    SubGhzProtocolKeeloq* instance,
    uint32_t fix,
    uint32_t hop) {
    uint16_t end_serial = (uint16_t)(fix & 0x3FF);
    uint8_t btn = (uint8_t)(fix >> 28);
    uint32_t decrypt = 0;
    uint64_t man_normal_learning;

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            switch(manufacture_code->type) {
            case KEELOQ_LEARNING_SIMPLE:
                //Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_NORMAL:
                // Normal_Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man_normal_learning =
                    subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_UNKNOWN:
                // Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                // Check for mirrored man
                uint64_t man_rev = 0;
                uint64_t man_rev_byte = 0;
                for(uint8_t i = 0; i < 64; i += 8) {
                    man_rev_byte = (uint8_t)(manufacture_code->key >> i);
                    man_rev = man_rev | man_rev_byte << (56 - i);
                }
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_rev);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                //###########################
                // Normal_Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man_normal_learning =
                    subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                // Check for mirrored man
                man_rev = 0;
                man_rev_byte = 0;
                for(uint8_t i = 0; i < 64; i += 8) {
                    man_rev_byte = (uint8_t)(manufacture_code->key >> i);
                    man_rev = man_rev | man_rev_byte << (56 - i);
                }
                man_normal_learning = subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if((decrypt >> 28 == btn) &&
                   (((((uint16_t)(decrypt >> 16)) & 0x3FF) == end_serial) ||
                    ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0))) {
                    instance->manufacture_name = string_get_cstr(manufacture_code->name);
                    instance->common.cnt = decrypt & 0x0000FFFF;
                    return 1;
                }
                break;
            }
        }

    instance->manufacture_name = "Unknown";
    instance->common.cnt = 0;

    return 0;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolKeeloq instance
 */
void subghz_protocol_keeloq_check_remote_controller(SubGhzProtocolKeeloq* instance) {
    uint64_t key = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t key_fix = key >> 32;
    uint32_t key_hop = key & 0x00000000ffffffff;
    // Check key AN-Motors
    if((key_hop >> 24) == ((key_hop >> 16) & 0x00ff) &&
       (key_fix >> 28) == ((key_hop >> 12) & 0x0f) && (key_hop & 0xFFF) == 0x404) {
        instance->manufacture_name = "AN-Motors";
        instance->common.cnt = key_hop >> 16;
    } else if((key_hop & 0xFFF) == (0x000) && (key_fix >> 28) == ((key_hop >> 12) & 0x0f)) {
        instance->manufacture_name = "HCS101";
        instance->common.cnt = key_hop >> 16;
    } else {
        subghz_protocol_keeloq_check_remote_controller_selector(instance, key_fix, key_hop);
    }
    instance->common.serial = key_fix & 0x0FFFFFFF;
    instance->common.btn = key_fix >> 28;
}

const char* subghz_protocol_keeloq_find_and_get_manufacture_name(void* context) {
    SubGhzProtocolKeeloq* instance = context;
    subghz_protocol_keeloq_check_remote_controller(instance);
    return instance->manufacture_name;
}

const char* subghz_protocol_keeloq_get_manufacture_name(void* context) {
    SubGhzProtocolKeeloq* instance = context;
    return instance->manufacture_name;
}

bool subghz_protocol_keeloq_set_manufacture_name(void* context, const char* manufacture_name) {
    SubGhzProtocolKeeloq* instance = context;
    instance->manufacture_name = manufacture_name;
    int res = 0;
        for
            M_EACH(
                manufacture_code,
                *subghz_keystore_get_data(instance->keystore),
                SubGhzKeyArray_t) {
                res = strcmp(string_get_cstr(manufacture_code->name), instance->manufacture_name);
                if(res == 0) return true;
            }
        instance->manufacture_name = "Unknown";
        return false;
}

uint64_t subghz_protocol_keeloq_gen_key(void* context) {
    SubGhzProtocolKeeloq* instance = context;
    uint32_t fix = instance->common.btn << 28 | instance->common.serial;
    uint32_t decrypt = instance->common.btn << 28 | (instance->common.serial & 0x3FF) << 16 |
                       instance->common.cnt;
    uint32_t hop = 0;
    uint64_t man_normal_learning = 0;
    int res = 0;

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(string_get_cstr(manufacture_code->name), instance->manufacture_name);
            if(res == 0) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_SIMPLE:
                    //Simple Learning
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, manufacture_code->key);
                    break;
                case KEELOQ_LEARNING_NORMAL:
                    //Simple Learning
                    man_normal_learning =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man_normal_learning);
                    break;
                case KEELOQ_LEARNING_UNKNOWN:
                    hop = 0; //todo
                    break;
                }
                break;
            }
        }
    uint64_t yek = (uint64_t)fix << 32 | hop;
    return subghz_protocol_common_reverse_key(yek, instance->common.code_last_count_bit);
}

bool subghz_protocol_keeloq_send_key(
    SubGhzProtocolKeeloq* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);

    //gen new key
    instance->common.cnt++;
    instance->common.code_last_found = subghz_protocol_keeloq_gen_key(instance);
    if(instance->common.callback)
        instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);

    if(!strcmp(instance->manufacture_name, "Unknown")) {
        return false;
    }

    size_t index = 0;
    encoder->size_upload = 11 * 2 + 2 + (instance->common.code_last_count_bit * 2) + 4;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;

    //Send header
    for(uint8_t i = 11; i > 0; i--) {
        encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
        encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);
    }
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
    encoder->upload[index++] =
        level_duration_make(false, (uint32_t)instance->common.te_short * 10);

    //Send key data
    for(uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
        if(bit_read(instance->common.code_last_found, i - 1)) {
            //send bit 1
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_short);
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_long);
        } else {
            //send bit 0
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_long);
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_short);
        }
    }
    // +send 2 status bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
    encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_long);

    //encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_long);
    //encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);

    // send end
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
    encoder->upload[index++] =
        level_duration_make(false, (uint32_t)instance->common.te_short * 40);

    return true;
}

void subghz_protocol_keeloq_reset(SubGhzProtocolKeeloq* instance) {
    instance->common.parser_step = 0;
}

void subghz_protocol_keeloq_parse(SubGhzProtocolKeeloq* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case 0:
        if((level) &&
           DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
            instance->common.parser_step = 1;
            instance->common.header_count++;
        } else {
            instance->common.parser_step = 0;
        }

        break;
    case 1:
        if((!level) &&
           (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
            instance->common.parser_step = 0;
            break;
        }
        if((instance->common.header_count > 2) &&
           (DURATION_DIFF(duration, instance->common.te_short * 10) <
            instance->common.te_delta * 10)) {
            // Found header
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
            instance->common.header_count = 0;
        }
        break;
    case 2:
        if(level) {
            instance->common.te_last = duration;
            instance->common.parser_step = 3;
        }
        break;
    case 3:
        if(!level) {
            if(duration >= (instance->common.te_short * 2 + instance->common.te_delta)) {
                // Found end TX
                instance->common.parser_step = 0;
                if(instance->common.code_count_bit >=
                   instance->common.code_min_count_bit_for_found) {
                    if(instance->common.code_last_found != instance->common.code_found) {
                        instance->common.code_last_found = instance->common.code_found;
                        instance->common.code_last_count_bit = instance->common.code_count_bit;
                        if(instance->common.callback)
                            instance->common.callback(
                                (SubGhzProtocolCommon*)instance, instance->common.context);
                    }
                    instance->common.code_found = 0;
                    instance->common.code_count_bit = 0;
                    instance->common.header_count = 0;
                }
                break;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                if(instance->common.code_count_bit <
                   instance->common.code_min_count_bit_for_found) {
                    subghz_protocol_common_add_bit(&instance->common, 1);
                }
                instance->common.parser_step = 2;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                if(instance->common.code_count_bit <
                   instance->common.code_min_count_bit_for_found) {
                    subghz_protocol_common_add_bit(&instance->common, 0);
                }
                instance->common.parser_step = 2;
            } else {
                instance->common.parser_step = 0;
                instance->common.header_count = 0;
            }
        } else {
            instance->common.parser_step = 0;
            instance->common.header_count = 0;
        }
        break;
    }
}

void subghz_protocol_keeloq_to_str(SubGhzProtocolKeeloq* instance, string_t output) {
    subghz_protocol_keeloq_check_remote_controller(instance);
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;
    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Fix:0x%08lX    Cnt:%04X\r\n"
        "Hop:0x%08lX    Btn:%02lX\r\n"
        "MF:%s\r\n"
        "Sn:0x%07lX \r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        instance->common.cnt,
        code_found_reverse_lo,
        instance->common.btn,
        instance->manufacture_name,
        instance->common.serial);
}

void subghz_protocol_keeloq_to_save_str(SubGhzProtocolKeeloq* instance, string_t output) {
    string_printf(
        output,
        "Protocol: %s\n"
        "Bit: %d\n"
        "Key: %08lX%08lX\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found >> 32),
        (uint32_t)(instance->common.code_last_found & 0xFFFFFFFF));
}

bool subghz_protocol_keeloq_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolKeeloq* instance) {
    bool loaded = false;
    string_t temp_str;
    string_init(temp_str);
    int res = 0;
    int data = 0;

    do {
        // Read and parse bit data from 2nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Bit: %d\n", &data);
        if(res != 1) {
            break;
        }
        instance->common.code_last_count_bit = (uint8_t)data;

        // Read and parse key data from 3nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Key: ") = 5
        string_right(temp_str, 5);

        uint8_t buf_key[8] = {0};
        if(!subghz_protocol_common_read_hex(temp_str, buf_key, 8)) {
            break;
        }

        for(uint8_t i = 0; i < 8; i++) {
            instance->common.code_last_found = instance->common.code_last_found << 8 | buf_key[i];
        }
        loaded = true;
    } while(0);
    string_clear(temp_str);

    return loaded;
}

void subghz_decoder_keeloq_to_load_protocol(SubGhzProtocolKeeloq* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_keeloq_check_remote_controller(instance);
}
