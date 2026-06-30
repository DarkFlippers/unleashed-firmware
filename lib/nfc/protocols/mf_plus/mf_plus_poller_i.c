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

// WritePerso (0xA8) targeting block 0x9090, which is intentionally invalid: a card still in SL0
// rejects the block with 0x09 instead of writing anything (so the probe never mutates the card),
// while an SL3 card rejects the command itself and DESFire/other answer with their own status.
// Same discriminator PM3 uses in `hf mfp info` (client/src/cmdhfmfp.c).
#define MF_PLUS_CMD_WRITE_PERSO       (0xA8)
#define MF_PLUS_WRITE_PERSO_PROBE_LEN (1 + 2 + 16) // cmd + invalid block addr + 16 data bytes

MfPlusError
    mf_plus_poller_probe_security_level(MfPlusPoller* instance, MfPlusProbeResult* result) {
    furi_check(instance);
    furi_check(result);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_WRITE_PERSO);
    bit_buffer_append_byte(instance->input_buffer, 0x90); // block addr LSB \_ 0x9090, invalid
    bit_buffer_append_byte(instance->input_buffer, 0x90); // block addr MSB /
    for(size_t i = 0; i < MF_PLUS_WRITE_PERSO_PROBE_LEN - 3; i++) {
        bit_buffer_append_byte(instance->input_buffer, 0x00);
    }

    Iso14443_4aError error = iso14443_4a_poller_send_block(
        instance->iso14443_4a_poller, instance->input_buffer, instance->result_buffer);
    if(error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(error);
    }

    const size_t len = bit_buffer_get_size_bytes(instance->result_buffer);
    if(len == 0) {
        return MfPlusErrorProtocol;
    }

    const uint8_t b0 = bit_buffer_get_byte(instance->result_buffer, 0);
    if(len >= 2) {
        const uint8_t b1 = bit_buffer_get_byte(instance->result_buffer, 1);
        // DESFire answers 67 00 (wrong length) or 1C 83 0C; 6D 00 is "INS not supported".
        const bool desfire_67 = (b0 == 0x67 && b1 == 0x00);
        const bool desfire_1c =
            (len >= 3 && b0 == 0x1C && b1 == 0x83 &&
             bit_buffer_get_byte(instance->result_buffer, 2) == 0x0C);
        const bool ins_unsupported = (b0 == 0x6D && b1 == 0x00);
        if(desfire_67 || desfire_1c || ins_unsupported) {
            *result = MfPlusProbeResultNotPlus;
            return MfPlusErrorNone;
        }
    }

    // 0x09 (block invalid) is returned only by a card still in SL0; anything else is a Plus in SL3.
    *result = (len > 1 && b0 == 0x09) ? MfPlusProbeResultSl0 : MfPlusProbeResultSl3;
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
