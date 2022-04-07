#include <cli/cli.h>
#include <furi.h>
#include <rpc/rpc.h>
#include <furi_hal.h>
#include <semphr.h>

#define TAG "RpcCli"

typedef struct {
    Cli* cli;
    bool session_close_request;
    osSemaphoreId_t terminate_semaphore;
} CliRpc;

#define CLI_READ_BUFFER_SIZE 64

static void rpc_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    furi_assert(context);
    furi_assert(bytes);
    furi_assert(bytes_len);
    CliRpc* cli_rpc = context;

    cli_write(cli_rpc->cli, bytes, bytes_len);
}

static void rpc_session_close_callback(void* context) {
    furi_assert(context);
    CliRpc* cli_rpc = context;

    cli_rpc->session_close_request = true;
}

static void rpc_session_terminated_callback(void* context) {
    furi_check(context);
    CliRpc* cli_rpc = context;

    osSemaphoreRelease(cli_rpc->terminate_semaphore);
}

void rpc_cli_command_start_session(Cli* cli, string_t args, void* context) {
    Rpc* rpc = context;

    uint32_t mem_before = memmgr_get_free_heap();
    FURI_LOG_D(TAG, "Free memory %d", mem_before);

    furi_hal_usb_lock();
    RpcSession* rpc_session = rpc_session_open(rpc);
    if(rpc_session == NULL) {
        printf("Session start error\r\n");
        furi_hal_usb_unlock();
        return;
    }

    CliRpc cli_rpc = {.cli = cli, .session_close_request = false};
    cli_rpc.terminate_semaphore = osSemaphoreNew(1, 0, NULL);
    rpc_session_set_context(rpc_session, &cli_rpc);
    rpc_session_set_send_bytes_callback(rpc_session, rpc_send_bytes_callback);
    rpc_session_set_close_callback(rpc_session, rpc_session_close_callback);
    rpc_session_set_terminated_callback(rpc_session, rpc_session_terminated_callback);

    uint8_t* buffer = malloc(CLI_READ_BUFFER_SIZE);
    size_t size_received = 0;

    while(1) {
        size_received = furi_hal_vcp_rx_with_timeout(buffer, CLI_READ_BUFFER_SIZE, 50);
        if(!furi_hal_vcp_is_connected() || cli_rpc.session_close_request) {
            break;
        }

        if(size_received) {
            size_t fed_bytes = rpc_session_feed(rpc_session, buffer, size_received, 3000);
            (void)fed_bytes;
            furi_assert(fed_bytes == size_received);
        }
    }

    rpc_session_close(rpc_session);

    furi_check(osSemaphoreAcquire(cli_rpc.terminate_semaphore, osWaitForever) == osOK);

    osSemaphoreDelete(cli_rpc.terminate_semaphore);

    free(buffer);
    furi_hal_usb_unlock();
}
