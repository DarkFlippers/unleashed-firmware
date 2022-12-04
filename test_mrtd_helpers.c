#include <stdio.h>
#include <stdlib.h>
#include <mbedtls/sha1.h>
#include <mbedtls/des.h>

#include "applications/main/nfc/test_bac_creds.h" //TODO: remove

#include "lib/nfc/protocols/mrtd_helpers.h"

// gcc -o test_mrtd_helpers -Wall -Ilib/mbedtls/include -Itoolchain/x86_64-linux/arm-none-eabi/include/ lib/nfc/protocols/mrtd_helpers.c lib/mbedtls/library/sha1.c lib/mbedtls/library/des.c lib/mbedtls/library/platform_util.c test_mrtd_helpers.c

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RESET "\033[0;0m"

void print_hex(const uint8_t* data, size_t length) {
    for(uint8_t i=0; i<length; ++i) {
        printf("%02X", data[i]);
    }
}

void test_mrtd_bac_check_digit(const char* input, const uint8_t exp_output) {
    uint8_t output = mrtd_bac_check_digit(input, strlen(input));
    if(output != exp_output) {
        printf(COLOR_RED "FAILED  - mrtd_bac_check_digit for %s is not %d, but %d\n" COLOR_RESET,
                input, exp_output, output);
        return;
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_bac_check_digit for %s is %d\n" COLOR_RESET,
            input, output);
}

void test_bac_get_kmrz(MrtdAuthData* auth, const char* exp_output) {
    bool result;
    char buffer[255];

    result = mrtd_bac_get_kmrz(auth, buffer, 255);
    if(!result) {
        printf(COLOR_RED "FAILED  - mrtd_bac_get_kmrz returned FALSE for" COLOR_RESET);
        return;
    }

    if(strcmp(exp_output, buffer)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_get_kmrz expected:\n%s, result:\n%s\n" COLOR_RESET,
            exp_output,
            buffer);
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_bac_get_kmrz is: %s\n" COLOR_RESET,
        buffer);
}

void test_sha1(const uint8_t* data, const uint8_t* exp_output) {
    uint8_t hash[20];
    mbedtls_sha1(data, strlen((char*)data), hash);

    if(memcmp(hash, exp_output, 20)) {
        printf(COLOR_RED "FAILED  - sha1 of %s, expected:\n", data);
        print_hex(exp_output, 20);
        printf(", result:\n");
    } else {
        printf(COLOR_GREEN "SUCCESS - sha1 of %s is: ", data);
    }

    print_hex(hash, 20);
    printf("\n" COLOR_RESET);
}

void test_mrtd_bac_keys_from_seed(const uint8_t kseed[16], const uint8_t exp_ksenc[16], const uint8_t exp_ksmac[16]) {
    uint8_t ksenc[16];
    uint8_t ksmac[16];
    if(!mrtd_bac_keys_from_seed(kseed, ksenc, ksmac)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys_from_seed returned FALSE for ");
        print_hex(kseed, 16);
        printf(COLOR_RESET "\n");
        return;
    }

    if(memcmp(exp_ksenc, ksenc, 16)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys_from_seed of ");
        print_hex(kseed, 16);
        printf(", expected ksenc:\n");
        print_hex(exp_ksenc, 16);
        printf(" is:\n");
        print_hex(ksenc, 16);
        return;
    } else if(memcmp(exp_ksmac, ksmac, 16)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys_from_seed of ");
        print_hex(kseed, 16);
        printf(", expected ksmac:\n");
        print_hex(exp_ksmac, 16);
        printf(" is:\n");
        print_hex(ksmac, 16);
        return;
    } else {
        printf(COLOR_GREEN "SUCCESS - mrtd_bac_keys_from_seed of ");
        print_hex(kseed, 16);
        printf(" ksenc: ");
        print_hex(ksenc, 16);
        printf(" ksmac: ");
        print_hex(ksmac, 16);
        printf(COLOR_RESET "\n");
    }
}

void test_mrtd_bac_keys(MrtdAuthData* auth, const uint8_t exp_ksenc[16], const uint8_t exp_ksmac[16]) {
    uint8_t ksenc[16];
    uint8_t ksmac[16];
    if(!mrtd_bac_keys(auth, ksenc, ksmac)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys returned FALSE\n" COLOR_RESET);
        return;
    }

    if(memcmp(exp_ksenc, ksenc, 16)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys, expected ksenc:\n");
        print_hex(exp_ksenc, 16);
        printf(" is:\n");
        print_hex(ksenc, 16);
        return;
    } else if(memcmp(exp_ksmac, ksmac, 16)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_keys, expected ksmac:\n");
        print_hex(exp_ksmac, 16);
        printf(" is:\n");
        print_hex(ksmac, 16);
        return;
    } else {
        printf(COLOR_GREEN "SUCCESS - mrtd_bac_keys ksenc: ");
        print_hex(ksenc, 16);
        printf(" ksmac: ");
        print_hex(ksmac, 16);
        printf(COLOR_RESET "\n");
    }
}

void test_mrtd_bac_encrypt(uint8_t* data, size_t data_length, uint8_t* key, uint8_t* exp_output, size_t exp_output_length) {
    uint8_t buffer[256];

    // buffer size must be at least ((data_length+8)/8)*8
    mrtd_bac_encrypt(data, data_length, key, buffer);

    if(memcmp(exp_output, buffer, exp_output_length)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_encrypt, expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(buffer, exp_output_length);
        return;
    } else {
        printf(COLOR_GREEN "SUCCESS - mrtd_bac_encrypt output: ");
        print_hex(buffer, exp_output_length);
        printf(COLOR_RESET "\n");
    }
}

void test_mrtd_bac_padded_mac(uint8_t* data, size_t data_length, uint8_t* key, uint8_t* exp_output, size_t exp_output_length) {
    uint8_t mac[8];
    if(!mrtd_bac_padded_mac(data, data_length, key, mac)) {
        printf("ERROR BAC MAC\n");
        return;
    }

    if(memcmp(exp_output, mac, exp_output_length)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_padded_mac, expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(mac, 8);
        printf(COLOR_RESET "\n");
        return;
    } else {
        printf(COLOR_GREEN "SUCCESS - mrtd_bac_padded_mac output: ");
        print_hex(mac, 8);
        printf(COLOR_RESET "\n");
    }
}

void test_mrtd_bac_mac_calls(uint8_t* data, size_t data_length, uint8_t* key, uint8_t* exp_output, size_t exp_output_length) {
    mrtd_bac_mac_ctx ctx;

    for(int break_at=0; break_at<data_length; break_at += 3) {
        if(!mrtd_bac_mac_init(&ctx, key)) {
            printf("ERROR mrtd_bac_mac_init (break_at: %d)\n", break_at);
            return;
        }

        uint8_t mac[8];

        if(!mrtd_bac_mac_update(&ctx, data, break_at)) {
            printf("ERROR mrtd_bac_mac_update 1 (break_at: %d)\n", break_at);
        }

        if(!mrtd_bac_mac_update(&ctx, data + break_at, data_length - break_at)) {
            printf("ERROR mrtd_bac_mac_update 2 (break_at: %d)\n", break_at);
        }

        if(!mrtd_bac_mac_finalize(&ctx, mac)) {
            printf("ERROR mrtd_bac_mac_finalize (break_at: %d)\n", break_at);
            return;
        }

        if(memcmp(exp_output, mac, exp_output_length)) {
            printf(COLOR_RED "FAILED  - mrtd_bac_mac calls (break_at: %d), expected output:\n", break_at);
            print_hex(exp_output, exp_output_length);
            printf(" is:\n");
            print_hex(mac, 8);
            printf(COLOR_RESET "\n");
            return;
        } else {
            printf(COLOR_GREEN "SUCCESS - mrtd_bac_mac calls (break_at: %d) output: ", break_at);
            print_hex(mac, 8);
            printf(COLOR_RESET "\n");
        }
    }
}

void test_mrtd_bac_mac_steps(uint8_t* data, size_t data_length, uint8_t* key, uint8_t* exp_output, size_t exp_output_length) {
    mrtd_bac_mac_ctx ctx;

    printf("A\n");

    if(!mrtd_bac_mac_init(&ctx, key)) {
        printf("ERROR mrtd_bac_mac_init (steps)\n");
        return;
    }

    if(!mrtd_bac_mac_update(&ctx, data, 8)) {
        printf("ERROR mrtd_bac_mac_update 1 (steps)\n");
    }

    if(!mrtd_bac_mac_update(&ctx, data + 8, 4)) {
        printf("ERROR mrtd_bac_mac_update 2 (steps)\n");
    }

    if(!mrtd_bac_mac_pad(&ctx)) {
        printf("ERROR mrtd_bac_mac_pad (steps)\n");
    }

    if(!mrtd_bac_mac_update(&ctx, data + 16, 11)) {
        printf("ERROR mrtd_bac_mac_update 3 (steps)\n");
    }

    uint8_t mac[8];
    if(!mrtd_bac_mac_finalize(&ctx, mac)) {
        printf("ERROR mrtd_bac_mac_finalize (steps)\n");
        return;
    }

    if(memcmp(exp_output, mac, exp_output_length)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_mac (steps), expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(mac, 8);
        printf(COLOR_RESET "\n");
        return;
    } else {
        printf(COLOR_GREEN "SUCCESS - mrtd_bac_mac (steps) output: ");
        print_hex(mac, 8);
        printf(COLOR_RESET "\n");
    }
}

void test_mrtd_bac_decrypt_verify(const uint8_t* data, size_t data_length, uint8_t* key_enc, uint8_t* key_mac, uint8_t* exp_output, size_t exp_output_length, bool should_verify) {
    uint8_t buffer[256];

    bool result = mrtd_bac_decrypt_verify(data, data_length, key_enc, key_mac, buffer);
    if(result != should_verify) {
        printf(COLOR_RED "FAILED  - mrtd_bac_decrypt_verify, expected verify: %d, but is: %d\n" COLOR_RESET, should_verify, result);
        return;
    }

    if(memcmp(exp_output, buffer, exp_output_length)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_decrypt_verify, expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(buffer, 32);
        printf(COLOR_RESET "\n");
        return;
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_bac_decrypt_verify output: ");
    print_hex(buffer, exp_output_length);
    printf(COLOR_RESET "\n");
}

void test_mrtd_bac_decrypt_verify_sm(const uint8_t* data, size_t data_length, uint8_t* key_enc, uint8_t* key_mac, uint64_t ssc, uint8_t* exp_output, size_t exp_output_length, uint16_t exp_code) {
    uint8_t buffer[256];
    uint16_t ret_code;
    size_t buffer_len;

    ret_code = mrtd_bac_decrypt_verify_sm(data, data_length, key_enc, key_mac, ssc, buffer, &buffer_len);
    if(ret_code != exp_code) {
        printf(COLOR_RED "FAILED  - mrtd_bac_decrypt_verify_sm, expected ret_code: %04X, but is: %04X\n" COLOR_RESET, exp_code, ret_code);
        return;
    }

    if(memcmp(exp_output, buffer, exp_output_length)) {
        printf(COLOR_RED "FAILED  - mrtd_bac_decrypt_verify_sm, expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(buffer, 32);
        printf(COLOR_RESET "\n");
        return;
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_bac_decrypt_verify_sm output: ");
    print_hex(buffer, exp_output_length);
    printf(COLOR_RESET "\n");
}

void test_mrtd_ssc_from_data(const uint8_t* rnd_ic, const uint8_t* rnd_ifd, uint64_t exp_ssc) {
    uint64_t ssc_long = mrtd_ssc_from_data(rnd_ic, rnd_ifd);

    if(ssc_long != exp_ssc) {
        printf(COLOR_RED "FAILED  - mrtd_ssc_from_data, expected ssc: %016lx, but is: %016lx\n" COLOR_RESET, exp_ssc, ssc_long);
        return;
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_ssc_from_data output: %016lx\n" COLOR_RESET, ssc_long);
}

void test_mrtd_protect_apdu(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t lc, const void* data, int16_t le, uint8_t* ks_enc, uint8_t* ks_mac, uint64_t ssc, uint8_t* exp_output, size_t exp_output_length) {
    uint8_t buffer[4096];

    size_t ret = mrtd_protect_apdu(cla, ins, p1, p2, lc, data, le, ks_enc, ks_mac, ssc, buffer);

    if(!ret) {
        printf(COLOR_RED "FAILED - mrtd_protect_apdu returned an error\n" COLOR_RESET);
        return;
    }

    if(memcmp(buffer, exp_output, ret)) {
        printf(COLOR_RED "FAILED - mrtd_protect_apdu, expected output:\n");
        print_hex(exp_output, exp_output_length);
        printf(" is:\n");
        print_hex(buffer, ret);
        printf("\n" COLOR_RESET);
        return;
    }

    printf(COLOR_GREEN "SUCCESS - mrtd_protect_apdu output: ");
    print_hex(buffer, ret);
    printf("\n" COLOR_RESET);
    //TODO: hexdump
}

int main(int argc, char** argv) {
    test_mrtd_bac_check_digit("D23145890734", 9);
    test_mrtd_bac_check_digit("340712", 7);
    test_mrtd_bac_check_digit("950712", 2);

    test_bac_get_kmrz(&(MrtdAuthData){
        .doc_number = "D23145890734",
        .birth_date = {34, 7, 12},
        .expiry_date = {95, 7, 12},
        }, "D23145890734934071279507122"
    );
    test_bac_get_kmrz(&(MrtdAuthData){
            .doc_number = "L898902C",
            .birth_date = {69, 8, 6},
            .expiry_date = {94, 6, 23},
        }, "L898902C<369080619406236"
    );

    test_sha1((uint8_t*)"L898902C<369080619406236", (uint8_t*)"\x23\x9a\xb9\xcb\x28\x2d\xaf\x66\x23\x1d\xc5\xa4\xdf\x6b\xfb\xae\xdf\x47\x75\x65");

    test_mrtd_bac_keys_from_seed(
        (uint8_t*)"\x23\x9a\xb9\xcb\x28\x2d\xaf\x66\x23\x1d\xc5\xa4\xdf\x6b\xfb\xae",
        (uint8_t*)"\xab\x94\xfd\xec\xf2\x67\x4f\xdf\xb9\xb3\x91\xf8\x5d\x7f\x76\xf2",
        (uint8_t*)"\x79\x62\xd9\xec\xe0\x3d\x1a\xcd\x4c\x76\x08\x9d\xce\x13\x15\x43"
    );

    test_mrtd_bac_keys(&(MrtdAuthData){
            .doc_number = "L898902C",
            .birth_date = {69, 8, 6},
            .expiry_date = {94, 6, 23},
        },
        (uint8_t*)"\xab\x94\xfd\xec\xf2\x67\x4f\xdf\xb9\xb3\x91\xf8\x5d\x7f\x76\xf2",
        (uint8_t*)"\x79\x62\xd9\xec\xe0\x3d\x1a\xcd\x4c\x76\x08\x9d\xce\x13\x15\x43"
    );

    test_mrtd_bac_encrypt(
        /*input*/ (uint8_t*)"\x78\x17\x23\x86\x0C\x06\xC2\x26\x46\x08\xF9\x19\x88\x70\x22\x12\x0B\x79\x52\x40\xCB\x70\x49\xB0\x1C\x19\xB3\x3E\x32\x80\x4F\x0B",
        /*size*/  32,
        /*key*/  (uint8_t*)"\xAB\x94\xFD\xEC\xF2\x67\x4F\xDF\xB9\xB3\x91\xF8\x5D\x7F\x76\xF2",
        /*exp output*/ (uint8_t*)"\x72\xC2\x9C\x23\x71\xCC\x9B\xDB\x65\xB7\x79\xB8\xE8\xD3\x7B\x29\xEC\xC1\x54\xAA\x56\xA8\x79\x9F\xAE\x2F\x49\x8F\x76\xED\x92\xF2",
        /*exp_output_size*/ 32
    );

    test_mrtd_bac_padded_mac(
        /*input*/ (uint8_t*)"\x72\xC2\x9C\x23\x71\xCC\x9B\xDB\x65\xB7\x79\xB8\xE8\xD3\x7B\x29\xEC\xC1\x54\xAA\x56\xA8\x79\x9F\xAE\x2F\x49\x8F\x76\xED\x92\xF2",
        /*size*/  32,
        /*key*/  (uint8_t*)"\x79\x62\xD9\xEC\xE0\x3D\x1A\xCD\x4C\x76\x08\x9D\xCE\x13\x15\x43",
        /*exp output*/ (uint8_t*)"\x5F\x14\x48\xEE\xA8\xAD\x90\xA7",
        /*exp_output_size*/ 8
    );

    test_mrtd_bac_mac_calls(
        /*input*/ (uint8_t*)"\x72\xC2\x9C\x23\x71\xCC\x9B\xDB\x65\xB7\x79\xB8\xE8\xD3\x7B\x29\xEC\xC1\x54\xAA\x56\xA8\x79\x9F\xAE\x2F\x49\x8F\x76\xED\x92\xF2",
        /*size*/  32,
        /*key*/  (uint8_t*)"\x79\x62\xD9\xEC\xE0\x3D\x1A\xCD\x4C\x76\x08\x9D\xCE\x13\x15\x43",
        /*exp output*/ (uint8_t*)"\x5F\x14\x48\xEE\xA8\xAD\x90\xA7",
        /*exp_output_size*/ 8
    );

    test_mrtd_bac_decrypt_verify(
        /*input*/ (uint8_t*)"\x46\xB9\x34\x2A\x41\x39\x6C\xD7\x38\x6B\xF5\x80\x31\x04\xD7\xCE\xDC\x12\x2B\x91\x32\x13\x9B\xAF\x2E\xED\xC9\x4E\xE1\x78\x53\x4F\x2F\x2D\x23\x5D\x07\x4D\x74\x49",
        /*size*/  40,
        /*key_enc*/  (uint8_t*)"\xAB\x94\xFD\xEC\xF2\x67\x4F\xDF\xB9\xB3\x91\xF8\x5D\x7F\x76\xF2",
        /*key_mac*/  (uint8_t*)"\x79\x62\xD9\xEC\xE0\x3D\x1A\xCD\x4C\x76\x08\x9D\xCE\x13\x15\x43",
        /*exp output*/ (uint8_t*)"\x46\x08\xF9\x19\x88\x70\x22\x12\x78\x17\x23\x86\x0C\x06\xC2\x26\x0B\x4F\x80\x32\x3E\xB3\x19\x1C\xB0\x49\x70\xCB\x40\x52\x79\x0B",
        /*exp_output_size*/ 32,
        /*should_verify*/ 1
    );

    //TODO: test that does not verify

    uint8_t* ks_enc = (uint8_t*)"\x97\x9E\xC1\x3B\x1C\xBF\xE9\xDC\xD0\x1A\xB0\xFE\xD3\x07\xEA\xE5";
    uint8_t* ks_mac = (uint8_t*)"\xF1\xCB\x1F\x1F\xB5\xAD\xF2\x08\x80\x6B\x89\xDC\x57\x9D\xC1\xF8";

    test_mrtd_bac_keys_from_seed(
        (uint8_t*)"\x00\x36\xD2\x72\xF5\xC3\x50\xAC\xAC\x50\xC3\xF5\x72\xD2\x36\x00",
        ks_enc,
        ks_mac
    );

    uint8_t* rnd_ic = (uint8_t*)"\x46\x08\xF9\x19\x88\x70\x22\x12";
    uint8_t* rnd_ifd = (uint8_t*)"\x78\x17\x23\x86\x0C\x06\xC2\x26";

    uint64_t ssc = 0x887022120C06C226;
    test_mrtd_ssc_from_data(rnd_ic, rnd_ifd, ssc);

    ssc++;

    test_mrtd_protect_apdu(0x00, 0xA4, 0x02, 0x0C, 0x02, "\x01\x1e", -1, ks_enc, ks_mac, ssc, (uint8_t*)"\x0C\xA4\x02\x0C\x15\x87\x09\x01\x63\x75\x43\x29\x08\xC0\x44\xF6\x8E\x08\xBF\x8B\x92\xD6\x35\xFF\x24\xF8\x00", 27);

    ssc++; // Increment for decrypt, verify

    test_mrtd_bac_decrypt_verify_sm((uint8_t*)"\x99\x02\x90\x00\x8E\x08\xFA\x85\x5A\x5D\x4C\x50\xA8\xED", 14, ks_enc, ks_mac, ssc, NULL, 0, 0x9000);

    ssc++; // Increment for encrypt, sign

    test_mrtd_protect_apdu(0x00, 0xB0, 0x00, 0x00, 0x00, NULL, 0x04, ks_enc, ks_mac, ssc, (uint8_t*)"\x0C\xB0\x00\x00\x0D\x97\x01\x04\x8E\x08\xED\x67\x05\x41\x7E\x96\xBA\x55\x00", 19);

    ssc++; // Increment for decrypt, verify

    test_mrtd_bac_decrypt_verify_sm((uint8_t*)"\x87\x09\x01\x9F\xF0\xEC\x34\xF9\x92\x26\x51\x99\x02\x90\x00\x8E\x08\xAD\x55\xCC\x17\x14\x0B\x2D\xED", 25, ks_enc, ks_mac, ssc, (uint8_t*)"\x60\x14\x5F\x01", 4, 0x9000);

    ssc++; // Increment for encrypt, sign

    test_mrtd_protect_apdu(0x00, 0xB0, 0x00, 0x04, 0x00, NULL, 0x12, ks_enc, ks_mac, ssc, (uint8_t*)"\x0C\xB0\x00\x04\x0D\x97\x01\x12\x8E\x08\x2E\xA2\x8A\x70\xF3\xC7\xB5\x35\x00", 19);

    // Verify working against mrtdreader

    printf("=====================================\n\n");

    //TODO: set auth data
    MrtdAuthData auth;
    auth.birth_date = TODO_REMOVE_ID_DOB;
    auth.expiry_date = TODO_REMOVE_ID_DOE;
    memcpy(auth.doc_number, TODO_REMOVE_ID_DOC, 9);
    uint8_t kenc[16];
    uint8_t kmac[16];
    mrtd_bac_keys(&auth, kenc, kmac);

    printf("kenc: "); print_hex(kenc, 16); printf("\n");
    printf("kmac: "); print_hex(kmac, 16); printf("\n");

    uint8_t buffer[32]; // RND.IC || RND.IFD || KIC

    //TODO: set challenge rx
    mrtd_bac_decrypt_verify((uint8_t*)"\xDA\x35\xDF\x28\x7E\x9C\xE1\x25\x39\xD5\x66\xBA\x16\xF7\x16\x46\xCA\x7A\xBC\x0C\x98\x54\x55\x84\x50\x9E\xC1\x91\xB3\x06\x6B\x56\xBD\x10\xD0\xE9\x13\x83\xA9\x97", 40, kenc, kmac, buffer);
    //TODO: set kifd
    uint8_t *kifd = (uint8_t*)"\x1D\xF3\x5C\xFF\x0F\xF9\xE0\xBA\x36\x89\x63\xAE\xAF\xC8\x26\x64";

    printf("buffer: "); print_hex(buffer, 32); printf("\n");
    // 8F763C0B1CDF9F9D|0983F7C136155248|7A705FD193C6A6328C42264A3804002C
    rnd_ic = buffer;
    rnd_ifd = buffer+8;
    uint8_t *kic = buffer+16;
    printf("kifd: "); print_hex(kifd, 16); printf("\n");
    printf("kicc: "); print_hex(kic, 16); printf("\n");
    uint8_t kseed[16];
    for(uint8_t i=0; i<16; ++i) {
        kseed[i] = kifd[i] ^ kic[i];
        printf("seed %2d = %02X ^ %02X = %02X\r\n", i, kifd[i], kic[i], kseed[i]);
    }
    printf("kseed: "); print_hex(kseed, 16); printf("\n");

    ks_enc = malloc(16);
    ks_mac = malloc(16);
    mrtd_bac_keys_from_seed(kseed, ks_enc, ks_mac);
    printf("ks_enc: "); print_hex(ks_enc, 16); printf("\n");
    printf("ks_mac: "); print_hex(ks_mac, 16); printf("\n");

    printf("rnd_ic: "); print_hex(rnd_ic, 8); printf("\n");
    printf("rnd_ifd: "); print_hex(rnd_ifd, 8); printf("\n");
    ssc = mrtd_ssc_from_data(rnd_ic, rnd_ifd);
    printf("ssc: %016lx\n", ssc);

    ssc++;

    ssc+=11;

    test_mrtd_bac_decrypt_verify_sm((uint8_t*)"\x87\x81\xE9\x01\x25\xF5\xD5\xB9\x8C\x5D\xF6\xDB\x5C\xC2\x79\x49\x1F\x3B\xDA\xA9\xC3\x55\x95\xE2\x33\xBD\xE6\x1F\xA5\x41\xD7\xF0\x8A\xCB\x01\x6F\xF7\xD3\xCF\x33\x3A\x65\x8C\x40\x37\x06\xDE\xB7\xB6\x1D\x73\x88\x04\x12\xC1\xD1\x52\x04\xC1\xA1\x84\x9F\xD9\x34\x60\x2B\x5F\x30\xD1\xDD\xFB\x37\xE7\x7D\xE8\xC1\x38\x72\x0F\x6C\x69\x12\x14\xB3\x8E\x4C\x19\x8A\x9F\x0F\x39\x08\xD4\xF5\xA4\xBE\x0C\xD0\xD9\x72\x24\xCE\x76\x45\xD3\xCC\xD2\x02\x53\xDE\x49\x77\x0F\xD5\x5E\xBE\x20\x8F\x9F\xFD\x89\x90\xBD\x5C\x44\x74\xE9\x76\xFB\xAA\x81\x35\x6B\xC0\x49\x5D\x5E\x1B\xC9\x18\x85\xFA\xC5\x82\x6F\x7B\x8F\x0F\x1B\x03\x30\xCE\x25\x90\x6E\x3E\xA0\xF4\x01\xA6\xF4\xAE\x02\xF8\x30\x29\x25\xEB\x0A\x10\x31\x8A\x89\xB6\x6B\x8C\xC5\x2E\xE6\xCC\xB8\xFA\xEC\x64\x36\x8D\x5A\x3F\x5A\x31\x67\x26\x01\x85\x19\x98\x0A\x69\x10\x8F\x5F\x71\xAA\x6C\x6E\x1C\xEB\x8A\x40\xD1\x87\xEE\x2A\x0D\xE7\xA3\x61\x92\x6A\x46\x3B\x8C\x79\x5F\x1E\xA2\xE4\x76\x59\x71\xD7\xE4\xFE\x41\xC0\x8A\x99\x02\x90\x00\x8E\x08\x0F\xE2\xD4\x0B\xED\xD6\x66\xA2", 250, ks_enc, ks_mac, ssc, NULL, 0, 0x9000);

    //TODO: set challenge TX for verification
    //test_mrtd_protect_apdu(0x00, 0xA4, 0x02, 0x0C, 0x02, "\x01\x01", -1, ks_enc, ks_mac, ssc,
            //(uint8_t*)"\x0c\xa4\x02\x0c\x15\x87\x09\x01\xc8\xcc\x50\x6f\x50\xae\x10\xc7\x8e\x08\xaf\xec\x2e\x03\x90\x26\x8f\xa5\x00", 27);

    /*
    uint8_t* select_ef_com = "\x0C\xA4\x02\x0C\x15\x87\x09\x01\xE2\x94\xA2\x9A\xF3\x73\xFD\x20\x8E\x08\x7E\x3B\xA9\xAA\x7C\xB9\x07\x0C\x00";
    uint8_t* select_ef_dg1 = "\x0C\xA4\x02\x0C\x15\x87\x09\x01\x9C\xD7\x89\x94\x97\x05\xB8\xF3\x8E\x08\x6C\xA2\xC1\x48\xA7\x47\xBA\x96\x00";
    uint8_t buffer2[256];

    mrtd_bac_decrypt(select_ef_dg1 + 8, 8, ks_enc, buffer2);
    printf("Decrypted: ");
    print_hex(buffer2, 8);
    */

    return 0;
}
