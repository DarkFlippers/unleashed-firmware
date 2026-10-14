#include "felica_listener_i.h"

#include "nfc/protocols/nfc_listener_base.h"
#include <nfc/helpers/felica_crc.h>
#include <furi_hal_nfc.h>

#define FELICA_LISTENER_MAX_BUFFER_SIZE     (128)
#define FELICA_LISTENER_CMD_POLLING         (0x00U)
#define FELICA_LISTENER_RESPONSE_POLLING    (0x01U)
#define FELICA_LISTENER_RESPONSE_CODE_READ  (0x07)
#define FELICA_LISTENER_RESPONSE_CODE_WRITE (0x09)

#define FELICA_LISTENER_REQUEST_NONE        (0x00U)
#define FELICA_LISTENER_REQUEST_SYSTEM_CODE (0x01U)
#define FELICA_LISTENER_REQUEST_PERFORMANCE (0x02U)

#define FELICA_LISTENER_SYSTEM_CODE_NDEF  (__builtin_bswap16(0x12FCU))
#define FELICA_LISTENER_SYSTEM_CODE_LITES (__builtin_bswap16(0x88B4U))

#define FELICA_LISTENER_PERFORMANCE_VALUE (__builtin_bswap16(0x0083U))

#define FELICA_BLOCK_LIST_ELEMENT_HEADER_SHORT_MASK         (0x80U)
#define FELICA_BLOCK_LIST_ELEMENT_HEADER_SERVICE_INDEX_MASK (0x0FU)

#define TAG "FelicaListener"

FelicaListener* felica_listener_alloc(Nfc* nfc, FelicaData* data) {
    furi_assert(nfc);
    furi_assert(data);

    FelicaListener* instance = malloc(sizeof(FelicaListener));
    instance->nfc = nfc;
    instance->data = data;
    instance->tx_buffer = bit_buffer_alloc(FELICA_LISTENER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(FELICA_LISTENER_MAX_BUFFER_SIZE);

    mbedtls_des3_init(&instance->auth.des_context);
    nfc_set_fdt_listen_fc(instance->nfc, FELICA_FDT_LISTEN_FC);

    memcpy(instance->mc_shadow.data, instance->data->data.fs.mc.data, FELICA_DATA_BLOCK_SIZE);
    instance->data->data.fs.state.data[0] = 0;
    instance->mode = 0;
    instance->current_system_idx = 0;
    nfc_config(instance->nfc, NfcModeListener, NfcTechFelica);

    // PMm bytes 2-6 encode the max response time a reader should allow per command
    // (Request Service / Request Response / Read / Write / Auth). Cards saved from a
    // real (hardware-fast) FeliCa IC carry short values here; our software emulation
    // is slower to respond, so real readers following those short timeouts abandon
    // the session mid-transaction. Widen them to the max so readers wait long enough.
    instance->pmm = data->pmm;
    memset(instance->pmm.data + 2, 0xFF, FELICA_PMM_SIZE - 2);

    uint16_t system_code = *(uint16_t*)data->data.fs.sys_c.data;
    // Standard cards keep their System Codes in the System list rather than in SYS_C
    if(data->workflow_type == FelicaStandard && simple_array_get_count(data->systems) > 0) {
        const FelicaSystem* system = simple_array_cget(data->systems, 0);
        system_code = __builtin_bswap16(system->system_code);
    }
    nfc_felica_listener_set_sensf_res_data(
        nfc,
        data->idm.data,
        sizeof(data->idm),
        instance->pmm.data,
        sizeof(instance->pmm),
        system_code);

    return instance;
}

void felica_listener_free(FelicaListener* instance) {
    furi_assert(instance);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    free(instance);
}

void felica_listener_set_callback(
    FelicaListener* listener,
    NfcGenericCallback callback,
    void* context) {
    UNUSED(listener);
    UNUSED(callback);
    UNUSED(context);
}

const FelicaData* felica_listener_get_data(const FelicaListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static FelicaError felica_listener_command_handler_read(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    const FelicaListenerReadRequest* request = (FelicaListenerReadRequest*)generic_request;
    FURI_LOG_D(TAG, "Read cmd");

    FelicaListenerReadCommandResponse* resp = malloc(
        sizeof(FelicaCommandResponseHeader) + 1 +
        FELICA_LISTENER_READ_BLOCK_COUNT_MAX * FELICA_DATA_BLOCK_SIZE);

    resp->header.response_code = FELICA_LISTENER_RESPONSE_CODE_READ;
    resp->header.idm = request->base.header.idm;
    resp->header.length = sizeof(FelicaCommandResponseHeader);

    if(felica_listener_validate_read_request_and_set_sf(instance, request, &resp->header)) {
        resp->block_count = request->base.header.block_count;
        resp->header.length++;
    } else {
        resp->block_count = 0;
    }

    instance->mac_calc_start = 0;
    memset(instance->requested_blocks, 0, sizeof(instance->requested_blocks));
    const FelicaBlockListElement* item =
        felica_listener_block_list_item_get_first(instance, request);
    for(uint8_t i = 0; i < resp->block_count; i++) {
        instance->requested_blocks[i] = item->block_number;
        FelicaCommanReadBlockHandler handler =
            felica_listener_get_read_block_handler(item->block_number);

        handler(instance, item->block_number, i, resp);

        item = felica_listener_block_list_item_get_next(instance, item);
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)resp, resp->header.length);
    free(resp);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_command_handler_write(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    FURI_LOG_D(TAG, "Write cmd");

    const FelicaListenerWriteRequest* request = (FelicaListenerWriteRequest*)generic_request;
    const FelicaListenerWriteBlockData* data_ptr =
        felica_listener_get_write_request_data_pointer(instance, generic_request);

    FelicaListenerWriteCommandResponse* resp = malloc(sizeof(FelicaListenerWriteCommandResponse));

    resp->response_code = FELICA_LISTENER_RESPONSE_CODE_WRITE;
    resp->idm = request->base.header.idm;
    resp->length = sizeof(FelicaListenerWriteCommandResponse);

    if(felica_listener_validate_write_request_and_set_sf(instance, request, data_ptr, resp)) {
        const FelicaBlockListElement* item =
            felica_listener_block_list_item_get_first(instance, request);
        for(uint8_t i = 0; i < request->base.header.block_count; i++) {
            FelicaCommandWriteBlockHandler handler =
                felica_listener_get_write_block_handler(item->block_number);

            handler(instance, item->block_number, &data_ptr->blocks[i]);

            item = felica_listener_block_list_item_get_next(instance, item);
        }
        felica_wcnt_increment(instance->data);
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)resp, resp->length);
    free(resp);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

// Returns the currently selected System (instance->current_system_idx), or NULL if
// there is none - callers must not assume System 0 is the active one, since Polling
// with a specific System Code can switch this mid-session.
static const FelicaSystem* felica_listener_get_current_system(const FelicaListener* instance) {
    uint32_t system_count = simple_array_get_count(instance->data->systems);
    if(instance->current_system_idx >= system_count) return NULL;
    return simple_array_cget(instance->data->systems, instance->current_system_idx);
}

static FelicaSystem* felica_listener_get_current_system_mut(FelicaListener* instance) {
    uint32_t system_count = simple_array_get_count(instance->data->systems);
    if(instance->current_system_idx >= system_count) return NULL;
    return simple_array_get(instance->data->systems, instance->current_system_idx);
}

// Returns the Service with the given code in the System, or NULL if there is none
static const FelicaService*
    felica_listener_get_service(const FelicaSystem* system, uint16_t service_code) {
    if(!system) return NULL;
    uint32_t svc_count = simple_array_get_count(system->services);
    for(uint32_t i = 0; i < svc_count; i++) {
        const FelicaService* svc = simple_array_cget(system->services, i);
        if(svc->code == service_code) return svc;
    }
    return NULL;
}

// Sends a response that carries only the Status Flags
static FelicaError felica_listener_send_status_response(
    FelicaListener* instance,
    uint8_t response_code,
    uint8_t sf1,
    uint8_t sf2) {
    uint8_t resp_buf[12];
    resp_buf[0] = sizeof(resp_buf);
    resp_buf[1] = response_code;
    const FelicaIDm current_idm = felica_listener_get_current_idm(instance);
    memcpy(resp_buf + 2, current_idm.data, sizeof(current_idm.data));
    resp_buf[10] = sf1;
    resp_buf[11] = sf2;

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, sizeof(resp_buf));

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

// Max blocks for Standard read that fit within the 128-byte tx buffer (with CRC)
#define FELICA_STANDARD_READ_BLOCK_MAX (7U)

static FelicaError felica_listener_command_handler_standard_read(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    const uint8_t* raw = (const uint8_t*)generic_request;
    uint8_t service_num = raw[10];
    if(service_num == 0 || service_num > 16) {
        return felica_listener_send_status_response(
            instance, FELICA_LISTENER_RESPONSE_CODE_READ, 0xFF, FELICA_SF2_ERROR_SERVICE_COUNT);
    }

    uint16_t service_codes[16];
    for(uint8_t i = 0; i < service_num; i++) {
        service_codes[i] = (uint16_t)(raw[11 + i * 2] | ((uint16_t)raw[12 + i * 2] << 8));
    }

    uint8_t block_count = raw[11 + service_num * 2];
    const uint8_t* bptr = raw + 12 + service_num * 2;

    const FelicaSystem* system = felica_listener_get_current_system(instance);

    uint8_t sf1 = FELICA_SF1_SUCCESS, sf2 = FELICA_SF2_SUCCESS;
    uint8_t block_data[FELICA_STANDARD_READ_BLOCK_MAX][FELICA_DATA_BLOCK_SIZE];
    uint8_t actual_block_count = 0;

    if(block_count == 0 || block_count > FELICA_STANDARD_READ_BLOCK_MAX) {
        sf1 = 0xFF;
        sf2 = FELICA_SF2_ERROR_BLOCK_COUNT;
    }

    for(uint8_t i = 0; sf1 == FELICA_SF1_SUCCESS && i < block_count; i++) {
        uint8_t ble_hdr = bptr[0];
        uint8_t svc_idx = ble_hdr & FELICA_BLOCK_LIST_ELEMENT_HEADER_SERVICE_INDEX_MASK;
        bool is_extended = (ble_hdr & FELICA_BLOCK_LIST_ELEMENT_HEADER_SHORT_MASK) == 0;
        uint16_t blk_num = bptr[1];

        if(is_extended) {
            blk_num |= (uint16_t)bptr[2] << 8;
        }
        bptr += is_extended ? 3 : 2;

        if(svc_idx >= service_num) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_BLOCK_LIST_SERVICE_ORDER;
            break;
        }

        uint16_t svc_code = service_codes[svc_idx];
        if(!felica_listener_get_service(system, svc_code)) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_SERVICE_CODE;
            break;
        }

        bool found = false;
        uint32_t pb_count = simple_array_get_count(system->public_blocks);
        for(uint32_t j = 0; j < pb_count; j++) {
            const FelicaPublicBlock* pb = simple_array_cget(system->public_blocks, j);
            if(pb->service_code == svc_code && pb->block_idx == blk_num) {
                memcpy(block_data[actual_block_count], pb->block.data, FELICA_DATA_BLOCK_SIZE);
                found = true;
                break;
            }
        }

        if(!found) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_BLOCK_NUMBER;
            break;
        }
        actual_block_count++;
    }

    size_t resp_size = (sf1 == FELICA_SF1_SUCCESS) ?
                           (size_t)(12 + 1 + actual_block_count * FELICA_DATA_BLOCK_SIZE) :
                           12;
    uint8_t* resp_buf = malloc(resp_size);
    resp_buf[0] = (uint8_t)resp_size;
    resp_buf[1] = FELICA_LISTENER_RESPONSE_CODE_READ;
    const FelicaIDm current_idm = felica_listener_get_current_idm(instance);
    memcpy(resp_buf + 2, current_idm.data, 8);
    resp_buf[10] = sf1;
    resp_buf[11] = sf2;
    if(sf1 == FELICA_SF1_SUCCESS) {
        resp_buf[12] = actual_block_count;
        for(uint8_t i = 0; i < actual_block_count; i++) {
            memcpy(
                resp_buf + 13 + i * FELICA_DATA_BLOCK_SIZE, block_data[i], FELICA_DATA_BLOCK_SIZE);
        }
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, resp_size);
    free(resp_buf);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_command_handler_standard_write(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    const uint8_t* raw = (const uint8_t*)generic_request;
    uint8_t service_num = raw[10];
    if(service_num == 0 || service_num > 16) {
        return felica_listener_send_status_response(
            instance, FELICA_LISTENER_RESPONSE_CODE_WRITE, 0xFF, FELICA_SF2_ERROR_SERVICE_COUNT);
    }

    uint16_t service_codes[16];
    for(uint8_t i = 0; i < service_num; i++) {
        service_codes[i] = (uint16_t)(raw[11 + i * 2] | ((uint16_t)raw[12 + i * 2] << 8));
    }

    uint8_t block_count = raw[11 + service_num * 2];
    const uint8_t* bptr = raw + 12 + service_num * 2;

    FelicaSystem* system = felica_listener_get_current_system_mut(instance);

    uint8_t sf1 = FELICA_SF1_SUCCESS, sf2 = FELICA_SF2_SUCCESS;

    FelicaPublicBlock* blocks[16];

    if(block_count == 0 || block_count > COUNT_OF(blocks)) {
        sf1 = 0xFF;
        sf2 = FELICA_SF2_ERROR_BLOCK_COUNT;
    }

    for(uint8_t i = 0; sf1 == FELICA_SF1_SUCCESS && i < block_count; i++) {
        uint8_t ble_hdr = bptr[0];
        uint8_t svc_idx = ble_hdr & FELICA_BLOCK_LIST_ELEMENT_HEADER_SERVICE_INDEX_MASK;
        bool is_extended = (ble_hdr & FELICA_BLOCK_LIST_ELEMENT_HEADER_SHORT_MASK) == 0;
        uint16_t blk_num = bptr[1];

        if(is_extended) {
            blk_num |= (uint16_t)bptr[2] << 8;
        }
        bptr += is_extended ? 3 : 2;

        if(svc_idx >= service_num) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_BLOCK_LIST_SERVICE_ORDER;
            break;
        }

        uint16_t svc_code = service_codes[svc_idx];
        const FelicaService* svc = felica_listener_get_service(system, svc_code);
        if(!svc || (svc->attr & FELICA_SERVICE_ATTRIBUTE_READ_ONLY)) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_SERVICE_CODE;
            break;
        }

        blocks[i] = NULL;
        uint32_t pb_count = simple_array_get_count(system->public_blocks);
        for(uint32_t j = 0; j < pb_count; j++) {
            FelicaPublicBlock* pb = simple_array_get(system->public_blocks, j);
            if(pb->service_code == svc_code && pb->block_idx == blk_num) {
                blocks[i] = pb;
                break;
            }
        }

        if(!blocks[i]) {
            sf1 = 0xFF;
            sf2 = FELICA_SF2_ERROR_BLOCK_NUMBER;
            break;
        }
    }

    // Block Data must be present in full for every block in the Block List
    const uint8_t* raw_end = raw + generic_request->length;
    if(sf1 == FELICA_SF1_SUCCESS &&
       (bptr > raw_end || (size_t)(raw_end - bptr) < block_count * FELICA_DATA_BLOCK_SIZE)) {
        return FelicaErrorProtocol;
    }

    for(uint8_t i = 0; sf1 == FELICA_SF1_SUCCESS && i < block_count; i++) {
        memcpy(blocks[i]->block.data, bptr + i * FELICA_DATA_BLOCK_SIZE, FELICA_DATA_BLOCK_SIZE);
    }

    return felica_listener_send_status_response(
        instance, FELICA_LISTENER_RESPONSE_CODE_WRITE, sf1, sf2);
}

static FelicaError felica_listener_command_handler_request_response(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    UNUSED(generic_request);

    // Response: length(1) + RC(1) + IDm(8) + mode(1) = 11 bytes
    const size_t resp_size = 11;
    uint8_t resp_buf[resp_size];
    resp_buf[0] = (uint8_t)resp_size;
    resp_buf[1] = FELICA_CMD_REQUEST_RESPONSE_RESP;
    const FelicaIDm current_idm = felica_listener_get_current_idm(instance);
    memcpy(resp_buf + 2, current_idm.data, 8);
    resp_buf[10] = instance->mode;

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, resp_size);
    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_command_handler_request_service(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    const uint8_t* raw = (const uint8_t*)generic_request;
    uint8_t n = raw[10];
    FURI_LOG_D(TAG, "Request Service cmd, n=%u", n);

    // Clamp: response is 11 + 2n bytes; tx buffer is FELICA_LISTENER_MAX_BUFFER_SIZE bytes (incl. 2-byte CRC)
    const uint8_t n_max = (FELICA_LISTENER_MAX_BUFFER_SIZE - 2 - 11) / 2;
    if(n > n_max) n = n_max;

    const FelicaSystem* system = felica_listener_get_current_system(instance);

    size_t resp_size = 11 + (size_t)n * 2;
    uint8_t* resp_buf = malloc(resp_size);
    resp_buf[0] = (uint8_t)resp_size;
    resp_buf[1] = FELICA_CMD_REQUEST_SERVICE_RESP;
    const FelicaIDm current_idm = felica_listener_get_current_idm(instance);
    memcpy(resp_buf + 2, current_idm.data, 8);
    resp_buf[10] = n;

    for(uint8_t i = 0; i < n; i++) {
        uint16_t req_code = (uint16_t)(raw[11 + i * 2] | ((uint16_t)raw[12 + i * 2] << 8));
        uint16_t kv = FELICA_KEY_VERSION_UNKNOWN;

        if(req_code == FELICA_SYSTEM_NODE_CODE) {
            kv = system ? system->key_version : FELICA_KEY_VERSION_UNKNOWN;
        } else if(system) {
            uint32_t area_count = simple_array_get_count(system->areas);
            for(uint32_t j = 0; j < area_count; j++) {
                const FelicaArea* area = simple_array_cget(system->areas, j);
                if(area->code == req_code) {
                    kv = area->key_version;
                    break;
                }
            }
            if(kv == FELICA_KEY_VERSION_UNKNOWN) {
                uint32_t svc_count = simple_array_get_count(system->services);
                for(uint32_t j = 0; j < svc_count; j++) {
                    const FelicaService* svc = simple_array_cget(system->services, j);
                    if(svc->code == req_code) {
                        kv = svc->key_version;
                        break;
                    }
                }
            }
        }

        resp_buf[11 + i * 2] = (uint8_t)(kv & 0xFF);
        resp_buf[12 + i * 2] = (uint8_t)(kv >> 8);
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, resp_size);
    free(resp_buf);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_command_handler_search_service_code(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    const uint8_t* raw_req = (const uint8_t*)generic_request;
    uint16_t counter = (uint16_t)(raw_req[10] | ((uint16_t)raw_req[11] << 8));
    FURI_LOG_D(TAG, "Search Service Code cmd, counter=%04X", counter);

    const FelicaSystem* system = felica_listener_get_current_system(instance);

    uint32_t area_count = system ? simple_array_get_count(system->areas) : 0;
    uint32_t service_count = system ? simple_array_get_count(system->services) : 0;

    uint32_t pos = 0;
    uint32_t ai = 0;
    uint32_t si = 0;
    bool found = false;
    bool is_area = false;
    uint16_t code_lo = 0xFFFF;
    uint16_t code_hi = 0;

    while(true) {
        bool pick_area = false;
        if(ai < area_count) {
            const FelicaArea* a = simple_array_cget(system->areas, ai);
            pick_area = (si >= a->first_idx);
        }

        if(pick_area) {
            if(pos == counter) {
                const FelicaArea* a = simple_array_cget(system->areas, ai);
                is_area = true;
                code_lo = a->code;
                code_hi = a->end_code;
                found = true;
                break;
            }
            ai++;
            pos++;
        } else if(si < service_count) {
            if(pos == counter) {
                const FelicaService* svc = simple_array_cget(system->services, si);
                is_area = false;
                code_lo = svc->code;
                found = true;
                break;
            }
            si++;
            pos++;
        } else {
            break;
        }
    }

    uint8_t resp_data_size = (found && is_area) ? 4 : 2;
    size_t resp_size = sizeof(FelicaCommandHeaderRaw) + resp_data_size;

    uint8_t* resp_buf = malloc(resp_size);
    FelicaListServiceCommandResponse* resp = (FelicaListServiceCommandResponse*)resp_buf;

    resp->header.length = (uint8_t)resp_size;
    resp->header.command = FELICA_CMD_LIST_SERVICE_CODE_RESP;
    resp->header.idm = felica_listener_get_current_idm(instance);

    if(found && is_area) {
        resp->data[0] = (uint8_t)(code_lo & 0xFF);
        resp->data[1] = (uint8_t)(code_lo >> 8);
        resp->data[2] = (uint8_t)(code_hi & 0xFF);
        resp->data[3] = (uint8_t)(code_hi >> 8);
    } else {
        // Service code or end-of-list (0xFFFF)
        resp->data[0] = (uint8_t)(code_lo & 0xFF);
        resp->data[1] = (uint8_t)(code_lo >> 8);
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, resp_size);
    free(resp_buf);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_command_handler_request_system_code(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    UNUSED(generic_request);
    FURI_LOG_D(TAG, "Request System Code cmd");

    uint32_t system_count = simple_array_get_count(instance->data->systems);
    size_t resp_size = sizeof(FelicaCommandHeaderRaw) + 1 + system_count * 2;

    uint8_t* resp_buf = malloc(resp_size);
    FelicaListSystemCodeCommandResponse* resp = (FelicaListSystemCodeCommandResponse*)resp_buf;

    resp->header.length = (uint8_t)resp_size;
    resp->header.command = FELICA_CMD_REQUEST_SYSTEM_CODE_RESP;
    resp->header.idm = felica_listener_get_current_idm(instance);
    resp->system_count = (uint8_t)system_count;

    for(uint32_t i = 0; i < system_count; i++) {
        const FelicaSystem* system = simple_array_cget(instance->data->systems, i);
        resp->system_code[i * 2] = (system->system_code >> 8) & 0xFF;
        resp->system_code[i * 2 + 1] = system->system_code & 0xFF;
    }

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_bytes(instance->tx_buffer, resp_buf, resp_size);
    free(resp_buf);

    return felica_listener_frame_exchange(instance, instance->tx_buffer);
}

static FelicaError felica_listener_process_request(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* generic_request) {
    const uint8_t cmd_code = generic_request->header.code;
    switch(cmd_code) {
    case FELICA_CMD_REQUEST_RESPONSE:
        return felica_listener_command_handler_request_response(instance, generic_request);
    case FELICA_CMD_REQUEST_SERVICE:
        return felica_listener_command_handler_request_service(instance, generic_request);
    case FELICA_CMD_READ_WITHOUT_ENCRYPTION:
        if(instance->data->workflow_type == FelicaStandard) {
            return felica_listener_command_handler_standard_read(instance, generic_request);
        }
        return felica_listener_command_handler_read(instance, generic_request);
    case FELICA_CMD_WRITE_WITHOUT_ENCRYPTION:
        if(instance->data->workflow_type == FelicaStandard) {
            return felica_listener_command_handler_standard_write(instance, generic_request);
        }
        return felica_listener_command_handler_write(instance, generic_request);
    case FELICA_CMD_LIST_SERVICE_CODE:
        return felica_listener_command_handler_search_service_code(instance, generic_request);
    case FELICA_CMD_REQUEST_SYSTEM_CODE:
        return felica_listener_command_handler_request_system_code(instance, generic_request);
    default:
        FURI_LOG_E(TAG, "FeliCa incorrect command");
        return FelicaErrorFeatureUnsupported;
    }
}

static void felica_listener_populate_polling_response_header(
    FelicaListener* instance,
    FelicaListenerPollingResponseHeader* resp) {
    resp->idm = felica_listener_get_current_idm(instance);
    resp->pmm = instance->pmm;
    resp->response_code = FELICA_LISTENER_RESPONSE_POLLING;
}

static bool felica_listener_check_system_code(
    const FelicaListenerGenericRequest* const generic_request,
    uint16_t code) {
    return (
        generic_request->polling.system_code == code ||
        generic_request->polling.system_code == (code | 0x00FFU) ||
        generic_request->polling.system_code == (code | 0xFF00U));
}

// Returns the matched System's wire-format code, or FELICA_SYSTEM_CODE_CODE if none
// matched. When the match is a real entry in instance->data->systems (as opposed to
// the virtual NDEF/Lite-S codes), *matched_idx is set to its index so the caller can
// derive that System's IDm; otherwise *matched_idx is left at 0.
static uint16_t felica_listener_get_response_system_code(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request,
    uint8_t* matched_idx) {
    uint16_t resp_system_code = FELICA_SYSTEM_CODE_CODE;
    *matched_idx = 0;
    // Standard cards only answer to Systems from their dump
    const bool is_standard = instance->data->workflow_type == FelicaStandard;
    if(!is_standard &&
       felica_listener_check_system_code(generic_request, FELICA_LISTENER_SYSTEM_CODE_NDEF) &&
       instance->data->data.fs.mc.data[FELICA_MC_SYS_OP] == 1) {
        // NDEF
        resp_system_code = FELICA_LISTENER_SYSTEM_CODE_NDEF;
    } else if(
        !is_standard &&
        felica_listener_check_system_code(generic_request, FELICA_LISTENER_SYSTEM_CODE_LITES)) {
        // Lite-S
        resp_system_code = FELICA_LISTENER_SYSTEM_CODE_LITES;
    } else {
        uint32_t system_count = simple_array_get_count(instance->data->systems);
        for(uint32_t i = 0; i < system_count; i++) {
            const FelicaSystem* system = simple_array_cget(instance->data->systems, i);
            uint16_t wire_code = __builtin_bswap16(system->system_code);
            if(felica_listener_check_system_code(generic_request, wire_code)) {
                resp_system_code = wire_code;
                *matched_idx = (uint8_t)i;
                break;
            }
        }
    }
    return resp_system_code;
}

static FelicaError felica_listener_process_system_code(
    FelicaListener* instance,
    const FelicaListenerGenericRequest* const generic_request) {
    // A card that doesn't have the requested System doesn't respond to the Polling
    FelicaError result = FelicaErrorIgnore;
    do {
        uint8_t matched_idx = 0;
        uint16_t resp_system_code =
            felica_listener_get_response_system_code(instance, generic_request, &matched_idx);
        if(resp_system_code == FELICA_SYSTEM_CODE_CODE) break;

        // Switch context to the newly selected System before building the response,
        // since the response IDm (and all subsequent commands) must reflect it.
        instance->current_system_idx = matched_idx;

        FelicaListenerPollingResponse* resp = malloc(sizeof(FelicaListenerPollingResponse));
        felica_listener_populate_polling_response_header(instance, &resp->header);

        resp->header.length = sizeof(FelicaListenerPollingResponse);
        if(generic_request->polling.request_code == FELICA_LISTENER_REQUEST_SYSTEM_CODE) {
            resp->optional_request_data = resp_system_code;
        } else if(generic_request->polling.request_code == FELICA_LISTENER_REQUEST_PERFORMANCE) {
            resp->optional_request_data = FELICA_LISTENER_PERFORMANCE_VALUE;
        } else {
            resp->header.length = sizeof(FelicaListenerPollingResponseHeader);
        }

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)resp, resp->header.length);
        free(resp);

        result = felica_listener_frame_exchange(instance, instance->tx_buffer);
    } while(false);

    return result;
}

NfcCommand felica_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    FelicaListener* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypeFieldOn) {
        FURI_LOG_D(TAG, "Field On");
    } else if(nfc_event->type == NfcEventTypeListenerActivated) {
        instance->state = Felica_ListenerStateActivated;
        // The NFC controller answered a Polling by itself, with the IDm of System 0
        instance->current_system_idx = 0;
        FURI_LOG_D(TAG, "Activated");
    } else if(nfc_event->type == NfcEventTypeFieldOff) {
        instance->state = Felica_ListenerStateIdle;
        FURI_LOG_D(TAG, "Field Off");
        felica_listener_reset(instance);
    } else if(nfc_event->type == NfcEventTypeRxEnd) {
        FURI_LOG_D(TAG, "Rx Done");
        FelicaError error = FelicaErrorNone;
        do {
            if(!felica_crc_check(nfc_event->data.buffer)) {
                FURI_LOG_E(TAG, "Wrong CRC");
                error = FelicaErrorWrongCrc;
                break;
            }

            FelicaListenerGenericRequest* request =
                (FelicaListenerGenericRequest*)bit_buffer_get_data(nfc_event->data.buffer);

            uint8_t size = bit_buffer_get_size_bytes(nfc_event->data.buffer) - 2;
            if((request->length != size) ||
               (!felica_listener_check_block_list_size(instance, request))) {
                FURI_LOG_E(TAG, "Wrong request length");
                error = FelicaErrorProtocol;
                break;
            }

            if(request->header.code == FELICA_LISTENER_CMD_POLLING) {
                // Will always respond at Time Slot 0 for now.
                nfc_felica_listener_timer_anticol_start(instance->nfc, 0);
                if(request->polling.system_code != FELICA_SYSTEM_CODE_CODE) {
                    error = felica_listener_process_system_code(instance, request);
                    if(error != FelicaErrorNone && error != FelicaErrorIgnore) {
                        FURI_LOG_E(
                            TAG, "Error when handling Polling with System Code: %2X", error);
                    }
                    break;
                } else {
                    FURI_LOG_E(TAG, "Hardware Polling command leaking through");
                    error = FelicaErrorIgnore;
                    break;
                }
            } else if(!felica_listener_select_system_by_idm(instance, &request->header.idm)) {
                FURI_LOG_E(TAG, "Wrong IDm");
                error = FelicaErrorIgnore;
                break;
            }

            error = felica_listener_process_request(instance, request);
            if(error != FelicaErrorNone) {
                FURI_LOG_E(TAG, "Processing error: %2X", error);
            }
        } while(false);

        // No response was sent, re-enable reception to keep listening
        if(error != FelicaErrorNone) {
            command = NfcCommandReset;
        }
        bit_buffer_reset(nfc_event->data.buffer);
    }
    return command;
}

const NfcListenerBase nfc_listener_felica = {
    .alloc = (NfcListenerAlloc)felica_listener_alloc,
    .free = (NfcListenerFree)felica_listener_free,
    .set_callback = (NfcListenerSetCallback)felica_listener_set_callback,
    .get_data = (NfcListenerGetData)felica_listener_get_data,
    .run = (NfcListenerRun)felica_listener_run,
};
