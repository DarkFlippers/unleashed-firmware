#pragma once

#define CRYPTO_IV_LENGTH (16)

// According to this explanation: https://github.com/flipperdevices/flipperzero-firmware/issues/2885#issuecomment-1646664666
// disabling usage of any key which is "the same across all devices"
#define ACCEPTABLE_CRYPTO_KEY_SLOT_START (12)
#define ACCEPTABLE_CRYPTO_KEY_SLOT_END (100)

#define DEFAULT_CRYPTO_KEY_SLOT ACCEPTABLE_CRYPTO_KEY_SLOT_START
#define CRYPTO_LATEST_VERSION (2)