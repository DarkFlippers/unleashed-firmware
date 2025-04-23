#include "../cli_main_commands.h"
#include <toolbox/cli/cli_registry.h>
#include <toolbox/cli/shell/cli_shell.h>
#include <toolbox/cli/cli_ansi.h>

#define RAINBOW_SUBCOMMAND                                                                \
    ANSI_FG_RED "s" ANSI_FG_YELLOW "u" ANSI_FG_BLUE "b" ANSI_FG_GREEN "c" ANSI_FG_MAGENTA \
                "o" ANSI_FG_RED "m" ANSI_FG_YELLOW "m" ANSI_FG_BLUE "a" ANSI_FG_GREEN     \
                "n" ANSI_FG_MAGENTA "d"

static void subcommand(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    printf("This is a ✨" RAINBOW_SUBCOMMAND ANSI_RESET "✨!");
}

static void motd(void* context) {
    UNUSED(context);
    printf("\r\n");
    printf("+------------------------------------+\r\n");
    printf("|            Hello world!            |\r\n");
    printf("| This is the " ANSI_FG_GREEN "MOTD" ANSI_RESET " for our " ANSI_FG_BLUE
           "subshell" ANSI_RESET "! |\r\n");
    printf("+------------------------------------+\r\n");
}

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);
    CliRegistry* registry = cli_registry_alloc();
    cli_registry_add_command(registry, "subcommand", CliCommandFlagParallelSafe, subcommand, NULL);

    CliShell* shell = cli_shell_alloc(motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "subshell");
    cli_shell_start(shell);
    cli_shell_join(shell);

    cli_shell_free(shell);
    cli_registry_free(registry);
}

CLI_COMMAND_INTERFACE(subshell_demo, execute, CliCommandFlagParallelSafe, 2048, CLI_APPID);
