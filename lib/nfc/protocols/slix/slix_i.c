#include "slix_i.h"

#include <nfc/protocols/iso15693_3/iso15693_3_i.h>

bool slix_error_response_parse(SlixError* error, const BitBuffer* buf) {
    Iso15693_3Error iso15693_3_error;
    const bool ret = iso15693_3_error_response_parse(&iso15693_3_error, buf);

    if(ret) {
        *error = slix_process_iso15693_3_error(iso15693_3_error);
    }

    return ret;
}

SlixError slix_process_iso15693_3_error(Iso15693_3Error iso15693_3_error) {
    switch(iso15693_3_error) {
    case Iso15693_3ErrorNone:
        return SlixErrorNone;
    case Iso15693_3ErrorTimeout:
        return SlixErrorTimeout;
    case Iso15693_3ErrorFormat:
        return SlixErrorFormat;
    case Iso15693_3ErrorInternal:
        return SlixErrorInternal;
    default:
        return SlixErrorUnknown;
    }
}

SlixError slix_get_nxp_system_info_response_parse(SlixData* data, const BitBuffer* buf) {
    furi_assert(data);
    SlixError error = SlixErrorNone;

    do {
        if(slix_error_response_parse(&error, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t pp_pointer;
            uint8_t pp_condition;
            uint8_t lock_bits;
            uint32_t feature_flags;
        } SlixGetNxpSystemInfoResponseLayout;

        const size_t size_received = bit_buffer_get_size_bytes(buf);
        const size_t size_required = sizeof(SlixGetNxpSystemInfoResponseLayout);

        if(size_received != size_required) {
            error = SlixErrorFormat;
            break;
        }

        const SlixGetNxpSystemInfoResponseLayout* response =
            (const SlixGetNxpSystemInfoResponseLayout*)bit_buffer_get_data(buf);

        SlixProtection* protection = &data->system_info.protection;
        protection->pointer = response->pp_pointer;
        protection->condition = response->pp_condition;

        Iso15693_3LockBits* iso15693_3_lock_bits = &data->iso15693_3_data->settings.lock_bits;
        iso15693_3_lock_bits->dsfid = response->lock_bits & SLIX_LOCK_BITS_DSFID;
        iso15693_3_lock_bits->afi = response->lock_bits & SLIX_LOCK_BITS_AFI;

        SlixLockBits* slix_lock_bits = &data->system_info.lock_bits;
        slix_lock_bits->eas = response->lock_bits & SLIX_LOCK_BITS_EAS;
        slix_lock_bits->ppl = response->lock_bits & SLIX_LOCK_BITS_PPL;

    } while(false);

    return error;
}

SlixError slix_read_signature_response_parse(SlixSignature data, const BitBuffer* buf) {
    SlixError error = SlixErrorNone;

    do {
        if(slix_error_response_parse(&error, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t signature[SLIX_SIGNATURE_SIZE];
        } SlixReadSignatureResponseLayout;

        const size_t size_received = bit_buffer_get_size_bytes(buf);
        const size_t size_required = sizeof(SlixReadSignatureResponseLayout);

        if(size_received != size_required) {
            error = SlixErrorFormat;
            break;
        }

        const SlixReadSignatureResponseLayout* response =
            (const SlixReadSignatureResponseLayout*)bit_buffer_get_data(buf);

        memcpy(data, response->signature, sizeof(SlixSignature));
    } while(false);

    return error;
}

SlixError slix_get_random_number_response_parse(SlixRandomNumber* data, const BitBuffer* buf) {
    SlixError error = SlixErrorNone;

    do {
        if(slix_error_response_parse(&error, buf)) break;

        typedef struct {
            uint8_t flags;
            uint8_t random_number[2];
        } SlixGetRandomNumberResponseLayout;

        const size_t size_received = bit_buffer_get_size_bytes(buf);
        const size_t size_required = sizeof(SlixGetRandomNumberResponseLayout);

        if(size_received != size_required) {
            error = SlixErrorFormat;
            break;
        }

        const SlixGetRandomNumberResponseLayout* response =
            (const SlixGetRandomNumberResponseLayout*)bit_buffer_get_data(buf);
        *data = (response->random_number[1] << 8) | response->random_number[0];
    } while(false);

    return error;
}

void slix_set_password(SlixData* data, SlixPasswordType password_type, SlixPassword password) {
    furi_assert(data);
    furi_assert(password_type < SlixPasswordTypeCount);

    data->passwords[password_type] = password;
}

void slix_set_privacy_mode(SlixData* data, bool set) {
    furi_assert(data);

    data->privacy = set;
}

void slix_increment_counter(SlixData* data) {
    furi_assert(data);

    const uint8_t* block_data =
        iso15693_3_get_block_data(data->iso15693_3_data, SLIX_COUNTER_BLOCK_NUM);

    SlixCounter counter;
    memcpy(counter.bytes, block_data, SLIX_BLOCK_SIZE);
    counter.value += 1;

    iso15693_3_set_block_data(
        data->iso15693_3_data, SLIX_COUNTER_BLOCK_NUM, counter.bytes, sizeof(SlixCounter));
}
