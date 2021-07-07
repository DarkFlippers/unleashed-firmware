#include "subghz_protocol.h"

#include "subghz_protocol_came.h"
#include "subghz_protocol_cfm.h"
#include "subghz_protocol_keeloq.h"
#include "subghz_protocol_nice_flo.h"
#include "subghz_protocol_nice_flor_s.h"
#include "subghz_protocol_princeton.h"

#include <furi.h>
#include <m-string.h>
#include <filesystem-api.h>

#define FILE_BUFFER_SIZE 64

struct SubGhzProtocol {
    SubGhzProtocolCame* came;
    SubGhzProtocolKeeloq* keeloq;
    SubGhzProtocolNiceFlo* nice_flo;
    SubGhzProtocolNiceFlorS* nice_flor_s;
    SubGhzProtocolPrinceton* princeton;

    SubGhzProtocolTextCallback text_callback;
    void* text_callback_context;
    SubGhzProtocolCommonCallbackDump parser_callback;
    void* parser_callback_context;
};

static void subghz_protocol_text_rx_callback(SubGhzProtocolCommon* parser, void* context) {
    SubGhzProtocol* instance = context;

    string_t output;
    string_init(output);
    subghz_protocol_common_to_str((SubGhzProtocolCommon*)parser, output);
    if (instance->text_callback) {
        instance->text_callback(output, instance->text_callback_context);
    } else {
        printf(string_get_cstr(output));
    }
    string_clear(output);
}

static void subghz_protocol_parser_rx_callback(SubGhzProtocolCommon* parser, void* context) {
    SubGhzProtocol* instance = context;
    if (instance->parser_callback) {
        instance->parser_callback(parser, instance->parser_callback_context);
    } 
}

SubGhzProtocol* subghz_protocol_alloc() {
    SubGhzProtocol* instance = furi_alloc(sizeof(SubGhzProtocol));

    instance->came = subghz_protocol_came_alloc();
    instance->keeloq = subghz_protocol_keeloq_alloc();
    instance->princeton = subghz_protocol_princeton_alloc();
    instance->nice_flo = subghz_protocol_nice_flo_alloc();
    instance->nice_flor_s = subghz_protocol_nice_flor_s_alloc();

    return instance;
}

void subghz_protocol_free(SubGhzProtocol* instance) {
    furi_assert(instance);

    subghz_protocol_came_free(instance->came);
    subghz_protocol_keeloq_free(instance->keeloq);
    subghz_protocol_princeton_free(instance->princeton);
    subghz_protocol_nice_flo_free(instance->nice_flo);
    subghz_protocol_nice_flor_s_free(instance->nice_flor_s);

    free(instance);
}

void subghz_protocol_enable_dump_text(SubGhzProtocol* instance, SubGhzProtocolTextCallback callback, void* context) {
    furi_assert(instance);

    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->came, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->keeloq, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->princeton, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flo, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flor_s, subghz_protocol_text_rx_callback, instance);

    instance->text_callback = callback;
    instance->text_callback_context = context;
}

void subghz_protocol_enable_dump(SubGhzProtocol* instance, SubGhzProtocolCommonCallbackDump callback, void* context) {
    furi_assert(instance);

    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->came, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->keeloq, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->princeton, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flo, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flor_s, subghz_protocol_parser_rx_callback, instance);
    instance->parser_callback = callback;
    instance->parser_callback_context = context;
}

static void subghz_protocol_load_keeloq_file_process_line(SubGhzProtocol* instance, string_t line) {
    uint64_t key = 0;
    uint16_t type = 0;
    char skey[17] = {0};
    char name[65] = {0};
    int ret = sscanf(string_get_cstr(line), "%16s:%hu:%64s", skey, &type, name);
    key = strtoull(skey, NULL, 16);
    if (ret == 3) {
        subghz_protocol_keeloq_add_manafacture_key(instance->keeloq, name, key, type);
    } else {
        printf("Failed to load line: %s\r\n", string_get_cstr(line));
    }
}

void subghz_protocol_load_nice_flor_s_file(SubGhzProtocol* instance, const char* file_name) {
    subghz_protocol_nice_flor_s_name_file(instance->nice_flor_s, file_name);
}

void subghz_protocol_load_keeloq_file(SubGhzProtocol* instance, const char* file_name) {
    File manufacture_keys_file;
    FS_Api* fs_api = furi_record_open("sdcard");
    fs_api->file.open(&manufacture_keys_file, file_name, FSAM_READ, FSOM_OPEN_EXISTING);
    string_t line;
    string_init(line);
    if(manufacture_keys_file.error_id == FSE_OK) {
        printf("Loading manufacture keys file %s\r\n", file_name);
        char buffer[FILE_BUFFER_SIZE];
        uint16_t ret;
        do {
            ret = fs_api->file.read(&manufacture_keys_file, buffer, FILE_BUFFER_SIZE);
            for (uint16_t i=0; i < ret; i++) {
                if (buffer[i] == '\n' && string_size(line) > 0) {
                    subghz_protocol_load_keeloq_file_process_line(instance, line);
                    string_clean(line);
                } else {
                    string_push_back(line, buffer[i]);
                }
            }
        } while(ret > 0);
    } else {
        printf("Manufacture keys file is not found: %s\r\n", file_name);
    }
    string_clear(line);
    fs_api->file.close(&manufacture_keys_file);
    furi_record_close("sdcard");
}

void subghz_protocol_reset(SubGhzProtocol* instance) {
    subghz_protocol_came_reset(instance->came);
    subghz_protocol_keeloq_reset(instance->keeloq);
    subghz_protocol_princeton_reset(instance->princeton);
    subghz_protocol_nice_flo_reset(instance->nice_flo);
    subghz_protocol_nice_flor_s_reset(instance->nice_flor_s);
}

void subghz_protocol_parse(SubGhzProtocol* instance, bool level, uint32_t duration) {
    subghz_protocol_came_parse(instance->came, level, duration);
    subghz_protocol_keeloq_parse(instance->keeloq, level, duration);
    subghz_protocol_princeton_parse(instance->princeton, level, duration);
    subghz_protocol_nice_flo_parse(instance->nice_flo, level, duration);
    subghz_protocol_nice_flor_s_parse(instance->nice_flor_s, level, duration);
}
