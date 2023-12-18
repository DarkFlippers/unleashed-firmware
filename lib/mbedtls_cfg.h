#pragma once

/**
* A subset of the mbedTLS configuration options that are relevant to the
* Flipper Zero firmware and apps. They are built to "mbedtls" library you can 
* link your apps with.
* 
* If you need more features, either bring the full mbedtls library into your
* app using "fap_private_libs" or open an issue on GitHub to add them to the
* default configuration.
**/

#define MBEDTLS_HAVE_ASM

#define MBEDTLS_NO_UDBL_DIVISION
#define MBEDTLS_NO_64BIT_MULTIPLICATION

#define MBEDTLS_DEPRECATED_WARNING

#define MBEDTLS_AES_FEWER_TABLES
// #define MBEDTLS_CHECK_RETURN_WARNING

#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CFB
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_CIPHER_MODE_OFB
#define MBEDTLS_CIPHER_MODE_XTS

#define MBEDTLS_CIPHER_PADDING_PKCS7
#define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
#define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
#define MBEDTLS_CIPHER_PADDING_ZEROS

/* Short Weierstrass curves (supporting ECP, ECDH, ECDSA) */
// #define MBEDTLS_ECP_DP_SECP192R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP384R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP521R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP192K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP256K1_ENABLED
// #define MBEDTLS_ECP_DP_BP256R1_ENABLED
// #define MBEDTLS_ECP_DP_BP384R1_ENABLED
// #define MBEDTLS_ECP_DP_BP512R1_ENABLED
/* Montgomery curves (supporting ECP) */
// #define MBEDTLS_ECP_DP_CURVE25519_ENABLED
// #define MBEDTLS_ECP_DP_CURVE448_ENABLED

#define MBEDTLS_ECP_NIST_OPTIM

#define MBEDTLS_GENPRIME
// #define MBEDTLS_PKCS1_V15
// #define MBEDTLS_PKCS1_V21

#define MBEDTLS_MD_C

#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C

// #define MBEDTLS_CHACHA20_C
// #define MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_DES_C
#define MBEDTLS_DHM_C

#define MBEDTLS_ECDH_C

#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C

#define MBEDTLS_GCM_C

#define MBEDTLS_AES_C
#define MBEDTLS_MD5_C

// #define MBEDTLS_PEM_PARSE_C
// #define MBEDTLS_PEM_WRITE_C

// #define MBEDTLS_PLATFORM_MEMORY
// #define MBEDTLS_PLATFORM_C

// #define MBEDTLS_RIPEMD160_C
// #define MBEDTLS_RSA_C
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA1_C

#define MBEDTLS_ERROR_C