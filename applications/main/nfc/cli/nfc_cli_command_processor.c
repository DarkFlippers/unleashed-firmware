#include "nfc_cli_command_processor.h"
#include "nfc_cli_commands.h"
#include "nfc_cli_command_base_i.h"

#include <m-string.h>
#include <args.h>
#include <hex.h>

#define TAG "NfcCliProcessor"

#define NFC_CLI_KEYS_FOUND_SIZE_BYTES (10 * sizeof(NfcCliKeyDescriptor*))

typedef enum {
    NfcCliArgumentTypeShortNameKey,
    NfcCliArgumentTypeShortNameKeyGroup,
    NfcCliArgumentTypeLongNameKey,

    NfcCliArgumentTypeUnknown
} NfcCliArgumentType;

/**
 * @brief Error codes for different processing states
 */
typedef enum {
    NfcCliProcessorErrorNone, /**< Command was parsed successfully and execute callback will be invoked*/
    NfcCliProcessorErrorNoneButHelp, /**< There was no error, but help needs to be printed. Command wil not be executed */
    NfcCliProcessorErrorActionNotFound, /**< Wrong action was passed as first command parameter */
    NfcCliProcessorErrorKeyNotSupported, /**< Unsupported key was passed in arguments. Details will be printed in erro_message*/
    NfcCliProcessorErrorKeyParameterInGroup, /**< Parameter which requires value was passed in group. Example: -sckd */
    NfcCliProcessorErrorKeyParameterValueMissing, /**< Value is missing for the parameter which requires it */
    NfcCliProcessorErrorKeyDuplication, /**< Some argument key was duplicated in input parameters */
    NfcCliProcessorErrorKeyParseError, /**< Error happened during argument value parsing */
    NfcCliProcessorErrorKeyRequiredMissing, /**< Some keys required for command execution is missing*/

    NfcCliProcessorErrorNum
} NfcCliProcessorError;

struct NfcCliProcessorContext {
    const NfcCliCommandDescriptor* cmd;
    const NfcCliActionDescriptor* action;
    const NfcCliKeyDescriptor** keys_found;
    uint8_t total_keys_found;
    uint8_t required_keys_expected;
    uint8_t required_keys_found;

    Nfc* nfc;
    void* action_context;

    FuriString* error_message;
};

static const NfcCliActionDescriptor*
    nfc_cli_get_action_from_args(const NfcCliCommandDescriptor* cmd, FuriString* args) {
    const NfcCliActionDescriptor* action = cmd->actions[0];

    bool multiple_action_cmd = nfc_cli_command_has_multiple_actions(cmd);
    if(multiple_action_cmd) {
        action = NULL;
        FuriString* arg_str = furi_string_alloc();
        if(args_read_string_and_trim(args, arg_str)) {
            action = nfc_cli_command_get_action_by_name(cmd, arg_str);
        }
        furi_string_free(arg_str);
    }

    return action;
}

static bool nfc_cli_action_can_reuse_context(
    NfcCliProcessorContext* instance,
    const NfcCliActionDescriptor* new_action) {
    bool result = false;
    do {
        if(instance->action != new_action) break;
        if(new_action->can_reuse == NULL) break;
        result = new_action->can_reuse(instance->action_context);
    } while(false);
    return result;
}

static void nfc_cli_action_free(NfcCliProcessorContext* instance) {
    if(instance->action && instance->action->free) {
        FURI_LOG_D(TAG, "Free previous \"%s\" action context", instance->action->name);
        instance->action->free(instance->action_context);
    }
    instance->action = NULL;
}

static NfcCliProcessorError
    nfc_cli_action_alloc(NfcCliProcessorContext* instance, FuriString* args) {
    const NfcCliCommandDescriptor* cmd = instance->cmd;

    NfcCliProcessorError result = NfcCliProcessorErrorNone;
    do {
        const NfcCliActionDescriptor* action = nfc_cli_get_action_from_args(cmd, args);
        if(action == NULL) {
            result = NfcCliProcessorErrorActionNotFound;
            furi_string_printf(instance->error_message, "Action not found");
            break;
        }

        if(!nfc_cli_action_can_reuse_context(instance, action)) {
            nfc_cli_action_free(instance);

            instance->action = action;
            if(action->alloc && action->free) {
                FURI_LOG_D(TAG, "Allocating context for action \"%s\"", action->name);
                instance->action_context = instance->action->alloc(instance->nfc);
            } else if(action->alloc && (action->free == NULL)) {
                FURI_LOG_W(
                    TAG,
                    "Free callback not defined for action \"%s\". Skip allocation to avoid memory leak.",
                    action->name);
                instance->action_context = NULL;
            } else {
                FURI_LOG_D(TAG, "No alloc context callback for action \"%s\"", action->name);
                instance->action_context = NULL;
            }
        } else
            FURI_LOG_D(TAG, "Reusing context from previous \"%s\" action", action->name);

        memset(instance->keys_found, 0, NFC_CLI_KEYS_FOUND_SIZE_BYTES);
        instance->required_keys_expected = nfc_cli_action_get_required_keys_count(action);
        instance->required_keys_found = 0;
        instance->total_keys_found = 0;
    } while(false);

    return result;
}

static NfcCliArgumentType nfc_cli_get_argument_type(FuriString* argument) {
    size_t arg_len = furi_string_size(argument);
    NfcCliArgumentType type = NfcCliArgumentTypeUnknown;

    if(arg_len > 2) {
        char ch1 = furi_string_get_char(argument, 0);
        char ch2 = furi_string_get_char(argument, 1);
        if(ch1 == '-') {
            type = (ch2 == '-') ? NfcCliArgumentTypeLongNameKey :
                                  NfcCliArgumentTypeShortNameKeyGroup;
        }
    } else if(arg_len == 2) {
        char ch1 = furi_string_get_char(argument, 0);
        type = (ch1 == '-') ? NfcCliArgumentTypeShortNameKey : NfcCliArgumentTypeUnknown;
    }

    return type;
}

static bool
    nfc_cli_check_duplicate_keys(NfcCliProcessorContext* instance, const NfcCliKeyDescriptor* key) {
    bool result = false;
    for(size_t i = 0; i < instance->total_keys_found; i++) {
        const NfcCliKeyDescriptor* buf = instance->keys_found[i];
        if(buf != key) continue;
        result = true;
        break;
    }

    return result;
}

static void nfc_cli_trim_multivalue_arg(FuriString* args, FuriString* value) {
    furi_string_set(value, args);
    size_t index = furi_string_search_char(value, '-', 0);
    if(index != STRING_FAILURE) {
        furi_string_left(value, index);
        furi_string_right(args, index);
    } else {
        furi_string_reset(args);
    }
}

static NfcCliProcessorError nfc_cli_parse_single_key(
    NfcCliProcessorContext* instance,
    FuriString* argument,
    FuriString* args,
    bool from_group) {
    FuriString* value_str = furi_string_alloc();

    NfcCliProcessorError result = NfcCliProcessorErrorNone;
    do {
        const NfcCliKeyDescriptor* key =
            nfc_cli_action_get_key_descriptor(instance->action, argument);
        if(key == NULL) {
            if(furi_string_equal_str(argument, "h"))
                result = NfcCliProcessorErrorNoneButHelp;
            else {
                furi_string_printf(
                    instance->error_message,
                    "Key \'%s\' is not supported",
                    furi_string_get_cstr(argument));
                result = NfcCliProcessorErrorKeyNotSupported;
            }
            break;
        }

        if(key->features.parameter && from_group) {
            furi_string_printf(
                instance->error_message,
                "Parameter key \'%s\' can\'t be grouped",
                furi_string_get_cstr(argument));
            result = NfcCliProcessorErrorKeyParameterInGroup;
            break;
        }

        if(nfc_cli_check_duplicate_keys(instance, key)) {
            furi_string_printf(
                instance->error_message, "Duplicated key \'%s\'", furi_string_get_cstr(argument));
            result = NfcCliProcessorErrorKeyDuplication;
            break;
        }

        if(key->features.multivalue && !key->features.parameter) break;
        if(key->features.multivalue) {
            nfc_cli_trim_multivalue_arg(args, value_str);
            FURI_LOG_D(TAG, "Multivalue: %s", furi_string_get_cstr(value_str));
        } else if(key->features.parameter && !args_read_string_and_trim(args, value_str)) {
            result = NfcCliProcessorErrorKeyParameterValueMissing;
            furi_string_printf(
                instance->error_message,
                "Missing value for \'%s\'",
                furi_string_get_cstr(argument));
            break;
        }

        if(key->parse == NULL) {
            furi_string_printf(
                instance->error_message,
                "Parse callback for key \'%s\' not defined",
                furi_string_get_cstr(argument));
            result = NfcCliProcessorErrorKeyParseError;
            break;
        }

        FURI_LOG_D(TAG, "Parsing key \"%s\"", furi_string_get_cstr(argument));
        if(!key->parse(value_str, instance->action_context)) {
            furi_string_printf(
                instance->error_message,
                "Unable to parse value \'%s\' for key \'%s\'",
                furi_string_get_cstr(value_str),
                furi_string_get_cstr(argument));
            result = NfcCliProcessorErrorKeyParseError;
            break;
        }

        instance->keys_found[instance->total_keys_found] = key;
        instance->total_keys_found++;
        if(key->features.required) instance->required_keys_found++;
    } while(false);
    furi_string_free(value_str);

    return result;
}

static NfcCliProcessorError
    nfc_cli_parse_group_key(NfcCliProcessorContext* instance, FuriString* argument) {
    NfcCliProcessorError result = NfcCliProcessorErrorNone;
    FURI_LOG_D(TAG, "Parsing key group\"%s\"", furi_string_get_cstr(argument));

    FuriString* arg_buf = furi_string_alloc();
    for(size_t i = 0; i < furi_string_size(argument); i++) {
        furi_string_set_n(arg_buf, argument, i, 1);
        result = nfc_cli_parse_single_key(instance, arg_buf, NULL, true);
        if(result != NfcCliProcessorErrorNone) break;
    }
    furi_string_free(arg_buf);

    return result;
}

static NfcCliProcessorError nfc_cli_parse_argument(
    NfcCliProcessorContext* instance,
    FuriString* argument,
    FuriString* args) {
    NfcCliArgumentType type = nfc_cli_get_argument_type(argument);

    furi_string_trim(argument, "-");

    NfcCliProcessorError result = NfcCliProcessorErrorNone;

    if(type == NfcCliArgumentTypeShortNameKeyGroup)
        result = nfc_cli_parse_group_key(instance, argument);
    else if((type == NfcCliArgumentTypeShortNameKey) || (type == NfcCliArgumentTypeLongNameKey)) {
        result = nfc_cli_parse_single_key(instance, argument, args, false);
    } else if(type == NfcCliArgumentTypeUnknown) { //-V547
        result = NfcCliProcessorErrorKeyNotSupported;
        furi_string_printf(
            instance->error_message,
            "Key \'%s\' is not supported",
            furi_string_get_cstr(argument));
    }

    return result;
}

static NfcCliProcessorError
    nfc_cli_process_arguments(NfcCliProcessorContext* instance, FuriString* args) {
    NfcCliProcessorError result = NfcCliProcessorErrorNone;

    FuriString* argument = furi_string_alloc();
    while(args_read_string_and_trim(args, argument)) {
        result = nfc_cli_parse_argument(instance, argument, args);
        if(result != NfcCliProcessorErrorNone) break;
    }
    furi_string_free(argument);

    if((result == NfcCliProcessorErrorNone) &&
       (instance->required_keys_expected != instance->required_keys_found)) {
        furi_string_printf(instance->error_message, "Some required keys missing");
        result = NfcCliProcessorErrorKeyRequiredMissing;
    }

    return result;
}

static inline void nfc_cli_command_process_error(
    const NfcCliProcessorContext* instance,
    NfcCliProcessorError error) {
    do {
        if(error == NfcCliProcessorErrorNone) break;

        if(error != NfcCliProcessorErrorNoneButHelp)
            printf(
                ANSI_FG_RED "Error: %s\r\n" ANSI_RESET,
                furi_string_get_cstr(instance->error_message));

        if(error == NfcCliProcessorErrorActionNotFound)
            nfc_cli_command_format_info(instance->cmd, instance->error_message);
        else
            nfc_cli_action_format_info(instance->action, instance->error_message);

        printf("\n%s", furi_string_get_cstr(instance->error_message));
    } while(false);
}

void nfc_cli_command_processor_run(
    const NfcCliCommandDescriptor* cmd,
    PipeSide* pipe,
    FuriString* args,
    void* context) {
    furi_assert(pipe);
    furi_assert(cmd);
    furi_assert(args);
    NfcCliProcessorContext* instance = context;
    furi_string_reset(instance->error_message);

    NfcCliProcessorError error = NfcCliProcessorErrorNone;
    instance->cmd = cmd;
    do {
        error = nfc_cli_action_alloc(instance, args);
        if(error != NfcCliProcessorErrorNone) break;

        error = nfc_cli_process_arguments(instance, args);
        if(error != NfcCliProcessorErrorNone) break;

        if(instance->action && instance->action->execute) {
            instance->action->execute(pipe, instance->action_context);
        } else {
            FURI_LOG_W(TAG, "Action execute callback missing");
        }
    } while(false);

    nfc_cli_command_process_error(instance, error);
}

NfcCliProcessorContext* nfc_cli_command_processor_alloc(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliProcessorContext* instance = malloc(sizeof(NfcCliProcessorContext));
    instance->nfc = nfc;
    instance->keys_found = malloc(NFC_CLI_KEYS_FOUND_SIZE_BYTES);
    instance->total_keys_found = 0;
    instance->required_keys_found = 0;
    instance->required_keys_expected = 0;

    instance->error_message = furi_string_alloc();
    return instance;
}

void nfc_cli_command_processor_free(NfcCliProcessorContext* instance) {
    furi_assert(instance);
    nfc_cli_action_free(instance);
    free(instance->keys_found);
    furi_string_free(instance->error_message);

    instance->nfc = NULL;
    free(instance);
}
