#include "infrared_signal.h"

#include <stdlib.h>
#include <string.h>
#include <core/check.h>
#include <infrared_worker.h>
#include <infrared_transmit.h>

#define TAG "InfraredSignal"

// Common keys
#define INFRARED_SIGNAL_NAME_KEY "name"
#define INFRARED_SIGNAL_TYPE_KEY "type"

// Type key values
#define INFRARED_SIGNAL_TYPE_RAW    "raw"
#define INFRARED_SIGNAL_TYPE_PARSED "parsed"

// Raw signal keys
#define INFRARED_SIGNAL_DATA_KEY       "data"
#define INFRARED_SIGNAL_FREQUENCY_KEY  "frequency"
#define INFRARED_SIGNAL_DUTY_CYCLE_KEY "duty_cycle"

// Parsed signal keys
#define INFRARED_SIGNAL_PROTOCOL_KEY "protocol"
#define INFRARED_SIGNAL_ADDRESS_KEY  "address"
#define INFRARED_SIGNAL_COMMAND_KEY  "command"

struct InfraredSignal {
    bool is_raw;
    union {
        InfraredMessage message;
        InfraredRawSignal raw;
    } payload;
};

static void infrared_signal_clear_timings(InfraredSignal* signal) {
    if(signal->is_raw) {
        free(signal->payload.raw.timings);
        signal->payload.raw.timings_size = 0;
        signal->payload.raw.timings = NULL;
    }
}

static bool infrared_signal_is_message_valid(const InfraredMessage* message) {
    if(!infrared_is_protocol_valid(message->protocol)) {
        FURI_LOG_E(TAG, "Unknown protocol");
        return false;
    }

    uint32_t address_length = infrared_get_protocol_address_length(message->protocol);
    uint32_t address_mask = (1UL << address_length) - 1;

    if(message->address != (message->address & address_mask)) {
        FURI_LOG_E(
            TAG,
            "Address is out of range (mask 0x%08lX): 0x%lX\r\n",
            address_mask,
            message->address);
        return false;
    }

    uint32_t command_length = infrared_get_protocol_command_length(message->protocol);
    uint32_t command_mask = (1UL << command_length) - 1;

    if(message->command != (message->command & command_mask)) {
        FURI_LOG_E(
            TAG,
            "Command is out of range (mask 0x%08lX): 0x%lX\r\n",
            command_mask,
            message->command);
        return false;
    }

    return true;
}

static bool infrared_signal_is_raw_valid(const InfraredRawSignal* raw) {
    if((raw->frequency > INFRARED_MAX_FREQUENCY) || (raw->frequency < INFRARED_MIN_FREQUENCY)) {
        FURI_LOG_E(
            TAG,
            "Frequency is out of range (%X - %X): %lX",
            INFRARED_MIN_FREQUENCY,
            INFRARED_MAX_FREQUENCY,
            raw->frequency);
        return false;

    } else if((raw->duty_cycle <= 0) || (raw->duty_cycle > 1)) {
        FURI_LOG_E(TAG, "Duty cycle is out of range (0 - 1): %f", (double)raw->duty_cycle);
        return false;

    } else if((raw->timings_size <= 0) || (raw->timings_size > MAX_TIMINGS_AMOUNT)) {
        FURI_LOG_E(
            TAG,
            "Timings amount is out of range (0 - %X): %zX",
            MAX_TIMINGS_AMOUNT,
            raw->timings_size);
        return false;
    }

    return true;
}

static inline bool
    infrared_signal_save_message(const InfraredMessage* message, FlipperFormat* ff) {
    const char* protocol_name = infrared_get_protocol_name(message->protocol);
    return flipper_format_write_string_cstr(
               ff, INFRARED_SIGNAL_TYPE_KEY, INFRARED_SIGNAL_TYPE_PARSED) &&
           flipper_format_write_string_cstr(ff, INFRARED_SIGNAL_PROTOCOL_KEY, protocol_name) &&
           flipper_format_write_hex(
               ff, INFRARED_SIGNAL_ADDRESS_KEY, (uint8_t*)&message->address, 4) &&
           flipper_format_write_hex(
               ff, INFRARED_SIGNAL_COMMAND_KEY, (uint8_t*)&message->command, 4);
}

static inline bool infrared_signal_save_raw(const InfraredRawSignal* raw, FlipperFormat* ff) {
    furi_assert(raw->timings_size <= MAX_TIMINGS_AMOUNT);
    return flipper_format_write_string_cstr(
               ff, INFRARED_SIGNAL_TYPE_KEY, INFRARED_SIGNAL_TYPE_RAW) &&
           flipper_format_write_uint32(ff, INFRARED_SIGNAL_FREQUENCY_KEY, &raw->frequency, 1) &&
           flipper_format_write_float(ff, INFRARED_SIGNAL_DUTY_CYCLE_KEY, &raw->duty_cycle, 1) &&
           flipper_format_write_uint32(
               ff, INFRARED_SIGNAL_DATA_KEY, raw->timings, raw->timings_size);
}

static inline bool infrared_signal_read_message(InfraredSignal* signal, FlipperFormat* ff) {
    FuriString* buf;
    buf = furi_string_alloc();
    bool success = false;

    do {
        if(!flipper_format_read_string(ff, INFRARED_SIGNAL_PROTOCOL_KEY, buf)) break;

        InfraredMessage message;
        message.protocol = infrared_get_protocol_by_name(furi_string_get_cstr(buf));

        if(!flipper_format_read_hex(ff, INFRARED_SIGNAL_ADDRESS_KEY, (uint8_t*)&message.address, 4))
            break;
        if(!flipper_format_read_hex(ff, INFRARED_SIGNAL_COMMAND_KEY, (uint8_t*)&message.command, 4))
            break;
        if(!infrared_signal_is_message_valid(&message)) break;

        infrared_signal_set_message(signal, &message);
        success = true;
    } while(false);

    furi_string_free(buf);
    return success;
}

static inline bool infrared_signal_read_raw(InfraredSignal* signal, FlipperFormat* ff) {
    bool success = false;

    do {
        uint32_t frequency;
        if(!flipper_format_read_uint32(ff, INFRARED_SIGNAL_FREQUENCY_KEY, &frequency, 1)) break;

        float duty_cycle;
        if(!flipper_format_read_float(ff, INFRARED_SIGNAL_DUTY_CYCLE_KEY, &duty_cycle, 1)) break;

        uint32_t timings_size;
        if(!flipper_format_get_value_count(ff, INFRARED_SIGNAL_DATA_KEY, &timings_size)) break;

        if(timings_size > MAX_TIMINGS_AMOUNT) break;

        uint32_t* timings = malloc(sizeof(uint32_t) * timings_size);
        if(!flipper_format_read_uint32(ff, INFRARED_SIGNAL_DATA_KEY, timings, timings_size)) {
            free(timings);
            break;
        }
        infrared_signal_set_raw_signal(signal, timings, timings_size, frequency, duty_cycle);
        free(timings);

        success = true;
    } while(false);

    return success;
}

bool infrared_signal_read_body(InfraredSignal* signal, FlipperFormat* ff) {
    FuriString* tmp = furi_string_alloc();

    bool success = false;

    do {
        if(!flipper_format_read_string(ff, INFRARED_SIGNAL_TYPE_KEY, tmp)) break;

        if(furi_string_equal(tmp, INFRARED_SIGNAL_TYPE_RAW)) {
            if(!infrared_signal_read_raw(signal, ff)) break;
        } else if(furi_string_equal(tmp, INFRARED_SIGNAL_TYPE_PARSED)) {
            if(!infrared_signal_read_message(signal, ff)) break;
        } else {
            FURI_LOG_E(TAG, "Unknown signal type: %s", furi_string_get_cstr(tmp));
            break;
        }

        success = true;
    } while(false);

    furi_string_free(tmp);
    return success;
}

InfraredSignal* infrared_signal_alloc(void) {
    InfraredSignal* signal = malloc(sizeof(InfraredSignal));

    signal->is_raw = false;
    signal->payload.message.protocol = InfraredProtocolUnknown;

    return signal;
}

void infrared_signal_free(InfraredSignal* signal) {
    infrared_signal_clear_timings(signal);
    free(signal);
}

bool infrared_signal_is_raw(const InfraredSignal* signal) {
    return signal->is_raw;
}

bool infrared_signal_is_valid(const InfraredSignal* signal) {
    return signal->is_raw ? infrared_signal_is_raw_valid(&signal->payload.raw) :
                            infrared_signal_is_message_valid(&signal->payload.message);
}

void infrared_signal_set_signal(InfraredSignal* signal, const InfraredSignal* other) {
    if(other->is_raw) {
        const InfraredRawSignal* raw = &other->payload.raw;
        infrared_signal_set_raw_signal(
            signal, raw->timings, raw->timings_size, raw->frequency, raw->duty_cycle);
    } else {
        const InfraredMessage* message = &other->payload.message;
        infrared_signal_set_message(signal, message);
    }
}

void infrared_signal_set_raw_signal(
    InfraredSignal* signal,
    const uint32_t* timings,
    size_t timings_size,
    uint32_t frequency,
    float duty_cycle) {
    infrared_signal_clear_timings(signal);

    // If the frequency is out of bounds, set it to the closest bound same for duty cycle
    // TODO: Should we return error instead? Also infrared_signal_is_valid is used only in CLI for some reason?!
    if(frequency > INFRARED_MAX_FREQUENCY) {
        frequency = INFRARED_MAX_FREQUENCY;
    } else if(frequency < INFRARED_MIN_FREQUENCY) {
        frequency = INFRARED_MIN_FREQUENCY;
    }
    if((duty_cycle <= (float)0) || (duty_cycle > (float)1)) {
        duty_cycle = (float)0.33;
    }
    // In case of timings out of bounds we just call return
    if((timings_size <= 0) || (timings_size > MAX_TIMINGS_AMOUNT)) {
        return;
    }

    signal->is_raw = true;

    signal->payload.raw.timings_size = timings_size;
    signal->payload.raw.frequency = frequency;
    signal->payload.raw.duty_cycle = duty_cycle;

    signal->payload.raw.timings = malloc(timings_size * sizeof(uint32_t));
    memcpy(signal->payload.raw.timings, timings, timings_size * sizeof(uint32_t));
}

const InfraredRawSignal* infrared_signal_get_raw_signal(const InfraredSignal* signal) {
    furi_assert(signal->is_raw);
    return &signal->payload.raw;
}

void infrared_signal_set_message(InfraredSignal* signal, const InfraredMessage* message) {
    infrared_signal_clear_timings(signal);

    signal->is_raw = false;
    signal->payload.message = *message;
}

const InfraredMessage* infrared_signal_get_message(const InfraredSignal* signal) {
    furi_assert(!signal->is_raw);
    return &signal->payload.message;
}

bool infrared_signal_save(const InfraredSignal* signal, FlipperFormat* ff, const char* name) {
    if(!flipper_format_write_comment_cstr(ff, "") ||
       !flipper_format_write_string_cstr(ff, INFRARED_SIGNAL_NAME_KEY, name)) {
        return false;
    } else if(signal->is_raw) {
        return infrared_signal_save_raw(&signal->payload.raw, ff);
    } else {
        return infrared_signal_save_message(&signal->payload.message, ff);
    }
}

bool infrared_signal_read(InfraredSignal* signal, FlipperFormat* ff, FuriString* name) {
    bool success = false;

    do {
        if(!infrared_signal_read_name(ff, name)) break;
        if(!infrared_signal_read_body(signal, ff)) break;

        success = true; //-V779
    } while(false);

    return success;
}

bool infrared_signal_read_name(FlipperFormat* ff, FuriString* name) {
    return flipper_format_read_string(ff, INFRARED_SIGNAL_NAME_KEY, name);
}

bool infrared_signal_search_by_name_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    const char* name) {
    bool success = false;
    FuriString* tmp = furi_string_alloc();

    while(infrared_signal_read_name(ff, tmp)) {
        if(furi_string_equal(tmp, name)) {
            success = infrared_signal_read_body(signal, ff);
            break;
        }
    }

    furi_string_free(tmp);
    return success;
}

bool infrared_signal_search_by_index_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    size_t index) {
    bool success = false;
    FuriString* tmp = furi_string_alloc();

    for(uint32_t i = 0; infrared_signal_read_name(ff, tmp); ++i) {
        if(i == index) {
            success = infrared_signal_read_body(signal, ff);
            break;
        }
    }

    furi_string_free(tmp);
    return success;
}

void infrared_signal_transmit(const InfraredSignal* signal) {
    if(signal->is_raw) {
        const InfraredRawSignal* raw_signal = &signal->payload.raw;
        infrared_send_raw_ext(
            raw_signal->timings,
            raw_signal->timings_size,
            true,
            raw_signal->frequency,
            raw_signal->duty_cycle);
    } else {
        const InfraredMessage* message = &signal->payload.message;
        infrared_send(message, 1);
    }
}
