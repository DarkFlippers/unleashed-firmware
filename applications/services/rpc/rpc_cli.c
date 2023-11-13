#include <cli/cli.h>
#include <furi.h>
#include <rpc/rpc.h>
#include <furi_hal.h>

#define TAG "RpcCli"

typedef struct {
    Cli* cli;
    bool session_close_request;
    FuriSemaphore* terminate_semaphore;
} CliRpc;

#define CLI_READ_BUFFER_SIZE 64

static void rpc_cli_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    furi_assert(context);
    furi_assert(bytes);
    furi_assert(bytes_len > 0);
    CliRpc* cli_rpc = context;

    cli_write(cli_rpc->cli, bytes, bytes_len);
}

static void rpc_cli_session_close_callback(void* context) {
    furi_assert(context);
    CliRpc* cli_rpc = context;

    cli_rpc->session_close_request = true;
}

static void rpc_cli_session_terminated_callback(void* context) {
    furi_check(context);
    CliRpc* cli_rpc = context;

    furi_semaphore_release(cli_rpc->terminate_semaphore);
}

void rpc_cli_command_start_session(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    furi_assert(cli);
    furi_assert(context);
    Rpc* rpc = context;

    uint32_t mem_before = memmgr_get_free_heap();
    FURI_LOG_D(TAG, "Free memory %lu", mem_before);

    furi_hal_usb_lock();
    RpcSession* rpc_session = rpc_session_open(rpc, RpcOwnerUsb);
    if(rpc_session == NULL) {
        printf("Session start error\r\n");
        furi_hal_usb_unlock();
        return;
    }

    CliRpc cli_rpc = {.cli = cli, .session_close_request = false};
    cli_rpc.terminate_semaphore = furi_semaphore_alloc(1, 0);
    rpc_session_set_context(rpc_session, &cli_rpc);
    rpc_session_set_send_bytes_callback(rpc_session, rpc_cli_send_bytes_callback);
    rpc_session_set_close_callback(rpc_session, rpc_cli_session_close_callback);
    rpc_session_set_terminated_callback(rpc_session, rpc_cli_session_terminated_callback);

    uint8_t* buffer = malloc(CLI_READ_BUFFER_SIZE);
    size_t size_received = 0;

    while(1) {
        size_received = cli_read_timeout(cli_rpc.cli, buffer, CLI_READ_BUFFER_SIZE, 50);
        if(!cli_is_connected(cli_rpc.cli) || cli_rpc.session_close_request) {
            break;
        }

        if(size_received) {
            size_t fed_bytes = rpc_session_feed(rpc_session, buffer, size_received, 3000);
            (void)fed_bytes;
            furi_assert(fed_bytes == size_received);
        }
    }

    rpc_session_close(rpc_session);

    furi_check(
        furi_semaphore_acquire(cli_rpc.terminate_semaphore, FuriWaitForever) == FuriStatusOk);

    furi_semaphore_free(cli_rpc.terminate_semaphore);

    free(buffer);
    furi_hal_usb_unlock();
}
