#include "mf_ultralight_poller_i.h"

#include <furi.h>

#define TAG "MfUltralightPoller"

MfUltralightError mf_ultralight_process_error(Iso14443_3aError error) {
    MfUltralightError ret = MfUltralightErrorNone;

    switch(error) {
    case Iso14443_3aErrorNone:
        ret = MfUltralightErrorNone;
        break;
    case Iso14443_3aErrorNotPresent:
        ret = MfUltralightErrorNotPresent;
        break;
    case Iso14443_3aErrorColResFailed:
    case Iso14443_3aErrorCommunication:
    case Iso14443_3aErrorWrongCrc:
        ret = MfUltralightErrorProtocol;
        break;
    case Iso14443_3aErrorTimeout:
        ret = MfUltralightErrorTimeout;
        break;
    default:
        ret = MfUltralightErrorProtocol;
        break;
    }

    return ret;
}

MfUltralightError mf_ultralight_poller_auth_pwd(
    MfUltralightPoller* instance,
    MfUltralightPollerAuthContext* data) {
    furi_check(instance);
    furi_check(data);

    uint8_t auth_cmd[5] = {MF_ULTRALIGHT_CMD_PWD_AUTH}; //-V1009
    memccpy(&auth_cmd[1], data->password.data, 0, MF_ULTRALIGHT_AUTH_PASSWORD_SIZE);
    bit_buffer_copy_bytes(instance->tx_buffer, auth_cmd, sizeof(auth_cmd));

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;
    do {
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != MF_ULTRALIGHT_AUTH_PACK_SIZE) {
            ret = MfUltralightErrorAuth;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data->pack.data, MF_ULTRALIGHT_AUTH_PACK_SIZE);
    } while(false);

    return ret;
}

static MfUltralightError mf_ultralight_poller_send_authenticate_cmd(
    MfUltralightPoller* instance,
    const uint8_t* cmd,
    const uint8_t length,
    const bool initial_cmd,
    uint8_t* response) {
    furi_check(instance);
    furi_check(cmd);
    furi_check(response);

    bit_buffer_copy_bytes(instance->tx_buffer, cmd, length);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;
    do {
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }

        const uint8_t expected_response_code = initial_cmd ? 0xAF : 0x00;
        if((bit_buffer_get_byte(instance->rx_buffer, 0) != expected_response_code) ||
           (bit_buffer_get_size_bytes(instance->rx_buffer) !=
            MF_ULTRALIGHT_C_AUTH_RESPONSE_SIZE)) {
            ret = MfUltralightErrorAuth;
            break;
        }

        memcpy(
            response,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_authentication_test(MfUltralightPoller* instance) {
    furi_check(instance);

    uint8_t auth_cmd[2] = {MF_ULTRALIGHT_CMD_AUTH, 0x00};
    uint8_t dummy[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE];
    return mf_ultralight_poller_send_authenticate_cmd(
        instance, auth_cmd, sizeof(auth_cmd), true, dummy);
}

MfUltralightError mf_ultralight_poller_authenticate_start(
    MfUltralightPoller* instance,
    const uint8_t* RndA,
    uint8_t* output) {
    furi_check(instance);
    furi_check(RndA);
    furi_check(output);

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        uint8_t encRndB[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE] = {0};
        uint8_t auth_cmd[2] = {MF_ULTRALIGHT_CMD_AUTH, 0x00};
        ret = mf_ultralight_poller_send_authenticate_cmd(
            instance, auth_cmd, sizeof(auth_cmd), true, encRndB /* instance->encRndB */);

        if(ret != MfUltralightErrorNone) break;

        uint8_t iv[MF_ULTRALIGHT_C_AUTH_IV_BLOCK_SIZE] = {0};
        uint8_t* RndB = output + MF_ULTRALIGHT_C_AUTH_RND_B_BLOCK_OFFSET;
        mf_ultralight_3des_decrypt(
            &instance->des_context,
            instance->mfu_event.data->auth_context.tdes_key.data,
            iv,
            encRndB,
            sizeof(encRndB),
            RndB);
        mf_ultralight_3des_shift_data(RndB);

        memcpy(output, RndA, MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE);

        mf_ultralight_3des_encrypt(
            &instance->des_context,
            instance->mfu_event.data->auth_context.tdes_key.data,
            encRndB,
            output,
            MF_ULTRALIGHT_C_AUTH_DATA_SIZE,
            output);

    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_authenticate_end(
    MfUltralightPoller* instance,
    const uint8_t* RndB,
    const uint8_t* request,
    uint8_t* response) {
    furi_check(instance);
    furi_check(RndB);
    furi_check(request);
    furi_check(response);

    uint8_t auth_cmd[MF_ULTRALIGHT_C_ENCRYPTED_PACK_SIZE] = {0xAF}; //-V1009
    memcpy(&auth_cmd[1], request, MF_ULTRALIGHT_C_AUTH_DATA_SIZE);
    bit_buffer_copy_bytes(instance->tx_buffer, auth_cmd, sizeof(auth_cmd));

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        ret = mf_ultralight_poller_send_authenticate_cmd(
            instance, auth_cmd, sizeof(auth_cmd), false, response);

        if(ret != MfUltralightErrorNone) break;

        mf_ultralight_3des_decrypt(
            &instance->des_context,
            instance->mfu_event.data->auth_context.tdes_key.data,
            RndB,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE,
            response);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_page_from_sector(
    MfUltralightPoller* instance,
    uint8_t sector,
    uint8_t tag,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t select_sector_cmd[2] = {MF_ULTRALIGHT_CMD_SECTOR_SELECT, 0xff};
        bit_buffer_copy_bytes(instance->tx_buffer, select_sector_cmd, sizeof(select_sector_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorWrongCrc) {
            FURI_LOG_D(TAG, "Failed to issue sector select command");
            ret = mf_ultralight_process_error(error);
            break;
        }

        const uint8_t read_sector_cmd[4] = {sector, 0x00, 0x00, 0x00};
        bit_buffer_copy_bytes(instance->tx_buffer, read_sector_cmd, sizeof(read_sector_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorTimeout) {
            // This is NOT a typo! The tag ACKs by not sending a response within 1ms.
            FURI_LOG_D(TAG, "Sector %u select NAK'd", sector);
            ret = MfUltralightErrorProtocol;
            break;
        }

        ret = mf_ultralight_poller_read_page(instance, tag, data);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_page(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t read_page_cmd[2] = {MF_ULTRALIGHT_CMD_READ_PAGE, start_page};
        bit_buffer_copy_bytes(instance->tx_buffer, read_page_cmd, sizeof(read_page_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) !=
           sizeof(MfUltralightPageReadCommandData)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightPageReadCommandData));
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_write_page(
    MfUltralightPoller* instance,
    uint8_t page,
    const MfUltralightPage* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t write_page_cmd[MF_ULTRALIGHT_PAGE_SIZE + 2] = {MF_ULTRALIGHT_CMD_WRITE_PAGE, page};
        memcpy(&write_page_cmd[2], data->data, MF_ULTRALIGHT_PAGE_SIZE);
        bit_buffer_copy_bytes(instance->tx_buffer, write_page_cmd, sizeof(write_page_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorWrongCrc) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_buffer) != 4) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        if(!bit_buffer_starts_with_byte(instance->rx_buffer, MF_ULTRALIGHT_CMD_ACK)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
    } while(false);

    return ret;
}

MfUltralightError
    mf_ultralight_poller_read_version(MfUltralightPoller* instance, MfUltralightVersion* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t get_version_cmd = MF_ULTRALIGHT_CMD_GET_VERSION;
        bit_buffer_copy_bytes(instance->tx_buffer, &get_version_cmd, sizeof(get_version_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightVersion)) {
            FURI_LOG_I(
                TAG, "Read Version failed: %zu", bit_buffer_get_size_bytes(instance->rx_buffer));
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightVersion));
    } while(false);

    return ret;
}

MfUltralightError
    mf_ultralight_poller_read_signature(MfUltralightPoller* instance, MfUltralightSignature* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t read_signature_cmd[2] = {MF_ULTRALIGHT_CMD_READ_SIG, 0x00};
        bit_buffer_copy_bytes(instance->tx_buffer, read_signature_cmd, sizeof(read_signature_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightSignature)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightSignature));
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_counter(
    MfUltralightPoller* instance,
    uint8_t counter_num,
    MfUltralightCounter* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t read_counter_cmd[2] = {MF_ULTRALIGHT_CMD_READ_CNT, counter_num};
        bit_buffer_copy_bytes(instance->tx_buffer, read_counter_cmd, sizeof(read_counter_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != MF_ULTRALIGHT_COUNTER_SIZE) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data->data, MF_ULTRALIGHT_COUNTER_SIZE);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_tearing_flag(
    MfUltralightPoller* instance,
    uint8_t tearing_falg_num,
    MfUltralightTearingFlag* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t check_tearing_cmd[2] = {MF_ULTRALIGHT_CMD_CHECK_TEARING, tearing_falg_num};
        bit_buffer_copy_bytes(instance->tx_buffer, check_tearing_cmd, sizeof(check_tearing_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightTearingFlag)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightTearingFlag));
    } while(false);

    return ret;
}
