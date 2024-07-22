#pragma once

#include "mf_ultralight_listener.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_listener.h>
#include <nfc/protocols/nfc_generic_event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MfUltralightListenerAuthStateIdle,
    MfUltralightListenerAuthStateSuccess,
} MfUltralightListenerAuthState;

typedef enum {
    MfUltralightListenerAccessTypeRead,
    MfUltralightListenerAccessTypeWrite,
} MfUltralightListenerAccessType;

typedef enum {
    MfUltralightCommandNotFound,
    MfUltralightCommandProcessed,
    MfUltralightCommandProcessedACK,
    MfUltralightCommandProcessedSilent,
    MfUltralightCommandNotProcessedNAK,
    MfUltralightCommandNotProcessedSilent,
    MfUltralightCommandNotProcessedAuthNAK,
} MfUltralightCommand;

typedef MfUltralightCommand (
    *MfUltralightListenerCommandCallback)(MfUltralightListener* instance, BitBuffer* buf);

typedef uint8_t MfUltralightListenerCompositeCommandData;

typedef struct {
    MfUltralightListenerCompositeCommandData data;
    MfUltralightListenerCommandCallback callback;
} MfUltralightListenerCompositeCommandContext;

typedef struct {
    uint8_t enabled;
    uint8_t ascii_offset;
    uint8_t ascii_end;
    uint8_t mirror_last_page;
    MfUltralightMirrorConf actual_mode;
    FuriString* ascii_mirror_data;
} MfUltralightMirrorMode;

typedef uint16_t MfUltralightStaticLockData;
typedef uint32_t MfUltralightDynamicLockData;

struct MfUltralightListener {
    Iso14443_3aListener* iso14443_3a_listener;
    MfUltralightListenerAuthState auth_state;
    MfUltralightData* data;
    BitBuffer* tx_buffer;
    MfUltralightFeatureSupport features;
    MfUltralightConfigPages* config;
    MfUltralightStaticLockData* static_lock;
    MfUltralightDynamicLockData* dynamic_lock;

    NfcGenericEvent generic_event;
    MfUltralightListenerEvent mfu_event;
    MfUltralightListenerEventData mfu_event_data;
    NfcGenericCallback callback;
    uint8_t sector;
    bool single_counter_increased;
    MfUltralightMirrorMode mirror;
    MfUltralightListenerCompositeCommandContext composite_cmd;
    mbedtls_des3_context des_context;
    uint8_t rndB[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE];
    uint8_t encB[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE];
    void* context;
};

void mf_ultralight_single_counter_try_increase(MfUltralightListener* instance);
void mf_ultralight_single_counter_try_to_unlock(
    MfUltralightListener* instance,
    Iso14443_3aListenerEventType type);

void mf_ultralight_mirror_prepare_emulation(MfUltralightListener* instance);
void mf_ultraligt_mirror_format_counter(MfUltralightListener* instance);
void mf_ultralight_mirror_read_prepare(uint8_t start_page, MfUltralightListener* instance);
void mf_ultralight_mirror_read_handler(
    uint8_t mirror_page_num,
    uint8_t* dest,
    MfUltralightListener* instance);

void mf_ultralight_composite_command_set_next(
    MfUltralightListener* instance,
    const MfUltralightListenerCommandCallback handler);
void mf_ultralight_composite_command_reset(MfUltralightListener* instance);
bool mf_ultralight_composite_command_in_progress(MfUltralightListener* instance);
MfUltralightCommand
    mf_ultralight_composite_command_run(MfUltralightListener* instance, BitBuffer* buffer);

bool mf_ultralight_is_i2c_tag(MfUltralightType type);
bool mf_ultralight_i2c_validate_pages(
    uint16_t start_page,
    uint16_t end_page,
    MfUltralightListener* instance);

uint16_t
    mf_ultralight_i2c_provide_page_by_requested(uint16_t page, MfUltralightListener* instance);

void mf_ultralight_static_lock_bytes_prepare(MfUltralightListener* instance);
void mf_ultralight_static_lock_bytes_write(
    MfUltralightStaticLockData* const lock_bits,
    uint16_t new_bits);
bool mf_ultralight_static_lock_check_page(
    const MfUltralightStaticLockData* const lock_bits,
    uint16_t page);

void mf_ultralight_capability_container_write(
    MfUltralightPage* const current_page,
    const uint8_t* const new_data);

void mf_ultralight_dynamic_lock_bytes_prepare(MfUltralightListener* instance);
bool mf_ultralight_is_page_dynamic_lock(const MfUltralightListener* instance, uint16_t start_page);
void mf_ultralight_dynamic_lock_bytes_write(
    MfUltralightDynamicLockData* const lock_bits,
    uint32_t new_bits);
bool mf_ultralight_dynamic_lock_check_page(const MfUltralightListener* instance, uint16_t page);
bool mf_ultralight_auth_limit_check_and_update(MfUltralightListener* instance, bool auth_success);
bool mf_ultralight_auth_check_password(
    const MfUltralightAuthPassword* config_pass,
    const MfUltralightAuthPassword* auth_pass);

bool mf_ultralight_common_check_access(
    const MfUltralightListener* instance,
    const uint16_t start_page,
    const MfUltralightListenerAccessType access_type);

bool mf_ultralight_c_check_access(
    const MfUltralightData* data,
    const uint16_t start_page,
    const MfUltralightListenerAccessType access_type,
    const MfUltralightListenerAuthState auth_state);
#ifdef __cplusplus
}
#endif
