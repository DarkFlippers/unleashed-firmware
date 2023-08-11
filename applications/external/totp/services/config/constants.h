#pragma once

#include <storage/storage.h>

#define CONFIG_FILE_DIRECTORY_PATH EXT_PATH("apps_data/totp")
#define CONFIG_FILE_HEADER "Flipper TOTP plugin config file"
#define CONFIG_FILE_ACTUAL_VERSION (9)

#define TOTP_CONFIG_KEY_TIMEZONE "Timezone"
#define TOTP_CONFIG_KEY_TOKEN_NAME "TokenName"
#define TOTP_CONFIG_KEY_TOKEN_SECRET "TokenSecret"
#define TOTP_CONFIG_KEY_TOKEN_ALGO "TokenAlgo"
#define TOTP_CONFIG_KEY_TOKEN_DIGITS "TokenDigits"
#define TOTP_CONFIG_KEY_TOKEN_DURATION "TokenDuration"
#define TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES "TokenAutomationFeatures"
#define TOTP_CONFIG_KEY_CRYPTO_VERIFY "Crypto"
#define TOTP_CONFIG_KEY_SALT "Salt"
#define TOTP_CONFIG_KEY_PINSET "PinIsSet"
#define TOTP_CONFIG_KEY_NOTIFICATION_METHOD "NotificationMethod"
#define TOTP_CONFIG_KEY_AUTOMATION_METHOD "AutomationMethod"
#define TOTP_CONFIG_KEY_AUTOMATION_KB_LAYOUT "AutomationKbLayout"
#define TOTP_CONFIG_KEY_FONT "Font"
#define TOTP_CONFIG_KEY_CRYPTO_VERSION "CryptoVersion"
#define TOTP_CONFIG_KEY_CRYPTO_KEY_SLOT "CryptoKeySlot"
