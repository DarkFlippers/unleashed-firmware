#pragma once

#include <toolbox/bit_buffer.h>
#include <nfc/protocols/nfc_device_base_i.h>
#include <mbedtls/include/mbedtls/des.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FELICA_IDM_SIZE        (8U)
#define FELICA_PMM_SIZE        (8U)
#define FELICA_DATA_BLOCK_SIZE (16U)

#define FELICA_CMD_READ_WITHOUT_ENCRYPTION  (0x06U)
#define FELICA_CMD_WRITE_WITHOUT_ENCRYPTION (0x08U)

#define FELICA_SERVICE_RW_ACCESS (0x0009U)
#define FELICA_SERVICE_RO_ACCESS (0x000BU)

#define FELICA_BLOCKS_TOTAL_COUNT    (28U)
#define FELICA_BLOCK_INDEX_REG       (0x0EU)
#define FELICA_BLOCK_INDEX_RC        (0x80U)
#define FELICA_BLOCK_INDEX_MAC       (0x81U)
#define FELICA_BLOCK_INDEX_ID        (0x82U)
#define FELICA_BLOCK_INDEX_D_ID      (0x83U)
#define FELICA_BLOCK_INDEX_SER_C     (0x84U)
#define FELICA_BLOCK_INDEX_SYS_C     (0x85U)
#define FELICA_BLOCK_INDEX_CKV       (0x86U)
#define FELICA_BLOCK_INDEX_CK        (0x87U)
#define FELICA_BLOCK_INDEX_MC        (0x88U)
#define FELICA_BLOCK_INDEX_WCNT      (0x90U)
#define FELICA_BLOCK_INDEX_MAC_A     (0x91U)
#define FELICA_BLOCK_INDEX_STATE     (0x92U)
#define FELICA_BLOCK_INDEX_CRC_CHECK (0xA0U)

#define FELICA_GUARD_TIME_US    (20000U)
#define FELICA_FDT_POLL_FC      (10000U)
#define FELICA_POLL_POLL_MIN_US (1280U)

#define FELICA_FDT_LISTEN_FC (1172)

#define FELICA_SYSTEM_CODE_CODE (0xFFFFU)
#define FELICA_TIME_SLOT_1      (0x00U)
#define FELICA_TIME_SLOT_2      (0x01U)
#define FELICA_TIME_SLOT_4      (0x03U)
#define FELICA_TIME_SLOT_8      (0x07U)
#define FELICA_TIME_SLOT_16     (0x0FU)

/** @brief Type of possible Felica errors */
typedef enum {
    FelicaErrorNone,
    FelicaErrorNotPresent,
    FelicaErrorColResFailed,
    FelicaErrorBufferOverflow,
    FelicaErrorCommunication,
    FelicaErrorFieldOff,
    FelicaErrorWrongCrc,
    FelicaErrorProtocol,
    FelicaErrorTimeout,
} FelicaError;

typedef struct {
    uint8_t data[FELICA_DATA_BLOCK_SIZE];
} FelicaBlockData;

/** @brief Separate type for card key block. Used in authentication process */
typedef struct {
    uint8_t data[FELICA_DATA_BLOCK_SIZE];
} FelicaCardKey;

/** @brief In Felica there two types of auth. Internal is the first one, after
  * which external became possible. Here are two flags representing which one
  * was passed */
typedef struct {
    bool internal : 1;
    bool external : 1;
} FelicaAuthenticationStatus;

/** @brief Struct which controls the process of authentication and can be passed as
  * a parameter to the application level. In order to force user to fill card key block data. */
typedef struct {
    bool skip_auth; /**< By default it is true, so auth is skipped. By setting this to false several auth steps will be performed in order to pass auth*/
    FelicaCardKey
        card_key; /**< User must fill this field with known card key in order to pass auth*/
    FelicaAuthenticationStatus auth_status; /**< Authentication status*/
} FelicaAuthenticationContext;

/**
 * @brief Stucture for holding Felica session key which is calculated from rc and ck.
*/
typedef struct {
    uint8_t data[FELICA_DATA_BLOCK_SIZE];
} FelicaSessionKey;

/**
 * @brief Structure used to hold authentication related fields.
*/
typedef struct {
    mbedtls_des3_context des_context; /**< Context for mbedtls des functions. */
    FelicaSessionKey session_key; /**< Calculated session key. */
    FelicaAuthenticationContext context; /**< Public auth context provided to upper levels. */
} FelicaAuthentication;

/** @brief Felica ID block */
typedef struct {
    uint8_t data[FELICA_IDM_SIZE];
} FelicaIDm;

/** @brief Felica PMm block */
typedef struct {
    uint8_t data[FELICA_PMM_SIZE];
} FelicaPMm;

/** @brief Felica block with status flags indicating last operation with it.
  * See Felica manual for more details on status codes. */
typedef struct {
    uint8_t SF1; /**< Status flag 1, equals to 0 when success*/
    uint8_t SF2; /**< Status flag 2, equals to 0 when success*/
    uint8_t data[FELICA_DATA_BLOCK_SIZE]; /**< Block data */
} FelicaBlock;

/** @brief Felica filesystem structure */
typedef struct {
    FelicaBlock spad[14];
    FelicaBlock reg;
    FelicaBlock rc;
    FelicaBlock mac;
    FelicaBlock id;
    FelicaBlock d_id;
    FelicaBlock ser_c;
    FelicaBlock sys_c;
    FelicaBlock ckv;
    FelicaBlock ck;
    FelicaBlock mc;
    FelicaBlock wcnt;
    FelicaBlock mac_a;
    FelicaBlock state;
    FelicaBlock crc_check;
} FelicaFileSystem;

/** @brief Union which represents filesystem in junction with plain data dump */
typedef union {
    FelicaFileSystem fs;
    uint8_t dump[sizeof(FelicaFileSystem)];
} FelicaFSUnion;

/** @brief Structure used to store Felica data and additional values about reading */
typedef struct {
    FelicaIDm idm;
    FelicaPMm pmm;
    uint8_t blocks_total;
    uint8_t blocks_read;
    FelicaFSUnion data;
} FelicaData;

#pragma pack(push, 1)
typedef struct {
    uint8_t code;
    FelicaIDm idm;
    uint8_t service_num;
    uint16_t service_code;
    uint8_t block_count;
} FelicaCommandHeader;
#pragma pack(pop)

typedef struct {
    uint8_t length;
    uint8_t response_code;
    FelicaIDm idm;
    uint8_t SF1;
    uint8_t SF2;
} FelicaCommandResponseHeader;

typedef struct {
    uint8_t service_code : 4;
    uint8_t access_mode  : 3;
    uint8_t length       : 1;
    uint8_t block_number;
} FelicaBlockListElement;

typedef struct {
    uint8_t length;
    uint8_t response_code;
    FelicaIDm idm;
    uint8_t SF1;
    uint8_t SF2;
    uint8_t block_count;
    uint8_t data[];
} FelicaPollerReadCommandResponse;

typedef struct {
    FelicaCommandResponseHeader header;
    uint8_t block_count;
    uint8_t data[];
} FelicaListenerReadCommandResponse;

typedef FelicaCommandResponseHeader FelicaListenerWriteCommandResponse;

typedef FelicaCommandResponseHeader FelicaPollerWriteCommandResponse;

extern const NfcDeviceBase nfc_device_felica;

FelicaData* felica_alloc(void);

void felica_free(FelicaData* data);

void felica_reset(FelicaData* data);

void felica_copy(FelicaData* data, const FelicaData* other);

bool felica_verify(FelicaData* data, const FuriString* device_type);

bool felica_load(FelicaData* data, FlipperFormat* ff, uint32_t version);

bool felica_save(const FelicaData* data, FlipperFormat* ff);

bool felica_is_equal(const FelicaData* data, const FelicaData* other);

const char* felica_get_device_name(const FelicaData* data, NfcDeviceNameType name_type);

const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len);

bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len);

FelicaData* felica_get_base_data(const FelicaData* data);

void felica_calculate_session_key(
    mbedtls_des3_context* ctx,
    const uint8_t* ck,
    const uint8_t* rc,
    uint8_t* out);

bool felica_check_mac(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* blocks,
    const uint8_t block_count,
    uint8_t* data);

void felica_calculate_mac_read(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* blocks,
    const uint8_t block_count,
    const uint8_t* data,
    uint8_t* mac);

void felica_calculate_mac_write(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* wcnt,
    const uint8_t* data,
    uint8_t* mac);
#ifdef __cplusplus
}
#endif
