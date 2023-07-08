#include "flipper.pb.h"
#include <core/record.h>
#include "rpc_i.h"
#include <furi.h>
#include <loader/loader.h>
#include "rpc_app.h"

#define TAG "RpcSystemApp"

struct RpcAppSystem {
    RpcSession* session;

    RpcAppSystemCallback app_callback;
    void* app_context;

    RpcAppSystemDataExchangeCallback data_exchange_callback;
    void* data_exchange_context;

    PB_Main* state_msg;
    PB_Main* error_msg;

    uint32_t last_id;
    char* last_data;
};

#define RPC_SYSTEM_APP_TEMP_ARGS_SIZE 16

static void rpc_system_app_start_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_start_request_tag);
    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    rpc_system_app_error_reset(rpc_app);
    furi_assert(session);
    char args_temp[RPC_SYSTEM_APP_TEMP_ARGS_SIZE];

    furi_assert(!rpc_app->last_id);
    furi_assert(!rpc_app->last_data);

    FURI_LOG_D(TAG, "StartProcess: id %lu", request->command_id);

    PB_CommandStatus result;

    Loader* loader = furi_record_open(RECORD_LOADER);
    const char* app_name = request->content.app_start_request.name;
    if(app_name) {
        const char* app_args = request->content.app_start_request.args;
        if(app_args && strcmp(app_args, "RPC") == 0) {
            // If app is being started in RPC mode - pass RPC context via args string
            snprintf(args_temp, RPC_SYSTEM_APP_TEMP_ARGS_SIZE, "RPC %08lX", (uint32_t)rpc_app);
            app_args = args_temp;
        }
        LoaderStatus status = loader_start(loader, app_name, app_args, NULL);
        if(status == LoaderStatusErrorAppStarted) {
            result = PB_CommandStatus_ERROR_APP_SYSTEM_LOCKED;
        } else if(status == LoaderStatusErrorInternal) {
            result = PB_CommandStatus_ERROR_APP_CANT_START;
        } else if(status == LoaderStatusErrorUnknownApp) {
            result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
        } else if(status == LoaderStatusOk) {
            result = PB_CommandStatus_OK;
        } else {
            furi_crash(NULL);
        }
    } else {
        result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
    }

    furi_record_close(RECORD_LOADER);

    FURI_LOG_D(TAG, "StartProcess: response id %lu, result %d", request->command_id, result);
    rpc_send_and_release_empty(session, request->command_id, result);
}

static void rpc_system_app_lock_status_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_lock_status_request_tag);
    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
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

    FURI_LOG_D(TAG, "LockStatus: response");
    rpc_send_and_release(session, &response);
    pb_release(&PB_Main_msg, &response);
}

static void rpc_system_app_exit_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_exit_request_tag);
    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;

    if(rpc_app->app_callback) {
        FURI_LOG_D(TAG, "ExitRequest: id %lu", request->command_id);
        furi_assert(!rpc_app->last_id);
        furi_assert(!rpc_app->last_data);
        rpc_app->last_id = request->command_id;
        rpc_app->app_callback(RpcAppEventAppExit, rpc_app->app_context);
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
        FURI_LOG_E(
            TAG, "ExitRequest: APP_NOT_RUNNING, id %lu, status: %d", request->command_id, status);
        rpc_send_and_release_empty(session, request->command_id, status);
    }
}

static void rpc_system_app_load_file(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_load_file_request_tag);
    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        FURI_LOG_D(TAG, "LoadFile: id %lu", request->command_id);
        furi_assert(!rpc_app->last_id);
        furi_assert(!rpc_app->last_data);
        rpc_app->last_id = request->command_id;
        rpc_app->last_data = strdup(request->content.app_load_file_request.path);
        rpc_app->app_callback(RpcAppEventLoadFile, rpc_app->app_context);
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
        FURI_LOG_E(
            TAG, "LoadFile: APP_NOT_RUNNING, id %lu, status: %d", request->command_id, status);
        rpc_send_and_release_empty(session, request->command_id, status);
    }
}

static void rpc_system_app_button_press(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    furi_assert(request->which_content == PB_Main_app_button_press_request_tag);
    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        FURI_LOG_D(TAG, "ButtonPress");
        furi_assert(!rpc_app->last_id);
        furi_assert(!rpc_app->last_data);
        rpc_app->last_id = request->command_id;
        rpc_app->last_data = strdup(request->content.app_button_press_request.args);
        rpc_app->app_callback(RpcAppEventButtonPress, rpc_app->app_context);
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
        FURI_LOG_E(
            TAG, "ButtonPress: APP_NOT_RUNNING, id %lu, status: %d", request->command_id, status);
        rpc_send_and_release_empty(session, request->command_id, status);
    }
}

static void rpc_system_app_button_release(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_button_release_request_tag);
    furi_assert(context);

    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus status;
    if(rpc_app->app_callback) {
        FURI_LOG_D(TAG, "ButtonRelease");
        furi_assert(!rpc_app->last_id);
        furi_assert(!rpc_app->last_data);
        rpc_app->last_id = request->command_id;
        rpc_app->app_callback(RpcAppEventButtonRelease, rpc_app->app_context);
    } else {
        status = PB_CommandStatus_ERROR_APP_NOT_RUNNING;
        FURI_LOG_E(
            TAG, "ButtonRelease: APP_NOT_RUNNING, id %lu, status: %d", request->command_id, status);
        rpc_send_and_release_empty(session, request->command_id, status);
    }
}

static void rpc_system_app_get_error_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_get_error_request_tag);
    furi_assert(context);

    RpcAppSystem* rpc_app = context;
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    rpc_app->error_msg->command_id = request->command_id;

    FURI_LOG_D(TAG, "GetError");
    rpc_send(session, rpc_app->error_msg);
}

static void rpc_system_app_data_exchange_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_data_exchange_request_tag);
    furi_assert(context);

    RpcAppSystem* rpc_app = context;
    rpc_system_app_error_reset(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_CommandStatus command_status;
    pb_bytes_array_t* data = request->content.app_data_exchange_request.data;

    if(rpc_app->data_exchange_callback) {
        uint8_t* data_bytes = NULL;
        size_t data_size = 0;
        if(data) {
            data_bytes = data->bytes;
            data_size = data->size;
        }
        rpc_app->data_exchange_callback(data_bytes, data_size, rpc_app->data_exchange_context);
        command_status = PB_CommandStatus_OK;
    } else {
        command_status = PB_CommandStatus_ERROR_APP_CMD_ERROR;
    }

    FURI_LOG_D(TAG, "DataExchange");
    rpc_send_and_release_empty(session, request->command_id, command_status);
}

void rpc_system_app_send_started(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    rpc_app->state_msg->content.app_state_response.state = PB_App_AppState_APP_STARTED;

    FURI_LOG_D(TAG, "SendStarted");
    rpc_send(session, rpc_app->state_msg);
}

void rpc_system_app_send_exited(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    rpc_app->state_msg->content.app_state_response.state = PB_App_AppState_APP_CLOSED;

    FURI_LOG_D(TAG, "SendExit");
    rpc_send(session, rpc_app->state_msg);
}

const char* rpc_system_app_get_data(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);
    furi_assert(rpc_app->last_data);
    return rpc_app->last_data;
}

void rpc_system_app_confirm(RpcAppSystem* rpc_app, RpcAppSystemEvent event, bool result) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);
    furi_assert(rpc_app->last_id);

    PB_CommandStatus status = result ? PB_CommandStatus_OK : PB_CommandStatus_ERROR_APP_CMD_ERROR;

    uint32_t last_id = 0;
    switch(event) {
    case RpcAppEventAppExit:
    case RpcAppEventLoadFile:
    case RpcAppEventButtonPress:
    case RpcAppEventButtonRelease:
        last_id = rpc_app->last_id;
        rpc_app->last_id = 0;
        if(rpc_app->last_data) {
            free(rpc_app->last_data);
            rpc_app->last_data = NULL;
        }
        FURI_LOG_D(TAG, "AppConfirm: event %d last_id %lu status %d", event, last_id, status);
        rpc_send_and_release_empty(session, last_id, status);
        break;
    default:
        furi_crash("RPC App state programming Error");
        break;
    }
}

void rpc_system_app_set_callback(RpcAppSystem* rpc_app, RpcAppSystemCallback callback, void* ctx) {
    furi_assert(rpc_app);

    rpc_app->app_callback = callback;
    rpc_app->app_context = ctx;
}

void rpc_system_app_set_error_code(RpcAppSystem* rpc_app, uint32_t error_code) {
    furi_assert(rpc_app);
    PB_App_GetErrorResponse* content = &rpc_app->error_msg->content.app_get_error_response;
    content->code = error_code;
}

void rpc_system_app_set_error_text(RpcAppSystem* rpc_app, const char* error_text) {
    furi_assert(rpc_app);
    PB_App_GetErrorResponse* content = &rpc_app->error_msg->content.app_get_error_response;

    if(content->text) {
        free(content->text);
    }

    content->text = error_text ? strdup(error_text) : NULL;
}

void rpc_system_app_error_reset(RpcAppSystem* rpc_app) {
    furi_assert(rpc_app);

    rpc_system_app_set_error_code(rpc_app, 0);
    rpc_system_app_set_error_text(rpc_app, NULL);
}

void rpc_system_app_set_data_exchange_callback(
    RpcAppSystem* rpc_app,
    RpcAppSystemDataExchangeCallback callback,
    void* ctx) {
    furi_assert(rpc_app);

    rpc_app->data_exchange_callback = callback;
    rpc_app->data_exchange_context = ctx;
}

void rpc_system_app_exchange_data(RpcAppSystem* rpc_app, const uint8_t* data, size_t data_size) {
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    PB_Main message = {
        .command_id = 0,
        .command_status = PB_CommandStatus_OK,
        .has_next = false,
        .which_content = PB_Main_app_data_exchange_request_tag,
    };

    PB_App_DataExchangeRequest* content = &message.content.app_data_exchange_request;

    if(data && data_size) {
        content->data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(data_size));
        content->data->size = data_size;
        memcpy(content->data->bytes, data, data_size);
    } else {
        content->data = NULL;
    }

    rpc_send_and_release(session, &message);
}

void* rpc_system_app_alloc(RpcSession* session) {
    furi_assert(session);

    RpcAppSystem* rpc_app = malloc(sizeof(RpcAppSystem));
    rpc_app->session = session;

    // App exit message
    rpc_app->state_msg = malloc(sizeof(PB_Main));
    rpc_app->state_msg->which_content = PB_Main_app_state_response_tag;
    rpc_app->state_msg->command_status = PB_CommandStatus_OK;

    // App error message
    rpc_app->error_msg = malloc(sizeof(PB_Main));
    rpc_app->error_msg->which_content = PB_Main_app_get_error_response_tag;
    rpc_app->error_msg->command_status = PB_CommandStatus_OK;
    rpc_app->error_msg->content.app_get_error_response.code = 0;
    rpc_app->error_msg->content.app_get_error_response.text = NULL;

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

    rpc_handler.message_handler = rpc_system_app_get_error_process;
    rpc_add_handler(session, PB_Main_app_get_error_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_data_exchange_process;
    rpc_add_handler(session, PB_Main_app_data_exchange_request_tag, &rpc_handler);

    return rpc_app;
}

void rpc_system_app_free(void* context) {
    RpcAppSystem* rpc_app = context;
    furi_assert(rpc_app);
    RpcSession* session = rpc_app->session;
    furi_assert(session);

    if(rpc_app->app_callback) {
        rpc_app->app_callback(RpcAppEventSessionClose, rpc_app->app_context);
    }

    while(rpc_app->app_callback) {
        furi_delay_tick(1);
    }

    furi_assert(!rpc_app->data_exchange_callback);

    if(rpc_app->last_data) free(rpc_app->last_data);

    pb_release(&PB_Main_msg, rpc_app->error_msg);

    free(rpc_app->error_msg);
    free(rpc_app->state_msg);
    free(rpc_app);
}
