#pragma once

#include "nfc_cli_command_base.h"
#include <toolbox/cli/cli_ansi.h>
#include <nfc/nfc.h>
#include <nfc/protocols/nfc_protocol.h>
#include "nfc_cli_command_processor.h"

/**
 * @brief How to add command.
 * 
 * There are 3 possible option on how to add new command to nfc_cli: 
 * 
 * @see Option 1 "Add action command directly to nfc_shell"
 * 
 * In this case command will be invoked with argument string from nfc_shell.
 * Command registration must be performed directly by user.
 * 
 * Steps: 
 * 1. Add new function for command to nfc_cli.c
 * 2. In nfc_cli_alloc function register command using cli_registry_add_command after nfc_cli_subscribe_commands 
 *
 * This option is NOT RECOMENDED, because such command will not have any 'help'
 * processing and parsing error checks. Argument parsing must also be done by hand. 
 * 
 * --------------------------------------------------------------------------
 *  
 * @see Option 2 "Add action command to collection without further processing"
 * 
 * In this case command will be invoked with argument string from nfc_shell.
 * nfc_cli_command_processor is skipped, so argument handling is up to the developer.
 * 
 * Steps:
 * 1. Add new pair of nfc_cli_command_<cmd>.c/.h files to /commands folder
 * 2. Define const NfcCliCommandDescriptor instance in .c file and its extern definition in .h file
 * 3. Include .h file to nfc_cli_commands.c file below comment "Include new commands here"
 * 4. Add new command reference to nfc_cli_commands array
 * 5. Add path to nfc_cli_command_<cmd>.c file into 'cli_nfc' plugin in application.fam file 
 *
 * This option suites for simple commands with no any parameters. 
 * @see nfc_cli_command_field.c implementation as an example.
 * 
 * --------------------------------------------------------------------------
 * 
 * @see Option 3 "Add action command to collection with full processing"
 * 
 * In this nfc_cli_command_processor will be invoked for parsing command arguments
 * and action execution. Also it will handle errors and help printing.
 * 
 * Steps:
 * 1. Add new pair of nfc_cli_command_<cmd>.c/.h files to /commands folder
 * 2. Use macro ADD_NFC_CLI_COMMAND to define command in .c file 
 * 3. Define command extern in .h file, using command name from macro in form of "<name>_cmd"
 * 4. Add all desired actions and keys to your command
 * 5. Include .h file to nfc_cli_commands.c file below comment "Include new commands here"
 * 6. Add new command reference to nfc_cli_commands array
 * 7. Add path to nfc_cli_command_<cmd>.c file into 'cli_nfc' plugin in application.fam file 
 *
 * This option suites for "difficult" commands which has actions with lots of keys. 
 * @see nfc_cli_command_emulate.c implementation as an example.
 * 
 */

/**
 * @brief Used to decorate argument with some properties
 */
typedef struct {
    bool required : 1; /**< Command always needs this argument. Missing arguments with this set to true will result execution error.*/
    bool parameter : 1; /**< Such argument requires value after its name, otherwise it is a simple on/off switch */
    bool multivalue : 1; /**< Such argument can take multiple values after its name, like this "-key value1 value2 .. valueN" */
} FURI_PACKED NfcCliKeyFeatureSupport;

/**
 * @brief Describes key for action
 */
struct NfcCliKeyDescriptor {
    NfcCliKeyFeatureSupport features; /**< Features supported defining key behaviour */
    const char* long_name; /**< Long key name starts with '--' symbol in argument string */
    const char* short_name; /**< Short key name starts with '-' symbol in argument string */
    const char* description; /**< Key description showed in help */
    NfcCliArgParseCallback parse; /**< Parsing callback */
};

/**
 * @brief Describes action
 */
struct NfcCliActionDescriptor {
    const char* name; /**< Action name MUST be the first argument after command.*/
    const char* description; /**< Description showed in help */
    size_t key_count; /**< Amount of key entries in keys array */
    const NfcCliKeyDescriptor* keys; /**< Keys available for action */

    NfcCliActionHandlerCallback execute; /**< Action callback, invoked if parsing is ok */
    NfcCliActionContextAlloc alloc; /**< Allocates action context during command processing */
    NfcCliActionContextFree free; /**< Frees action context */
    NfcCliActionContextCanReuse can_reuse; /**< Checks context reuse possibility */
};

/**
 * @brief Describes command
 */
struct NfcCliCommandDescriptor {
    const char* name; /** Used to register command in cli shell */
    const char* description; /**< Description showed in help */
    size_t action_count; /** Amount of actions available in scope of this particular command */
    const NfcCliActionDescriptor** actions; /**< Actions available for command */
    CliCommandExecuteCallback callback; /** Entry point for command */
};

/**
 * @brief This macro simplifies command creation. It fills instance of 
 * NfcCliCommandDescriptor and generates a callback which invokes 
 * nfc_cli_command_processor inside
 */
#define ADD_NFC_CLI_COMMAND(name, description, actions)                  \
    static void nfc_cli_command_##name##_callback(                       \
        PipeSide* pipe, FuriString* args, void* context);                \
                                                                         \
    const NfcCliCommandDescriptor name##_cmd = {                         \
        #name,                                                           \
        #description,                                                    \
        COUNT_OF(actions),                                               \
        actions,                                                         \
        nfc_cli_command_##name##_callback,                               \
    };                                                                   \
                                                                         \
    static void nfc_cli_command_##name##_callback(                       \
        PipeSide* pipe, FuriString* args, void* context) {               \
        nfc_cli_command_processor_run(&name##_cmd, pipe, args, context); \
    }
