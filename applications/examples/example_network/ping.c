#include <furi.h>
#include <gui/gui.h>
#include <network/network.h>
#include <string.h>

/* Multi-protocol ping over the companion Network bridge.
 *
 * Left/Right switches the probe protocol, OK restarts, Back exits:
 *  - DNS/UDP: the firmware builds a real RFC 1035 A query, sends it over a
 *    UDP socket and parses the reply (RTT, answer address, record TTL).
 *  - TCP+HTTP: each probe opens its own TCP connection (SYN RTT), then a raw
 *    "HEAD / HTTP/1.1" written with network_send() and timed to the first
 *    response byte.
 *  - HTTPS: HEAD request through the companion HTTP API (DNS+TCP+TLS+HTTP).
 *
 * Network API used: network_connect() (TCP and UDP), network_send(),
 * network_close(), network_http_request(), network_set_event_callback().
 * Connection ids are client-assigned; this app never reuses an id after
 * closing it (see id_counter).
 */

#define PROBE_COUNT       4
#define PROBE_INTERVAL_MS 1000
#define PROBE_TIMEOUT_MS  3000
#define HTTPS_TIMEOUT_MS  8000

#define TARGET_NAME "one.one.one.one"
#define DNS_SERVER  "1.1.1.1"
#define DNS_PORT    53
#define TCP_PORT    80
#define HTTPS_URL   "https://one.one.one.one/"

#define CONNECTION_ID_BASE 0x50

static const char TCP_HEAD_REQUEST[] = "HEAD / HTTP/1.1\r\n"
                                       "Host: " TARGET_NAME "\r\n"
                                       "User-Agent: Flipper-NetPing\r\n"
                                       "Connection: close\r\n\r\n";

typedef enum {
    ProbeModeDns,
    ProbeModeTcp,
    ProbeModeHttps,
    ProbeModeCount,
} ProbeMode;

typedef enum {
    RunStateIdle,
    RunStateNoBridge,
    RunStateOpening,
    RunStateProbing,
    RunStateDone,
    RunStateError,
} RunState;

typedef struct {
    bool done;
    bool ok;
    uint32_t time_ms;
    uint32_t aux_ms;
    size_t rx_bytes;
    NetworkError error;
    char detail[40];
} ProbeResult;

typedef struct {
    FuriMutex* mutex;

    ProbeMode mode;
    RunState state;
    NetworkError error;

    uint32_t id_counter;
    uint32_t cur_id;

    uint32_t active_seq;
    bool probe_active;
    bool start_probe;
    uint32_t next_probe_tick;
    uint32_t started_tick;

    uint16_t dns_txid;
    bool socket_open;

    bool send_tcp_request;
    bool tcp_request_sent;
    uint32_t tcp_syn_ms;

    ProbeResult results[PROBE_COUNT];
    uint32_t sent;
    uint32_t received;
    uint32_t min_ms;
    uint32_t max_ms;
    uint32_t total_ms;
    char resolved_ip[48];

    uint32_t close_ids[8];
    uint32_t close_count;
} PingApp;

static size_t dns_build_query(uint8_t* buf, size_t cap, uint16_t txid, const char* name) {
    size_t need = 12 + strlen(name) + 2 + 4;
    if(cap < need) return 0;

    memset(buf, 0, 12);
    buf[0] = txid >> 8;
    buf[1] = txid & 0xFF;
    buf[2] = 0x01; /* recursion desired */
    buf[5] = 0x01; /* one question */

    size_t pos = 12;
    const char* label = name;
    while(*label) {
        const char* dot = strchr(label, '.');
        size_t len = dot ? (size_t)(dot - label) : strlen(label);
        if(len == 0 || len > 63) return 0;
        buf[pos++] = len;
        memcpy(&buf[pos], label, len);
        pos += len;
        label += len;
        if(*label == '.') label++;
    }
    buf[pos++] = 0;
    buf[pos++] = 0x00;
    buf[pos++] = 0x01; /* QTYPE A */
    buf[pos++] = 0x00;
    buf[pos++] = 0x01; /* QCLASS IN */
    return pos;
}

static bool dns_parse_response(
    const uint8_t* data,
    size_t size,
    uint16_t txid,
    char* ip,
    size_t ip_size,
    uint32_t* ttl) {
    if(size < 12) return false;
    if((((uint16_t)data[0] << 8) | data[1]) != txid) return false;
    uint16_t flags = ((uint16_t)data[2] << 8) | data[3];
    if((flags & 0x8000) == 0) return false; /* not a response */
    if((flags & 0x000F) != 0) return false; /* RCODE != NOERROR */
    uint16_t qdcount = ((uint16_t)data[4] << 8) | data[5];
    uint16_t ancount = ((uint16_t)data[6] << 8) | data[7];
    if(ancount == 0) return false;

    size_t pos = 12;
    for(uint16_t i = 0; i < qdcount; i++) {
        while(pos < size && data[pos] != 0 && (data[pos] & 0xC0) != 0xC0) {
            pos += (size_t)data[pos] + 1;
        }
        if(pos >= size) return false;
        pos += (data[pos] & 0xC0) == 0xC0 ? 2 : 1;
        pos += 4; /* QTYPE + QCLASS */
    }

    for(uint16_t i = 0; i < ancount; i++) {
        if(pos >= size) return false;
        if((data[pos] & 0xC0) == 0xC0) {
            pos += 2;
        } else {
            while(pos < size && data[pos] != 0) {
                pos += (size_t)data[pos] + 1;
            }
            pos++;
        }
        if(pos + 10 > size) return false;
        uint16_t type = ((uint16_t)data[pos] << 8) | data[pos + 1];
        uint16_t dns_class = ((uint16_t)data[pos + 2] << 8) | data[pos + 3];
        uint32_t rec_ttl = ((uint32_t)data[pos + 4] << 24) | ((uint32_t)data[pos + 5] << 16) |
                           ((uint32_t)data[pos + 6] << 8) | data[pos + 7];
        uint16_t rdlen = ((uint16_t)data[pos + 8] << 8) | data[pos + 9];
        pos += 10;
        if(pos + rdlen > size) return false;
        if(type == 1 && dns_class == 1 && rdlen == 4) {
            snprintf(ip, ip_size, "%u.%u.%u.%u", data[pos], data[pos + 1], data[pos + 2], data[pos + 3]);
            if(ttl) *ttl = rec_ttl;
            return true;
        }
        pos += rdlen;
    }
    return false;
}

static uint32_t http_parse_status(const uint8_t* data, size_t size) {
    if(size < 12) return 0;
    if(memcmp(data, "HTTP/1.", 7) != 0) return 0;
    uint32_t status = 0;
    for(size_t i = 9; i < 12; i++) {
        if(data[i] < '0' || data[i] > '9') return 0;
        status = status * 10 + (data[i] - '0');
    }
    return status;
}

/* Helpers below expect app->mutex to be held by the caller. */

static void ping_push_close(PingApp* app, uint32_t id) {
    if(id == 0) return;
    if(app->close_count < COUNT_OF(app->close_ids)) {
        app->close_ids[app->close_count++] = id;
    }
}

static void ping_record(
    PingApp* app,
    bool ok,
    uint32_t time_ms,
    uint32_t aux_ms,
    size_t rx_bytes,
    NetworkError error,
    const char* detail) {
    if(app->active_seq >= PROBE_COUNT) return;
    ProbeResult* result = &app->results[app->active_seq];
    result->done = true;
    result->ok = ok;
    result->time_ms = time_ms ? time_ms : 1;
    result->aux_ms = aux_ms;
    result->rx_bytes = rx_bytes;
    result->error = error;
    strlcpy(result->detail, detail, sizeof(result->detail));

    if(ok) {
        app->received++;
        app->total_ms += result->time_ms;
        if(app->received == 1 || result->time_ms < app->min_ms) app->min_ms = result->time_ms;
        if(result->time_ms > app->max_ms) app->max_ms = result->time_ms;
    } else if(error != NetworkErrorNone) {
        app->error = error;
    }
}

static void ping_probe_finish(PingApp* app) {
    app->probe_active = false;
    app->send_tcp_request = false;
    if(app->mode == ProbeModeTcp) {
        ping_push_close(app, app->cur_id);
    }
    app->active_seq++;
    app->start_probe = app->active_seq < PROBE_COUNT;
    if(app->active_seq >= PROBE_COUNT) {
        app->state = app->received ? RunStateDone : RunStateError;
        if(!app->received && app->error == NetworkErrorNone) app->error = NetworkErrorTimeout;
        if(app->socket_open) {
            ping_push_close(app, app->cur_id);
            app->socket_open = false;
        }
    }
}

static void ping_handle_dns_event(PingApp* app, const NetworkEvent* event) {
    switch(event->type) {
    /* Answer to network_connect(): event->state/error report the outcome,
     * event->resolved_ip (not used here) carries the peer address. */
    case NetworkEventConnected:
        if(app->state == RunStateOpening) {
            if(event->error == NetworkErrorNone && event->state == NetworkStateConnected) {
                app->state = RunStateProbing;
                app->start_probe = true;
                app->next_probe_tick = furi_get_tick();
            } else {
                app->state = RunStateError;
                app->error = event->error != NetworkErrorNone ? event->error :
                                                                NetworkErrorInternal;
                ping_push_close(app, app->cur_id);
                app->socket_open = false;
            }
        }
        break;
    case NetworkEventReceived:
        if(app->probe_active && event->data && event->size) {
            char ip[16];
            uint32_t ttl = 0;
            if(dns_parse_response(event->data, event->size, app->dns_txid, ip, sizeof(ip), &ttl)) {
                char detail[40];
                snprintf(detail, sizeof(detail), "A %s ttl=%lus", ip, (unsigned long)ttl);
                strlcpy(app->resolved_ip, ip, sizeof(app->resolved_ip));
                ping_record(
                    app,
                    true,
                    furi_get_tick() - app->started_tick,
                    0,
                    event->size,
                    NetworkErrorNone,
                    detail);
                ping_probe_finish(app);
            }
            /* Mismatched txid: a late reply to a lost probe, keep waiting. */
        }
        break;
    /* Ack for network_send(): only interesting when the datagram failed. */
    case NetworkEventSent:
        if(app->probe_active && event->error != NetworkErrorNone) {
            ping_record(app, false, 0, 0, 0, event->error, "");
            ping_probe_finish(app);
        }
        break;
    /* Socket died (peer/companion side) or close was acknowledged. */
    case NetworkEventStateChanged:
    case NetworkEventClosed:
        if(app->state == RunStateProbing && network_state_is_terminal(event->state)) {
            NetworkError error = event->error != NetworkErrorNone ? event->error :
                                                                    NetworkErrorNotConnected;
            if(app->probe_active) {
                ping_record(app, false, 0, 0, 0, error, "");
            }
            app->probe_active = false;
            app->start_probe = false;
            app->state = RunStateError;
            app->error = error;
            app->socket_open = false;
        }
        break;
    default:
        break;
    }
}

static void ping_handle_tcp_event(PingApp* app, const NetworkEvent* event) {
    if(!app->probe_active) return;

    switch(event->type) {
    case NetworkEventConnected:
        if(!app->tcp_request_sent) {
            if(event->error == NetworkErrorNone && event->state == NetworkStateConnected) {
                app->tcp_syn_ms = furi_get_tick() - app->started_tick;
                if(event->resolved_ip && event->resolved_ip[0]) {
                    strlcpy(app->resolved_ip, event->resolved_ip, sizeof(app->resolved_ip));
                }
                app->send_tcp_request = true;
            } else {
                ping_record(app, false, 0, 0, 0, event->error, "");
                ping_probe_finish(app);
            }
        }
        break;
    case NetworkEventReceived:
        /* First chunk of the response stops the clock; trailing chunks arrive
         * with probe_active already false and are dropped. */
        if(app->tcp_request_sent && event->data && event->size) {
            char detail[40];
            uint32_t status = http_parse_status(event->data, event->size);
            if(status) {
                snprintf(
                    detail,
                    sizeof(detail),
                    "HTTP %lu %.21s",
                    (unsigned long)status,
                    app->resolved_ip[0] ? app->resolved_ip : TARGET_NAME);
            } else {
                snprintf(
                    detail,
                    sizeof(detail),
                    "raw %.21s",
                    app->resolved_ip[0] ? app->resolved_ip : TARGET_NAME);
            }
            ping_record(
                app,
                true,
                furi_get_tick() - app->started_tick,
                app->tcp_syn_ms,
                event->size,
                NetworkErrorNone,
                detail);
            ping_probe_finish(app);
        }
        break;
    case NetworkEventSent:
        if(event->error != NetworkErrorNone) {
            ping_record(app, false, 0, 0, 0, event->error, "");
            ping_probe_finish(app);
        }
        break;
    case NetworkEventStateChanged:
    case NetworkEventClosed:
        if(network_state_is_terminal(event->state)) {
            NetworkError error = event->error != NetworkErrorNone ? event->error :
                                                                    NetworkErrorNotConnected;
            ping_record(app, false, 0, 0, 0, error, "");
            ping_probe_finish(app);
        }
        break;
    default:
        break;
    }
}

static void ping_handle_https_event(PingApp* app, const NetworkEvent* event) {
    if(!app->probe_active) return;
    /* HTTP requests produce exactly one terminal event; with no save_path the
     * body would also stream in as NetworkEventReceived (ignored here). */
    if(event->type != NetworkEventHttpResponse) return;

    if(event->error == NetworkErrorNone) {
        char detail[40];
        snprintf(detail, sizeof(detail), "HTTP %lu via TLS", (unsigned long)event->http_status);
        ping_record(
            app,
            true,
            furi_get_tick() - app->started_tick,
            0,
            event->size,
            NetworkErrorNone,
            detail);
    } else {
        ping_record(app, false, 0, 0, 0, event->error, "");
    }
    ping_probe_finish(app);
}

/* Called from the RPC session thread; events for stale connection ids (closed
 * probes, previous runs) are dropped by the cur_id check. The event payload is
 * valid only for the duration of the call. */
static void ping_event_callback(const NetworkEvent* event, void* context) {
    PingApp* app = context;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    if(event->connection_id == app->cur_id) {
        switch(app->mode) {
        case ProbeModeDns:
            ping_handle_dns_event(app, event);
            break;
        case ProbeModeTcp:
            ping_handle_tcp_event(app, event);
            break;
        case ProbeModeHttps:
            ping_handle_https_event(app, event);
            break;
        default:
            break;
        }
    }
    furi_mutex_release(app->mutex);
}

static void ping_start_run(PingApp* app, Network* network, ProbeMode mode) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    uint32_t stale_id = app->socket_open || (app->mode == ProbeModeTcp && app->probe_active) ?
                            app->cur_id :
                            0;
    app->mode = mode;
    app->error = NetworkErrorNone;
    memset(app->results, 0, sizeof(app->results));
    app->active_seq = 0;
    app->probe_active = false;
    app->send_tcp_request = false;
    app->tcp_request_sent = false;
    app->sent = 0;
    app->received = 0;
    app->min_ms = 0;
    app->max_ms = 0;
    app->total_ms = 0;
    app->tcp_syn_ms = 0;
    app->resolved_ip[0] = '\0';
    app->socket_open = false;
    app->cur_id = ++app->id_counter;
    uint32_t run_id = app->cur_id;

    if(mode == ProbeModeDns) {
        app->state = RunStateOpening;
        app->start_probe = false;
        app->started_tick = furi_get_tick();
    } else {
        app->state = RunStateProbing;
        app->start_probe = true;
        app->next_probe_tick = furi_get_tick();
    }
    furi_mutex_release(app->mutex);

    if(stale_id) network_close(network, stale_id);

    if(mode == ProbeModeDns) {
        /* One UDP socket for the whole run. The call only queues the request:
         * the outcome comes back as NetworkEventConnected. */
        bool opened = network_connect(
            network, run_id, DNS_SERVER, DNS_PORT, NetworkProtocolUdp, PROBE_TIMEOUT_MS);
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        if(opened) {
            app->socket_open = true;
        } else {
            app->state = RunStateNoBridge;
        }
        furi_mutex_release(app->mutex);
    }
}

static void ping_process(PingApp* app, Network* network) {
    uint32_t closes[COUNT_OF(app->close_ids)];
    uint32_t close_count;
    bool dns_send = false;
    bool head_send = false;
    bool tcp_connect = false;
    bool https_request = false;
    uint32_t action_id = 0;
    uint16_t txid = 0;

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    uint32_t now = furi_get_tick();

    if(app->state == RunStateOpening &&
       now - app->started_tick > furi_ms_to_ticks(PROBE_TIMEOUT_MS)) {
        app->state = RunStateError;
        app->error = NetworkErrorTimeout;
        if(app->socket_open) {
            ping_push_close(app, app->cur_id);
            app->socket_open = false;
        }
    }

    if(app->probe_active) {
        uint32_t timeout_ms = app->mode == ProbeModeHttps ? HTTPS_TIMEOUT_MS : PROBE_TIMEOUT_MS;
        if(now - app->started_tick > furi_ms_to_ticks(timeout_ms)) {
            ping_record(app, false, 0, 0, 0, NetworkErrorTimeout, "");
            ping_probe_finish(app);
        }
    }

    if(app->send_tcp_request && app->probe_active) {
        app->send_tcp_request = false;
        app->tcp_request_sent = true;
        head_send = true;
        action_id = app->cur_id;
    }

    bool launch = app->state == RunStateProbing && app->start_probe && !app->probe_active &&
                  app->active_seq < PROBE_COUNT && now >= app->next_probe_tick;
    if(launch) {
        app->start_probe = false;
        app->probe_active = true;
        app->sent++;
        app->started_tick = now;
        app->next_probe_tick = now + furi_ms_to_ticks(PROBE_INTERVAL_MS);
        app->tcp_request_sent = false;
        app->tcp_syn_ms = 0;
        if(app->mode == ProbeModeDns) {
            app->dns_txid = (uint16_t)((now << 4) | app->active_seq);
            dns_send = true;
            action_id = app->cur_id;
            txid = app->dns_txid;
        } else {
            app->cur_id = ++app->id_counter;
            action_id = app->cur_id;
            if(app->mode == ProbeModeTcp) {
                tcp_connect = true;
            } else {
                https_request = true;
            }
        }
    }

    close_count = app->close_count;
    memcpy(closes, app->close_ids, close_count * sizeof(uint32_t));
    app->close_count = 0;
    furi_mutex_release(app->mutex);

    /* Fire-and-forget: the companion confirms each close with
     * NetworkEventClosed. */
    for(uint32_t i = 0; i < close_count; i++) {
        network_close(network, closes[i]);
    }

    /* Every network_* call below returns false only when there is no live
     * companion session; probe results arrive via ping_event_callback. */
    bool bridge_ok = true;
    if(dns_send) {
        /* One datagram per probe over the open UDP socket
         * (payload <= NETWORK_MAX_DATA_SIZE); reply = NetworkEventReceived. */
        uint8_t query[64];
        size_t query_size = dns_build_query(query, sizeof(query), txid, TARGET_NAME);
        bridge_ok = query_size && network_send(network, action_id, query, query_size);
    } else if(head_send) {
        /* Raw bytes over the connected TCP socket - the service does not
         * interpret them, HTTP framing is this app's own. */
        bridge_ok = network_send(
            network, action_id, (const uint8_t*)TCP_HEAD_REQUEST, strlen(TCP_HEAD_REQUEST));
    } else if(tcp_connect) {
        /* Fresh client-assigned id per probe: each probe is its own TCP
         * connection, timed from here to NetworkEventConnected. */
        bridge_ok = network_connect(
            network, action_id, TARGET_NAME, TCP_PORT, NetworkProtocolTcp, PROBE_TIMEOUT_MS);
    } else if(https_request) {
        /* Full HTTP stack on the companion; concludes with
         * NetworkEventHttpResponse for this request id. */
        const NetworkHttpRequest request = {
            .method = NetworkHttpMethodHead,
            .url = HTTPS_URL,
            .timeout_ms = HTTPS_TIMEOUT_MS,
        };
        bridge_ok = network_http_request(network, action_id, &request);
    }

    if(!bridge_ok) {
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->state = RunStateNoBridge;
        app->probe_active = false;
        app->start_probe = false;
        app->socket_open = false;
        furi_mutex_release(app->mutex);
    }
}

static const char* ping_mode_title(ProbeMode mode) {
    switch(mode) {
    case ProbeModeDns:
        return "< DNS/UDP 1.1.1.1:53 >";
    case ProbeModeTcp:
        return "< TCP + raw HTTP :80 >";
    case ProbeModeHttps:
        return "< HTTPS request >";
    default:
        return "";
    }
}

static void ping_render_callback(Canvas* canvas, void* context) {
    PingApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    char line[64];
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignBottom, ping_mode_title(app->mode));

    uint32_t cursor = app->active_seq;
    if(app->probe_active && cursor < PROBE_COUNT) {
        cursor++;
    }
    uint32_t first = cursor > 2 ? cursor - 2 : 0;
    uint32_t visible = cursor - first;
    if(visible > 2) visible = 2;

    for(uint32_t slot = 0; slot < visible; slot++) {
        uint32_t i = first + slot;
        const ProbeResult* result = &app->results[i];
        uint8_t y = 21 + slot * 20;

        if(result->done && result->ok) {
            switch(app->mode) {
            case ProbeModeTcp:
                snprintf(
                    line,
                    sizeof(line),
                    "seq=%lu syn=%lu http=%lums",
                    (unsigned long)i,
                    (unsigned long)result->aux_ms,
                    (unsigned long)result->time_ms);
                break;
            case ProbeModeDns:
                snprintf(
                    line,
                    sizeof(line),
                    "seq=%lu time=%lums rx=%uB",
                    (unsigned long)i,
                    (unsigned long)result->time_ms,
                    (unsigned)result->rx_bytes);
                break;
            default:
                snprintf(
                    line,
                    sizeof(line),
                    "seq=%lu time=%lums",
                    (unsigned long)i,
                    (unsigned long)result->time_ms);
                break;
            }
            canvas_draw_str(canvas, 0, y, line);
            canvas_draw_str(canvas, 0, y + 9, result->detail);
        } else if(result->done) {
            snprintf(line, sizeof(line), "seq=%lu lost", (unsigned long)i);
            canvas_draw_str(canvas, 0, y, line);
            canvas_draw_str(canvas, 0, y + 9, network_error_to_string(result->error));
        } else if(app->probe_active && app->active_seq == i) {
            snprintf(line, sizeof(line), "seq=%lu", (unsigned long)i);
            canvas_draw_str(canvas, 0, y, line);
            if(app->mode == ProbeModeDns) {
                snprintf(line, sizeof(line), "txid=0x%04X...", app->dns_txid);
            } else if(app->mode == ProbeModeTcp) {
                if(app->tcp_request_sent) {
                    snprintf(
                        line, sizeof(line), "syn=%lums wait rsp", (unsigned long)app->tcp_syn_ms);
                } else {
                    snprintf(line, sizeof(line), "connecting...");
                }
            } else {
                snprintf(line, sizeof(line), "TLS handshake...");
            }
            canvas_draw_str(canvas, 0, y + 9, line);
        }
    }

    if(app->state == RunStateNoBridge) {
        canvas_draw_str(canvas, 0, 63, "No USB/BLE connection");
    } else if(app->state == RunStateOpening) {
        canvas_draw_str(canvas, 0, 63, "Opening UDP socket...");
    } else if(app->state == RunStateDone || app->state == RunStateError) {
        uint32_t loss = ((PROBE_COUNT - app->received) * 100) / PROBE_COUNT;
        if(app->received) {
            snprintf(
                line,
                sizeof(line),
                "%lu/%u rx %lu%% loss %lu/%lu/%lums",
                (unsigned long)app->received,
                PROBE_COUNT,
                (unsigned long)loss,
                (unsigned long)app->min_ms,
                (unsigned long)(app->total_ms / app->received),
                (unsigned long)app->max_ms);
        } else {
            snprintf(
                line,
                sizeof(line),
                "0/%u rx: %s",
                PROBE_COUNT,
                network_error_to_string(app->error));
        }
        canvas_draw_str(canvas, 0, 63, line);
    } else {
        snprintf(
            line,
            sizeof(line),
            "%.21s %lu/%u",
            app->resolved_ip[0] ? app->resolved_ip : TARGET_NAME,
            (unsigned long)app->sent,
            PROBE_COUNT);
        canvas_draw_str(canvas, 0, 63, line);
    }

    furi_mutex_release(app->mutex);
}

static void ping_input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t network_ping_app(void* p) {
    UNUSED(p);

    PingApp* app = malloc(sizeof(PingApp));
    memset(app, 0, sizeof(PingApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->state = RunStateIdle;
    app->id_counter = CONNECTION_ID_BASE;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    Network* network = furi_record_open(RECORD_NETWORK);
    network_set_event_callback(network, ping_event_callback, app);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, ping_render_callback, app);
    view_port_input_callback_set(view_port, ping_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    ping_start_run(app, network, ProbeModeDns);

    InputEvent event;
    for(bool processing = true; processing;) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk &&
           event.type == InputTypeShort) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            ProbeMode mode = app->mode;
            furi_mutex_release(app->mutex);

            if(event.key == InputKeyBack) {
                processing = false;
            } else if(event.key == InputKeyOk) {
                ping_start_run(app, network, mode);
            } else if(event.key == InputKeyLeft || event.key == InputKeyRight) {
                uint32_t step = event.key == InputKeyRight ? 1 : ProbeModeCount - 1;
                ping_start_run(app, network, (ProbeMode)((mode + step) % ProbeModeCount));
            }
        }

        ping_process(app, network);
        view_port_update(view_port);
    }

    furi_mutex_acquire(app->mutex, FuriWaitForever);
    if(app->socket_open || app->probe_active) {
        ping_push_close(app, app->cur_id);
        app->socket_open = false;
    }
    uint32_t close_count = app->close_count;
    uint32_t closes[COUNT_OF(app->close_ids)];
    memcpy(closes, app->close_ids, close_count * sizeof(uint32_t));
    app->close_count = 0;
    furi_mutex_release(app->mutex);
    for(uint32_t i = 0; i < close_count; i++) {
        network_close(network, closes[i]);
    }

    /* Unsubscribe before closing the record so late events cannot reach a
     * freed context. */
    network_set_event_callback(network, NULL, NULL);
    furi_record_close(RECORD_NETWORK);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
