#include "flipper.pb.h"
#include <core/record.h>
#include "rpc_i.h"
#include <furi.h>
#include <loader/loader.h>
#include "rpc_app.h"

#define TAG "RpcSystemApp"
#define APP_BUTTON_TIMEOUT 1000

struct RpcAppSystem {
    RpcSession* session;
    RpcAppSystemCallback app_callback;
    void* app_context;
    PB_Main* state_msg;
    FuriTimer* timer;
};

static void rpc_system_app_timer_callback(void* context) {
    furi_assert(context);
    RpcAppSystem* rpc_app = context;

    if(rpc_app->app_callback) {
        rpc_app->app_callback(RpcAppEventButtonRelease, NULL, rpc_app->app_context);
    }
}

static void rpc_system_app_start_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_start_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);
    char args_temp[16];

    FURI_LOG_D(TAG, "Start");

    PB_CommandStatus result = PB_CommandStatus_ERROR_APP_CANT_START;

    Loader* loader = furi_record_open(RECORD_LOADER);
    const char* app_name = request->content.app_start_request.name;
    if(app_name) {
        const char* app_args = request->content.app_start_request.args;
        if(strcmp(app_args, "RPC") == 0) {
            // If app is being started in RPC mode - pass RPC context via args string
            snprintf(args_temp, 16, "RPC %08lX", (uint32_t)rpc_app);
            app_args = args_temp;
        }
        LoaderStatus status = loader_start(loader, app_name, app_args);
        if(status == LoaderStatusErrorAppStarted) {
            result = PB_CommandStatus_ERROR_APP_SYSTEM_LOCKED;
        } else if(status == LoaderStatusErrorInternal) {
            result = PB_CommandStatus_ERROR_APP_CANT_START;
        } else if(status == LoaderStatusErrorUnknownApp) {
            result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
        } else if(status == LoaderStatusOk) {
            result = PB_CommandStatus_OK;
        } else {
            furi_assert(0);
        }
    } else {
        result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
    }

    furi_record_close(RECORD_LOADER);

    rpc_send_and_release_empty(session, request->command_id, result);
}

static void rpc_system_app_lock_status_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_lock_status_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    FURI_LOG_D(TAG, "LockStatus");

    Loader* loader = furi_record_open(RECORD_LOADER);

    PB_Main response = {
        .has_next = false,
        .command_status = PB_CommandStatus_OK,
        .command_id = request->command_id,
        .which_content = PB_Main_app_lock_status_response_tag,
    };

    response.content.app_lock_status_response.locked = loader_is_locked(loader);

    furi_record_close(RECORD_LOADER);

    rpc_send_and_release(session, &response);
    pb_release(&PB_Main_msg, &response);
}

static void rpc_system_app_exit_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_exit_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;

    if(rpc_app->app_callback) {
        if(rpc_app->app_callback(RpcAppEventAppExit, NULL, rpc_app->app_context)) {
            status = PB_CommandStatus_OK;
            furi_timer_stop(rpc_app->timer);
        } else {
            status = PB_CommandStatus_ERROR_APP_CMD_ERROR;
        }
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
    }

    rpc_send_and_release_empty(session, request->command_id, status);
}

static void rpc_system_app_load_file(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_load_file_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        const char* file_path = request->content.app_load_file_request.path;
        if(rpc_app->app_callback(RpcAppEventLoadFile, file_path, rpc_app->app_context)) {
            status = PB_CommandStatus_OK;
        } else {
            status = PB_CommandStatus_ERROR_APP_CMD_ERROR;
        }
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
    }

    rpc_send_and_release_empty(session, request->command_id, status);
}

static void rpc_system_app_button_press(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_button_press_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        const char* args = request->content.app_button_press_request.args;
        if(rpc_app->app_callback(RpcAppEventButtonPress, args, rpc_app->app_context)) {
            status = PB_CommandStatus_OK;
            furi_timer_start(rpc_app->timer, APP_BUTTON_TIMEOUT);
        } else {
            status = PB_CommandStatus_ERROR_APP_CMD_ERROR;
        }
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
    }

    rpc_send_and_release_empty(session, request->command_id, status);
}

static void rpc_system_app_button_release(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_button_release_request_tag);
    furi_assert(context);

    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        if(rpc_app->app_callback(RpcAppEventButtonRelease, NULL, rpc_app->app_context)) {
            status = PB_CommandStatus_OK;
            furi_timer_stop(rpc_app->timer);
        } else {
            status = PB_CommandStatus_ERROR_APP_CMD_ERROR;
        }
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
    }

    rpc_send_and_release_empty(session, request->command_id, status);
}

void rpc_system_app_send_started(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    rpc_app->state_msg->content.app_state_response.state = PB_App_AppState_APP_STARTED;
    rpc_send(session, rpc_app->state_msg);
}

void rpc_system_app_send_exited(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    rpc_app->state_msg->content.app_state_response.state = PB_App_AppState_APP_CLOSED;
    rpc_send(session, rpc_app->state_msg);
}

void rpc_system_app_set_callback(RpcAppSystem* rpc_app, RpcAppSystemCallback callback, void* ctx) {
    furi_assert(rpc_app);

    rpc_app->app_callback = callback;
    rpc_app->app_context = ctx;
}

void* rpc_system_app_alloc(RpcSession* session) {
    furi_assert(session);

    RpcAppSystem* rpc_app = malloc(sizeof(RpcAppSystem));
    rpc_app->session = session;

    rpc_app->timer = furi_timer_alloc(rpc_system_app_timer_callback, FuriTimerTypeOnce, rpc_app);

    // App exit message
    rpc_app->state_msg = malloc(sizeof(PB_Main));
    rpc_app->state_msg->which_content = PB_Main_app_state_response_tag;
    rpc_app->state_msg->command_status = PB_CommandStatus_OK;

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_app,
    };

    rpc_handler.message_handler = rpc_system_app_start_process;
    rpc_add_handler(session, PB_Main_app_start_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_lock_status_process;
    rpc_add_handler(session, PB_Main_app_lock_status_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_exit_request;
    rpc_add_handler(session, PB_Main_app_exit_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_load_file;
    rpc_add_handler(session, PB_Main_app_load_file_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_button_press;
    rpc_add_handler(session, PB_Main_app_button_press_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_button_release;
    rpc_add_handler(session, PB_Main_app_button_release_request_tag, &rpc_handler);

    return rpc_app;
}

void rpc_system_app_free(void* context) {
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    furi_timer_free(rpc_app->timer);

    if(rpc_app->app_callback) {
        rpc_app->app_callback(RpcAppEventSessionClose, NULL, rpc_app->app_context);
    }

    free(rpc_app->state_msg);
    free(rpc_app);
}
