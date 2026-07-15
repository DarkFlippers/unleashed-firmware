#pragma once

#include <furi.h>
#include <toolbox/cli/cli_command.h>
#include <nfc/nfc.h>

/**
 * @brief Type for action context to be created before action execution
 * must be hanlded through callbacks in each action separately
 */
typedef void NfcCliActionContext;

/**
 * @brief Callback type for function of action context allocation
 * @param nfc Instance of NFC subsystem, will be used during action execution 
 * @return Pointer to action context
 */
typedef NfcCliActionContext* (*NfcCliActionContextAlloc)(Nfc* nfc);

/**
 * @brief Callback for action context deleting
 * @param action_ctx Action context to be freed
 */
typedef void (*NfcCliActionContextFree)(NfcCliActionContext* action_ctx);

/**
 * @brief Callback invoked by command processor to determine whether already
 * existing context (from previously executed command) can be reused for the new one.
 * 
 * @param action_ctx Action context
 * 
 * In most cases re-creating of a new context is not needed.  
 * It is used in 'raw' command, where nfc field sometimes need to stay turned on between
 * commands. 
 * 
 * Handling of this situation and decision about reusing action context is on developer, 
 * who need to decide, can this command reuse context. 
 * 
 * It can be done by comparing parameters of previously executed command and a new one, 
 * or by some args in command.
 * 
 * See implementation of 'keep_field' flag in 'raw' command for example.
 */
typedef bool (*NfcCliActionContextCanReuse)(NfcCliActionContext* ctx);

/**
 * @brief Action execution callback
 * @param pipe provided by cli shell, can be used for command termination
 * @param ctx Action context
 */
typedef void (*NfcCliActionHandlerCallback)(PipeSide* pipe, NfcCliActionContext* ctx);

/**
 * @brief Callback used for parsing argument key.
 * Each key added to command must have this, otherwise parsing result will always be
 * false and command will never be executed.
 * 
 * @param value Text value for the key to be parsed
 * @param ctx Action context
 * @return true when parsing was fine, otherwise false. If any argument in command input
 * generates false during parsing, command will not be executed and error will be shown 
 */
typedef bool (*NfcCliArgParseCallback)(FuriString* value, NfcCliActionContext* ctx);

typedef struct NfcCliKeyDescriptor NfcCliKeyDescriptor;

typedef struct NfcCliActionDescriptor NfcCliActionDescriptor;

typedef struct NfcCliCommandDescriptor NfcCliCommandDescriptor;
