#include "u2f.h"
#include "u2f_data.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <littlefs/lfs_util.h> // for lfs_tobe32

#include <mbedtls/sha256.h>
#include <mbedtls/md.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/error.h>

#define TAG "U2f"

#define WORKER_TAG TAG "Worker"

#define MCHECK(expr) furi_check((expr) == 0)

#define U2F_CMD_REGISTER     0x01
#define U2F_CMD_AUTHENTICATE 0x02
#define U2F_CMD_VERSION      0x03

typedef enum {
    U2fCheckOnly = 0x07, // "check-only" - only check key handle, don't send auth response
    U2fEnforce =
        0x03, // "enforce-user-presence-and-sign" - send auth response only if user is present
    U2fDontEnforce =
        0x08, // "dont-enforce-user-presence-and-sign" - send auth response even if user is missing
} U2fAuthMode;

#define U2F_HASH_SIZE      32
#define U2F_NONCE_SIZE     32
#define U2F_CHALLENGE_SIZE 32
#define U2F_APP_ID_SIZE    32

#define U2F_EC_KEY_SIZE    32
#define U2F_EC_BIGNUM_SIZE 32
#define U2F_EC_POINT_SIZE  65

typedef struct {
    uint8_t format;
    uint8_t xy[64];
} FURI_PACKED U2fPubKey;
_Static_assert(sizeof(U2fPubKey) == U2F_EC_POINT_SIZE, "U2fPubKey size mismatch");

typedef struct {
    uint8_t len;
    uint8_t hash[U2F_HASH_SIZE];
    uint8_t nonce[U2F_NONCE_SIZE];
} FURI_PACKED U2fKeyHandle;

typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t len[3];
    uint8_t challenge[U2F_CHALLENGE_SIZE];
    uint8_t app_id[U2F_APP_ID_SIZE];
} FURI_PACKED U2fRegisterReq;

typedef struct {
    uint8_t reserved;
    U2fPubKey pub_key;
    U2fKeyHandle key_handle;
    uint8_t cert[];
} FURI_PACKED U2fRegisterResp;

typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t len[3];
    uint8_t challenge[U2F_CHALLENGE_SIZE];
    uint8_t app_id[U2F_APP_ID_SIZE];
    U2fKeyHandle key_handle;
} FURI_PACKED U2fAuthReq;

typedef struct {
    uint8_t user_present;
    uint32_t counter;
    uint8_t signature[];
} FURI_PACKED U2fAuthResp;

static const uint8_t ver_str[] = {"U2F_V2"};

static const uint8_t state_no_error[] = {0x90, 0x00};
static const uint8_t state_not_supported[] = {0x6D, 0x00};
static const uint8_t state_user_missing[] = {0x69, 0x85};
static const uint8_t state_wrong_data[] = {0x6A, 0x80};

struct U2fData {
    uint8_t device_key[U2F_EC_KEY_SIZE];
    uint8_t cert_key[U2F_EC_KEY_SIZE];
    uint32_t counter;
    bool ready;
    bool user_present;
    U2fEvtCallback callback;
    void* context;
    mbedtls_ecp_group group;
};

static int u2f_uecc_random_cb(void* context, uint8_t* dest, unsigned size) {
    UNUSED(context);
    furi_hal_random_fill_buf(dest, size);
    return 0;
}

U2fData* u2f_alloc(void) {
    return malloc(sizeof(U2fData));
}

void u2f_free(U2fData* U2F) {
    furi_assert(U2F);
    mbedtls_ecp_group_free(&U2F->group);
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

    mbedtls_ecp_group_init(&U2F->group);
    mbedtls_ecp_group_load(&U2F->group, MBEDTLS_ECP_DP_SECP256R1);

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
    len += u2f_der_encode_int(der + len, sig, U2F_HASH_SIZE);
    len += u2f_der_encode_int(der + len, sig + U2F_HASH_SIZE, U2F_HASH_SIZE);

    der[1] = len - 2;
    return len;
}

static void
    u2f_ecc_sign(mbedtls_ecp_group* grp, const uint8_t* key, uint8_t* hash, uint8_t* signature) {
    mbedtls_mpi r, s, d;

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_mpi_init(&d);

    MCHECK(mbedtls_mpi_read_binary(&d, key, U2F_EC_KEY_SIZE));
    MCHECK(mbedtls_ecdsa_sign(grp, &r, &s, &d, hash, U2F_HASH_SIZE, u2f_uecc_random_cb, NULL));
    MCHECK(mbedtls_mpi_write_binary(&r, signature, U2F_EC_BIGNUM_SIZE));
    MCHECK(mbedtls_mpi_write_binary(&s, signature + U2F_EC_BIGNUM_SIZE, U2F_EC_BIGNUM_SIZE));

    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&d);
}

static void u2f_ecc_compute_public_key(
    mbedtls_ecp_group* grp,
    const uint8_t* private_key,
    U2fPubKey* public_key) {
    mbedtls_ecp_point Q;
    mbedtls_mpi d;
    size_t olen;

    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&d);

    MCHECK(mbedtls_mpi_read_binary(&d, private_key, U2F_EC_KEY_SIZE));
    MCHECK(mbedtls_ecp_mul(grp, &Q, &d, &grp->G, u2f_uecc_random_cb, NULL));
    MCHECK(mbedtls_ecp_check_privkey(grp, &d));

    MCHECK(mbedtls_ecp_point_write_binary(
        grp, &Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, (unsigned char*)public_key, sizeof(U2fPubKey)));

    mbedtls_ecp_point_free(&Q);
    mbedtls_mpi_free(&d);
}

///////////////////////////////////////////

static uint16_t u2f_register(U2fData* U2F, uint8_t* buf) {
    U2fRegisterReq* req = (U2fRegisterReq*)buf;
    U2fRegisterResp* resp = (U2fRegisterResp*)buf;
    U2fKeyHandle handle;
    uint8_t private[U2F_EC_KEY_SIZE];
    U2fPubKey pub_key;
    uint8_t hash[U2F_HASH_SIZE];
    uint8_t signature[U2F_EC_BIGNUM_SIZE * 2];

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

    handle.len = U2F_HASH_SIZE * 2;

    // Generate random nonce
    furi_hal_random_fill_buf(handle.nonce, 32);

    {
        mbedtls_md_context_t hmac_ctx;
        mbedtls_md_init(&hmac_ctx);
        MCHECK(mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1));
        MCHECK(mbedtls_md_hmac_starts(&hmac_ctx, U2F->device_key, sizeof(U2F->device_key)));

        // Generate private key
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, req->app_id, sizeof(req->app_id)));
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, handle.nonce, sizeof(handle.nonce)));
        MCHECK(mbedtls_md_hmac_finish(&hmac_ctx, private));

        MCHECK(mbedtls_md_hmac_reset(&hmac_ctx));

        // Generate private key handle
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, private, sizeof(private)));
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, req->app_id, sizeof(req->app_id)));
        MCHECK(mbedtls_md_hmac_finish(&hmac_ctx, handle.hash));
    }

    // Generate public key
    u2f_ecc_compute_public_key(&U2F->group, private, &pub_key);

    // Generate signature
    {
        uint8_t reserved_byte = 0;

        mbedtls_sha256_context sha_ctx;

        mbedtls_sha256_init(&sha_ctx);
        mbedtls_sha256_starts(&sha_ctx, 0);

        mbedtls_sha256_update(&sha_ctx, &reserved_byte, 1);
        mbedtls_sha256_update(&sha_ctx, req->app_id, sizeof(req->app_id));
        mbedtls_sha256_update(&sha_ctx, req->challenge, sizeof(req->challenge));
        mbedtls_sha256_update(&sha_ctx, handle.hash, handle.len);
        mbedtls_sha256_update(&sha_ctx, (uint8_t*)&pub_key, sizeof(U2fPubKey));

        mbedtls_sha256_finish(&sha_ctx, hash);
        mbedtls_sha256_free(&sha_ctx);
    }

    // Sign hash
    u2f_ecc_sign(&U2F->group, U2F->cert_key, hash, signature);

    // Encode response message
    resp->reserved = 0x05;
    memcpy(&(resp->pub_key), &pub_key, sizeof(U2fPubKey));
    memcpy(&(resp->key_handle), &handle, sizeof(U2fKeyHandle));
    uint32_t cert_len = u2f_data_cert_load(resp->cert);
    uint8_t signature_len = u2f_der_encode_signature(resp->cert + cert_len, signature);
    memcpy(resp->cert + cert_len + signature_len, state_no_error, 2);

    return sizeof(U2fRegisterResp) + cert_len + signature_len + 2;
}

static uint16_t u2f_authenticate(U2fData* U2F, uint8_t* buf) {
    U2fAuthReq* req = (U2fAuthReq*)buf;
    U2fAuthResp* resp = (U2fAuthResp*)buf;
    uint8_t priv_key[U2F_EC_KEY_SIZE];
    uint8_t mac_control[32];
    uint8_t flags = 0;
    uint8_t hash[U2F_HASH_SIZE];
    uint8_t signature[U2F_HASH_SIZE * 2];
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
    {
        mbedtls_sha256_context sha_ctx;

        mbedtls_sha256_init(&sha_ctx);
        mbedtls_sha256_starts(&sha_ctx, 0);

        mbedtls_sha256_update(&sha_ctx, req->app_id, sizeof(req->app_id));
        mbedtls_sha256_update(&sha_ctx, &flags, 1);
        mbedtls_sha256_update(&sha_ctx, (uint8_t*)&(be_u2f_counter), sizeof(be_u2f_counter));
        mbedtls_sha256_update(&sha_ctx, req->challenge, sizeof(req->challenge));

        mbedtls_sha256_finish(&sha_ctx, hash);
        mbedtls_sha256_free(&sha_ctx);
    }

    {
        mbedtls_md_context_t hmac_ctx;
        mbedtls_md_init(&hmac_ctx);
        MCHECK(mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1));
        MCHECK(mbedtls_md_hmac_starts(&hmac_ctx, U2F->device_key, sizeof(U2F->device_key)));

        // Recover private key
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, req->app_id, sizeof(req->app_id)));
        MCHECK(mbedtls_md_hmac_update(
            &hmac_ctx, req->key_handle.nonce, sizeof(req->key_handle.nonce)));
        MCHECK(mbedtls_md_hmac_finish(&hmac_ctx, priv_key));

        MCHECK(mbedtls_md_hmac_reset(&hmac_ctx));

        // Generate and verify private key handle
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, priv_key, sizeof(priv_key)));
        MCHECK(mbedtls_md_hmac_update(&hmac_ctx, req->app_id, sizeof(req->app_id)));
        MCHECK(mbedtls_md_hmac_finish(&hmac_ctx, mac_control));
    }

    if(memcmp(req->key_handle.hash, mac_control, sizeof(mac_control)) != 0) {
        FURI_LOG_W(TAG, "Wrong handle!");
        memcpy(&buf[0], state_wrong_data, 2);
        return 2;
    }

    if(req->p1 == U2fCheckOnly) { // Check-only: don't need to send full response
        memcpy(&buf[0], state_user_missing, 2);
        return 2;
    }

    // Sign hash
    u2f_ecc_sign(&U2F->group, priv_key, hash, signature);

    resp->user_present = flags;
    resp->counter = be_u2f_counter;
    uint8_t signature_len = u2f_der_encode_signature(resp->signature, signature);
    memcpy(resp->signature + signature_len, state_no_error, 2);

    U2F->counter++;
    FURI_LOG_D(TAG, "Counter: %lu", U2F->counter);
    u2f_data_cnt_write(U2F->counter);

    if(U2F->callback != NULL) U2F->callback(U2fNotifyAuthSuccess, U2F->context);

    return sizeof(U2fAuthResp) + signature_len + 2;
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
