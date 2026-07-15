#pragma once

#include "nfc_cli_command_base.h"
#include <toolbox/cli/cli_command.h>
#include <toolbox/cli/cli_ansi.h>

size_t nfc_cli_command_get_count();

const NfcCliCommandDescriptor* nfc_cli_command_get_by_index(size_t index);

const char* nfc_cli_command_get_name(const NfcCliCommandDescriptor* cmd);

CliCommandExecuteCallback nfc_cli_command_get_execute(const NfcCliCommandDescriptor* cmd);

bool nfc_cli_command_has_multiple_actions(const NfcCliCommandDescriptor* cmd);

const NfcCliActionDescriptor*
    nfc_cli_command_get_action_by_name(const NfcCliCommandDescriptor* cmd, const FuriString* name);

size_t nfc_cli_action_get_required_keys_count(const NfcCliActionDescriptor* action);

const NfcCliKeyDescriptor*
    nfc_cli_action_get_key_descriptor(const NfcCliActionDescriptor* action, FuriString* argument);

void nfc_cli_command_format_info(const NfcCliCommandDescriptor* cmd, FuriString* output);

void nfc_cli_action_format_info(const NfcCliActionDescriptor* action, FuriString* output);
