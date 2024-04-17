#include "mf_plus_i.h"

bool mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfPlusVersion);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfPlusVersion));
    }

    return can_parse;
}