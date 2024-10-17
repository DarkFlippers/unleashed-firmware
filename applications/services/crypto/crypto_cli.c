#include <furi_hal.h>
#include <furi.h>

#include <lib/toolbox/args.h>
#include <cli/cli.h>

void crypto_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf(
        "\tencrypt <key_slot:int> <iv:hex>\t - Using key from secure enclave and IV encrypt plain text with AES256CBC and encode to hex\r\n");
    printf(
        "\tdecrypt <key_slot:int> <iv:hex>\t - Using key from secure enclave and IV decrypt hex encoded encrypted with AES256CBC data to plain text\r\n");
    printf("\thas_key <key_slot:int>\t - Check if secure enclave has key in slot\r\n");
    printf(
        "\tstore_key <key_slot:int> <key_type:str> <key_size:int> <key_data:hex>\t - Store key in secure enclave. !!! NON-REVERSABLE OPERATION - READ MANUAL FIRST !!!\r\n");
}

void crypto_cli_encrypt(Cli* cli, FuriString* args) {
    int key_slot = 0;
    bool key_loaded = false;
    uint8_t iv[16];

    do {
        if(!args_read_int_and_trim(args, &key_slot) || !(key_slot > 0 && key_slot <= 100)) {
            printf("Incorrect or missing slot, expected int 1-100");
            break;
        }

        if(!args_read_hex_bytes(args, iv, 16)) {
            printf("Incorrect or missing IV, expected 16 bytes in hex");
            break;
        }

        if(!furi_hal_crypto_enclave_load_key(key_slot, iv)) {
            printf("Unable to load key from slot %d", key_slot);
            break;
        }
        key_loaded = true;

        printf("Enter plain text and press Ctrl+C to complete encryption:\r\n");

        FuriString* input;
        input = furi_string_alloc();
        char c;
        while(cli_read(cli, (uint8_t*)&c, 1) == 1) {
            if(c == CliSymbolAsciiETX) {
                printf("\r\n");
                break;
            } else if(c >= 0x20 && c < 0x7F) {
                putc(c, stdout);
                fflush(stdout);
                furi_string_push_back(input, c);
            } else if(c == CliSymbolAsciiCR) {
                printf("\r\n");
                furi_string_cat(input, "\r\n");
            }
        }

        size_t size = furi_string_size(input);
        if(size > 0) {
            // C-string null termination and block alignments
            size++;
            size_t remain = size % 16;
            if(remain) {
                size = size - remain + 16;
            }
            furi_string_reserve(input, size);
            uint8_t* output = malloc(size);
            if(!furi_hal_crypto_encrypt(
                   (const uint8_t*)furi_string_get_cstr(input), output, size)) {
                printf("Failed to encrypt input");
            } else {
                printf("Hex-encoded encrypted data:\r\n");
                for(size_t i = 0; i < size; i++) {
                    if(i % 80 == 0) printf("\r\n");
                    printf("%02x", output[i]);
                }
                printf("\r\n");
            }
            free(output);
        } else {
            printf("No input");
        }

        furi_string_free(input);
    } while(0);

    if(key_loaded) {
        furi_hal_crypto_enclave_unload_key(key_slot);
    }
}

void crypto_cli_decrypt(Cli* cli, FuriString* args) {
    int key_slot = 0;
    bool key_loaded = false;
    uint8_t iv[16];

    do {
        if(!args_read_int_and_trim(args, &key_slot) || !(key_slot > 0 && key_slot <= 100)) {
            printf("Incorrect or missing slot, expected int 1-100");
            break;
        }

        if(!args_read_hex_bytes(args, iv, 16)) {
            printf("Incorrect or missing IV, expected 16 bytes in hex");
            break;
        }

        if(!furi_hal_crypto_enclave_load_key(key_slot, iv)) {
            printf("Unable to load key from slot %d", key_slot);
            break;
        }
        key_loaded = true;

        printf("Enter Hex-encoded data and press Ctrl+C to complete decryption:\r\n");

        FuriString* hex_input;
        hex_input = furi_string_alloc();
        char c;
        while(cli_read(cli, (uint8_t*)&c, 1) == 1) {
            if(c == CliSymbolAsciiETX) {
                printf("\r\n");
                break;
            } else if(c >= 0x20 && c < 0x7F) {
                putc(c, stdout);
                fflush(stdout);
                furi_string_push_back(hex_input, c);
            } else if(c == CliSymbolAsciiCR) {
                printf("\r\n");
            }
        }

        furi_string_trim(hex_input);
        size_t hex_size = furi_string_size(hex_input);
        if(hex_size > 0 && hex_size % 2 == 0) {
            size_t size = hex_size / 2;
            uint8_t* input = malloc(size);
            uint8_t* output = malloc(size);

            if(args_read_hex_bytes(hex_input, input, size)) {
                if(furi_hal_crypto_decrypt(input, output, size)) {
                    printf("Decrypted data:\r\n");
                    printf("%s\r\n", output); //-V576
                } else {
                    printf("Failed to decrypt\r\n");
                }
            } else {
                printf("Failed to parse input");
            }

            free(input);
            free(output);
        } else {
            printf("Invalid or empty input");
        }

        furi_string_free(hex_input);
    } while(0);

    if(key_loaded) {
        furi_hal_crypto_enclave_unload_key(key_slot);
    }
}

void crypto_cli_has_key(Cli* cli, FuriString* args) {
    UNUSED(cli);
    int key_slot = 0;
    uint8_t iv[16] = {0};

    do {
        if(!args_read_int_and_trim(args, &key_slot) || !(key_slot > 0 && key_slot <= 100)) {
            printf("Incorrect or missing slot, expected int 1-100");
            break;
        }

        if(!furi_hal_crypto_enclave_load_key(key_slot, iv)) {
            printf("Unable to load key from slot %d", key_slot);
            break;
        }

        printf("Successfully loaded key from slot %d", key_slot);

        furi_hal_crypto_enclave_unload_key(key_slot);
    } while(0);
}

void crypto_cli_store_key(Cli* cli, FuriString* args) {
    UNUSED(cli);
    int key_slot = 0;
    int key_size = 0;
    FuriString* key_type;
    key_type = furi_string_alloc();

    uint8_t data[32 + 12] = {};
    FuriHalCryptoKey key;
    key.data = data;
    size_t data_size = 0;

    do {
        if(!args_read_int_and_trim(args, &key_slot)) {
            printf("Incorrect or missing key type, expected master, simple or encrypted");
            break;
        }
        if(!args_read_string_and_trim(args, key_type)) {
            printf("Incorrect or missing key type, expected master, simple or encrypted");
            break;
        }

        if(furi_string_cmp_str(key_type, "master") == 0) {
            if(key_slot != 0) {
                printf("Master keyslot must be is 0");
                break;
            }
            key.type = FuriHalCryptoKeyTypeMaster;
        } else if(furi_string_cmp_str(key_type, "simple") == 0) {
            if(key_slot < 1 || key_slot > 99) {
                printf("Simple keyslot must be in range");
                break;
            }
            key.type = FuriHalCryptoKeyTypeSimple;
        } else if(furi_string_cmp_str(key_type, "encrypted") == 0) {
            key.type = FuriHalCryptoKeyTypeEncrypted;
            data_size += 12;
        } else {
            printf("Incorrect or missing key type, expected master, simple or encrypted");
            break;
        }

        if(!args_read_int_and_trim(args, &key_size)) {
            printf("Incorrect or missing key size, expected 128 or 256");
            break;
        }

        if(key_size == 128) {
            key.size = FuriHalCryptoKeySize128;
            data_size += 16;
        } else if(key_size == 256) {
            key.size = FuriHalCryptoKeySize256;
            data_size += 32;
        } else {
            printf("Incorrect or missing key size, expected 128 or 256");
        }

        if(!args_read_hex_bytes(args, data, data_size)) {
            printf("Incorrect or missing key data, expected hex encoded key with or without IV.");
            break;
        }

        if(key_slot > 0) {
            uint8_t iv[16] = {0};
            if(key_slot > 1) {
                if(!furi_hal_crypto_enclave_load_key(key_slot - 1, iv)) {
                    printf(
                        "Slot %d before %d is empty, which is not allowed",
                        key_slot - 1,
                        key_slot);
                    break;
                }
                furi_hal_crypto_enclave_unload_key(key_slot - 1);
            }

            if(furi_hal_crypto_enclave_load_key(key_slot, iv)) {
                furi_hal_crypto_enclave_unload_key(key_slot);
                printf("Key slot %d is already used", key_slot);
                break;
            }
        }

        uint8_t slot;
        if(furi_hal_crypto_enclave_store_key(&key, &slot)) {
            printf("Success. Stored to slot: %d", slot);
        } else {
            printf("Failure");
        }
    } while(0);

    furi_string_free(key_type);
}

static void crypto_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_cli_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "encrypt") == 0) {
            crypto_cli_encrypt(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "decrypt") == 0) {
            crypto_cli_decrypt(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "has_key") == 0) {
            crypto_cli_has_key(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "store_key") == 0) {
            crypto_cli_store_key(cli, args);
            break;
        }

        crypto_cli_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void crypto_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "crypto", CliCommandFlagDefault, crypto_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(crypto_cli);
#endif
}
