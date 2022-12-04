#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <mbedtls/des.h>

#include "../helpers/iso7816.h"

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
} MrtdDate;

// NULL terminated document ID
#define MRTD_DOCNR_MAX_LENGTH 21

typedef enum {
    MrtdAuthMethodNone,
    MrtdAuthMethodAny,
    MrtdAuthMethodBac,
    MrtdAuthMethodPace,
} MrtdAuthMethod;

typedef enum {
    MrtdTypeUnknown,
    MrtdTypeTD1,
    MrtdTypeTD2,
    MrtdTypeTD3,
} MrtdType;

typedef struct {
    MrtdAuthMethod method;

    // BAC input fields
    MrtdDate birth_date;
    MrtdDate expiry_date;
    char doc_number[MRTD_DOCNR_MAX_LENGTH];

    //TODO: PACE
} MrtdAuthData;

typedef struct {
    mbedtls_des_context des;
    uint8_t key[16];
    uint8_t mac[8];
    uint8_t xormac[8];

    uint8_t buffer_in[8];
    uint8_t idx_in;
} mrtd_bac_mac_ctx;

typedef struct {
    const char* name;
    const uint8_t short_id;
    const uint16_t file_id;
    const uint8_t tag;
} EFFile;

struct EFFormat {
    // Under Master File (MF)
    const EFFile ATR;
    const EFFile DIR;
    const EFFile CardAccess;
    const EFFile CardSecurity;

    // Under LDS1 eMRTD Application
    const EFFile COM;
    const EFFile SOD;
    const EFFile DG1;
    const EFFile DG2;
    const EFFile DG3;
    const EFFile DG4;
    const EFFile DG5;
    const EFFile DG6;
    const EFFile DG7;
    const EFFile DG8;
    const EFFile DG9;
    const EFFile DG10;
    const EFFile DG11;
    const EFFile DG12;
    const EFFile DG13;
    const EFFile DG14;
    const EFFile DG15;
    const EFFile DG16;
};

extern const struct EFFormat EF;

typedef uint8_t AIDValue[7];

struct AIDSet {
    AIDValue eMRTDApplication;
    AIDValue TravelRecords;
    AIDValue VisaRecords;
    AIDValue AdditionalBiometrics;
};

extern struct AIDSet AID;

#define MAX_EFDIR_APPS 4

typedef struct {
    AIDValue applications[MAX_EFDIR_APPS];
    uint8_t applications_count;
} EF_DIR_contents;

#define MAX_EFCOM_TAGS 18

typedef struct {
    uint16_t lds_version; // xxyy => xx.yy (major.minor)
    uint32_t unicode_version; // aabbcc => aa.bb.cc (major.minor.release)
    uint8_t tag_list[MAX_EFCOM_TAGS];
} EF_COM_contents;

typedef struct {
    MrtdType type;
    // ICAO9303 max sizes + 1 for 0-byte
    uint8_t doctype[3];
    uint8_t issuing_state[4];
    uint8_t name[40];
    MrtdDate birth_date;
    uint8_t docnr[10];
    uint8_t nationality[4];
    uint8_t sex[2];
    MrtdDate expiry_date;
} EF_DG1_contents;

typedef struct {
    MrtdAuthData auth;
    bool auth_success;
    MrtdAuthMethod auth_method_used;

    struct {
        EF_DIR_contents EF_DIR;
        EF_COM_contents EF_COM;
        EF_DG1_contents DG1;
    } files;
} MrtdData;

const char* mrtd_auth_method_string(MrtdAuthMethod method);

bool mrtd_auth_method_parse_string(MrtdAuthMethod* method, const char* str);

uint8_t mrtd_bac_check_digit(const char* input, const uint8_t length);

//TODO: swap order, all other functions have output last
void mrtd_print_date(char* output, MrtdDate* date);

void mrtd_parse_date(MrtdDate* date, const unsigned char* input);

bool mrtd_bac_get_kmrz(MrtdAuthData* auth, char* output, uint8_t output_size);

bool mrtd_bac_keys_from_seed(const uint8_t* kseed, uint8_t* ksenc, uint8_t* ksmac);

bool mrtd_bac_keys(MrtdAuthData* auth, uint8_t ksenc[16], uint8_t ksmac[16]);

bool mrtd_bac_encrypt(const uint8_t* data, size_t data_length, const uint8_t* key, uint8_t* output);

bool mrtd_bac_mac(const uint8_t* data, size_t data_length, const uint8_t* key, uint8_t* output);

bool mrtd_bac_mac_init(mrtd_bac_mac_ctx* ctx, const uint8_t key[16]);

bool mrtd_bac_mac_update(mrtd_bac_mac_ctx* ctx, const uint8_t* data, size_t data_length);

bool mrtd_bac_mac_finalize(mrtd_bac_mac_ctx* ctx, uint8_t output[8]);

bool mrtd_bac_mac_pad(mrtd_bac_mac_ctx* ctx); // TODO: internal only, remove from .h?

bool mrtd_bac_padded_mac(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output);

bool mrtd_bac_decrypt(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output);

bool mrtd_bac_decrypt_verify(
    const uint8_t* data,
    size_t data_length,
    uint8_t* key_enc,
    uint8_t* key_mac,
    uint8_t* output);

//TODO: add some consts
uint16_t mrtd_bac_decrypt_verify_sm(
    const uint8_t* data,
    size_t data_length,
    uint8_t* key_enc,
    uint8_t* key_mac,
    uint64_t ssc,
    uint8_t* output,
    size_t* output_written);

#include <machine/_endian.h>
#define htonll(x) ((((uint64_t)__htonl(x)) << 32) + __htonl((x) >> 32))

static __inline uint64_t mrtd_ssc_from_data(const uint8_t* rnd_ic, const uint8_t* rnd_ifd) {
#if _BYTE_ORDER == _LITTLE_ENDIAN
    return (((uint64_t)rnd_ic[4] << 56) & 0xff00000000000000) |
           (((uint64_t)rnd_ic[5] << 48) & 0x00ff000000000000) |
           (((uint64_t)rnd_ic[6] << 40) & 0x0000ff0000000000) |
           (((uint64_t)rnd_ic[7] << 32) & 0x000000ff00000000) |
           (((uint64_t)rnd_ifd[4] << 24) & 0x00000000ff000000) |
           (((uint64_t)rnd_ifd[5] << 16) & 0x0000000000ff0000) |
           (((uint64_t)rnd_ifd[6] << 8) & 0x000000000000ff00) |
           (((uint64_t)rnd_ifd[7]) & 0x00000000000000ff);
#else
#error Using untested code, please verify first!
    return (*((uint64_t*)(rnd_ic + 4)) & 0xffffffff) + (*((uint64_t*)(rnd_ifd + 4)) * 0x100000000);
#endif
}

size_t mrtd_protect_apdu(
    uint8_t cla,
    uint8_t ins,
    uint8_t p1,
    uint8_t p2,
    uint8_t lc,
    const void* data,
    int16_t le,
    const uint8_t* key_enc,
    const uint8_t* key_mac,
    uint64_t ssc,
    uint8_t* output);

int tlv_number(TlvInfo tlv);

const EFFile* mrtd_tag_to_file(uint8_t tag);
