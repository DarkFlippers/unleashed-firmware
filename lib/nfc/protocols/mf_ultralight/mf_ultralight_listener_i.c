#include "mf_ultralight_listener_i.h"

#include <furi.h>

#define MF_ULTRALIGHT_STATIC_BIT_LOCK_OTP_CC   0
#define MF_ULTRALIGHT_STATIC_BIT_LOCK_BL_9_4   1
#define MF_ULTRALIGHT_STATIC_BIT_LOCK_BL_15_10 2

#define MF_ULTRALIGHT_BIT_ACTIVE(lock_bits, bit) (((lock_bits) & (1U << (bit))) != 0)
#define MF_ULTRALIGHT_BITS_SET(lock_bits, mask)  ((lock_bits) |= (mask))
#define MF_ULTRALIGHT_BITS_CLR(lock_bits, mask)  ((lock_bits) &= ~(mask))

#define MF_ULTRALIGHT_PAGE_LOCKED(lock_bits, page) MF_ULTRALIGHT_BIT_ACTIVE(lock_bits, page)

#define MF_ULTRALIGHT_STATIC_BIT_OTP_CC_LOCKED(lock_bits) \
    MF_ULTRALIGHT_BIT_ACTIVE(lock_bits, MF_ULTRALIGHT_STATIC_BIT_LOCK_OTP_CC)

#define MF_ULTRALIGHT_STATIC_BITS_9_4_LOCKED(lock_bits) \
    MF_ULTRALIGHT_BIT_ACTIVE(lock_bits, MF_ULTRALIGHT_STATIC_BIT_LOCK_BL_9_4)

#define MF_ULTRALIGHT_STATIC_BITS_15_10_LOCKED(lock_bits) \
    MF_ULTRALIGHT_BIT_ACTIVE(lock_bits, MF_ULTRALIGHT_STATIC_BIT_LOCK_BL_15_10)

#define MF_ULTRALIGHT_STATIC_LOCK_L_OTP_CC_MASK (1U << 3)
#define MF_ULTRALIGHT_STATIC_LOCK_L_9_4_MASK \
    ((1U << 9) | (1U << 8) | (1U << 7) | (1U << 6) | (1U << 5) | (1U << 4))
#define MF_ULTRALIGHT_STATIC_LOCK_L_15_10_MASK \
    ((1U << 15) | (1U << 14) | (1U << 13) | (1U << 12) | (1U << 11) | (1U << 10))

#define MF_ULTRALIGHT_PAGE_IN_BOUNDS(page, start, end) (((page) >= (start)) && ((page) <= (end)))

#define MF_ULTRALIGHT_I2C_PAGE_ON_SESSION_REG(page) \
    MF_ULTRALIGHT_PAGE_IN_BOUNDS(page, 0x00EC, 0x00ED)

#define MF_ULTRALIGHT_I2C_PAGE_ON_MIRRORED_SESSION_REG(page) \
    MF_ULTRALIGHT_PAGE_IN_BOUNDS(page, 0x00F8, 0x00F9)

#define MF_ULTRALIGHT_AUTH_RESET_ATTEMPTS(instance)    (instance->data->auth_attempts = 0)
#define MF_ULTRALIGHT_AUTH_INCREASE_ATTEMPTS(instance) (instance->data->auth_attempts++)

static MfUltralightMirrorConf mf_ultralight_mirror_check_mode(
    const MfUltralightConfigPages* const config,
    const MfUltralightListenerAuthState auth_state) {
    MfUltralightMirrorConf mirror_mode = config->mirror.mirror_conf;

    if(mirror_mode == MfUltralightMirrorNone || mirror_mode == MfUltralightMirrorUid)
        return mirror_mode;

    if(!config->access.nfc_cnt_en ||
       (config->access.nfc_cnt_pwd_prot && auth_state != MfUltralightListenerAuthStateSuccess)) {
        mirror_mode = mirror_mode == MfUltralightMirrorCounter ? MfUltralightMirrorNone :
                                                                 MfUltralightMirrorUid;
    }
    return mirror_mode;
}

static bool mf_ultralight_mirror_check_boundaries(MfUltralightListener* instance) {
    const MfUltralightConfigPages* const conf = instance->config;

    uint8_t last_user_page = mf_ultralight_get_config_page_num(instance->data->type) - 2;

    uint8_t max_page_offset = 0;
    uint8_t max_byte_offset = 2;

    MfUltralightMirrorConf mode = mf_ultralight_mirror_check_mode(conf, instance->auth_state);

    bool result = false;
    bool again = false;
    do {
        if(mode == MfUltralightMirrorNone) {
            break;
        } else if(mode == MfUltralightMirrorUid) {
            max_page_offset = 3;
        } else if(mode == MfUltralightMirrorCounter) {
            max_page_offset = 1;
        } else if(mode == MfUltralightMirrorUidCounter) {
            max_page_offset = 5;
            max_byte_offset = 3;
        }

        instance->mirror.actual_mode = mode;

        if(conf->mirror_page <= 3) break;
        if(conf->mirror_page < last_user_page - max_page_offset) {
            result = true;
            break;
        }
        if(conf->mirror_page == last_user_page - max_page_offset) {
            result = (conf->mirror.mirror_byte <= max_byte_offset);
            break;
        }

        if(conf->mirror_page > last_user_page - max_page_offset &&
           mode == MfUltralightMirrorUidCounter) {
            mode = MfUltralightMirrorUid;
            again = true;
        }
    } while(again);

    return result;
}

static bool mf_ultralight_mirror_enabled(MfUltralightListener* instance) {
    bool mirror_enabled = false;
    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportAsciiMirror) &&
       (instance->config != NULL) && mf_ultralight_mirror_check_boundaries(instance)) {
        mirror_enabled = true;
    }
    instance->mirror.enabled = mirror_enabled;
    return instance->mirror.enabled;
}

static uint8_t mf_ultralight_get_mirror_data_size(MfUltralightMirrorConf mode) {
    switch(mode) {
    case MfUltralightMirrorUid:
        return 14;
    case MfUltralightMirrorCounter:
        return 6;
    case MfUltralightMirrorUidCounter:
        return 21;
    default:
        return 0;
    }
}

static uint8_t mf_ultralight_get_mirror_last_page(MfUltralightListener* instance) {
    uint8_t strSize = mf_ultralight_get_mirror_data_size(instance->mirror.actual_mode);
    return instance->config->mirror_page + 1U + strSize / 4;
}

static uint8_t mf_ultralight_get_ascii_offset(uint8_t start_page, MfUltralightListener* instance) {
    uint8_t start_offset = 0;
    if(instance->config->mirror.mirror_conf == MfUltralightMirrorCounter) start_offset = 15;

    uint8_t ascii_offset = start_offset;

    if(start_page > instance->config->mirror_page)
        ascii_offset = (start_page - instance->config->mirror_page) * 4 -
                       instance->config->mirror.mirror_byte + start_offset;

    return ascii_offset;
}

static uint8_t mf_ultralight_get_ascii_end(MfUltralightMirrorConf mode) {
    return (mode == MfUltralightMirrorUid) ? 14 : 21;
}

static uint8_t mf_ultralight_get_byte_offset(
    uint8_t current_page,
    const MfUltralightConfigPages* const config) {
    return (current_page > config->mirror_page) ? 0 : config->mirror.mirror_byte;
}

static void mf_ultralight_format_mirror_data(
    FuriString* str,
    const uint8_t* const data,
    const uint8_t data_len) {
    for(uint8_t i = 0; i < data_len; i++)
        furi_string_cat_printf(str, "%02X", data[i]);
}

void mf_ultralight_mirror_read_prepare(uint8_t start_page, MfUltralightListener* instance) {
    if(mf_ultralight_mirror_enabled(instance)) {
        instance->mirror.ascii_offset = mf_ultralight_get_ascii_offset(start_page, instance);
        instance->mirror.ascii_end = mf_ultralight_get_ascii_end(instance->mirror.actual_mode);

        instance->mirror.mirror_last_page = mf_ultralight_get_mirror_last_page(instance);
    }
}

void mf_ultralight_mirror_read_handler(
    uint8_t mirror_page_num,
    uint8_t* dest,
    MfUltralightListener* instance) {
    if(instance->mirror.enabled && mirror_page_num >= instance->config->mirror_page &&
       mirror_page_num <= instance->mirror.mirror_last_page) {
        uint8_t byte_offset = mf_ultralight_get_byte_offset(mirror_page_num, instance->config);

        uint8_t ascii_offset = instance->mirror.ascii_offset;
        uint8_t ascii_end = instance->mirror.ascii_end;
        uint8_t* source = (uint8_t*)furi_string_get_cstr(instance->mirror.ascii_mirror_data);
        for(uint8_t j = byte_offset; (j < 4) && (ascii_offset < ascii_end); j++) {
            dest[j] = source[ascii_offset];
            ascii_offset++;
        }
        instance->mirror.ascii_offset = ascii_offset;
    }
}

void mf_ultralight_mirror_prepare_emulation(MfUltralightListener* instance) {
    mf_ultralight_format_mirror_data(
        instance->mirror.ascii_mirror_data,
        instance->data->iso14443_3a_data->uid,
        instance->data->iso14443_3a_data->uid_len);

    furi_string_push_back(instance->mirror.ascii_mirror_data, 'x');

    mf_ultraligt_mirror_format_counter(instance);
}

void mf_ultraligt_mirror_format_counter(MfUltralightListener* instance) {
    furi_string_left(
        instance->mirror.ascii_mirror_data, instance->data->iso14443_3a_data->uid_len * 2 + 1);

    uint8_t* c = instance->data->counter[2].data;
    furi_string_cat_printf(instance->mirror.ascii_mirror_data, "%02X%02X%02X", c[2], c[1], c[0]);
}

bool mf_ultralight_composite_command_in_progress(MfUltralightListener* instance) {
    return instance->composite_cmd.callback != NULL;
}

MfUltralightCommand
    mf_ultralight_composite_command_run(MfUltralightListener* instance, BitBuffer* buffer) {
    MfUltralightCommand command = (instance->composite_cmd.callback)(instance, buffer);
    mf_ultralight_composite_command_reset(instance);
    return command;
}

void mf_ultralight_composite_command_reset(MfUltralightListener* instance) {
    instance->composite_cmd.callback = NULL;
    instance->composite_cmd.data = 0;
}

void mf_ultralight_composite_command_set_next(
    MfUltralightListener* instance,
    const MfUltralightListenerCommandCallback handler) {
    instance->composite_cmd.callback = handler;
}

void mf_ultralight_single_counter_try_increase(MfUltralightListener* instance) {
    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportSingleCounter) &&
       instance->config->access.nfc_cnt_en && !instance->single_counter_increased) {
        if(instance->data->counter[2].counter < MF_ULTRALIGHT_MAX_CNTR_VAL) {
            instance->data->counter[2].counter++;
            mf_ultraligt_mirror_format_counter(instance);
        }
        instance->single_counter_increased = true;
    }
}

void mf_ultralight_single_counter_try_to_unlock(
    MfUltralightListener* instance,
    Iso14443_3aListenerEventType type) {
    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportSingleCounter) &&
       type == Iso14443_3aListenerEventTypeFieldOff) {
        instance->single_counter_increased = false;
    }
}

static bool mf_ultralight_i2c_page_validator_for_sector0(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightType type) {
    UNUSED(type);
    bool valid = false;
    if(type == MfUltralightTypeNTAGI2CPlus1K || type == MfUltralightTypeNTAGI2CPlus2K) {
        if(start_page <= 0xE9 && end_page <= 0xE9) {
            valid = true;
        } else if(
            MF_ULTRALIGHT_I2C_PAGE_ON_SESSION_REG(start_page) &&
            MF_ULTRALIGHT_I2C_PAGE_ON_SESSION_REG(end_page)) {
            valid = true;
        }
    } else if(type == MfUltralightTypeNTAGI2C1K) {
        if((start_page <= 0xE2) || MF_ULTRALIGHT_PAGE_IN_BOUNDS(start_page, 0x00E8, 0x00E9)) {
            valid = true;
        }
    } else if(type == MfUltralightTypeNTAGI2C2K) {
        valid = (start_page <= 0xFF && end_page <= 0xFF);
    }

    return valid;
}

static bool mf_ultralight_i2c_page_validator_for_sector1(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightType type) {
    bool valid = false;
    if(type == MfUltralightTypeNTAGI2CPlus2K) {
        valid = (start_page <= 0xFF && end_page <= 0xFF);
    } else if(type == MfUltralightTypeNTAGI2C2K) {
        valid = (MF_ULTRALIGHT_PAGE_IN_BOUNDS(start_page, 0x00E8, 0x00E9) || (start_page <= 0xE0));
    } else if(type == MfUltralightTypeNTAGI2C1K || type == MfUltralightTypeNTAGI2CPlus1K) {
        valid = false;
    }

    return valid;
}

static bool mf_ultralight_i2c_page_validator_for_sector2(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightType type) {
    UNUSED(start_page);
    UNUSED(end_page);
    UNUSED(type);
    return false;
}

static bool mf_ultralight_i2c_page_validator_for_sector3(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightType type) {
    UNUSED(type);
    UNUSED(end_page);
    return MF_ULTRALIGHT_I2C_PAGE_ON_MIRRORED_SESSION_REG(start_page);
}

typedef bool (
    *MfUltralightI2CValidator)(uint16_t start_page, uint16_t end_page, MfUltralightType type);

typedef uint16_t (*MfUltralightI2CPageProvider)(uint16_t page, MfUltralightType type);

const MfUltralightI2CValidator validation_methods[] = {
    mf_ultralight_i2c_page_validator_for_sector0,
    mf_ultralight_i2c_page_validator_for_sector1,
    mf_ultralight_i2c_page_validator_for_sector2,
    mf_ultralight_i2c_page_validator_for_sector3,
};

bool mf_ultralight_i2c_validate_pages(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightListener* instance) {
    bool valid = false;
    if(instance->sector < COUNT_OF(validation_methods)) {
        MfUltralightI2CValidator validate = validation_methods[instance->sector];
        valid = validate(start_page, end_page, instance->data->type);
    }
    return valid;
}

bool mf_ultralight_is_i2c_tag(MfUltralightType type) {
    return type == MfUltralightTypeNTAGI2C1K || type == MfUltralightTypeNTAGI2C2K ||
           type == MfUltralightTypeNTAGI2CPlus1K || type == MfUltralightTypeNTAGI2CPlus2K;
}

static uint16_t mf_ultralight_i2c_page_provider_for_sector0(uint16_t page, MfUltralightType type) {
    uint8_t new_page = page;
    if(type == MfUltralightTypeNTAGI2CPlus1K || type == MfUltralightTypeNTAGI2CPlus2K) {
        if(page == 0x00EC) {
            new_page = 234;
        } else if(page == 0x00ED) {
            new_page = 235;
        }
    } else if(type == MfUltralightTypeNTAGI2C1K) {
        if(page == 0x00E8) {
            new_page = 232;
        } else if(page == 0x00E9) {
            new_page = 233;
        }
    } else if(type == MfUltralightTypeNTAGI2C2K) {
        new_page = page;
    }
    return new_page;
}

static uint16_t mf_ultralight_i2c_page_provider_for_sector1(uint16_t page, MfUltralightType type) {
    UNUSED(type);
    uint16_t new_page = page;
    if(type == MfUltralightTypeNTAGI2CPlus2K) new_page = page + 236;
    if(type == MfUltralightTypeNTAGI2C2K) new_page = page + 256;
    return new_page;
}

static uint16_t mf_ultralight_i2c_page_provider_for_sector2(uint16_t page, MfUltralightType type) {
    UNUSED(type);
    return page;
}

static uint16_t mf_ultralight_i2c_page_provider_for_sector3(uint16_t page, MfUltralightType type) {
    uint16_t new_page = page;
    if(type == MfUltralightTypeNTAGI2CPlus1K || type == MfUltralightTypeNTAGI2CPlus2K) {
        if(page == 0x00F8)
            new_page = 234;
        else if(page == 0x00F9)
            new_page = 235;
    } else if(type == MfUltralightTypeNTAGI2C1K || type == MfUltralightTypeNTAGI2C2K) {
        if(page == 0x00F8)
            new_page = (type == MfUltralightTypeNTAGI2C1K) ? 227 : 481;
        else if(page == 0x00F9)
            new_page = (type == MfUltralightTypeNTAGI2C1K) ? 228 : 482;
    }
    return new_page;
}

const MfUltralightI2CPageProvider provider_methods[] = {
    mf_ultralight_i2c_page_provider_for_sector0,
    mf_ultralight_i2c_page_provider_for_sector1,
    mf_ultralight_i2c_page_provider_for_sector2,
    mf_ultralight_i2c_page_provider_for_sector3,
};

uint16_t
    mf_ultralight_i2c_provide_page_by_requested(uint16_t page, MfUltralightListener* instance) {
    uint16_t result = page;
    if(instance->sector < COUNT_OF(provider_methods)) {
        MfUltralightI2CPageProvider provider = provider_methods[instance->sector];
        result = provider(page, instance->data->type);
    }
    return result;
}

void mf_ultralight_static_lock_bytes_prepare(MfUltralightListener* instance) {
    instance->static_lock = (uint16_t*)&instance->data->page[2].data[2];
}

void mf_ultralight_static_lock_bytes_write(
    MfUltralightStaticLockData* const lock_bits,
    uint16_t new_bits) {
    uint16_t current_locks = *lock_bits;

    if(MF_ULTRALIGHT_STATIC_BIT_OTP_CC_LOCKED(current_locks))
        MF_ULTRALIGHT_BITS_CLR(new_bits, MF_ULTRALIGHT_STATIC_LOCK_L_OTP_CC_MASK);

    if(MF_ULTRALIGHT_STATIC_BITS_9_4_LOCKED(current_locks))
        MF_ULTRALIGHT_BITS_CLR(new_bits, MF_ULTRALIGHT_STATIC_LOCK_L_9_4_MASK);

    if(MF_ULTRALIGHT_STATIC_BITS_15_10_LOCKED(current_locks))
        MF_ULTRALIGHT_BITS_CLR(new_bits, MF_ULTRALIGHT_STATIC_LOCK_L_15_10_MASK);

    MF_ULTRALIGHT_BITS_SET(current_locks, new_bits);
    *lock_bits = current_locks;
}

bool mf_ultralight_static_lock_check_page(
    const MfUltralightStaticLockData* const lock_bits,
    uint16_t page) {
    bool locked = false;
    if(MF_ULTRALIGHT_PAGE_IN_BOUNDS(page, 0x0003, 0x000F)) {
        uint16_t current_locks = *lock_bits;
        locked = MF_ULTRALIGHT_PAGE_LOCKED(current_locks, page);
    }
    return locked;
}

void mf_ultralight_capability_container_write(
    MfUltralightPage* const current_page,
    const uint8_t* const new_data) {
    for(uint8_t i = 0; i < MF_ULTRALIGHT_PAGE_SIZE; i++) {
        current_page->data[i] |= new_data[i];
    }
}

static uint16_t mf_ultralight_dynamic_lock_page_num(const MfUltralightData* data) {
    uint16_t lock_page;
    if(data->type == MfUltralightTypeNTAGI2C1K)
        lock_page = 226;
    else if(data->type == MfUltralightTypeNTAGI2C2K)
        lock_page = 480;
    else
        lock_page = mf_ultralight_get_config_page_num(data->type) - 1;
    return lock_page;
}

void mf_ultralight_dynamic_lock_bytes_prepare(MfUltralightListener* instance) {
    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportDynamicLock)) {
        uint16_t lock_page = mf_ultralight_dynamic_lock_page_num(instance->data);
        instance->dynamic_lock = (uint32_t*)instance->data->page[lock_page].data;
    } else {
        instance->dynamic_lock = NULL;
    }
}

bool mf_ultralight_is_page_dynamic_lock(const MfUltralightListener* instance, uint16_t page) {
    bool is_lock = false;
    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportDynamicLock)) {
        uint16_t linear_page = page + instance->sector * 256;

        uint16_t lock_page = mf_ultralight_dynamic_lock_page_num(instance->data);
        is_lock = linear_page == lock_page;
    }
    return is_lock;
}

void mf_ultralight_dynamic_lock_bytes_write(
    MfUltralightDynamicLockData* const lock_bits,
    uint32_t new_bits) {
    furi_assert(lock_bits != NULL);
    new_bits &= 0x00FFFFFF;
    uint32_t current_lock = *lock_bits;
    for(uint8_t i = 0; i < 8; i++) {
        uint8_t bl_bit = i + 16;

        if(MF_ULTRALIGHT_BIT_ACTIVE(current_lock, bl_bit)) {
            uint8_t lock_bit = i * 2;
            uint32_t mask = (1U << lock_bit) | (1U << (lock_bit + 1));
            MF_ULTRALIGHT_BITS_CLR(new_bits, mask);
        }
    }
    MF_ULTRALIGHT_BITS_SET(current_lock, new_bits);
    *lock_bits = current_lock;
}

static uint8_t mf_ultralight_dynamic_lock_granularity(MfUltralightType type) {
    switch(type) {
    case MfUltralightTypeUL21:
    case MfUltralightTypeNTAG213:
        return 2;
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
    case MfUltralightTypeNTAGI2C1K:
    case MfUltralightTypeNTAGI2CPlus1K:
        return 16;
    case MfUltralightTypeNTAGI2C2K:
    case MfUltralightTypeNTAGI2CPlus2K:
        return 32;
    default:
        return 1;
    }
}

static uint16_t mf_ultralight_get_upper_page_bound(MfUltralightType type) {
    uint16_t upper_page_bound;

    if(type == MfUltralightTypeNTAGI2CPlus2K)
        upper_page_bound = 511;
    else if(type == MfUltralightTypeNTAGI2C2K)
        upper_page_bound = 479;
    else if(type == MfUltralightTypeNTAGI2C1K)
        upper_page_bound = 225;
    else {
        upper_page_bound = mf_ultralight_get_config_page_num(type) - 2;
    }

    return upper_page_bound;
}

bool mf_ultralight_dynamic_lock_check_page(const MfUltralightListener* instance, uint16_t page) {
    UNUSED(page);
    bool locked = false;
    uint16_t upper_page_bound = mf_ultralight_get_upper_page_bound(instance->data->type);
    uint16_t linear_page = page + instance->sector * 256;

    if(mf_ultralight_support_feature(instance->features, MfUltralightFeatureSupportDynamicLock) &&
       MF_ULTRALIGHT_PAGE_IN_BOUNDS(linear_page, 0x0010, upper_page_bound)) {
        uint8_t granularity = mf_ultralight_dynamic_lock_granularity(instance->data->type);
        uint8_t bit = (linear_page - 16) / granularity;
        uint16_t current_locks = *instance->dynamic_lock;
        locked = MF_ULTRALIGHT_PAGE_LOCKED(current_locks, bit);
    }
    return locked;
}

static bool mf_ultralight_auth_check_attempts(const MfUltralightListener* instance) {
    uint8_t authlim = ((instance->data->type == MfUltralightTypeNTAGI2CPlus1K) ||
                       (instance->data->type == MfUltralightTypeNTAGI2CPlus2K)) ?
                          (1U << instance->config->access.authlim) :
                          instance->config->access.authlim;

    return instance->data->auth_attempts >= authlim;
}

bool mf_ultralight_auth_limit_check_and_update(MfUltralightListener* instance, bool auth_success) {
    bool card_locked = false;

    do {
        if(instance->config->access.authlim == 0) break;
        card_locked = mf_ultralight_auth_check_attempts(instance);
        if(card_locked) break;

        if(auth_success) {
            MF_ULTRALIGHT_AUTH_RESET_ATTEMPTS(instance);
        } else {
            MF_ULTRALIGHT_AUTH_INCREASE_ATTEMPTS(instance);
        }

        card_locked = mf_ultralight_auth_check_attempts(instance);
    } while(false);

    return card_locked;
}

bool mf_ultralight_auth_check_password(
    const MfUltralightAuthPassword* config_pass,
    const MfUltralightAuthPassword* auth_pass) {
    return memcmp(config_pass->data, auth_pass->data, sizeof(MfUltralightAuthPassword)) == 0;
}

bool mf_ultralight_common_check_access(
    const MfUltralightListener* instance,
    const uint16_t start_page,
    const MfUltralightListenerAccessType access_type) {
    bool access_success = false;
    bool is_write_op = (access_type == MfUltralightListenerAccessTypeWrite);

    do {
        if(instance->auth_state != MfUltralightListenerAuthStateSuccess) {
            if((instance->config->auth0 <= start_page) &&
               (instance->config->access.prot || is_write_op)) {
                break;
            }
        }

        if(instance->config->access.cfglck && is_write_op) {
            uint16_t config_page_start = instance->data->pages_total - 4;
            if((start_page == config_page_start) || (start_page == config_page_start + 1)) {
                break;
            }
        }

        access_success = true;
    } while(false);

    return access_success;
}

bool mf_ultralight_c_check_access(
    const MfUltralightData* data,
    const uint16_t start_page,
    const MfUltralightListenerAccessType access_type,
    const MfUltralightListenerAuthState auth_state) {
    bool access_success = false;
    bool is_write_op = (access_type == MfUltralightListenerAccessTypeWrite);

    do {
        if(start_page >= 44) break;

        const uint8_t auth0 = data->page[42].data[0];
        const uint8_t auth1 = data->page[43].data[0] & 0x01;

        if(auth0 < 0x03 || auth0 >= 0x30 || auth_state == MfUltralightListenerAuthStateSuccess) {
            access_success = true;
            break;
        }

        if((auth0 <= start_page) && ((auth1 == 0) || (auth1 == 1 || is_write_op))) { //-V560
            break;
        }

        access_success = true;
    } while(false);

    return access_success;
}
