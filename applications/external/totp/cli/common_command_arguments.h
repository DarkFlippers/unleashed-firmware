#pragma once
#include <stdlib.h>
#include "../types/token_info.h"
#include "cli_helpers.h"

#define TOTP_CLI_COMMAND_ARG_NAME "name"
#define TOTP_CLI_COMMAND_ARG_NAME_PREFIX "-n"
#define TOTP_CLI_COMMAND_ARG_ALGO "algo"
#define TOTP_CLI_COMMAND_ARG_ALGO_PREFIX "-a"
#define TOTP_CLI_COMMAND_ARG_DIGITS "digits"
#define TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX "-d"
#define TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX "-u"
#define TOTP_CLI_COMMAND_ARG_DURATION "duration"
#define TOTP_CLI_COMMAND_ARG_DURATION_PREFIX "-l"
#define TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX "-b"
#define TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE "feature"
#define TOTP_CLI_COMMAND_ARG_INDEX "index"
#define TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX "-e"
#define TOTP_CLI_COMMAND_ARG_SECRET_ENCODING "encoding"

void totp_cli_printf_unknown_argument(const FuriString* arg);
void totp_cli_printf_missed_argument_value(char* arg);
bool totp_cli_try_read_algo(TokenInfo* token_info, FuriString* arg, FuriString* args, bool* parsed);
bool totp_cli_try_read_digits(
    TokenInfo* token_info,
    const FuriString* arg,
    FuriString* args,
    bool* parsed);
bool totp_cli_try_read_duration(
    TokenInfo* token_info,
    const FuriString* arg,
    FuriString* args,
    bool* parsed);
bool totp_cli_try_read_automation_features(
    TokenInfo* token_info,
    FuriString* arg,
    FuriString* args,
    bool* parsed);
bool totp_cli_try_read_unsecure_flag(const FuriString* arg, bool* parsed, bool* unsecure_flag);

bool totp_cli_try_read_plain_token_secret_encoding(
    FuriString* arg,
    FuriString* args,
    bool* parsed,
    PlainTokenSecretEncoding* secret_encoding);