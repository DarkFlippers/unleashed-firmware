#include "infrared_signal.h"

#include <stdlib.h>
#include <string.h>
#include <core/check.h>
#include <infrared_worker.h>
#include <infrared_transmit.h>

#define TAG "InfraredSignal"

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

static bool infrared_signal_is_message_valid(InfraredMessage* message) {
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

static bool infrared_signal_is_raw_valid(InfraredRawSignal* raw) {
    if((raw->frequency > INFRARED_MAX_FREQUENCY) || (raw->frequency < INFRARED_MIN_FREQUENCY)) {
        FURI_LOG_E(
            TAG,
            "Frequency is out of range (%lX - %lX): %lX",
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
            "Timings amount is out of range (0 - %lX): %lX",
            MAX_TIMINGS_AMOUNT,
            raw->timings_size);
        return false;
    }

    return true;
}

static inline bool infrared_signal_save_message(InfraredMessage* message, FlipperFormat* ff) {
    const char* protocol_name = infrared_get_protocol_name(message->protocol);
    return flipper_format_write_string_cstr(ff, "type", "parsed") &&
           flipper_format_write_string_cstr(ff, "protocol", protocol_name) &&
           flipper_format_write_hex(ff, "address", (uint8_t*)&message->address, 4) &&
           flipper_format_write_hex(ff, "command", (uint8_t*)&message->command, 4);
}

static inline bool infrared_signal_save_raw(InfraredRawSignal* raw, FlipperFormat* ff) {
    furi_assert(raw->timings_size <= MAX_TIMINGS_AMOUNT);
    return flipper_format_write_string_cstr(ff, "type", "raw") &&
           flipper_format_write_uint32(ff, "frequency", &raw->frequency, 1) &&
           flipper_format_write_float(ff, "duty_cycle", &raw->duty_cycle, 1) &&
           flipper_format_write_uint32(ff, "data", raw->timings, raw->timings_size);
}

static inline bool infrared_signal_read_message(InfraredSignal* signal, FlipperFormat* ff) {
    string_t buf;
    string_init(buf);
    bool success = false;

    do {
        if(!flipper_format_read_string(ff, "protocol", buf)) break;

        InfraredMessage message;
        message.protocol = infrared_get_protocol_by_name(string_get_cstr(buf));

        success = flipper_format_read_hex(ff, "address", (uint8_t*)&message.address, 4) &&
                  flipper_format_read_hex(ff, "command", (uint8_t*)&message.command, 4) &&
                  infrared_signal_is_message_valid(&message);

        if(!success) break;

        infrared_signal_set_message(signal, &message);
    } while(0);

    string_clear(buf);
    return success;
}

static inline bool infrared_signal_read_raw(InfraredSignal* signal, FlipperFormat* ff) {
    uint32_t timings_size, frequency;
    float duty_cycle;

    bool success = flipper_format_read_uint32(ff, "frequency", &frequency, 1) &&
                   flipper_format_read_float(ff, "duty_cycle", &duty_cycle, 1) &&
                   flipper_format_get_value_count(ff, "data", &timings_size);

    if(!success || timings_size > MAX_TIMINGS_AMOUNT) {
        return false;
    }

    uint32_t* timings = malloc(sizeof(uint32_t) * timings_size);
    success = flipper_format_read_uint32(ff, "data", timings, timings_size);

    if(success) {
        infrared_signal_set_raw_signal(signal, timings, timings_size, frequency, duty_cycle);
    }

    free(timings);
    return success;
}

static bool infrared_signal_read_body(InfraredSignal* signal, FlipperFormat* ff) {
    string_t tmp;
    string_init(tmp);
    bool success = false;

    do {
        if(!flipper_format_read_string(ff, "type", tmp)) break;
        if(string_equal_p(tmp, "raw")) {
            success = infrared_signal_read_raw(signal, ff);
        } else if(string_equal_p(tmp, "parsed")) {
            success = infrared_signal_read_message(signal, ff);
        } else {
            FURI_LOG_E(TAG, "Unknown signal type");
        }
    } while(false);

    string_clear(tmp);
    return success;
}

InfraredSignal* infrared_signal_alloc() {
    InfraredSignal* signal = malloc(sizeof(InfraredSignal));

    signal->is_raw = false;
    signal->payload.message.protocol = InfraredProtocolUnknown;

    return signal;
}

void infrared_signal_free(InfraredSignal* signal) {
    infrared_signal_clear_timings(signal);
    free(signal);
}

bool infrared_signal_is_raw(InfraredSignal* signal) {
    return signal->is_raw;
}

bool infrared_signal_is_valid(InfraredSignal* signal) {
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

    signal->is_raw = true;

    signal->payload.raw.timings_size = timings_size;
    signal->payload.raw.frequency = frequency;
    signal->payload.raw.duty_cycle = duty_cycle;

    signal->payload.raw.timings = malloc(timings_size * sizeof(uint32_t));
    memcpy(signal->payload.raw.timings, timings, timings_size * sizeof(uint32_t));
}

InfraredRawSignal* infrared_signal_get_raw_signal(InfraredSignal* signal) {
    furi_assert(signal->is_raw);
    return &signal->payload.raw;
}

void infrared_signal_set_message(InfraredSignal* signal, const InfraredMessage* message) {
    infrared_signal_clear_timings(signal);

    signal->is_raw = false;
    signal->payload.message = *message;
}

InfraredMessage* infrared_signal_get_message(InfraredSignal* signal) {
    furi_assert(!signal->is_raw);
    return &signal->payload.message;
}

bool infrared_signal_save(InfraredSignal* signal, FlipperFormat* ff, const char* name) {
    if(!flipper_format_write_comment_cstr(ff, "") ||
       !flipper_format_write_string_cstr(ff, "name", name)) {
        return false;
    } else if(signal->is_raw) {
        return infrared_signal_save_raw(&signal->payload.raw, ff);
    } else {
        return infrared_signal_save_message(&signal->payload.message, ff);
    }
}

bool infrared_signal_read(InfraredSignal* signal, FlipperFormat* ff, string_t name) {
    string_t tmp;
    string_init(tmp);
    bool success = false;

    do {
        if(!flipper_format_read_string(ff, "name", tmp)) break;
        string_set(name, tmp);
        if(!infrared_signal_read_body(signal, ff)) break;
        success = true;
    } while(0);

    string_clear(tmp);
    return success;
}

bool infrared_signal_search_and_read(
    InfraredSignal* signal,
    FlipperFormat* ff,
    const string_t name) {
    bool success = false;
    string_t tmp;
    string_init(tmp);

    do {
        bool is_name_found = false;
        while(flipper_format_read_string(ff, "name", tmp)) {
            is_name_found = string_equal_p(name, tmp);
            if(is_name_found) break;
        }
        if(!is_name_found) break;
        if(!infrared_signal_read_body(signal, ff)) break;
        success = true;
    } while(false);

    string_clear(tmp);
    return success;
}

void infrared_signal_transmit(InfraredSignal* signal) {
    if(signal->is_raw) {
        InfraredRawSignal* raw_signal = &signal->payload.raw;
        infrared_send_raw_ext(
            raw_signal->timings,
            raw_signal->timings_size,
            true,
            raw_signal->frequency,
            raw_signal->duty_cycle);
    } else {
        InfraredMessage* message = &signal->payload.message;
        infrared_send(message, 1);
    }
}
