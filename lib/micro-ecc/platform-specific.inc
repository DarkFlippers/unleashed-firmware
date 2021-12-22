/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_PLATFORM_SPECIFIC_H_
#define _UECC_PLATFORM_SPECIFIC_H_

#include "types.h"

#if (defined(_WIN32) || defined(_WIN64))
/* Windows */

// use pragma syntax to prevent tweaking the linker script for getting CryptXYZ function
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

static int default_RNG(uint8_t *dest, unsigned size) {
    HCRYPTPROV prov;
    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return 0;
    }

    CryptGenRandom(prov, size, (BYTE *)dest);
    CryptReleaseContext(prov, 0);
    return 1;
}
#define default_RNG_defined 1

#elif defined(unix) || defined(__linux__) || defined(__unix__) || defined(__unix) || \
    (defined(__APPLE__) && defined(__MACH__)) || defined(uECC_POSIX)

/* Some POSIX-like system with /dev/urandom or /dev/random. */
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_CLOEXEC
    #define O_CLOEXEC 0
#endif

static int default_RNG(uint8_t *dest, unsigned size) {
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        fd = open("/dev/random", O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            return 0;
        }
    }
    
    char *ptr = (char *)dest;
    size_t left = size;
    while (left > 0) {
        ssize_t bytes_read = read(fd, ptr, left);
        if (bytes_read <= 0) { // read failed
            close(fd);
            return 0;
        }
        left -= bytes_read;
        ptr += bytes_read;
    }
    
    close(fd);
    return 1;
}
#define default_RNG_defined 1

#elif defined(RIOT_VERSION)

#include <random.h>

static int default_RNG(uint8_t *dest, unsigned size) {
    random_bytes(dest, size);
    return 1;
}
#define default_RNG_defined 1

#elif defined(NRF52_SERIES)

#include "app_error.h"
#include "nrf_crypto_rng.h"

static int default_RNG(uint8_t *dest, unsigned size) 
{
    // make sure to call nrf_crypto_init and nrf_crypto_rng_init first
    ret_code_t ret_code = nrf_crypto_rng_vector_generate(dest, size);
    return (ret_code == NRF_SUCCESS) ? 1 : 0;
}
#define default_RNG_defined 1

#endif /* platform */

#endif /* _UECC_PLATFORM_SPECIFIC_H_ */
