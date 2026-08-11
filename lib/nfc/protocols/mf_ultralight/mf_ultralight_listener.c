#include "mf_ultralight_listener_i.h"
#include "mf_ultralight_listener_defs.h"
#include "mf_ultralight_aes_crypto.h"

#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener_i.h>

#include <furi.h>
#include <furi_hal.h>
#include <mbedtls/aes.h>

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
        access_success = (instance->data->type == MfUltralightTypeUltralightAES) ?
                             mf_ultralight_aes_check_access(
                                 instance->data, start_page, access_type, instance->auth_state) :
                             mf_ultralight_c_check_access(
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

// Send a standard-frame response, MAC-wrapping it when UL-AES secure messaging is active: append the
// 8-byte MAC over (CmdCtr || response-data), then advance the command counter. Used in place of a
// bare iso14443_3a_listener_send_standard_frame by the data-returning command handlers.
static void mf_ultralight_listener_send_response(MfUltralightListener* instance) {
    if(instance->aes_sec_msg) {
        const size_t n = bit_buffer_get_size_bytes(instance->tx_buffer);
        // The MAC-appended response must fit the tx buffer; always true for UL-AES, whose secure
        // (MAC-wrapped) responses are at most 16 bytes. The 48-byte READ_SIG reply is served plain,
        // not through this path.
        furi_check(n + MF_ULTRALIGHT_AES_CMAC_SIZE <= MF_ULTRALIGHT_LISTENER_MAX_TX_BUFF_SIZE);
        uint8_t mac8[MF_ULTRALIGHT_AES_CMAC_SIZE];
        mf_ultralight_aes_cmac8_ctr(
            instance->aes_cmac_key,
            instance->aes_cmac_ctr,
            bit_buffer_get_data(instance->tx_buffer),
            n,
            mac8);
        bit_buffer_append_bytes(instance->tx_buffer, mac8, sizeof(mac8));
        instance->aes_cmac_ctr++;
    }
    iso14443_3a_listener_send_standard_frame(instance->iso14443_3a_listener, instance->tx_buffer);
}

// Verify and strip the trailing 8-byte command MAC of a secure-messaging command and advance the
// command counter. Returns false if the MAC is invalid (caller drops the session). On success the
// rx buffer is shrunk to the plain command so the command lookup matches its cmd_len_bits.
static bool mf_ultralight_aes_verify_strip_cmd(MfUltralightListener* instance, BitBuffer* rx) {
    const size_t n = bit_buffer_get_size_bytes(rx);
    if(n < MF_ULTRALIGHT_AES_CMAC_SIZE || n > MF_ULTRALIGHT_LISTENER_MAX_TX_BUFF_SIZE)
        return false;
    const size_t cmd_len = n - MF_ULTRALIGHT_AES_CMAC_SIZE;
    const uint8_t* data = bit_buffer_get_data(rx);

    uint8_t mac8[MF_ULTRALIGHT_AES_CMAC_SIZE];
    mf_ultralight_aes_cmac8_ctr(
        instance->aes_cmac_key, instance->aes_cmac_ctr, data, cmd_len, mac8);
    if(memcmp(mac8, data + cmd_len, MF_ULTRALIGHT_AES_CMAC_SIZE) != 0) return false;

    bit_buffer_set_size(rx, cmd_len * 8);
    instance->aes_cmac_ctr++;
    return true;
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
        else if(instance->data->type == MfUltralightTypeUltralightAES && page >= 0x30 && page <= 0x37)
            // UL-AES key pages (DataProtKey/UIDRetrKey) always read back as zero; never leak them.
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
        mf_ultralight_listener_send_response(instance);
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
        if(page_cnt > COUNT_OF(pages)) {
            command = MfUltralightCommandNotProcessedNAK;
            break;
        }
        mf_ultralight_listener_perform_read(pages, instance, start_page, page_cnt, do_i2c_check);

        bit_buffer_copy_bytes(instance->tx_buffer, (uint8_t*)pages, page_cnt * 4);
        mf_ultralight_listener_send_response(instance);
        command = MfUltralightCommandProcessed;
    } while(false);

    return command;
}

static MfUltralightCommand
    mf_ultralight_listener_write_page_handler(MfUltralightListener* instance, BitBuffer* buffer) {
    uint8_t start_page = bit_buffer_get_byte(buffer, 1);
    uint16_t pages_total = instance->data->pages_total;
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;

    FURI_LOG_T(TAG, "CMD_WRITE page %d", start_page);

    do {
        bool do_i2c_check = mf_ultralight_is_i2c_tag(instance->data->type);

        if(do_i2c_check) {
            if(!mf_ultralight_i2c_validate_pages(start_page, start_page, instance)) break;
        } else if(pages_total < start_page) {
            break;
        }

        // PATCHED: For Ultralight-C, allow writes to pages 44-47 (3DES key area)
        // This enables "magic card" emulation for key grabbing
        bool is_ulc_key_page = (instance->data->type == MfUltralightTypeMfulC) &&
                               (start_page >= 44 && start_page <= 47);

        if(!is_ulc_key_page) {
            // Normal access check for all other pages
            if(!mf_ultralight_listener_check_access(
                   instance, start_page, MfUltralightListenerAccessTypeWrite))
                break;

            if(mf_ultralight_static_lock_check_page(instance->static_lock, start_page)) break;
            if(mf_ultralight_dynamic_lock_check_page(instance, start_page)) break;
        } else {
            FURI_LOG_I(TAG, "MAGIC: Allowing write to ULC key page %d", start_page);
        }

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
        mf_ultralight_listener_send_response(instance);
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

    if(instance->data->type == MfUltralightTypeUltralightAES) {
        // UL-AES serves its 48-byte signature (not in the shared feature set), and only when one was
        // captured - so emulating a card without one stays silent like the real card.
        if(instance->data->aes_signature_present) {
            bit_buffer_copy_bytes(
                instance->tx_buffer,
                instance->data->aes_signature,
                MF_ULTRALIGHT_AES_SIGNATURE_SIZE);
            iso14443_3a_listener_send_standard_frame(
                instance->iso14443_3a_listener, instance->tx_buffer);
            command = MfUltralightCommandProcessed;
        }
        return command;
    }

    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportReadSignature)) {
        bit_buffer_copy_bytes(
            instance->tx_buffer, instance->data->signature.data, sizeof(MfUltralightSignature));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    }

    return command;
}

// UL-AES counter 2 read/increment can require authentication, selected by an ACCESS-byte enable bit
// (CNT_RD_EN / CNT_INC_EN). Returns true when that bit is clear and the session isn't authenticated.
static bool
    mf_ultralight_aes_counter2_gated(const MfUltralightListener* instance, uint8_t enable_bit) {
    return instance->data->type == MfUltralightTypeUltralightAES &&
           (instance->data->page[MF_ULTRALIGHT_AES_ACCESS_PAGE].data[0] & enable_bit) == 0 &&
           instance->auth_state != MfUltralightListenerAuthStateSuccess;
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

        // UL-AES: counter 2 read is gated by CNT_RD_EN; counters 0 and 1 are always readable.
        if(counter_num == 2 &&
           mf_ultralight_aes_counter2_gated(instance, MF_ULTRALIGHT_AES_ACCESS_CNT_RD_EN))
            break;

        uint8_t cnt_value[3] = {
            (instance->data->counter[counter_num].counter >> 0) & 0xff,
            (instance->data->counter[counter_num].counter >> 8) & 0xff,
            (instance->data->counter[counter_num].counter >> 16) & 0xff,
        };
        bit_buffer_copy_bytes(instance->tx_buffer, cnt_value, sizeof(cnt_value));
        mf_ultralight_listener_send_response(instance);
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

        // UL-AES: counter 2 increment is gated by CNT_INC_EN.
        if(counter_num == 2 &&
           mf_ultralight_aes_counter2_gated(instance, MF_ULTRALIGHT_AES_ACCESS_CNT_INC_EN))
            break;

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

// --- UL-AES 3-pass mutual authentication (listener / card side) ---

// AES-128 CBC with a zero IV (single-shot: inits/frees the context).
static void mf_ultralight_aes_listener_crypt(
    const uint8_t* key,
    int mode,
    size_t length,
    const uint8_t* input,
    uint8_t* output) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    uint8_t iv[MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
    if(mode == MBEDTLS_AES_ENCRYPT) {
        mbedtls_aes_setkey_enc(&ctx, key, 128);
    } else {
        mbedtls_aes_setkey_dec(&ctx, key, 128);
    }
    mbedtls_aes_crypt_cbc(&ctx, mode, length, iv, input, output);
    mbedtls_aes_free(&ctx);
}

static MfUltralightCommand
    mf_ultralight_aes_authenticate_handler_p2(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    FURI_LOG_T(TAG, "CMD_ULAES_AUTH_2");
    do {
        if(bit_buffer_get_byte(buffer, 0) != 0xAF ||
           bit_buffer_get_size_bytes(buffer) != MF_ULTRALIGHT_AES_AUTH_P2_CMD_SIZE)
            break;

        uint8_t key[MF_ULTRALIGHT_AES_KEY_SIZE];
        mf_ultralight_aes_get_key(instance->data, key);

        // Decrypt ek(RndA || RndB') -> RndA || RndB'
        const uint8_t* enc = bit_buffer_get_data(buffer) + 1;
        uint8_t dec[2 * MF_ULTRALIGHT_AES_BLOCK_SIZE];
        mf_ultralight_aes_listener_crypt(key, MBEDTLS_AES_DECRYPT, sizeof(dec), enc, dec);

        uint8_t* rnd_a = dec;
        const uint8_t* rnd_b_prime = dec + MF_ULTRALIGHT_AES_BLOCK_SIZE;

        // Verify the reader's RndB' equals our RndB rotated left by one byte.
        uint8_t expected[MF_ULTRALIGHT_AES_BLOCK_SIZE];
        memcpy(expected, instance->aes_rnd_b, sizeof(expected));
        mf_ultralight_aes_rol16(expected);
        if(memcmp(rnd_b_prime, expected, sizeof(expected)) == 0) {
            instance->auth_state = MfUltralightListenerAuthStateSuccess;

            // If the card enables secure messaging, derive the session key (same RndA/RndB the
            // reader used) and enter MAC mode for all subsequent commands. derive expects the
            // randoms rotated once (it un-rotates them), so rotate the originals here.
            if(instance->data->page[MF_ULTRALIGHT_AES_CFG_PAGE].data[0] &
               MF_ULTRALIGHT_AES_CFG_SEC_MSG_ACT) {
                uint8_t ra_rot[MF_ULTRALIGHT_AES_BLOCK_SIZE];
                uint8_t rb_rot[MF_ULTRALIGHT_AES_BLOCK_SIZE];
                memcpy(ra_rot, rnd_a, sizeof(ra_rot));
                memcpy(rb_rot, instance->aes_rnd_b, sizeof(rb_rot));
                mf_ultralight_aes_rol16(ra_rot);
                mf_ultralight_aes_rol16(rb_rot);
                mf_ultralight_aes_derive_session_key(key, ra_rot, rb_rot, instance->aes_cmac_key);
                instance->aes_cmac_ctr = 0;
                instance->aes_sec_msg = true;
            }
        }

        // Respond with ek(RndA') regardless: a wrong key yields an RndA' the reader will reject.
        mf_ultralight_aes_rol16(rnd_a);
        uint8_t enc_a[MF_ULTRALIGHT_AES_BLOCK_SIZE];
        mf_ultralight_aes_listener_crypt(key, MBEDTLS_AES_ENCRYPT, sizeof(enc_a), rnd_a, enc_a);

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, 0x00);
        bit_buffer_append_bytes(instance->tx_buffer, enc_a, sizeof(enc_a));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
    } while(false);
    return command;
}

static MfUltralightCommand
    mf_ultralight_aes_authenticate_handler_p1(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = MfUltralightCommandNotProcessedNAK;
    FURI_LOG_T(TAG, "CMD_ULAES_AUTH_1");
    UNUSED(buffer); // key number argument is ignored; only DataProtKey is emulated
    do {
        // A new authentication drops any prior authenticated state (and secure messaging) until
        // part 2 succeeds.
        instance->auth_state = MfUltralightListenerAuthStateIdle;
        instance->aes_sec_msg = false;

        uint8_t key[MF_ULTRALIGHT_AES_KEY_SIZE];
        mf_ultralight_aes_get_key(instance->data, key);

        furi_hal_random_fill_buf(instance->aes_rnd_b, sizeof(instance->aes_rnd_b));

        uint8_t enc_b[MF_ULTRALIGHT_AES_BLOCK_SIZE];
        mf_ultralight_aes_listener_crypt(
            key, MBEDTLS_AES_ENCRYPT, sizeof(enc_b), instance->aes_rnd_b, enc_b);

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_byte(instance->tx_buffer, 0xAF);
        bit_buffer_append_bytes(instance->tx_buffer, enc_b, sizeof(enc_b));
        iso14443_3a_listener_send_standard_frame(
            instance->iso14443_3a_listener, instance->tx_buffer);
        command = MfUltralightCommandProcessed;
        mf_ultralight_composite_command_set_next(
            instance, mf_ultralight_aes_authenticate_handler_p2);
    } while(false);
    return command;
}

// AUTHENTICATE (0x1A) shares its command byte between UL-C (3DES) and UL-AES; dispatch by type.
static MfUltralightCommand
    mf_ultralight_authenticate_handler_p1(MfUltralightListener* instance, BitBuffer* buffer) {
    if(instance->data->type == MfUltralightTypeUltralightAES) {
        return mf_ultralight_aes_authenticate_handler_p1(instance, buffer);
    }
    return mf_ultralight_c_authenticate_handler_p1(instance, buffer);
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
        .callback = mf_ultralight_authenticate_handler_p1,
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
        if(instance->aes_sec_msg) {
            // Secure messaging replaces the ACK with a standalone MAC over CmdCtr (§8.8.3). An empty
            // response body makes send_response MAC exactly the counter, then advance it.
            bit_buffer_set_size(instance->tx_buffer, 0);
            mf_ultralight_listener_send_response(instance);
        } else {
            mf_ultralight_listener_send_short_resp(instance, MF_ULTRALIGHT_CMD_ACK);
        }
        command = NfcCommandContinue;
    } else if(mfu_command == MfUltralightCommandProcessedSilent) {
        command = NfcCommandReset;
    } else if(mfu_command != MfUltralightCommandProcessed) {
        instance->auth_state = MfUltralightListenerAuthStateIdle;
        // Any error in secure messaging drops the authenticated session; the NAK carries no MAC.
        instance->aes_sec_msg = false;
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
    instance->aes_sec_msg = false;
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

        // UL-AES secure messaging: verify and strip the trailing command MAC before dispatch, so the
        // command lookup matches the plain frame size. AUTHENTICATE (0x1A) stays plain even in MAC
        // mode; the composite continuation (auth part 2) is dispatched separately below.
        if(instance->aes_sec_msg && cmd != MF_ULTRALIGHT_CMD_AUTH &&
           !mf_ultralight_composite_command_in_progress(instance) &&
           !mf_ultralight_aes_verify_strip_cmd(instance, rx_buffer)) {
            // Bad command MAC (desync, relay, or attack): drop the authenticated session and NAK.
            FURI_LOG_W(TAG, "UL-AES command MAC mismatch, dropping session");
            instance->auth_state = MfUltralightListenerAuthStateIdle;
            instance->aes_sec_msg = false;
            mfu_command = MfUltralightCommandNotProcessedNAK;
        } else if(mf_ultralight_composite_command_in_progress(instance)) {
            mfu_command = mf_ultralight_composite_command_run(instance, rx_buffer);
        } else {
            size =
                bit_buffer_get_size(rx_buffer); // MAC may have been stripped; match cmd_len_bits
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
