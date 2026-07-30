#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Shared UL-AES (MF0AES20) crypto primitives, used by both the poller and the listener for the
// 3-pass authentication and secure-messaging (CMAC). Kept internal to the mf_ultralight protocol
// (mirrors mf_plus_crypto.c) rather than depending across protocol libraries.

// Rotate a 16-byte block left by one byte (RndX -> RndX'), as required by the AES 3-pass scheme.
void mf_ultralight_aes_rol16(uint8_t* data);

// AES-CMAC (NIST SP 800-38B / RFC 4493) over `data`; `mac` receives the full 16-byte CMAC.
void mf_ultralight_aes_cmac(const uint8_t* key, const uint8_t* data, size_t len, uint8_t* mac);

// The UL-AES message MAC is the 8 odd-indexed bytes of the full 16-byte CMAC.
void mf_ultralight_aes_cmac8(const uint8_t* mac16, uint8_t* out8);

// Derive the secure-messaging session key from the auth randoms (SP 800-108 counter mode, MF0AES20
// §8.8.1). rnd_a/rnd_b are passed still rotated (as left after the 3-pass); they are un-rotated once
// here. `session_key` receives 16 bytes.
void mf_ultralight_aes_derive_session_key(
    const uint8_t* key,
    const uint8_t* rnd_a_rot,
    const uint8_t* rnd_b_rot,
    uint8_t* session_key);

#ifdef __cplusplus
}
#endif
