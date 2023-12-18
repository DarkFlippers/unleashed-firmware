#include "mf_ultralight_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfUltralightPoller"

typedef NfcCommand (*MfUltralightPollerReadHandler)(MfUltralightPoller* instance);

static bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_1k(
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left) {
    bool tag_calculated = false;
    // 0 - 226: sector 0
    // 227 - 228: config registers
    // 229 - 230: session registers

    if(lin_addr > 230) {
        *pages_left = 0;
    } else if(lin_addr >= 229) {
        *sector = 3;
        *pages_left = 2 - (lin_addr - 229);
        *tag = lin_addr - 229 + 248;
        tag_calculated = true;
    } else if(lin_addr >= 227) {
        *sector = 0;
        *pages_left = 2 - (lin_addr - 227);
        *tag = lin_addr - 227 + 232;
        tag_calculated = true;
    } else {
        *sector = 0;
        *pages_left = 227 - lin_addr;
        *tag = lin_addr;
        tag_calculated = true;
    }

    return tag_calculated;
}

static bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_2k(
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left) {
    bool tag_calculated = false;
    // 0 - 255: sector 0
    // 256 - 480: sector 1
    // 481 - 482: config registers
    // 483 - 484: session registers

    if(lin_addr > 484) {
        *pages_left = 0;
    } else if(lin_addr >= 483) {
        *sector = 3;
        *pages_left = 2 - (lin_addr - 483);
        *tag = lin_addr - 483 + 248;
        tag_calculated = true;
    } else if(lin_addr >= 481) {
        *sector = 1;
        *pages_left = 2 - (lin_addr - 481);
        *tag = lin_addr - 481 + 232;
        tag_calculated = true;
    } else if(lin_addr >= 256) {
        *sector = 1;
        *pages_left = 225 - (lin_addr - 256);
        *tag = lin_addr - 256;
        tag_calculated = true;
    } else {
        *sector = 0;
        *pages_left = 256 - lin_addr;
        *tag = lin_addr;
        tag_calculated = true;
    }

    return tag_calculated;
}

static bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_plus_1k(
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left) {
    bool tag_calculated = false;
    // 0 - 233: sector 0 + registers
    // 234 - 235: session registers

    if(lin_addr > 235) {
        *pages_left = 0;
    } else if(lin_addr >= 234) {
        *sector = 0;
        *pages_left = 2 - (lin_addr - 234);
        *tag = lin_addr - 234 + 236;
        tag_calculated = true;
    } else {
        *sector = 0;
        *pages_left = 234 - lin_addr;
        *tag = lin_addr;
        tag_calculated = true;
    }

    return tag_calculated;
}

static bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_plus_2k(
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left) {
    bool tag_calculated = false;
    // 0 - 233: sector 0 + registers
    // 234 - 235: session registers
    // 236 - 491: sector 1

    if(lin_addr > 491) {
        *pages_left = 0;
    } else if(lin_addr >= 236) {
        *sector = 1;
        *pages_left = 256 - (lin_addr - 236);
        *tag = lin_addr - 236;
        tag_calculated = true;
    } else if(lin_addr >= 234) {
        *sector = 0;
        *pages_left = 2 - (lin_addr - 234);
        *tag = lin_addr - 234 + 236;
        tag_calculated = true;
    } else {
        *sector = 0;
        *pages_left = 234 - lin_addr;
        *tag = lin_addr;
        tag_calculated = true;
    }

    return tag_calculated;
}

bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag(
    MfUltralightPoller* instance,
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left) {
    furi_assert(instance);
    furi_assert(sector);
    furi_assert(tag);
    furi_assert(pages_left);

    bool tag_calculated = false;

    if(instance->data->type == MfUltralightTypeNTAGI2C1K) {
        tag_calculated = mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_1k(
            lin_addr, sector, tag, pages_left);
    } else if(instance->data->type == MfUltralightTypeNTAGI2C2K) {
        tag_calculated = mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_2k(
            lin_addr, sector, tag, pages_left);
    } else if(instance->data->type == MfUltralightTypeNTAGI2CPlus1K) {
        tag_calculated = mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_plus_1k(
            lin_addr, sector, tag, pages_left);
    } else if(instance->data->type == MfUltralightTypeNTAGI2CPlus2K) {
        tag_calculated = mf_ultralight_poller_ntag_i2c_addr_lin_to_tag_ntag_i2c_plus_2k(
            lin_addr, sector, tag, pages_left);
    }

    return tag_calculated;
}

MfUltralightPoller* mf_ultralight_poller_alloc(Iso14443_3aPoller* iso14443_3a_poller) {
    furi_assert(iso14443_3a_poller);

    MfUltralightPoller* instance = malloc(sizeof(MfUltralightPoller));
    instance->iso14443_3a_poller = iso14443_3a_poller;
    instance->tx_buffer = bit_buffer_alloc(MF_ULTRALIGHT_MAX_BUFF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(MF_ULTRALIGHT_MAX_BUFF_SIZE);
    instance->data = mf_ultralight_alloc();

    instance->mfu_event.data = &instance->mfu_event_data;

    instance->general_event.protocol = NfcProtocolMfUltralight;
    instance->general_event.event_data = &instance->mfu_event;
    instance->general_event.instance = instance;

    return instance;
}

void mf_ultralight_poller_free(MfUltralightPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    mf_ultralight_free(instance->data);
    free(instance);
}

static void mf_ultralight_poller_set_callback(
    MfUltralightPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

const MfUltralightData* mf_ultralight_poller_get_data(MfUltralightPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static NfcCommand mf_ultralight_poller_handler_idle(MfUltralightPoller* instance) {
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);
    iso14443_3a_copy(
        instance->data->iso14443_3a_data,
        iso14443_3a_poller_get_data(instance->iso14443_3a_poller));
    instance->counters_read = 0;
    instance->counters_total = 3;
    instance->tearing_flag_read = 0;
    instance->tearing_flag_total = 3;
    instance->pages_read = 0;
    instance->state = MfUltralightPollerStateRequestMode;
    instance->current_page = 0;
    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_request_mode(MfUltralightPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    instance->mfu_event.type = MfUltralightPollerEventTypeRequestMode;
    instance->mfu_event.data->poller_mode = MfUltralightPollerModeRead;

    command = instance->callback(instance->general_event, instance->context);
    instance->mode = instance->mfu_event.data->poller_mode;

    instance->state = MfUltralightPollerStateReadVersion;
    return command;
}

static NfcCommand mf_ultralight_poller_handler_read_version(MfUltralightPoller* instance) {
    instance->error = mf_ultralight_poller_read_version(instance, &instance->data->version);
    if(instance->error == MfUltralightErrorNone) {
        FURI_LOG_D(TAG, "Read version success");
        instance->data->type = mf_ultralight_get_type_by_version(&instance->data->version);
        instance->state = MfUltralightPollerStateGetFeatureSet;
    } else {
        FURI_LOG_D(TAG, "Didn't response. Check Ultralight C");
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
        instance->state = MfUltralightPollerStateDetectMfulC;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_check_ultralight_c(MfUltralightPoller* instance) {
    instance->error = mf_ultralight_poller_authenticate(instance);
    if(instance->error == MfUltralightErrorNone) {
        FURI_LOG_D(TAG, "Ultralight C detected");
        instance->data->type = MfUltralightTypeMfulC;
        instance->state = MfUltralightPollerStateGetFeatureSet;
    } else {
        FURI_LOG_D(TAG, "Didn't response. Check NTAG 203");
        instance->state = MfUltralightPollerStateDetectNtag203;
    }
    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_check_ntag_203(MfUltralightPoller* instance) {
    MfUltralightPollerState next_state = MfUltralightPollerStateGetFeatureSet;
    MfUltralightPageReadCommandData data = {};
    instance->error = mf_ultralight_poller_read_page(instance, 41, &data);
    if(instance->error == MfUltralightErrorNone) {
        FURI_LOG_D(TAG, "NTAG203 detected");
        instance->data->type = MfUltralightTypeNTAG203;
    } else {
        FURI_LOG_D(TAG, "Original Ultralight detected");
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
        instance->data->type = MfUltralightTypeUnknown;
        if(instance->mode == MfUltralightPollerModeWrite) {
            instance->mfu_event.type = MfUltralightPollerEventTypeCardMismatch;
            instance->callback(instance->general_event, instance->context);
            next_state = MfUltralightPollerStateWriteFail;
        }
    }
    instance->state = next_state;

    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_get_feature_set(MfUltralightPoller* instance) {
    instance->feature_set = mf_ultralight_get_feature_support_set(instance->data->type);
    instance->pages_total = mf_ultralight_get_pages_total(instance->data->type);
    instance->data->pages_total = instance->pages_total;
    FURI_LOG_D(
        TAG,
        "%s detected. Total pages: %d",
        mf_ultralight_get_device_name(instance->data, NfcDeviceNameTypeFull),
        instance->pages_total);

    instance->state = MfUltralightPollerStateReadSignature;
    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_read_signature(MfUltralightPoller* instance) {
    MfUltralightPollerState next_state = MfUltralightPollerStateAuth;
    if(mf_ultralight_support_feature(
           instance->feature_set, MfUltralightFeatureSupportReadSignature)) {
        FURI_LOG_D(TAG, "Reading signature");
        instance->error =
            mf_ultralight_poller_read_signature(instance, &instance->data->signature);
        if(instance->error != MfUltralightErrorNone) {
            FURI_LOG_D(TAG, "Read signature failed");
            next_state = MfUltralightPollerStateReadFailed;
        }
    } else {
        FURI_LOG_D(TAG, "Skip reading signature");
    }
    instance->state = next_state;

    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_read_counters(MfUltralightPoller* instance) {
    do {
        if(!mf_ultralight_support_feature(
               instance->feature_set, MfUltralightFeatureSupportReadCounter) ||
           !mf_ultralight_is_counter_configured(instance->data)) {
            FURI_LOG_D(TAG, "Skip reading counters");
            instance->state = MfUltralightPollerStateReadTearingFlags;
            break;
        }

        MfUltralightConfigPages* config = NULL;
        mf_ultralight_get_config_page(instance->data, &config);

        if(config->access.nfc_cnt_pwd_prot && !instance->auth_context.auth_success) {
            FURI_LOG_D(TAG, "Counter reading is protected with password");
            instance->state = MfUltralightPollerStateReadTearingFlags;
            break;
        }

        if(instance->counters_read == instance->counters_total) {
            instance->state = MfUltralightPollerStateReadTearingFlags;
            break;
        }

        if(mf_ultralight_support_feature(
               instance->feature_set, MfUltralightFeatureSupportSingleCounter)) {
            instance->counters_read = 2;
        }

        FURI_LOG_D(TAG, "Reading counter %d", instance->counters_read);
        instance->error = mf_ultralight_poller_read_counter(
            instance, instance->counters_read, &instance->data->counter[instance->counters_read]);
        if(instance->error != MfUltralightErrorNone) {
            FURI_LOG_D(TAG, "Failed to read %d counter", instance->counters_read);
            instance->state = MfUltralightPollerStateReadTearingFlags;
        } else {
            instance->counters_read++;
        }

    } while(false);

    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_read_tearing_flags(MfUltralightPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    if(mf_ultralight_support_feature(
           instance->feature_set,
           MfUltralightFeatureSupportCheckTearingFlag | MfUltralightFeatureSupportSingleCounter)) {
        if(instance->tearing_flag_read == instance->tearing_flag_total) {
            instance->state = MfUltralightPollerStateTryDefaultPass;
            command = NfcCommandReset;
        } else {
            bool single_counter = mf_ultralight_support_feature(
                instance->feature_set, MfUltralightFeatureSupportSingleCounter);
            if(single_counter) instance->tearing_flag_read = 2;

            FURI_LOG_D(TAG, "Reading tearing flag %d", instance->tearing_flag_read);
            instance->error = mf_ultralight_poller_read_tearing_flag(
                instance,
                instance->tearing_flag_read,
                &instance->data->tearing_flag[instance->tearing_flag_read]);
            if((instance->error == MfUltralightErrorProtocol) && single_counter) {
                instance->tearing_flag_read++;
            } else if(instance->error != MfUltralightErrorNone) {
                FURI_LOG_D(TAG, "Reading tearing flag %d failed", instance->tearing_flag_read);
                instance->state = MfUltralightPollerStateTryDefaultPass;
                command = NfcCommandReset;
            } else {
                instance->tearing_flag_read++;
            }
        }
    } else {
        FURI_LOG_D(TAG, "Skip reading tearing flags");
        instance->state = MfUltralightPollerStateTryDefaultPass;
        command = NfcCommandReset;
    }

    return command;
}

static NfcCommand mf_ultralight_poller_handler_auth(MfUltralightPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    if(mf_ultralight_support_feature(
           instance->feature_set, MfUltralightFeatureSupportPasswordAuth)) {
        instance->mfu_event.type = MfUltralightPollerEventTypeAuthRequest;

        command = instance->callback(instance->general_event, instance->context);
        if(!instance->mfu_event.data->auth_context.skip_auth) {
            instance->auth_context.password = instance->mfu_event.data->auth_context.password;
            uint32_t pass = nfc_util_bytes2num(
                instance->auth_context.password.data, sizeof(MfUltralightAuthPassword));
            FURI_LOG_D(TAG, "Trying to authenticate with password %08lX", pass);
            instance->error = mf_ultralight_poller_auth_pwd(instance, &instance->auth_context);
            if(instance->error == MfUltralightErrorNone) {
                FURI_LOG_D(TAG, "Auth success");
                instance->auth_context.auth_success = true;
                instance->mfu_event.data->auth_context = instance->auth_context;
                instance->mfu_event.type = MfUltralightPollerEventTypeAuthSuccess;
                command = instance->callback(instance->general_event, instance->context);
            } else {
                FURI_LOG_D(TAG, "Auth failed");
                instance->auth_context.auth_success = false;
                instance->mfu_event.type = MfUltralightPollerEventTypeAuthFailed;
                command = instance->callback(instance->general_event, instance->context);
                iso14443_3a_poller_halt(instance->iso14443_3a_poller);
            }
        }
    }
    instance->state = MfUltralightPollerStateReadPages;

    return command;
}

static NfcCommand mf_ultralight_poller_handler_read_pages(MfUltralightPoller* instance) {
    MfUltralightPageReadCommandData data = {};
    uint16_t start_page = instance->pages_read;
    if(MF_ULTRALIGHT_IS_NTAG_I2C(instance->data->type)) {
        uint8_t tag = 0;
        uint8_t sector = 0;
        uint8_t pages_left = 0;
        if(mf_ultralight_poller_ntag_i2c_addr_lin_to_tag(
               instance, start_page, &sector, &tag, &pages_left)) {
            instance->error =
                mf_ultralight_poller_read_page_from_sector(instance, sector, tag, &data);
        } else {
            FURI_LOG_D(TAG, "Failed to calculate sector and tag from %d page", start_page);
            instance->error = MfUltralightErrorProtocol;
        }
    } else {
        instance->error = mf_ultralight_poller_read_page(instance, start_page, &data);
    }

    if(instance->error == MfUltralightErrorNone) {
        for(size_t i = 0; i < 4; i++) {
            if(start_page + i < instance->pages_total) {
                FURI_LOG_D(TAG, "Read page %d success", start_page + i);
                instance->data->page[start_page + i] = data.page[i];
                instance->pages_read++;
                instance->data->pages_read = instance->pages_read;
            }
        }
        if(instance->pages_read == instance->pages_total) {
            instance->state = MfUltralightPollerStateReadCounters;
        }
    } else {
        FURI_LOG_D(TAG, "Read page %d failed", instance->pages_read);
        if(instance->pages_read) {
            instance->state = MfUltralightPollerStateReadCounters;
        } else {
            instance->state = MfUltralightPollerStateReadFailed;
        }
    }

    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_try_default_pass(MfUltralightPoller* instance) {
    do {
        if(!mf_ultralight_support_feature(
               instance->feature_set, MfUltralightFeatureSupportPasswordAuth))
            break;

        MfUltralightConfigPages* config = NULL;
        mf_ultralight_get_config_page(instance->data, &config);
        if(instance->auth_context.auth_success) {
            config->password = instance->auth_context.password;
            config->pack = instance->auth_context.pack;
        } else if(config->access.authlim == 0) {
            FURI_LOG_D(TAG, "No limits in authentication. Trying default password");
            nfc_util_num2bytes(
                MF_ULTRALIGHT_DEFAULT_PASSWORD,
                sizeof(MfUltralightAuthPassword),
                instance->auth_context.password.data);
            instance->error = mf_ultralight_poller_auth_pwd(instance, &instance->auth_context);
            if(instance->error == MfUltralightErrorNone) {
                FURI_LOG_D(TAG, "Default password detected");
                nfc_util_num2bytes(
                    MF_ULTRALIGHT_DEFAULT_PASSWORD,
                    sizeof(MfUltralightAuthPassword),
                    config->password.data);
                config->pack = instance->auth_context.pack;
                instance->auth_context.auth_success = true;
            }
        }

        if(instance->pages_read != instance->pages_total) {
            // Probably password protected, fix AUTH0 and PROT so before AUTH0
            // can be written and since AUTH0 won't be readable, like on the
            // original card
            config->auth0 = instance->pages_read;
            config->access.prot = true;
        } else if(!instance->auth_context.auth_success) {
            instance->pages_read -= 2;
            instance->data->pages_read -= 2;
        }
    } while(false);

    instance->state = MfUltralightPollerStateReadSuccess;
    return NfcCommandContinue;
}

static NfcCommand mf_ultralight_poller_handler_read_fail(MfUltralightPoller* instance) {
    FURI_LOG_D(TAG, "Read Failed");
    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    instance->mfu_event.type = MfUltralightPollerEventTypeReadFailed;
    instance->mfu_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = MfUltralightPollerStateIdle;
    return command;
}

static NfcCommand mf_ultralight_poller_handler_read_success(MfUltralightPoller* instance) {
    FURI_LOG_D(TAG, "Read success");
    instance->mfu_event.type = MfUltralightPollerEventTypeReadSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    if(instance->mode == MfUltralightPollerModeRead) {
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
        instance->state = MfUltralightPollerStateIdle;
    } else {
        instance->state = MfUltralightPollerStateRequestWriteData;
    }

    return command;
}

static NfcCommand mf_ultralight_poller_handler_request_write_data(MfUltralightPoller* instance) {
    FURI_LOG_D(TAG, "Check writing capability");
    NfcCommand command = NfcCommandContinue;
    MfUltralightPollerState next_state = MfUltralightPollerStateWritePages;
    instance->current_page = 4;

    instance->mfu_event.type = MfUltralightPollerEventTypeRequestWriteData;
    instance->callback(instance->general_event, instance->context);

    const MfUltralightData* write_data = instance->mfu_event.data->write_data;
    const MfUltralightData* tag_data = instance->data;
    uint32_t features = mf_ultralight_get_feature_support_set(tag_data->type);

    bool check_passed = false;
    do {
        if(write_data->type != tag_data->type) {
            FURI_LOG_D(TAG, "Incorrect tag type");
            instance->mfu_event.type = MfUltralightPollerEventTypeCardMismatch;
            break;
        }

        if(!instance->auth_context.auth_success) {
            FURI_LOG_D(TAG, "Unknown password");
            instance->mfu_event.type = MfUltralightPollerEventTypeCardLocked;
            break;
        }

        const MfUltralightPage staticlock_page = tag_data->page[2];
        if(staticlock_page.data[2] != 0 || staticlock_page.data[3] != 0) {
            FURI_LOG_D(TAG, "Static lock bits are set");
            instance->mfu_event.type = MfUltralightPollerEventTypeCardLocked;
            break;
        }

        if(mf_ultralight_support_feature(features, MfUltralightFeatureSupportDynamicLock)) {
            uint8_t dynlock_num = mf_ultralight_get_config_page_num(tag_data->type) - 1;
            const MfUltralightPage dynlock_page = tag_data->page[dynlock_num];
            if(dynlock_page.data[0] != 0 || dynlock_page.data[1] != 0) {
                FURI_LOG_D(TAG, "Dynamic lock bits are set");
                instance->mfu_event.type = MfUltralightPollerEventTypeCardLocked;
                break;
            }
        }

        check_passed = true;
    } while(false);

    if(!check_passed) {
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
        command = instance->callback(instance->general_event, instance->context);
        next_state = MfUltralightPollerStateWriteFail;
    }

    instance->state = next_state;
    return command;
}

static NfcCommand mf_ultralight_poller_handler_write_pages(MfUltralightPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    do {
        const MfUltralightData* write_data = instance->mfu_event.data->write_data;
        uint8_t end_page = mf_ultralight_get_config_page_num(write_data->type) - 1;
        if(instance->current_page == end_page) {
            instance->state = MfUltralightPollerStateWriteSuccess;
            break;
        }
        FURI_LOG_D(TAG, "Writing page %d", instance->current_page);
        MfUltralightError error = mf_ultralight_poller_write_page(
            instance, instance->current_page, &write_data->page[instance->current_page]);
        if(error != MfUltralightErrorNone) {
            instance->state = MfUltralightPollerStateWriteFail;
            instance->error = error;
            break;
        }
        instance->current_page++;
    } while(false);

    return command;
}

static NfcCommand mf_ultralight_poller_handler_write_fail(MfUltralightPoller* instance) {
    FURI_LOG_D(TAG, "Write failed");
    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    instance->mfu_event.data->error = instance->error;
    instance->mfu_event.type = MfUltralightPollerEventTypeWriteFail;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static NfcCommand mf_ultralight_poller_handler_write_success(MfUltralightPoller* instance) {
    FURI_LOG_D(TAG, "Write success");
    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    instance->mfu_event.type = MfUltralightPollerEventTypeWriteSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const MfUltralightPollerReadHandler
    mf_ultralight_poller_read_handler[MfUltralightPollerStateNum] = {
        [MfUltralightPollerStateIdle] = mf_ultralight_poller_handler_idle,
        [MfUltralightPollerStateRequestMode] = mf_ultralight_poller_handler_request_mode,
        [MfUltralightPollerStateReadVersion] = mf_ultralight_poller_handler_read_version,
        [MfUltralightPollerStateDetectMfulC] = mf_ultralight_poller_handler_check_ultralight_c,
        [MfUltralightPollerStateDetectNtag203] = mf_ultralight_poller_handler_check_ntag_203,
        [MfUltralightPollerStateGetFeatureSet] = mf_ultralight_poller_handler_get_feature_set,
        [MfUltralightPollerStateReadSignature] = mf_ultralight_poller_handler_read_signature,
        [MfUltralightPollerStateReadCounters] = mf_ultralight_poller_handler_read_counters,
        [MfUltralightPollerStateReadTearingFlags] =
            mf_ultralight_poller_handler_read_tearing_flags,
        [MfUltralightPollerStateAuth] = mf_ultralight_poller_handler_auth,
        [MfUltralightPollerStateTryDefaultPass] = mf_ultralight_poller_handler_try_default_pass,
        [MfUltralightPollerStateReadPages] = mf_ultralight_poller_handler_read_pages,
        [MfUltralightPollerStateReadFailed] = mf_ultralight_poller_handler_read_fail,
        [MfUltralightPollerStateReadSuccess] = mf_ultralight_poller_handler_read_success,
        [MfUltralightPollerStateRequestWriteData] =
            mf_ultralight_poller_handler_request_write_data,
        [MfUltralightPollerStateWritePages] = mf_ultralight_poller_handler_write_pages,
        [MfUltralightPollerStateWriteFail] = mf_ultralight_poller_handler_write_fail,
        [MfUltralightPollerStateWriteSuccess] = mf_ultralight_poller_handler_write_success,

};

static NfcCommand mf_ultralight_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    MfUltralightPoller* instance = context;
    furi_assert(instance->callback);

    const Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        command = mf_ultralight_poller_read_handler[instance->state](instance);
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        instance->mfu_event.type = MfUltralightPollerEventTypeReadFailed;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

static bool mf_ultralight_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    bool protocol_detected = false;
    MfUltralightPoller* instance = context;
    const Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        MfUltralightPageReadCommandData read_page_cmd_data = {};
        MfUltralightError error = mf_ultralight_poller_read_page(instance, 0, &read_page_cmd_data);
        protocol_detected = (error == MfUltralightErrorNone);
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    }

    return protocol_detected;
}

const NfcPollerBase mf_ultralight_poller = {
    .alloc = (NfcPollerAlloc)mf_ultralight_poller_alloc,
    .free = (NfcPollerFree)mf_ultralight_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_ultralight_poller_set_callback,
    .run = (NfcPollerRun)mf_ultralight_poller_run,
    .detect = (NfcPollerDetect)mf_ultralight_poller_detect,
    .get_data = (NfcPollerGetData)mf_ultralight_poller_get_data,
};
