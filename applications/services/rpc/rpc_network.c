#include "flipper.pb.h"
#include "rpc_i.h"
#include <network/network_i.h>

#include <string.h>

#define TAG "RpcNetwork"

typedef struct {
    RpcSession* session;
    Network* network;
    PB_Main* request;
} RpcNetwork;

static char* rpc_network_strdup_or_null(const char* value) {
    if(!value || value[0] == '\0') return NULL;
    return strdup(value);
}

static PB_Network_HttpMethod rpc_network_method(NetworkHttpMethod method) {
    switch(method) {
    case NetworkHttpMethodPost:
        return PB_Network_HttpMethod_HTTP_POST;
    case NetworkHttpMethodPut:
        return PB_Network_HttpMethod_HTTP_PUT;
    case NetworkHttpMethodPatch:
        return PB_Network_HttpMethod_HTTP_PATCH;
    case NetworkHttpMethodDelete:
        return PB_Network_HttpMethod_HTTP_DELETE;
    case NetworkHttpMethodHead:
        return PB_Network_HttpMethod_HTTP_HEAD;
    case NetworkHttpMethodGet:
    default:
        return PB_Network_HttpMethod_HTTP_GET;
    }
}

static void rpc_network_send_request(
    NetworkRpcCommand command,
    const NetworkRpcRequest* req,
    void* context) {
    RpcNetwork* rpc_network = context;

    PB_Main* request = rpc_network->request;
    memset(request, 0, sizeof(PB_Main));
    request->command_status = PB_CommandStatus_OK;

    switch(command) {
    case NetworkRpcCommandConnect: {
        request->which_content = PB_Main_network_connect_request_tag;
        PB_Network_ConnectRequest* connect = &request->content.network_connect_request;
        connect->connection_id = req->connection_id;
        connect->host = strdup(req->host);
        connect->port = req->port;
        connect->protocol = (req->protocol == NetworkProtocolUdp) ? PB_Network_Protocol_UDP :
                                                                    PB_Network_Protocol_TCP;
        connect->timeout_ms = req->timeout_ms;
        break;
    }
    case NetworkRpcCommandSend: {
        request->which_content = PB_Main_network_send_request_tag;
        PB_Network_SendRequest* send = &request->content.network_send_request;
        send->connection_id = req->connection_id;
        send->binary = req->binary;
        send->data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(req->size));
        send->data->size = req->size;
        memcpy(send->data->bytes, req->data, req->size);
        break;
    }
    case NetworkRpcCommandClose:
        request->which_content = PB_Main_network_close_request_tag;
        request->content.network_close_request.connection_id = req->connection_id;
        break;
    case NetworkRpcCommandHttpRequest: {
        request->which_content = PB_Main_network_http_request_tag;
        PB_Network_HttpRequest* http = &request->content.network_http_request;
        http->request_id = req->connection_id;
        http->method = rpc_network_method(req->method);
        http->url = strdup(req->url);
        http->headers = rpc_network_strdup_or_null(req->headers);
        http->send_path = rpc_network_strdup_or_null(req->send_path);
        http->save_path = rpc_network_strdup_or_null(req->save_path);
        http->timeout_ms = req->timeout_ms;
        http->include_headers = req->include_headers;
        if(req->data && req->size) {
            http->body = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(req->size));
            http->body->size = req->size;
            memcpy(http->body->bytes, req->data, req->size);
        }
        break;
    }
    case NetworkRpcCommandWebSocketOpen: {
        request->which_content = PB_Main_network_websocket_open_request_tag;
        PB_Network_WebSocketOpenRequest* ws = &request->content.network_websocket_open_request;
        ws->connection_id = req->connection_id;
        ws->url = strdup(req->url);
        ws->headers = rpc_network_strdup_or_null(req->headers);
        ws->timeout_ms = req->timeout_ms;
        break;
    }
    }

    rpc_send_and_release(rpc_network->session, request);
}

static void rpc_network_on_connect_response(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_ConnectResponse* source = &request->content.network_connect_response;
    const NetworkEvent event = {
        .type = NetworkEventConnected,
        .connection_id = source->connection_id,
        .state = (NetworkState)source->state,
        .error = (NetworkError)source->error,
        .resolved_ip = source->resolved_ip,
    };
    network_on_event(rpc_network->network, &event);
}

static void rpc_network_on_send_response(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_SendResponse* source = &request->content.network_send_response;
    const NetworkEvent event = {
        .type = NetworkEventSent,
        .connection_id = source->connection_id,
        .state = NetworkStateConnected,
        .error = (NetworkError)source->error,
        .size = source->bytes_sent,
    };
    network_on_event(rpc_network->network, &event);
}

static void rpc_network_on_receive_data(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_ReceiveData* source = &request->content.network_receive_data;
    const NetworkEvent event = {
        .type = NetworkEventReceived,
        .connection_id = source->connection_id,
        .state = NetworkStateConnected,
        .error = NetworkErrorNone,
        .data = source->data ? source->data->bytes : NULL,
        .size = source->data ? source->data->size : 0,
        .binary = source->binary,
    };
    network_on_event(rpc_network->network, &event);
}

static void rpc_network_on_close_response(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_CloseResponse* source = &request->content.network_close_response;
    const NetworkEvent event = {
        .type = NetworkEventClosed,
        .connection_id = source->connection_id,
        .state = NetworkStateDisconnected,
        .error = (NetworkError)source->error,
    };
    network_on_event(rpc_network->network, &event);
}

static void rpc_network_on_state_changed(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_StateChanged* source = &request->content.network_state_changed;
    const NetworkEvent event = {
        .type = NetworkEventStateChanged,
        .connection_id = source->connection_id,
        .state = (NetworkState)source->state,
        .error = (NetworkError)source->error,
    };
    network_on_event(rpc_network->network, &event);
}

static void rpc_network_on_http_response(const PB_Main* request, void* context) {
    RpcNetwork* rpc_network = context;
    const PB_Network_HttpResponse* source = &request->content.network_http_response;
    const NetworkEvent event = {
        .type = NetworkEventHttpResponse,
        .connection_id = source->request_id,
        .state = NetworkStateConnected,
        .error = (NetworkError)source->error,
        .http_headers = source->headers,
        .size = source->body_size,
        .http_status = source->status,
        .saved_to_file = source->saved_to_file,
    };
    network_on_event(rpc_network->network, &event);
}

void* rpc_network_alloc(RpcSession* session) {
    furi_assert(session);

    RpcNetwork* rpc_network = malloc(sizeof(RpcNetwork));
    rpc_network->session = session;
    rpc_network->network = furi_record_open(RECORD_NETWORK);
    rpc_network->request = malloc(sizeof(PB_Main));

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_network,
    };

    rpc_handler.message_handler = rpc_network_on_connect_response;
    rpc_add_handler(session, PB_Main_network_connect_response_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_network_on_send_response;
    rpc_add_handler(session, PB_Main_network_send_response_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_network_on_receive_data;
    rpc_add_handler(session, PB_Main_network_receive_data_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_network_on_close_response;
    rpc_add_handler(session, PB_Main_network_close_response_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_network_on_state_changed;
    rpc_add_handler(session, PB_Main_network_state_changed_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_network_on_http_response;
    rpc_add_handler(session, PB_Main_network_http_response_tag, &rpc_handler);

    network_set_rpc_bridge(rpc_network->network, rpc_network_send_request, rpc_network);

    return rpc_network;
}

void rpc_network_free(void* context) {
    furi_assert(context);
    RpcNetwork* rpc_network = context;

    network_set_rpc_bridge(rpc_network->network, NULL, NULL);
    furi_record_close(RECORD_NETWORK);

    rpc_network->session = NULL;
    free(rpc_network->request);
    free(rpc_network);
}
