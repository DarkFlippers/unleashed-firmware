/**
 * @file cli_command.h
 * Command metadata and helpers
 */

#pragma once

#include <furi.h>
#include <toolbox/pipe.h>
#include <lib/flipper_application/flipper_application.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_PLUGIN_API_VERSION 1

typedef enum {
    CliCommandFlagDefault = 0, /**< Default */
    CliCommandFlagParallelSafe = (1 << 0), /**< Safe to run in parallel with other apps */
    CliCommandFlagInsomniaSafe = (1 << 1), /**< Safe to run with insomnia mode on */
    CliCommandFlagDontAttachStdio = (1 << 2), /**< Do no attach I/O pipe to thread stdio */
    CliCommandFlagUseShellThread =
        (1
         << 3), /**< Don't start a separate thread to run the command in. Incompatible with DontAttachStdio */

    // internal flags (do not set them yourselves!)

    CliCommandFlagExternal = (1 << 4), /**< The command comes from a .fal file */
} CliCommandFlag;

/** 
 * @brief CLI command execution callback pointer
 * 
 * This callback will be called from a separate thread spawned just for your
 * command. The pipe will be installed as the thread's stdio, so you can use
 * `printf`, `getchar` and other standard functions to communicate with the
 * user.
 * 
 * @param [in] pipe     Pipe that can be used to send and receive data. If
 *                      `CliCommandFlagDontAttachStdio` was not set, you can
 *                      also use standard C functions (printf, getc, etc.) to
 *                      access this pipe.
 * @param [in] args     String with what was passed after the command
 * @param [in] context  Whatever you provided to `cli_add_command`
 */
typedef void (*CliCommandExecuteCallback)(PipeSide* pipe, FuriString* args, void* context);

typedef struct {
    char* name;
    CliCommandExecuteCallback execute_callback;
    CliCommandFlag flags;
    size_t stack_depth;
} CliCommandDescriptor;

/**
 * @brief Configuration for locating external commands
 */
typedef struct {
    const char* search_directory; //<! The directory to look in
    const char* fal_prefix; //<! File name prefix that commands should have
    const char* appid; //<! Expected plugin-reported appid
} CliCommandExternalConfig;

/**
 * @brief Detects if Ctrl+C has been pressed or session has been terminated
 * 
 * @param [in] side Pointer to pipe side given to the command thread
 * @warning This function also assumes that the pipe is installed as the
 *          thread's stdio
 * @warning This function will consume 0 or 1 bytes from the pipe
 */
bool cli_is_pipe_broken_or_is_etx_next_char(PipeSide* side);

/** Print unified cmd usage tip
 *
 * @param      cmd    cmd name
 * @param      usage  usage tip
 * @param      arg    arg passed by user
 */
void cli_print_usage(const char* cmd, const char* usage, const char* arg);

#define CLI_COMMAND_INTERFACE(name, execute_callback, flags, stack_depth, app_id) \
    static const CliCommandDescriptor cli_##name##_desc = {                       \
        #name,                                                                    \
        &execute_callback,                                                        \
        flags,                                                                    \
        stack_depth,                                                              \
    };                                                                            \
                                                                                  \
    static const FlipperAppPluginDescriptor plugin_descriptor = {                 \
        .appid = app_id,                                                          \
        .ep_api_version = CLI_PLUGIN_API_VERSION,                                 \
        .entry_point = &cli_##name##_desc,                                        \
    };                                                                            \
                                                                                  \
    const FlipperAppPluginDescriptor* cli_##name##_ep(void) {                     \
        return &plugin_descriptor;                                                \
    }

#ifdef __cplusplus
}
#endif
