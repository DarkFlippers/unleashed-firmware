#include "mf_plus_poller_i.h"

#include <furi.h>

#include "mf_plus_i.h"

#define TAG "MfPlusPoller"

MfPlusError mf_plus_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return MfPlusErrorNone;
    case Iso14443_4aErrorNotPresent:
        return MfPlusErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return MfPlusErrorTimeout;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError mf_plus_process_status_code(uint8_t status_code) {
    switch(status_code) {
    case NXP_NATIVE_COMMAND_STATUS_OPERATION_OK:
        return MfPlusErrorNone;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError mf_plus_poller_send_chunks(
    MfPlusPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);

    NxpNativeCommandStatus status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError iso14443_4a_error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status_code,
        tx_buffer,
        rx_buffer,
        NxpNativeCommandModePlain,
        instance->tx_buffer,
        instance->rx_buffer);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(iso14443_4a_error);
    }

    return mf_plus_process_status_code(status_code);
}

// WritePerso (0xA8) to the intentionally-invalid block 0x9090: the block is always rejected (no
// write happens), only the status byte varies by card state -- the discriminator PM3 uses in
// `hf mfp info`. Classified in mf_plus_poller_probe_security_level below.
#define MF_PLUS_CMD_WRITE_PERSO       (0xA8)
#define MF_PLUS_WRITE_PERSO_PROBE_LEN (1 + 2 + 16) // cmd + invalid block addr + 16 data bytes

MfPlusError
    mf_plus_poller_probe_security_level(MfPlusPoller* instance, MfPlusProbeResult* result) {
    furi_check(instance);
    furi_check(result);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_WRITE_PERSO);
    bit_buffer_append_byte(instance->input_buffer, 0x90); // block 0x9090 (little-endian), invalid
    bit_buffer_append_byte(instance->input_buffer, 0x90);
    for(size_t i = 0; i < MF_PLUS_WRITE_PERSO_PROBE_LEN - 3; i++) {
        bit_buffer_append_byte(instance->input_buffer, 0x00);
    }

    Iso14443_4aError error = iso14443_4a_poller_send_block(
        instance->iso14443_4a_poller, instance->input_buffer, instance->result_buffer);
    if(error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(error);
    }

    if(bit_buffer_get_size_bytes(instance->result_buffer) == 0) {
        return MfPlusErrorProtocol;
    }

    // MIFARE Plus answers WritePerso with a single status byte: 0x09 = invalid block rejected (still
    // in SL0 personalization), 0x06/0x0B = the documented SL3 rejections. Treat any other reply
    // (DESFire status words, unsupported-INS, ...) as not-a-Plus so the caller keeps its
    // DESFire-safe fallback -- no real DESFire status code is 0x06/0x09/0x0B. (PM3 hf mfp info.)
    const uint8_t status = bit_buffer_get_byte(instance->result_buffer, 0);
    FURI_LOG_D(TAG, "SL probe (SAK 20) WritePerso status 0x%02X", status);
    switch(status) {
    case 0x09:
        *result = MfPlusProbeResultSl0;
        break;
    case 0x06:
    case 0x0B:
        *result = MfPlusProbeResultSl3;
        break;
    default:
        *result = MfPlusProbeResultNotPlus;
        break;
    }
    return MfPlusErrorNone;
}

MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_GET_VERSION);

    MfPlusError error =
        mf_plus_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);
    if(error == MfPlusErrorNone) {
        error = mf_plus_version_parse(data, instance->result_buffer);
    }

    return error;
}
