#pragma once

#include <m-string.h>
#include <furi-hal.h>
#include <stdint.h>
#include "file-worker.h"

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit) ((value) |= (1UL << (bit)))
#define bit_clear(value, bit) ((value) &= ~(1UL << (bit)))
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))

#define SUBGHZ_TX_PIN_HIGTH()
#define SUBGHZ_TX_PIN_LOW()
#define DURATION_DIFF(x, y) ((x < y) ? (y - x) : (x - y))

//#define SUBGHZ_APP_PATH_FOLDER "/ext/subghz/saved"
#define SUBGHZ_APP_FOLDER "/any/subghz"
#define SUBGHZ_APP_PATH_FOLDER "/any/subghz/saved"
#define SUBGHZ_APP_EXTENSION ".sub"

typedef struct SubGhzProtocolCommon SubGhzProtocolCommon;

typedef void (*SubGhzProtocolCommonCallback)(SubGhzProtocolCommon* parser, void* context);

typedef void (*SubGhzProtocolCommonToStr)(SubGhzProtocolCommon* instance, string_t output);

//Save
typedef void (*SubGhzProtocolCommonGetStrSave)(SubGhzProtocolCommon* instance, string_t output);

//Load
typedef bool (*SubGhzProtocolCommonLoad)(FileWorker* file_worker, SubGhzProtocolCommon* instance);

struct SubGhzProtocolCommon {
    const char* name;
    uint16_t te_long;
    uint16_t te_shot;
    uint16_t te_delta;
    uint8_t code_count_bit;
    uint8_t code_last_count_bit;
    uint64_t code_found;
    uint64_t code_last_found;
    uint8_t code_min_count_bit_for_found;
    uint8_t parser_step;
    uint32_t te_last;
    uint8_t header_count;
    uint16_t cnt;
    uint32_t serial;
    uint8_t btn;

    /* Standard Callback for on rx complete event */
    SubGhzProtocolCommonCallback callback;
    void* context;

    /* Dump To String */
    SubGhzProtocolCommonToStr to_string;
    /* Get string to save */
    SubGhzProtocolCommonGetStrSave to_save_string;
    /*Load protocol by file*/
    SubGhzProtocolCommonLoad to_load_protocol;
};

/** Add data bit to code_found
 * 
 * @param common - SubGhzProtocolCommon common
 * @param bit - add bit
 */
void subghz_protocol_common_add_bit(SubGhzProtocolCommon* common, uint8_t bit);

/** Checking that the duration is included in the interval
 * 
 * @param common - SubGhzProtocolCommon common
 * @param duration duration reference
 * @param duration_check duration checked
 * @return true on success
 */
bool subghz_protocol_common_check_interval(
    SubGhzProtocolCommon* common,
    uint32_t duration,
    uint16_t duration_check);

/** Bit-by-bit data mirroring
 * 
 * @param key - data to mirror
 * @param count_bit number of data bits
 * @return mirrored data
 */
uint64_t subghz_protocol_common_reverse_key(uint64_t key, uint8_t count_bit);

/** Callback protocol
 * 
 * @param instance - SubGhzProtocolCommon* instance
 * @param callback
 * @param context
 */
void subghz_protocol_common_set_callback(
    SubGhzProtocolCommon* instance,
    SubGhzProtocolCommonCallback callback,
    void* context);

/** outputting information from the parser
 * 
 * @param instance - SubGhzProtocolCommon* instance
 * @param output   - output string
 */
void subghz_protocol_common_to_str(SubGhzProtocolCommon* instance, string_t output);
