#include "ntag4xx_i.h"

#define TAG "Ntag4xx"

#define NTAG4XX_FFF_VERSION_KEY \
    NTAG4XX_FFF_PICC_PREFIX " " \
                            "Version"

Ntag4xxError ntag4xx_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return Ntag4xxErrorNone;
    case Iso14443_4aErrorNotPresent:
        return Ntag4xxErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return Ntag4xxErrorTimeout;
    default:
        return Ntag4xxErrorProtocol;
    }
}

Ntag4xxError ntag4xx_process_status_code(uint8_t status_code) {
    switch(status_code) {
    case NXP_NATIVE_COMMAND_STATUS_OPERATION_OK:
        return Ntag4xxErrorNone;
    default:
        return Ntag4xxErrorProtocol;
    }
}

bool ntag4xx_version_parse(Ntag4xxVersion* data, const BitBuffer* buf) {
    const size_t buf_size = bit_buffer_get_size_bytes(buf);
    const bool can_parse = buf_size == sizeof(Ntag4xxVersion) ||
                           buf_size == sizeof(Ntag4xxVersion) - sizeof(data->optional);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(Ntag4xxVersion));
        if(buf_size < sizeof(Ntag4xxVersion)) {
            memset(&data->optional, 0, sizeof(data->optional));
        }
    }

    return can_parse && (data->hw_type & 0x0F) == 0x04;
}

bool ntag4xx_version_load(Ntag4xxVersion* data, FlipperFormat* ff) {
    return flipper_format_read_hex(
        ff, NTAG4XX_FFF_VERSION_KEY, (uint8_t*)data, sizeof(Ntag4xxVersion));
}

bool ntag4xx_version_save(const Ntag4xxVersion* data, FlipperFormat* ff) {
    return flipper_format_write_hex(
        ff, NTAG4XX_FFF_VERSION_KEY, (const uint8_t*)data, sizeof(Ntag4xxVersion));
}
