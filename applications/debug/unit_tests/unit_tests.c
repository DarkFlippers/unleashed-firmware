#include <furi.h>
#include <toolbox/pipe.h>
#include <toolbox/cli/cli_command.h>
#include <toolbox/cli/cli_registry.h>
#include <cli/cli_main_commands.h>

#include "test_runner.h"

void unit_tests_cli(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    TestRunner* test_runner = test_runner_alloc(pipe, args);
    test_runner_run(test_runner);
    test_runner_free(test_runner);
}

void unit_tests_on_system_start(void) {
#ifdef SRV_CLI
    CliRegistry* registry = furi_record_open(RECORD_CLI);
    cli_registry_add_command(
        registry, "unit_tests", CliCommandFlagParallelSafe, unit_tests_cli, NULL);
    furi_record_close(RECORD_CLI);
#endif
}
