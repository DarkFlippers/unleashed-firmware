#include "princeton_for_testing.h"

#include <furi_hal.h>
#include "math.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

#define SUBGHZ_PT_SHORT         300
#define SUBGHZ_PT_LONG          (SUBGHZ_PT_SHORT * 3)
#define SUBGHZ_PT_GUARD         (SUBGHZ_PT_SHORT * 30)
#define SUBGHZ_PT_COUNT_KEY_433 9
#define SUBGHZ_PT_TIMEOUT_433   900
#define SUBGHZ_PT_COUNT_KEY_868 9
#define SUBGHZ_PT_TIMEOUT_868   14000

#define TAG "SubGhzProtocolPrinceton"

struct SubGhzEncoderPrinceton {
    uint32_t key;
    uint16_t te;
    size_t repeat;
    size_t front;
    size_t count_key;
    size_t count_key_package;
    uint32_t time_high;
    uint32_t time_low;
    uint32_t timeout;
    uint32_t time_stop;
};

typedef enum {
    PrincetonDecoderStepReset = 0,
    PrincetonDecoderStepSaveDuration,
    PrincetonDecoderStepCheckDuration,
} PrincetonDecoderStep;

SubGhzEncoderPrinceton* subghz_encoder_princeton_for_testing_alloc(void) {
    SubGhzEncoderPrinceton* instance = malloc(sizeof(SubGhzEncoderPrinceton));
    return instance;
}

void subghz_encoder_princeton_for_testing_free(SubGhzEncoderPrinceton* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_encoder_princeton_for_testing_stop(
    SubGhzEncoderPrinceton* instance,
    uint32_t time_stop) {
    instance->time_stop = time_stop;
}

void subghz_encoder_princeton_for_testing_set(
    SubGhzEncoderPrinceton* instance,
    uint32_t key,
    size_t repeat,
    uint32_t frequency) {
    furi_assert(instance);
    instance->te = SUBGHZ_PT_SHORT;
    instance->key = key;
    instance->repeat = repeat + 1;
    instance->front = 48;
    instance->time_high = 0;
    instance->time_low = 0;
    if(frequency < 700000000) {
        instance->count_key_package = SUBGHZ_PT_COUNT_KEY_433;
        instance->timeout = SUBGHZ_PT_TIMEOUT_433;
    } else {
        instance->count_key_package = SUBGHZ_PT_COUNT_KEY_868;
        instance->timeout = SUBGHZ_PT_TIMEOUT_868;
    }

    instance->count_key = instance->count_key_package + 3;

    if((furi_get_tick() - instance->time_stop) < instance->timeout) {
        instance->time_stop = (instance->timeout - (furi_get_tick() - instance->time_stop)) * 1000;
    } else {
        instance->time_stop = 0;
    }
}

size_t subghz_encoder_princeton_for_testing_get_repeat_left(SubGhzEncoderPrinceton* instance) {
    furi_assert(instance);
    return instance->repeat;
}

void subghz_encoder_princeton_for_testing_print_log(void* context) {
    SubGhzEncoderPrinceton* instance = context;
    float duty_cycle =
        ((float)instance->time_high / (instance->time_high + instance->time_low)) * 100;
    FURI_LOG_I(
        TAG "Encoder",
        "Radio tx_time=%luus  ON=%luus, OFF=%luus, DutyCycle=%lu,%lu%%",
        instance->time_high + instance->time_low,
        instance->time_high,
        instance->time_low,
        (uint32_t)duty_cycle,
        (uint32_t)((duty_cycle - (uint32_t)duty_cycle) * 100UL));
}

LevelDuration subghz_encoder_princeton_for_testing_yield(void* context) {
    SubGhzEncoderPrinceton* instance = context;
    if(instance->repeat == 0) {
        subghz_encoder_princeton_for_testing_print_log(instance);
        return level_duration_reset();
    }

    size_t bit = instance->front / 2;
    bool level = !(instance->front % 2);

    LevelDuration ret;
    if(bit < 24) {
        uint8_t byte = bit / 8;
        uint8_t bit_in_byte = bit % 8;
        bool value = (((uint8_t*)&instance->key)[2 - byte] >> (7 - bit_in_byte)) & 1;
        if(value) {
            ret = level_duration_make(level, level ? instance->te * 3 : instance->te);
            if(level)
                instance->time_high += instance->te * 3;
            else
                instance->time_low += instance->te;
        } else {
            ret = level_duration_make(level, level ? instance->te : instance->te * 3);
            if(level)
                instance->time_high += instance->te;
            else
                instance->time_low += instance->te * 3;
        }
    } else {
        if(instance->time_stop) {
            ret = level_duration_make(level, level ? instance->te : instance->time_stop);
            if(level)
                instance->time_high += instance->te;
            else {
                instance->time_low += instance->time_stop;
                instance->time_stop = 0;
                instance->front = 47;
            }
        } else {
            if(--instance->count_key != 0) {
                ret = level_duration_make(level, level ? instance->te : instance->te * 30);
                if(level)
                    instance->time_high += instance->te;
                else
                    instance->time_low += instance->te * 30;
            } else {
                instance->count_key = instance->count_key_package + 2;
                instance->front = 48;
                ret = level_duration_make(level, level ? instance->te : instance->timeout * 1000);
                if(level)
                    instance->time_high += instance->te;
                else
                    instance->time_low += instance->timeout * 1000;
            }
        }
    }

    instance->front++;
    if(instance->front == 50) {
        instance->repeat--;
        instance->front = 0;
    }
    return ret;
}

struct SubGhzDecoderPrinceton {
    const char* name;
    uint16_t te_long;
    uint16_t te_short;
    uint16_t te_delta;
    uint8_t code_count_bit;
    uint8_t code_last_count_bit;
    uint64_t code_found;
    uint64_t code_last_found;
    uint8_t code_min_count_bit_for_found;
    uint8_t btn;
    uint32_t te_last;
    uint32_t serial;
    uint32_t parser_step;
    uint16_t cnt;
    uint32_t te;

    SubGhzDecoderPrincetonCallback callback;
    void* context;
};

SubGhzDecoderPrinceton* subghz_decoder_princeton_for_testing_alloc(void) {
    SubGhzDecoderPrinceton* instance = malloc(sizeof(SubGhzDecoderPrinceton));

    instance->te = SUBGHZ_PT_SHORT;
    instance->name = "Princeton";
    instance->code_min_count_bit_for_found = 24;
    instance->te_short = 400;
    instance->te_long = 1200;
    instance->te_delta = 250;
    return instance;
}

void subghz_decoder_princeton_for_testing_free(SubGhzDecoderPrinceton* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_decoder_princeton_for_testing_set_callback(
    SubGhzDecoderPrinceton* instance,
    SubGhzDecoderPrincetonCallback callback,
    void* context) {
    instance->callback = callback;
    instance->context = context;
}

void subghz_decoder_princeton_for_testing_reset(SubGhzDecoderPrinceton* instance) {
    instance->parser_step = PrincetonDecoderStepReset;
}

static void
    subghz_decoder_princeton_for_testing_add_bit(SubGhzDecoderPrinceton* instance, uint8_t bit) {
    instance->code_found = instance->code_found << 1 | bit;
    instance->code_count_bit++;
}

void subghz_decoder_princeton_for_testing_parse(
    SubGhzDecoderPrinceton* instance,
    bool level,
    uint32_t duration) {
    switch(instance->parser_step) {
    case PrincetonDecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, instance->te_short * 36) < instance->te_delta * 36)) {
            //Found Preambula
            instance->parser_step = PrincetonDecoderStepSaveDuration;
            instance->code_found = 0;
            instance->code_count_bit = 0;
            instance->te = 0;
        }
        break;
    case PrincetonDecoderStepSaveDuration:
        //save duration
        if(level) {
            instance->te_last = duration;
            instance->te += duration;
            instance->parser_step = PrincetonDecoderStepCheckDuration;
        }
        break;
    case PrincetonDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)instance->te_short * 10 + instance->te_delta)) {
                instance->parser_step = PrincetonDecoderStepSaveDuration;
                if(instance->code_count_bit == instance->code_min_count_bit_for_found) {
                    instance->te /= (instance->code_count_bit * 4 + 1);

                    instance->code_last_found = instance->code_found;
                    instance->code_last_count_bit = instance->code_count_bit;
                    instance->serial = instance->code_found >> 4;
                    instance->btn = (uint8_t)instance->code_found & 0x00000F;

                    if(instance->callback) instance->callback(instance, instance->context);
                }
                instance->code_found = 0;
                instance->code_count_bit = 0;
                instance->te = 0;
                break;
            }

            instance->te += duration;

            if((DURATION_DIFF(instance->te_last, instance->te_short) < instance->te_delta) &&
               (DURATION_DIFF(duration, instance->te_long) < instance->te_delta * 3)) {
                subghz_decoder_princeton_for_testing_add_bit(instance, 0);
                instance->parser_step = PrincetonDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->te_last, instance->te_long) < instance->te_delta * 3) &&
                (DURATION_DIFF(duration, instance->te_short) < instance->te_delta)) {
                subghz_decoder_princeton_for_testing_add_bit(instance, 1);
                instance->parser_step = PrincetonDecoderStepSaveDuration;
            } else {
                instance->parser_step = PrincetonDecoderStepReset;
            }
        } else {
            instance->parser_step = PrincetonDecoderStepReset;
        }
        break;
    }
}
