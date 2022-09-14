#include <furi.h>
#include "u2f.h"
#include "u2f_hid.h"
#include "u2f_data.h"
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <littlefs/lfs_util.h> // for lfs_tobe32

#include "toolbox/sha256.h"
#include "toolbox/hmac_sha256.h"
#include "micro-ecc/uECC.h"

#define TAG "U2F"
#define WORKER_TAG TAG "Worker"

#define U2F_CMD_REGISTER 0x01
#define U2F_CMD_AUTHENTICATE 0x02
#define U2F_CMD_VERSION 0x03

typedef enum {
    U2fCheckOnly = 0x07, // "check-only" - only check key handle, don't send auth response
    U2fEnforce =
        0x03, // "enforce-user-presence-and-sign" - send auth response only if user is present
    U2fDontEnforce =
        0x08, // "dont-enforce-user-presence-and-sign" - send auth response even if user is missing
} U2fAuthMode;

typedef struct {
    uint8_t format;
    uint8_t xy[64];
} __attribute__((packed)) U2fPubKey;

typedef struct {
    uint8_t len;
    uint8_t hash[32];
    uint8_t nonce[32];
} __attribute__((packed)) U2fKeyHandle;

typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t len[3];
    uint8_t challenge[32];
    uint8_t app_id[32];
} __attribute__((packed)) U2fRegisterReq;

typedef struct {
    uint8_t reserved;
    U2fPubKey pub_key;
    U2fKeyHandle key_handle;
    uint8_t cert[];
} __attribute__((packed)) U2fRegisterResp;

typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t len[3];
    uint8_t challenge[32];
    uint8_t app_id[32];
    U2fKeyHandle key_handle;
} __attribute__((packed)) U2fAuthReq;

typedef struct {
    uint8_t user_present;
    uint32_t counter;
    uint8_t signature[];
} __attribute__((packed)) U2fAuthResp;

static const uint8_t ver_str[] = {"U2F_V2"};

static const uint8_t state_no_error[] = {0x90, 0x00};
static const uint8_t state_not_supported[] = {0x6D, 0x00};
static const uint8_t state_user_missing[] = {0x69, 0x85};
static const uint8_t state_wrong_data[] = {0x6A, 0x80};

struct U2fData {
    uint8_t device_key[32];
    uint8_t cert_key[32];
    uint32_t counter;
    const struct uECC_Curve_t* p_curve;
    bool ready;
    bool user_present;
    U2fEvtCallback callback;
    void* context;
};

static int u2f_uecc_random(uint8_t* dest, unsigned size) {
    furi_hal_random_fill_buf(dest, size);
    return 1;
}

U2fData* u2f_alloc() {
    return malloc(sizeof(U2fData));
}

void u2f_free(U2fData* U2F) {
    furi_assert(U2F);
    free(U2F);
}

bool u2f_init(U2fData* U2F) {
    furi_assert(U2F);

    if(u2f_data_cert_check() == false) {
        FURI_LOG_E(TAG, "Certificate load error");
        return false;
    }
    if(u2f_data_cert_key_load(U2F->cert_key) == false) {
        FURI_LOG_E(TAG, "Certificate key load error");
        return false;
    }
    if(u2f_data_key_load(U2F->device_key) == false) {
        FURI_LOG_W(TAG, "Key loading error, generating new");
        if(u2f_data_key_generate(U2F->device_key) == false) {
            FURI_LOG_E(TAG, "Key write failed");
            return false;
        }
    }
    if(u2f_data_cnt_read(&U2F->counter) == false) {
        FURI_LOG_W(TAG, "Counter loading error, resetting counter");
        U2F->counter = 0;
        if(u2f_data_cnt_write(0) == false) {
            FURI_LOG_E(TAG, "Counter write failed");
            return false;
        }
    }

    U2F->p_curve = uECC_secp256r1();
    uECC_set_rng(u2f_uecc_random);

    U2F->ready = true;
    return true;
}

void u2f_set_event_callback(U2fData* U2F, U2fEvtCallback callback, void* context) {
    furi_assert(U2F);
    furi_assert(callback);
    U2F->callback = callback;
    U2F->context = context;
}

void u2f_confirm_user_present(U2fData* U2F) {
    U2F->user_present = true;
}

static uint8_t u2f_der_encode_int(uint8_t* der, uint8_t* val, uint8_t val_len) {
    der[0] = 0x02; // Integer

    uint8_t len = 2;
    // Omit leading zeros
    while(val[0] == 0 && val_len > 0) {
        ++val;
        --val_len;
    }

    // Check if integer is negative
    if(val[0] > 0x7f) der[len++] = 0;

    memcpy(der + len, val, val_len);
    len += val_len;

    der[1] = len - 2;
    return len;
}

static uint8_t u2f_der_encode_signature(uint8_t* der, uint8_t* sig) {
    der[0] = 0x30;

    uint8_t len = 2;
    len += u2f_der_encode_int(der + len, sig, 32);
    len += u2f_der_encode_int(der + len, sig + 32, 32);

    der[1] = len - 2;
    return len;
}

static uint16_t u2f_register(U2fData* U2F, uint8_t* buf) {
    U2fRegisterReq* req = (U2fRegisterReq*)buf;
    U2fRegisterResp* resp = (U2fRegisterResp*)buf;
    U2fKeyHandle handle;
    uint8_t private[32];
    U2fPubKey pub_key;
    uint8_t hash[32];
    uint8_t signature[64];

    if(u2f_data_check(false) == false) {
        U2F->ready = false;
        if(U2F->callback != NULL) U2F->callback(U2fNotifyError, U2F->context);
        memcpy(&buf[0], state_not_supported, 2);
        return 2;
    }

    if(U2F->callback != NULL) U2F->callback(U2fNotifyRegister, U2F->context);
    if(U2F->user_present == false) {
        memcpy(&buf[0], state_user_missing, 2);
        return 2;
    }
    U2F->user_present = false;

    hmac_sha256_context hmac_ctx;
    sha256_context sha_ctx;

    handle.len = 32 * 2;
    // Generate random nonce
    furi_hal_random_fill_buf(handle.nonce, 32);

    // Generate private key
    hmac_sha256_init(&hmac_ctx, U2F->device_key);
    hmac_sha256_update(&hmac_ctx, req->app_id, 32);
    hmac_sha256_update(&hmac_ctx, handle.nonce, 32);
    hmac_sha256_finish(&hmac_ctx, U2F->device_key, private);

    // Generate private key handle
    hmac_sha256_init(&hmac_ctx, U2F->device_key);
    hmac_sha256_update(&hmac_ctx, private, 32);
    hmac_sha256_update(&hmac_ctx, req->app_id, 32);
    hmac_sha256_finish(&hmac_ctx, U2F->device_key, handle.hash);

    // Generate public key
    pub_key.format = 0x04; // Uncompressed point
    uECC_compute_public_key(private, pub_key.xy, U2F->p_curve);

    // Generate signature
    uint8_t reserved_byte = 0;
    sha256_start(&sha_ctx);
    sha256_update(&sha_ctx, &reserved_byte, 1);
    sha256_update(&sha_ctx, req->app_id, 32);
    sha256_update(&sha_ctx, req->challenge, 32);
    sha256_update(&sha_ctx, handle.hash, handle.len);
    sha256_update(&sha_ctx, (uint8_t*)&pub_key, 65);
    sha256_finish(&sha_ctx, hash);

    uECC_sign(U2F->cert_key, hash, 32, signature, U2F->p_curve);

    // Encode response message
    resp->reserved = 0x05;
    memcpy(&(resp->pub_key), &pub_key, sizeof(U2fPubKey));
    memcpy(&(resp->key_handle), &handle, sizeof(U2fKeyHandle));
    uint32_t cert_len = u2f_data_cert_load(resp->cert);
    uint8_t signature_len = u2f_der_encode_signature(resp->cert + cert_len, signature);
    memcpy(resp->cert + cert_len + signature_len, state_no_error, 2);

    return (sizeof(U2fRegisterResp) + cert_len + signature_len + 2);
}

static uint16_t u2f_authenticate(U2fData* U2F, uint8_t* buf) {
    U2fAuthReq* req = (U2fAuthReq*)buf;
    U2fAuthResp* resp = (U2fAuthResp*)buf;
    uint8_t priv_key[32];
    uint8_t mac_control[32];
    hmac_sha256_context hmac_ctx;
    sha256_context sha_ctx;
    uint8_t flags = 0;
    uint8_t hash[32];
    uint8_t signature[64];
    uint32_t be_u2f_counter;

    if(u2f_data_check(false) == false) {
        U2F->ready = false;
        if(U2F->callback != NULL) U2F->callback(U2fNotifyError, U2F->context);
        memcpy(&buf[0], state_not_supported, 2);
        return 2;
    }

    if(U2F->callback != NULL) U2F->callback(U2fNotifyAuth, U2F->context);
    if(U2F->user_present == true) {
        flags |= 1;
    } else {
        if(req->p1 == U2fEnforce) {
            memcpy(&buf[0], state_user_missing, 2);
            return 2;
        }
    }
    U2F->user_present = false;

    // The 4 byte counter is represented in big endian. Increment it before use
    be_u2f_counter = lfs_tobe32(U2F->counter + 1);

    // Generate hash
    sha256_start(&sha_ctx);
    sha256_update(&sha_ctx, req->app_id, 32);
    sha256_update(&sha_ctx, &flags, 1);
    sha256_update(&sha_ctx, (uint8_t*)&(be_u2f_counter), 4);
    sha256_update(&sha_ctx, req->challenge, 32);
    sha256_finish(&sha_ctx, hash);

    // Recover private key
    hmac_sha256_init(&hmac_ctx, U2F->device_key);
    hmac_sha256_update(&hmac_ctx, req->app_id, 32);
    hmac_sha256_update(&hmac_ctx, req->key_handle.nonce, 32);
    hmac_sha256_finish(&hmac_ctx, U2F->device_key, priv_key);

    // Generate and verify private key handle
    hmac_sha256_init(&hmac_ctx, U2F->device_key);
    hmac_sha256_update(&hmac_ctx, priv_key, 32);
    hmac_sha256_update(&hmac_ctx, req->app_id, 32);
    hmac_sha256_finish(&hmac_ctx, U2F->device_key, mac_control);

    if(memcmp(req->key_handle.hash, mac_control, 32) != 0) {
        FURI_LOG_W(TAG, "Wrong handle!");
        memcpy(&buf[0], state_wrong_data, 2);
        return 2;
    }

    if(req->p1 == U2fCheckOnly) { // Check-only: don't need to send full response
        memcpy(&buf[0], state_user_missing, 2);
        return 2;
    }

    uECC_sign(priv_key, hash, 32, signature, U2F->p_curve);

    resp->user_present = flags;
    resp->counter = be_u2f_counter;
    uint8_t signature_len = u2f_der_encode_signature(resp->signature, signature);
    memcpy(resp->signature + signature_len, state_no_error, 2);

    U2F->counter++;
    FURI_LOG_D(TAG, "Counter: %lu", U2F->counter);
    u2f_data_cnt_write(U2F->counter);

    if(U2F->callback != NULL) U2F->callback(U2fNotifyAuthSuccess, U2F->context);

    return (sizeof(U2fAuthResp) + signature_len + 2);
}

uint16_t u2f_msg_parse(U2fData* U2F, uint8_t* buf, uint16_t len) {
    furi_assert(U2F);
    if(!U2F->ready) return 0;
    if((buf[0] != 0x00) && (len < 5)) return 0;
    if(buf[1] == U2F_CMD_REGISTER) { // Register request
        return u2f_register(U2F, buf);

    } else if(buf[1] == U2F_CMD_AUTHENTICATE) { // Authenticate request
        return u2f_authenticate(U2F, buf);

    } else if(buf[1] == U2F_CMD_VERSION) { // Get U2F version string
        memcpy(&buf[0], ver_str, 6);
        memcpy(&buf[6], state_no_error, 2);
        return 8;
    } else {
        memcpy(&buf[0], state_not_supported, 2);
        return 2;
    }
    return 0;
}

void u2f_wink(U2fData* U2F) {
    if(U2F->callback != NULL) U2F->callback(U2fNotifyWink, U2F->context);
}

void u2f_set_state(U2fData* U2F, uint8_t state) {
    if(state == 0) {
        if(U2F->callback != NULL) U2F->callback(U2fNotifyDisconnect, U2F->context);
    } else {
        if(U2F->callback != NULL) U2F->callback(U2fNotifyConnect, U2F->context);
    }
    U2F->user_present = false;
}
