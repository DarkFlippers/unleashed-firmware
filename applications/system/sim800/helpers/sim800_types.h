#pragma once

#include <furi.h>
#include <storage/storage.h>

#define SIM800_PHONEBOOK_FOLDER EXT_PATH("phonebook")
#define SIM800_PHONE_EXTENSION  ".phone"
#define SIM800_PRESET_FOLDER    APP_DATA_PATH("presets")
#define SIM800_PRESET_EXTENSION ".smspr"
#define SIM800_SETTINGS_PATH    APP_DATA_PATH("sim800.conf")
#define SIM800_HISTORY_PATH     APP_DATA_PATH("history.log")

#define SIM800_NUMBER_MAX   24
#define SIM800_NAME_MAX     32
// One GSM-7 SMS part is 160 septets
#define SIM800_SMS_TEXT_MAX 161
#define SIM800_AT_CMD_MAX   80
#define SIM800_USSD_MAX     24
#define SIM800_DTMF_MAX     24

// Shared text input buffer, so it holds the longest thing any scene asks for
#define SIM800_TEXT_STORE_SIZE SIM800_SMS_TEXT_MAX

#define SIM800_INBOX_MAX 24
#define SIM800_LOG_MAX   2048

#define SIM800_PRESET_MAX 20
