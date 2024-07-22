#include "mf_ultralight_listener_i.h"
#include "mf_ultralight_listener_defs.h"

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener_i.h>

#include <furi.h>
#include <furi_hal.h>

#define TAG "MfUltralightListener"

#define MF_ULTRALIGHT_LISTENER_MAX_TX_BUFF_SIZE (256)

typedef struct {
    uint8_t cmd;
    size_t cmd_len_bits;
    MfUltralightListenerCommandCallback callback;
} MfUltralightListenerCmdHandler;

static bool mf_ultralight_listener_check_access(
    MfUltralightListener* instance,
    uint16_t start_page,
    MfUltralightListenerAccessType access_type) {
    bool access_success = true;

    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportAuthenticate)) {
        access_success = mf_ultralight_c_check_access(
            instance->data, start_page, access_type, instance->auth_state);
    } else if(mf_ultralight_support_feature(
                  instance->features, MfUltralightFeatureSupportPasswordAuth)) {
        access_success = mf_ultralight_common_check_access(instance, start_page, access_type);
    }
    return access_success;
}

static void mf_ultralight_listener_send_short_resp(MfUltralightListener* instance, uint8_t data) {
    furi_assert(instance->tx_buffer);

    bit_buffer_set_size(instance->tx_buffer, 4);
    bit_buffer_set_byte(instance->tx_buffer, 0, data);
    iso14443_3a_listener_tx(instance->iso14443_3a_listener, instance->tx_buffer);
}

static void mf_ultralight_listener_perform_read(
    MfUltralightPage* pages,
    MfUltralightListener* instance,
    uint16_t start_page,
    uint8_t page_cnt,
    bool do_i2c_page_check) {
    uint16_t pages_total = instance->data->pages_total;
    mf_ultralight_mirror_read_prepare(start_page, instance);
    for(uint8_t i = 0, rollover = 0; i < page_cnt; i++) {
        uint16_t page = start_page + i;

        bool page_restricted = !mf_ultralight_listener_check_access(
            instance, page, MfUltralightListenerAccessTypeRead);

        if(do_i2c_page_check && !mf_ultralight_i2c_validate_pages(page, page, instance))
            memset(pages[i].data, 0, sizeof(MfUltralightPage));
        else if(mf_ultralight_is_page_pwd_or_pack(instance->data->type, page))
            memset(pages[i].data, 0, sizeof(MfUltralightPage));
        else {
            if(do_i2c_page_check)
                page = mf_ultralight_i2c_provide_page_by_requested(page, instance);

            page = page_restricted ? rollover++ : page % pages_total;
            pages[i] = instance->data->page[page];

            mf_ultralight_mirror_read_handler(page, pages[i].data, instance);
        }
    }
    mf_ultralight_single_counter_try_increase(instance);
}

static MfUltralightCommand mf_ultralight_listener_perform_write(
    MfUltralightListener* instance,
    const uint8_t* const rx_data,
    uint16_t start_page,
    bool do_i2c_check) {
    MfUltralightCommand command = MfUltralightCommandProcessedACK;

    if(start_page < 2 && instance->sector == 0)
        command = MfUltralightCommandNotProcessedNAK;
    else if(start_page == 2 && instance->sector == 0)
        mf_ultralight_static_lock_bytes_write(instance->static_lock, *((uint16_t*)&rx_data[2]));
    else if(start_page == 3 && instance->sector == 0)
        mf_ultralight_capability_container_write(&instance->data->page[start_page], rx_data);
    else if(mf_ultralight_is_page_dynamic_lock(instance, start_page))
        mf_ultralight_dynamic_lock_bytes_write(instance->dynamic_lock, *((uint32_t*)rx_data));
    else {
        uint16_t page = start_page;
        if(do_i2c_check) page = mf_ultralight_i2c_provide_page_by_requested(start_page, instance);

        memcpy(instance->data->page[page].data, rx_data, sizeof(MfUltralightPage));
    }

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_read_page_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    uint16_t start_page = bit_buffer_get_byte(buffer, 1);
    uint16_t pages_total = instance->data->pages_total;
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_READ: %d", start_page);

    do {
        bool do_i2c_check = mf_ultralight_is_i2c_tag(instance->data->type);

        if(do_i2c_check) {
            if(!mf_ultralight_i2c_validate_pages(start_page, start_page, instance)) break;
        } else if(pages_total < start_page) {
            break;
        }

        if(!mf_ultralight_listener_check_access(
               instance, start_page, MfUltralightListenerAccessTypeRead)) {
            break;
        }

        MfUltralightPage pages[4] = {};
        mf_ultralight_listener_perform_read(pages, instance, start_page, 4, do_i2c_check);

        bit_buffer_copy_bytes(instance->tx_buffer, (uint8_t*)pages, sizeof(pages));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;

    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_fast_read_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;
    FURI_LOG_T(TAG, "CMD_FAST_READ");

    do {
        if(!mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportFastRead))
            break;
        uint16_t pages_total = instance->data->pages_total;
        uint16_t start_page = bit_buffer_get_byte(buffer, 1);
        uint16_t end_page = bit_buffer_get_byte(buffer, 2);
        bool do_i2c_check = mf_ultralight_is_i2c_tag(instance->data->type);

        if(do_i2c_check) {
            if(!mf_ultralight_i2c_validate_pages(start_page, end_page, instance)) {
                command = MfUltralightCommandNotProcessedNAK;
                break;
            }
        } else if(end_page > pages_total - 1) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        if(end_page < start_page) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        if(!mf_ultralight_listener_check_access(
               instance, start_page, MfUltralightListenerAccessTypeRead) ||
           !mf_ultralight_listener_check_access(
               instance, end_page, MfUltralightListenerAccessTypeRead)) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        MfUltralightPage pages[64] = {};
        uint8_t page_cnt = (end_page - start_page) + 1;
        mf_ultralight_listener_perform_read(pages, instance, start_page, page_cnt, do_i2c_check);

        bit_buffer_copy_bytes(instance->tx_buffer, (uint8_t*)pages, page_cnt * 4);
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_write_page_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    uint8_t start_page = bit_buffer_get_byte(buffer, 1);
    uint16_t pages_total = instance->data->pages_total;
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_WRITE");

    do {
        bool do_i2c_check = mf_ultralight_is_i2c_tag(instance->data->type);

        if(do_i2c_check) {
            if(!mf_ultralight_i2c_validate_pages(start_page, start_page, instance)) break;
        } else if(pages_total < start_page) {
            break;
        }

        if(!mf_ultralight_listener_check_access(
               instance, start_page, MfUltralightListenerAccessTypeWrite))
            break;

        if(mf_ultralight_static_lock_check_page(instance->static_lock, start_page)) break;
        if(mf_ultralight_dynamic_lock_check_page(instance, start_page)) break;

        const uint8_t* rx_data = bit_buffer_get_data(buffer);
        command =
            mf_ultralight_listener_perform_write(instance, &rx_data[2], start_page, do_i2c_check);
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_fast_write_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;
    FURI_LOG_T(TAG, "CMD_FAST_WRITE");

    do {
        if(!mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportFastWrite))
            break;

        uint8_t start_page = bit_buffer_get_byte(buffer, 1);
        uint8_t end_page = bit_buffer_get_byte(buffer, 2);
        if(start_page != 0xF0 || end_page != 0xFF) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        // No SRAM emulation support

        command = MfUltralightCommandProcessedACK;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_read_version_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    UNUSED(buffer);
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;

    FURI_LOG_T(TAG, "CMD_GET_VERSION");

    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportReadVersion)) {
        bit_buffer_copy_bytes(
            instance->tx_buffer, (uint8_t*)&instance->data->version, sizeof(MfUltralightVersion));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    }

    return command;
}

static MfUltralightCommand mf_ultralight_listener_read_signature_handler(
    MfUltralightListener* instance,
    BitBuffer* buffer) {
    UNUSED(buffer);
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;

    FURI_LOG_T(TAG, "CMD_READ_SIG");

    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportReadSignature)) {
        bit_buffer_copy_bytes(
            instance->tx_buffer, instance->data->signature.data, sizeof(MfUltralightSignature));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    }

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_read_counter_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_READ_CNT");

    do {
        uint8_t counter_num = bit_buffer_get_byte(buffer, 1);
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportReadCounter))
            break;

        if(mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportSingleCounter)) {
            if(instance->config == NULL) break;

            if(!instance->config->access.nfc_cnt_en || counter_num != 2) break;

            if(instance->config->access.nfc_cnt_pwd_prot) {
                if(instance->auth_state != MfUltralightListenerAuthStateSuccess) {
                    break;
                }
            }
        }

        if(counter_num > 2) break;
        uint8_t cnt_value[3] = {
            (instance->data->counter[counter_num].counter >> 0) & 0xff,
            (instance->data->counter[counter_num].counter >> 8) & 0xff,
            (instance->data->counter[counter_num].counter >> 16) & 0xff,
        };
        bit_buffer_copy_bytes(instance->tx_buffer, cnt_value, sizeof(cnt_value));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    } while(false);

    return command;
}

static MfUltralightCommand mf_ultralight_listener_increase_counter_handler(
    MfUltralightListener* instance,
    BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_INCR_CNT");

    do {
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportIncCounter)) {
            command = MfUltralightCommandNotProcessedSilent;
            break;
        }

        uint8_t counter_num = bit_buffer_get_byte(buffer, 1);
        if(counter_num > 2) break;

        if(instance->data->counter[counter_num].counter == MF_ULTRALIGHT_MAX_CNTR_VAL) {
            command = MfUltralightCommandProcessed;
            break;
        }

        MfUltralightCounter buf_counter = {};
        bit_buffer_write_bytes_mid(buffer, buf_counter.data, 2, sizeof(buf_counter.data));
        uint32_t incr_value = buf_counter.counter;

        if(instance->data->counter[counter_num].counter + incr_value > MF_ULTRALIGHT_MAX_CNTR_VAL)
            break;

        instance->data->counter[counter_num].counter += incr_value;
        command = MfUltralightCommandProcessedACK;
    } while(false);

    return command;
}

static MfUltralightCommand mf_ultralight_listener_check_tearing_handler(
    MfUltralightListener* instance,
    BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_CHECK_TEARING");

    do {
        uint8_t tearing_flag_num = bit_buffer_get_byte(buffer, 1);
        if(!mf_ultralight_support_feature(
               instance->features,
               MfUltralightFeatureSupportCheckTearingFlag |
                   MfUltralightFeatureSupportSingleCounter)) {
            break;
        }
        if(mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportSingleCounter) &&
           (tearing_flag_num != 2)) {
            break;
        }
        if(tearing_flag_num >= MF_ULTRALIGHT_TEARING_FLAG_NUM) {
            break;
        }

        bit_buffer_set_size_bytes(instance->tx_buffer, 1);
        bit_buffer_set_byte(
            instance->tx_buffer, 0, instance->data->tearing_flag[tearing_flag_num].data);
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;

    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_vcsl_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;
    UNUSED(instance);
    UNUSED(buffer);
    FURI_LOG_T(TAG, "CMD_VCSL");
    do {
        if(!mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportVcsl))
            break;

        MfUltralightConfigPages* config;
        if(!mf_ultralight_get_config_page(instance->data, &config)) break;

        bit_buffer_set_size_bytes(instance->tx_buffer, 1);
        bit_buffer_set_byte(instance->tx_buffer, 0, config->vctid);
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_auth_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_AUTH");

    do {
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportPasswordAuth))
            break;

        MfUltralightAuthPassword password;
        bit_buffer_write_bytes_mid(buffer, password.data, 1, sizeof(password.data));

        if(instance->callback) {
            instance->mfu_event_data.password = password;
            instance->mfu_event.type = MfUltralightListenerEventTypeAuth;
            instance->callback(instance->generic_event, instance->context);
        }

        bool auth_success =
            mf_ultralight_auth_check_password(&instance->config->password, &password);
        bool card_locked = mf_ultralight_auth_limit_check_and_update(instance, auth_success);

        if(card_locked) {
            command = MfUltralightCommandNotProcessedAuthNAK;
            break;
        }

        if(!auth_success) break;

        bit_buffer_copy_bytes(
            instance->tx_buffer, instance->config->pack.data, sizeof(MfUltralightAuthPack));
        instance->auth_state = MfUltralightListenerAuthStateSuccess;
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);

        command = MfUltralightCommandProcessed;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_comp_write_handler_p2(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    FURI_LOG_T(TAG, "CMD_CM_WR_2");

    do {
        if(bit_buffer_get_size_bytes(buffer) != 16) break;

        const uint8_t* rx_data = bit_buffer_get_data(buffer);
        uint8_t start_page = instance->composite_cmd.data;

        command = mf_ultralight_listener_perform_write(instance, rx_data, start_page, false);
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_comp_write_handler_p1(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedSilent;

    FURI_LOG_T(TAG, "CMD_CM_WR_1");

    do {
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportCompatibleWrite))
            break;

        uint8_t start_page = bit_buffer_get_byte(buffer, 1);
        uint16_t last_page = instance->data->pages_total - 1;

        if(start_page < 2 || start_page > last_page) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        if(!mf_ultralight_listener_check_access(
               instance, start_page, MfUltralightListenerAccessTypeWrite)) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        if(mf_ultralight_static_lock_check_page(instance->static_lock, start_page) ||
           mf_ultralight_dynamic_lock_check_page(instance, start_page)) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }

        instance->composite_cmd.data = start_page;
        command = MfUltralightCommandProcessedACK;
        mf_ultralight_composite_command_set_next(instance, mf_ultralight_comp_write_handler_p2);
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_sector_select_handler_p2(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    UNUSED(instance);
    UNUSED(buffer);
    FURI_LOG_T(TAG, "CMD_SEC_SEL_2");

    do {
        if(bit_buffer_get_size_bytes(buffer) != 4) break;
        uint8_t sector = bit_buffer_get_byte(buffer, 0);
        if(sector == 0xFF) break;

        instance->sector = sector;
        command = MfUltralightCommandProcessedSilent;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_sector_select_handler_p1(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    UNUSED(buffer);
    FURI_LOG_T(TAG, "CMD_SEC_SEL_1");

    do {
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportSectorSelect) &&
           bit_buffer_get_byte(buffer, 1) == 0xFF)
            break;

        command = MfUltralightCommandProcessedACK;
        mf_ultralight_composite_command_set_next(instance, mf_ultralight_sector_select_handler_p2);
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_c_authenticate_handler_p2(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    FURI_LOG_T(TAG, "CMD_ULC_AUTH_2");
    UNUSED(instance);
    do {
        if(bit_buffer_get_byte(buffer, 0) != 0xAF ||
           bit_buffer_get_size_bytes(buffer) != MF_ULTRALIGHT_C_ENCRYPTED_PACK_SIZE ||
           !mf_ultralight_3des_key_valid(instance->data))
            break;

        const uint8_t* data = bit_buffer_get_data(buffer) + 1;
        const uint8_t* iv = data + MF_ULTRALIGHT_C_AUTH_RND_B_BLOCK_OFFSET;

        uint8_t out[MF_ULTRALIGHT_C_AUTH_DATA_SIZE] = {0};

        const uint8_t* ck = mf_ultralight_3des_get_key(instance->data);
        mf_ultralight_3des_decrypt(
            &instance->des_context, ck, instance->encB, data, sizeof(out), out);

        uint8_t* rndA = out;
        const uint8_t* decoded_shifted_rndB = out + MF_ULTRALIGHT_C_AUTH_RND_B_BLOCK_OFFSET;

        mf_ultralight_3des_shift_data(rndA);
        mf_ultralight_3des_shift_data(instance->rndB);
        if(memcmp(decoded_shifted_rndB, instance->rndB, sizeof(instance->rndB)) == 0) {
            instance->auth_state = MfUltralightListenerAuthStateSuccess;
        }

        mf_ultralight_3des_encrypt(
            &instance->des_context, ck, iv, rndA, MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE, rndA);

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, 0x00);
        bit_buffer_append_bytes(instance->tx_buffer, rndA, MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE);

        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);

        command = MfUltralightCommandProcessed;
    } while(false);
    return command;
}

static MfUltralightCommand
    mf_ultralight_c_authenticate_handler_p1(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    FURI_LOG_T(TAG, "CMD_ULC_AUTH_1");
    do {
        if(!mf_ultralight_support_feature(
               instance->features, MfUltralightFeatureSupportAuthenticate) &&
           bit_buffer_get_byte(buffer, 1) == 0x00)
            break;

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, 0xAF);

        furi_hal_random_fill_buf(instance->rndB, sizeof(instance->rndB));

        const uint8_t iv[MF_ULTRALIGHT_C_AUTH_IV_BLOCK_SIZE] = {0};
        const uint8_t* ck = mf_ultralight_3des_get_key(instance->data);

        mf_ultralight_3des_encrypt(
            &instance->des_context, ck, iv, instance->rndB, sizeof(instance->rndB), instance->encB);

        bit_buffer_append_bytes(instance->tx_buffer, instance->encB, sizeof(instance->encB));

        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
        mf_ultralight_composite_command_set_next(
            instance, mf_ultralight_c_authenticate_handler_p2);
    } while(false);
    return command;
}

static const MfUltralightListenerCmdHandler mf_ultralight_command[] = {
    {
        .cmd = MF_ULTRALIGHT_CMD_READ_PAGE,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_listener_read_page_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_FAST_READ,
        .cmd_len_bits = 3 * 8,
        .callback = mf_ultralight_listener_fast_read_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_WRITE_PAGE,
        .cmd_len_bits = 6 * 8,
        .callback = mf_ultralight_listener_write_page_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_FAST_WRITE,
        .cmd_len_bits = 67 * 8,
        .callback = mf_ultralight_listener_fast_write_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_GET_VERSION,
        .cmd_len_bits = 8,
        .callback = mf_ultralight_listener_read_version_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_READ_SIG,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_listener_read_signature_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_READ_CNT,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_listener_read_counter_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_CHECK_TEARING,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_listener_check_tearing_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_PWD_AUTH,
        .cmd_len_bits = 5 * 8,
        .callback = mf_ultralight_listener_auth_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_INCR_CNT,
        .cmd_len_bits = 6 * 8,
        .callback = mf_ultralight_listener_increase_counter_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_SECTOR_SELECT,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_sector_select_handler_p1,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_COMP_WRITE,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_comp_write_handler_p1,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_VCSL,
        .cmd_len_bits = 21 * 8,
        .callback = mf_ultralight_listener_vcsl_handler,
    },
    {
        .cmd = MF_ULTRALIGHT_CMD_AUTH,
        .cmd_len_bits = 2 * 8,
        .callback = mf_ultralight_c_authenticate_handler_p1,
    }};

static void mf_ultralight_listener_prepare_emulation(MfUltralightListener* instance) {
    MfUltralightData* data = instance->data;
    instance->features = mf_ultralight_get_feature_support_set(data->type);
    mf_ultralight_get_config_page(data, &instance->config);
    mf_ultralight_mirror_prepare_emulation(instance);
    mf_ultralight_static_lock_bytes_prepare(instance);
    mf_ultralight_dynamic_lock_bytes_prepare(instance);
}

static NfcCommand mf_ultralight_command_postprocess(
    MfUltralightCommand mfu_command,
    MfUltralightListener* instance) {
    NfcCommand command = NfcCommandContinue;

    if(mfu_command == MfUltralightCommandProcessedACK) {
        mf_ultralight_listener_send_short_resp(instance, MF_ULTRALIGHT_CMD_ACK);
        command = NfcCommandContinue;
    } else if(mfu_command == MfUltralightCommandProcessedSilent) {
        command = NfcCommandReset;
    } else if(mfu_command != MfUltralightCommandProcessed) {
        instance->auth_state = MfUltralightListenerAuthStateIdle;
        command = NfcCommandSleep;

        if(mfu_command == MfUltralightCommandNotProcessedNAK) {
            mf_ultralight_listener_send_short_resp(instance, MF_ULTRALIGHT_CMD_NACK);
        } else if(mfu_command == MfUltralightCommandNotProcessedAuthNAK) {
            mf_ultralight_listener_send_short_resp(instance, MF_ULTRALIGHT_CMD_AUTH_NAK);
        }
    }

    return command;
}

static NfcCommand mf_ultralight_reset_listener_state(
    MfUltralightListener* instance,
    Iso14443_3aListenerEventType event_type) {
    mf_ultralight_composite_command_reset(instance);
    mf_ultralight_single_counter_try_to_unlock(instance, event_type);
    instance->sector = 0;
    instance->auth_state = MfUltralightListenerAuthStateIdle;
    return NfcCommandSleep;
}

MfUltralightListener* mf_ultralight_listener_alloc(
    Iso14443_3aListener* iso14443_3a_listener,
    MfUltralightData* data) {
    furi_assert(iso14443_3a_listener);

    MfUltralightListener* instance = malloc(sizeof(MfUltralightListener));
    instance->mirror.ascii_mirror_data = furi_string_alloc();
    instance->iso14443_3a_listener = iso14443_3a_listener;
    instance->data = data;
    mf_ultralight_static_lock_bytes_prepare(instance);
    mf_ultralight_listener_prepare_emulation(instance);
    mf_ultralight_composite_command_reset(instance);
    instance->sector = 0;
    instance->tx_buffer = bit_buffer_alloc(MF_ULTRALIGHT_LISTENER_MAX_TX_BUFF_SIZE);

    instance->mfu_event.data = &instance->mfu_event_data;
    instance->generic_event.protocol = NfcProtocolMfUltralight;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->mfu_event;
    mbedtls_des3_init(&instance->des_context);

    return instance;
}

void mf_ultralight_listener_free(MfUltralightListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    furi_string_free(instance->mirror.ascii_mirror_data);
    mbedtls_des3_free(&instance->des_context);
    free(instance);
}

const MfUltralightData* mf_ultralight_listener_get_data(MfUltralightListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

void mf_ultralight_listener_set_callback(
    MfUltralightListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

NfcCommand mf_ultralight_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);
    furi_assert(event.event_data);

    MfUltralightListener* instance = context;
    Iso14443_3aListenerEvent* iso14443_3a_event = event.event_data;
    BitBuffer* rx_buffer = iso14443_3a_event->data->buffer;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_3a_event->type == Iso14443_3aListenerEventTypeReceivedStandardFrame) {
        MfUltralightCommand mfu_command = MfUltralightCommandNotFound;
        size_t size = bit_buffer_get_size(rx_buffer);
        uint8_t cmd = bit_buffer_get_byte(rx_buffer, 0);

        if(mf_ultralight_composite_command_in_progress(instance)) {
            mfu_command = mf_ultralight_composite_command_run(instance, rx_buffer);
        } else {
            for(size_t i = 0; i < COUNT_OF(mf_ultralight_command); i++) {
                if(size != mf_ultralight_command[i].cmd_len_bits) continue;
                if(cmd != mf_ultralight_command[i].cmd) continue;
                mfu_command = mf_ultralight_command[i].callback(instance, rx_buffer);

                if(mfu_command != MfUltralightCommandNotFound) break;
            }
        }
        command = mf_ultralight_command_postprocess(mfu_command, instance);
    } else if(
        iso14443_3a_event->type == Iso14443_3aListenerEventTypeReceivedData ||
        iso14443_3a_event->type == Iso14443_3aListenerEventTypeFieldOff ||
        iso14443_3a_event->type == Iso14443_3aListenerEventTypeHalted) {
        command = mf_ultralight_reset_listener_state(instance, iso14443_3a_event->type);
    }

    return command;
}

const NfcListenerBase mf_ultralight_listener = {
    .alloc = (NfcListenerAlloc)mf_ultralight_listener_alloc,
    .free = (NfcListenerFree)mf_ultralight_listener_free,
    .get_data = (NfcListenerGetData)mf_ultralight_listener_get_data,
    .set_callback = (NfcListenerSetCallback)mf_ultralight_listener_set_callback,
    .run = (NfcListenerRun)mf_ultralight_listener_run,
};
