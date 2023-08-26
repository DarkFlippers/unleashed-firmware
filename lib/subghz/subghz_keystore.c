#include "subghz_keystore.h"
#include "subghz_keystore_i.h"

#include <furi.h>
#include <furi_hal.h>

#include <storage/storage.h>
#include <toolbox/hex.h>
#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>

#define TAG "SubGhzKeystore"

#define FILE_BUFFER_SIZE 64

#define SUBGHZ_KEYSTORE_FILE_TYPE "Flipper SubGhz Keystore File"
#define SUBGHZ_KEYSTORE_FILE_RAW_TYPE "Flipper SubGhz Keystore RAW File"
#define SUBGHZ_KEYSTORE_FILE_VERSION 0

#define SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT 1
#define SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE 512
#define SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE (SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE * 2)

typedef enum {
    SubGhzKeystoreEncryptionNone,
    SubGhzKeystoreEncryptionAES256,
} SubGhzKeystoreEncryption;

SubGhzKeystore* subghz_keystore_alloc() {
    SubGhzKeystore* instance = malloc(sizeof(SubGhzKeystore));

    SubGhzKeyArray_init(instance->data);

    subghz_keystore_reset_kl(instance);

    return instance;
}

void subghz_keystore_reset_kl(SubGhzKeystore* instance) {
    furi_assert(instance);

    instance->mfname = "";
    instance->kl_type = 0;
}

void subghz_keystore_free(SubGhzKeystore* instance) {
    furi_assert(instance);

    for
        M_EACH(manufacture_code, instance->data, SubGhzKeyArray_t) {
            furi_string_free(manufacture_code->name);
            manufacture_code->key = 0;
        }
    SubGhzKeyArray_clear(instance->data);

    free(instance);
}

static void subghz_keystore_add_key(
    SubGhzKeystore* instance,
    const char* name,
    uint64_t key,
    uint16_t type) {
    SubGhzKey* manufacture_code = SubGhzKeyArray_push_raw(instance->data);
    manufacture_code->name = furi_string_alloc_set(name);
    manufacture_code->key = key;
    manufacture_code->type = type;
}

static bool subghz_keystore_process_line(SubGhzKeystore* instance, char* line) {
    uint64_t key = 0;
    uint16_t type = 0;
    char skey[17] = {0};
    char name[65] = {0};
    int ret = sscanf(line, "%16s:%hu:%64s", skey, &type, name);
    key = strtoull(skey, NULL, 16);
    if(ret == 3) {
        subghz_keystore_add_key(instance, name, key, type);
        return true;
    } else {
        FURI_LOG_E(TAG, "Failed to load line: %s\r\n", line);
        return false;
    }
}

static void subghz_keystore_mess_with_iv(uint8_t* iv) {
    // Alignment check for `ldrd` instruction
    furi_assert(((uint32_t)iv) % 4 == 0);
    // Please do not share decrypted manufacture keys
    // Sharing them will bring some discomfort to legal owners
    // And potential legal action against you
    // While you reading this code think about your own personal responsibility
    asm volatile("nani%=:                  \n"
                 "ldrd  r0, r2, [%0, #0x0] \n"
                 "lsl   r1, r0, #8         \n"
                 "lsl   r3, r2, #8         \n"
                 "orr   r3, r3, r0, lsr #24\n"
                 "uadd8 r1, r1, r0         \n"
                 "uadd8 r3, r3, r2         \n"
                 "strd  r1, r3, [%0, #0x0] \n"
                 "ldrd  r1, r3, [%0, #0x8] \n"
                 "lsl   r0, r1, #8         \n"
                 "orr   r0, r0, r2, lsr #24\n"
                 "lsl   r2, r3, #8         \n"
                 "orr   r2, r2, r1, lsr #24\n"
                 "uadd8 r1, r1, r0         \n"
                 "uadd8 r3, r3, r2         \n"
                 "strd  r1, r3, [%0, #0x8] \n"
                 :
                 : "r"(iv)
                 : "r0", "r1", "r2", "r3", "memory");
}

static bool subghz_keystore_read_file(SubGhzKeystore* instance, Stream* stream, uint8_t* iv) {
    bool result = true;
    uint8_t buffer[FILE_BUFFER_SIZE];

    char* decrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);
    char* encrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);
    size_t encrypted_line_cursor = 0;

    do {
        if(iv) {
            if(!furi_hal_crypto_enclave_load_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT, iv)) {
                FURI_LOG_E(TAG, "Unable to load decryption key");
                break;
            }
        }

        size_t ret = 0;
        do {
            ret = stream_read(stream, buffer, FILE_BUFFER_SIZE);
            for(uint16_t i = 0; i < ret; i++) {
                if(buffer[i] == '\n' && encrypted_line_cursor > 0) {
                    // Process line
                    if(iv) {
                        // Data alignment check, 32 instead of 16 because of hex encoding
                        size_t len = strlen(encrypted_line);
                        if(len % 32 == 0) {
                            // Inplace hex to bin conversion
                            for(size_t i = 0; i < len; i += 2) {
                                uint8_t hi_nibble = 0;
                                uint8_t lo_nibble = 0;
                                hex_char_to_hex_nibble(encrypted_line[i], &hi_nibble);
                                hex_char_to_hex_nibble(encrypted_line[i + 1], &lo_nibble);
                                encrypted_line[i / 2] = (hi_nibble << 4) | lo_nibble;
                            }
                            len /= 2;

                            if(furi_hal_crypto_decrypt(
                                   (uint8_t*)encrypted_line, (uint8_t*)decrypted_line, len)) {
                                subghz_keystore_process_line(instance, decrypted_line);
                            } else {
                                FURI_LOG_E(TAG, "Decryption failed");
                                result = false;
                                break;
                            }
                        } else {
                            FURI_LOG_E(TAG, "Invalid encrypted data: %s", encrypted_line);
                        }
                    } else {
                        subghz_keystore_process_line(instance, encrypted_line);
                    }
                    // reset line buffer
                    memset(decrypted_line, 0, SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);
                    memset(encrypted_line, 0, SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);
                    encrypted_line_cursor = 0;
                } else if(buffer[i] == '\r' || buffer[i] == '\n') {
                    // do not add line endings to the buffer
                } else {
                    if(encrypted_line_cursor < SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE) {
                        encrypted_line[encrypted_line_cursor] = buffer[i];
                        encrypted_line_cursor++;
                    } else {
                        FURI_LOG_E(TAG, "Malformed file");
                        result = false;
                        break;
                    }
                }
            }
        } while(ret > 0 && result);

        if(iv) furi_hal_crypto_enclave_unload_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT);
    } while(false);

    free(encrypted_line);
    free(decrypted_line);

    return result;
}

bool subghz_keystore_load(SubGhzKeystore* instance, const char* file_name) {
    furi_assert(instance);
    bool result = false;
    uint8_t iv[16];
    uint32_t version;
    uint32_t encryption;

    FuriString* filetype;
    filetype = furi_string_alloc();

    FURI_LOG_I(TAG, "Loading keystore %s", file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FlipperFormat* flipper_format = flipper_format_file_alloc(storage);
    do {
        if(!flipper_format_file_open_existing(flipper_format, file_name)) {
            FURI_LOG_E(TAG, "Unable to open file for read: %s", file_name);
            break;
        }
        if(!flipper_format_read_header(flipper_format, filetype, &version)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "Encryption", (uint32_t*)&encryption, 1)) {
            FURI_LOG_E(TAG, "Missing encryption type");
            break;
        }

        if(strcmp(furi_string_get_cstr(filetype), SUBGHZ_KEYSTORE_FILE_TYPE) != 0 ||
           version != SUBGHZ_KEYSTORE_FILE_VERSION) {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        Stream* stream = flipper_format_get_raw_stream(flipper_format);
        if(encryption == SubGhzKeystoreEncryptionNone) {
            result = subghz_keystore_read_file(instance, stream, NULL);
        } else if(encryption == SubGhzKeystoreEncryptionAES256) {
            if(!flipper_format_read_hex(flipper_format, "IV", iv, 16)) {
                FURI_LOG_E(TAG, "Missing IV");
                break;
            }
            subghz_keystore_mess_with_iv(iv);
            result = subghz_keystore_read_file(instance, stream, iv);
        } else {
            FURI_LOG_E(TAG, "Unknown encryption");
            break;
        }
    } while(0);
    flipper_format_free(flipper_format);

    furi_record_close(RECORD_STORAGE);

    furi_string_free(filetype);

    return result;
}

bool subghz_keystore_save(SubGhzKeystore* instance, const char* file_name, uint8_t* iv) {
    furi_assert(instance);
    bool result = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    char* decrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);
    char* encrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);

    FlipperFormat* flipper_format = flipper_format_file_alloc(storage);
    do {
        if(!flipper_format_file_open_always(flipper_format, file_name)) {
            FURI_LOG_E(TAG, "Unable to open file for write: %s", file_name);
            break;
        }
        if(!flipper_format_write_header_cstr(
               flipper_format, SUBGHZ_KEYSTORE_FILE_TYPE, SUBGHZ_KEYSTORE_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }
        uint32_t encryption = SubGhzKeystoreEncryptionAES256;
        if(!flipper_format_write_uint32(flipper_format, "Encryption", &encryption, 1)) {
            FURI_LOG_E(TAG, "Unable to add Encryption");
            break;
        }
        if(!flipper_format_write_hex(flipper_format, "IV", iv, 16)) {
            FURI_LOG_E(TAG, "Unable to add IV");
            break;
        }

        subghz_keystore_mess_with_iv(iv);

        if(!furi_hal_crypto_enclave_load_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT, iv)) {
            FURI_LOG_E(TAG, "Unable to load encryption key");
            break;
        }

        Stream* stream = flipper_format_get_raw_stream(flipper_format);
        size_t encrypted_line_count = 0;
        for
            M_EACH(key, instance->data, SubGhzKeyArray_t) {
                // Wipe buffer before packing
                memset(decrypted_line, 0, SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);
                memset(encrypted_line, 0, SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);
                // Form unecreypted line
                int len = snprintf(
                    decrypted_line,
                    SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE,
                    "%08lX%08lX:%hu:%s",
                    (uint32_t)(key->key >> 32),
                    (uint32_t)key->key,
                    key->type,
                    furi_string_get_cstr(key->name));
                // Verify length and align
                furi_assert(len > 0);
                if(len % 16 != 0) {
                    len += (16 - len % 16);
                }
                furi_assert(len % 16 == 0);
                furi_assert(len <= SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);
                // Form encrypted line
                if(!furi_hal_crypto_encrypt(
                       (uint8_t*)decrypted_line, (uint8_t*)encrypted_line, len)) {
                    FURI_LOG_E(TAG, "Encryption failed");
                    break;
                }
                // HEX Encode encrypted line
                const char xx[] = "0123456789ABCDEF";
                for(int i = 0; i < len; i++) {
                    size_t cursor = len - i - 1;
                    size_t hex_cursor = len * 2 - i * 2 - 1;
                    encrypted_line[hex_cursor] = xx[encrypted_line[cursor] & 0xF];
                    encrypted_line[hex_cursor - 1] = xx[(encrypted_line[cursor] >> 4) & 0xF];
                }
                stream_write_cstring(stream, encrypted_line);
                stream_write_char(stream, '\n');
                encrypted_line_count++;
            }
        furi_hal_crypto_enclave_unload_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT);
        size_t total_keys = SubGhzKeyArray_size(instance->data);
        result = encrypted_line_count == total_keys;
        if(result) {
            FURI_LOG_I(TAG, "Success. Encrypted: %zu of %zu", encrypted_line_count, total_keys);
        } else {
            FURI_LOG_E(TAG, "Failure. Encrypted: %zu of %zu", encrypted_line_count, total_keys);
        }
    } while(0);
    flipper_format_free(flipper_format);

    free(encrypted_line);
    free(decrypted_line);
    furi_record_close(RECORD_STORAGE);

    return result;
}

SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance) {
    furi_assert(instance);
    return &instance->data;
}

bool subghz_keystore_raw_encrypted_save(
    const char* input_file_name,
    const char* output_file_name,
    uint8_t* iv) {
    bool encrypted = false;
    uint32_t version;
    uint32_t encryption;
    FuriString* filetype;
    filetype = furi_string_alloc();

    Storage* storage = furi_record_open(RECORD_STORAGE);

    char* encrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);

    FlipperFormat* input_flipper_format = flipper_format_file_alloc(storage);
    do {
        if(!flipper_format_file_open_existing(input_flipper_format, input_file_name)) {
            FURI_LOG_E(TAG, "Unable to open file for read: %s", input_file_name);
            break;
        }
        if(!flipper_format_read_header(input_flipper_format, filetype, &version)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }
        if(!flipper_format_read_uint32(
               input_flipper_format, "Encryption", (uint32_t*)&encryption, 1)) {
            FURI_LOG_E(TAG, "Missing encryption type");
            break;
        }

        if(strcmp(furi_string_get_cstr(filetype), SUBGHZ_KEYSTORE_FILE_RAW_TYPE) != 0 ||
           version != SUBGHZ_KEYSTORE_FILE_VERSION) {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        if(encryption != SubGhzKeystoreEncryptionNone) {
            FURI_LOG_E(TAG, "Already encryption");
            break;
        }
        Stream* input_stream = flipper_format_get_raw_stream(input_flipper_format);

        FlipperFormat* output_flipper_format = flipper_format_file_alloc(storage);

        if(!flipper_format_file_open_always(output_flipper_format, output_file_name)) {
            FURI_LOG_E(TAG, "Unable to open file for write: %s", output_file_name);
            break;
        }
        if(!flipper_format_write_header_cstr(
               output_flipper_format,
               furi_string_get_cstr(filetype),
               SUBGHZ_KEYSTORE_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }
        uint32_t encryption = SubGhzKeystoreEncryptionAES256;
        if(!flipper_format_write_uint32(output_flipper_format, "Encryption", &encryption, 1)) {
            FURI_LOG_E(TAG, "Unable to add Encryption");
            break;
        }
        if(!flipper_format_write_hex(output_flipper_format, "IV", iv, 16)) {
            FURI_LOG_E(TAG, "Unable to add IV");
            break;
        }

        if(!flipper_format_write_string_cstr(output_flipper_format, "Encrypt_data", "RAW")) {
            FURI_LOG_E(TAG, "Unable to add Encrypt_data");
            break;
        }

        subghz_keystore_mess_with_iv(iv);

        if(!furi_hal_crypto_enclave_load_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT, iv)) {
            FURI_LOG_E(TAG, "Unable to load encryption key");
            break;
        }

        Stream* output_stream = flipper_format_get_raw_stream(output_flipper_format);
        uint8_t buffer[FILE_BUFFER_SIZE];
        bool result = true;

        size_t ret = 0;
        furi_assert(FILE_BUFFER_SIZE % 16 == 0);

        //skip the end of the previous line "\n"
        stream_read(input_stream, buffer, 1);

        do {
            memset(buffer, 0, FILE_BUFFER_SIZE);
            ret = stream_read(input_stream, buffer, FILE_BUFFER_SIZE);
            if(ret == 0) {
                break;
            }

            for(uint16_t i = 0; i < FILE_BUFFER_SIZE - 1; i += 2) {
                uint8_t hi_nibble = 0;
                uint8_t lo_nibble = 0;
                hex_char_to_hex_nibble(buffer[i], &hi_nibble);
                hex_char_to_hex_nibble(buffer[i + 1], &lo_nibble);
                buffer[i / 2] = (hi_nibble << 4) | lo_nibble;
            }

            memset(encrypted_line, 0, SUBGHZ_KEYSTORE_FILE_ENCRYPTED_LINE_SIZE);
            // Form encrypted line
            if(!furi_hal_crypto_encrypt(
                   (uint8_t*)buffer, (uint8_t*)encrypted_line, FILE_BUFFER_SIZE / 2)) {
                FURI_LOG_E(TAG, "Encryption failed");
                result = false;
                break;
            }

            // HEX Encode encrypted line
            const char xx[] = "0123456789ABCDEF";
            for(size_t i = 0; i < FILE_BUFFER_SIZE / 2; i++) {
                size_t cursor = FILE_BUFFER_SIZE / 2 - i - 1;
                size_t hex_cursor = FILE_BUFFER_SIZE - i * 2 - 1;
                encrypted_line[hex_cursor] = xx[encrypted_line[cursor] & 0xF];
                encrypted_line[hex_cursor - 1] = xx[(encrypted_line[cursor] >> 4) & 0xF];
            }
            stream_write_cstring(output_stream, encrypted_line);

        } while(true);

        flipper_format_free(output_flipper_format);

        furi_hal_crypto_enclave_unload_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT);

        if(!result) break;

        encrypted = true;
    } while(0);

    flipper_format_free(input_flipper_format);

    free(encrypted_line);

    furi_record_close(RECORD_STORAGE);

    return encrypted;
}

bool subghz_keystore_raw_get_data(const char* file_name, size_t offset, uint8_t* data, size_t len) {
    bool result = false;
    uint8_t iv[16];
    uint32_t version;
    uint32_t encryption;

    FuriString* str_temp;
    str_temp = furi_string_alloc();

    Storage* storage = furi_record_open(RECORD_STORAGE);
    char* decrypted_line = malloc(SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);

    FlipperFormat* flipper_format = flipper_format_file_alloc(storage);
    do {
        if(!flipper_format_file_open_existing(flipper_format, file_name)) {
            FURI_LOG_E(TAG, "Unable to open file for read: %s", file_name);
            break;
        }
        if(!flipper_format_read_header(flipper_format, str_temp, &version)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "Encryption", (uint32_t*)&encryption, 1)) {
            FURI_LOG_E(TAG, "Missing encryption type");
            break;
        }

        if(strcmp(furi_string_get_cstr(str_temp), SUBGHZ_KEYSTORE_FILE_RAW_TYPE) != 0 ||
           version != SUBGHZ_KEYSTORE_FILE_VERSION) {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        Stream* stream = flipper_format_get_raw_stream(flipper_format);
        if(encryption != SubGhzKeystoreEncryptionAES256) {
            FURI_LOG_E(TAG, "Unknown encryption");
            break;
        }

        if(offset < 16) {
            if(!flipper_format_read_hex(flipper_format, "IV", iv, 16)) {
                FURI_LOG_E(TAG, "Missing IV");
                break;
            }
            subghz_keystore_mess_with_iv(iv);
        }

        if(!flipper_format_read_string(flipper_format, "Encrypt_data", str_temp)) {
            FURI_LOG_E(TAG, "Missing Encrypt_data");
            break;
        }

        size_t bufer_size;
        if(len <= (16 - offset % 16)) {
            bufer_size = 32;
        } else {
            bufer_size = (((len) / 16) + 2) * 32;
        }
        furi_assert(SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE >= bufer_size / 2);

        uint8_t buffer[bufer_size];
        size_t ret = 0;
        bool decrypted = true;
        //skip the end of the previous line "\n"
        stream_read(stream, buffer, 1);

        size_t size = stream_size(stream);
        size -= stream_tell(stream);
        if(size < (offset * 2 + len * 2)) {
            FURI_LOG_E(TAG, "Seek position exceeds file size");
            break;
        }

        if(offset >= 16) {
            stream_seek(stream, ((offset / 16) - 1) * 32, StreamOffsetFromCurrent);
            ret = stream_read(stream, buffer, 32);
            furi_assert(ret == 32);
            for(uint16_t i = 0; i < ret - 1; i += 2) {
                uint8_t hi_nibble = 0;
                uint8_t lo_nibble = 0;
                hex_char_to_hex_nibble(buffer[i], &hi_nibble);
                hex_char_to_hex_nibble(buffer[i + 1], &lo_nibble);
                iv[i / 2] = (hi_nibble << 4) | lo_nibble;
            }
        }

        if(!furi_hal_crypto_enclave_load_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT, iv)) {
            FURI_LOG_E(TAG, "Unable to load encryption key");
            break;
        }

        do {
            memset(buffer, 0, bufer_size);
            ret = stream_read(stream, buffer, bufer_size);
            furi_assert(ret == bufer_size);
            for(uint16_t i = 0; i < ret - 1; i += 2) {
                uint8_t hi_nibble = 0;
                uint8_t lo_nibble = 0;
                hex_char_to_hex_nibble(buffer[i], &hi_nibble);
                hex_char_to_hex_nibble(buffer[i + 1], &lo_nibble);
                buffer[i / 2] = (hi_nibble << 4) | lo_nibble;
            }

            memset(decrypted_line, 0, SUBGHZ_KEYSTORE_FILE_DECRYPTED_LINE_SIZE);

            if(!furi_hal_crypto_decrypt(
                   (uint8_t*)buffer, (uint8_t*)decrypted_line, bufer_size / 2)) {
                decrypted = false;
                FURI_LOG_E(TAG, "Decryption failed");
                break;
            }
            memcpy(data, (uint8_t*)decrypted_line + (offset - (offset / 16) * 16), len);

        } while(0);
        furi_hal_crypto_enclave_unload_key(SUBGHZ_KEYSTORE_FILE_ENCRYPTION_KEY_SLOT);
        if(decrypted) result = true;
    } while(0);
    flipper_format_free(flipper_format);

    furi_record_close(RECORD_STORAGE);

    free(decrypted_line);

    furi_string_free(str_temp);

    return result;
}
