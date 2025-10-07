#include "nfc_cli_commands.h"
#include "nfc_cli_command_base_i.h"

/** Include new commands here */
#include "commands/raw/nfc_cli_command_raw.h"
#include "commands/apdu/nfc_cli_command_apdu.h"
#include "commands/dump/nfc_cli_command_dump.h"
#include "commands/mfu/nfc_cli_command_mfu.h"
#include "commands/nfc_cli_command_emulate.h"
#include "commands/nfc_cli_command_scanner.h"
#include "commands/nfc_cli_command_field.h"

#define TAG "NfcCliCommands"

/** Add new commands here */
static const NfcCliCommandDescriptor* nfc_cli_commands[] = {
    &apdu_cmd,
    &raw_cmd,
    &emulate_cmd,
    &mfu_cmd,
    &scanner_cmd,
    &dump_cmd,
    &field_cmd,
};

size_t nfc_cli_command_get_count() {
    return COUNT_OF(nfc_cli_commands);
}

const NfcCliActionDescriptor*
    nfc_cli_command_get_action_by_name(const NfcCliCommandDescriptor* cmd, const FuriString* name) {
    furi_assert(cmd);
    furi_assert(name);

    for(size_t i = 0; i < cmd->action_count; i++) {
        const NfcCliActionDescriptor* action = cmd->actions[i];
        if(furi_string_equal_str(name, action->name)) return action;
    }
    return NULL;
}

const NfcCliCommandDescriptor* nfc_cli_command_get_by_index(size_t index) {
    furi_assert(index < COUNT_OF(nfc_cli_commands));
    return nfc_cli_commands[index];
}

bool nfc_cli_command_has_multiple_actions(const NfcCliCommandDescriptor* cmd) {
    furi_assert(cmd);
    furi_check(cmd->action_count > 0);
    return (cmd->action_count > 1);
}

const char* nfc_cli_command_get_name(const NfcCliCommandDescriptor* cmd) {
    furi_assert(cmd);
    return cmd->name;
}

CliCommandExecuteCallback nfc_cli_command_get_execute(const NfcCliCommandDescriptor* cmd) {
    furi_assert(cmd);
    return cmd->callback;
}

static inline const NfcCliKeyDescriptor* nfc_cli_action_get_key_by_name(
    const NfcCliActionDescriptor* action,
    const FuriString* name,
    bool long_name) {
    for(size_t i = 0; i < action->key_count; i++) {
        const NfcCliKeyDescriptor* key = &action->keys[i];
        const char* buf = long_name ? key->long_name : key->short_name;
        if((buf != NULL) && furi_string_equal_str(name, buf)) return key;
    }
    return NULL;
}

const NfcCliKeyDescriptor*
    nfc_cli_action_get_key_descriptor(const NfcCliActionDescriptor* action, FuriString* argument) {
    furi_assert(action);
    furi_assert(argument);

    return nfc_cli_action_get_key_by_name(action, argument, furi_string_size(argument) > 1);
}

size_t nfc_cli_action_get_required_keys_count(const NfcCliActionDescriptor* action) {
    furi_assert(action);

    size_t required_key_count = 0;
    for(size_t i = 0; i < action->key_count; i++) {
        const NfcCliKeyDescriptor* key = &action->keys[i];
        if(!key->features.required) continue;
        required_key_count++;
    }
    return required_key_count;
}

static int nfc_cli_action_format_key_name(const NfcCliKeyDescriptor* key, FuriString* output) {
    int len = 0;
    FuriString* name = furi_string_alloc();
    if(key->short_name && key->long_name) {
        len = furi_string_printf(name, "-%s, --%s", key->short_name, key->long_name);
    } else if(key->short_name && (key->long_name == NULL)) {
        len = furi_string_printf(name, "-%s", key->short_name);
    } else if((key->short_name == NULL) && key->long_name) {
        len = furi_string_printf(name, "--%s", key->long_name);
    }

    const char* color = key->features.required ? ANSI_FLIPPER_BRAND_ORANGE : ANSI_RESET;
    furi_string_printf(output, "%s%s%s", color, furi_string_get_cstr(name), ANSI_RESET);
    furi_string_free(name);
    return len;
}

void nfc_cli_action_format_info(const NfcCliActionDescriptor* action, FuriString* output) {
    furi_assert(action);
    furi_assert(output);
    furi_string_printf(
        output,
        action->description ? "%s - %s\r\n\n" : "%s\r\n\n",
        action->name,
        action->description);

    if(action->key_count == 0) return;

    FuriString* buf = furi_string_alloc();
    furi_string_cat_printf(
        output,
        ANSI_FG_BR_GREEN "Keys " ANSI_FLIPPER_BRAND_ORANGE "(required) " ANSI_RESET
                         "(optional):\r\n");

    for(size_t i = 0; i < action->key_count; i++) {
        const NfcCliKeyDescriptor* key = &action->keys[i];

        int len = nfc_cli_action_format_key_name(key, buf);
        furi_string_cat_printf(output, "%s", furi_string_get_cstr(buf));

        if(key->description) {
            const int offset = 20;
            furi_string_cat_printf(
                output, ANSI_CURSOR_RIGHT_BY("%d") "%s", offset - len, key->description);
        }
        furi_string_cat_printf(output, "\r\n");
    }
    furi_string_free(buf);
}

void nfc_cli_command_format_info(const NfcCliCommandDescriptor* cmd, FuriString* output) {
    furi_assert(cmd);
    furi_assert(output);
    furi_string_printf(output, "%s - %s\r\n", cmd->name, cmd->description);
    if(cmd->action_count > 1) {
        furi_string_cat_printf(output, "Possible actions: \r\n");

        for(size_t i = 0; i < cmd->action_count; i++) {
            const NfcCliActionDescriptor* action = cmd->actions[i];
            furi_string_cat_printf(
                output,
                action->description ? "\t%s\t-\t%s\r\n" : "%s\r\n",
                action->name,
                action->description);
        }
    }
}
