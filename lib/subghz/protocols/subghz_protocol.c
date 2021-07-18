#include "subghz_protocol.h"
#include "subghz_protocol_came.h"
#include "subghz_protocol_cfm.h"
#include "subghz_protocol_keeloq.h"
#include "subghz_protocol_nice_flo.h"
#include "subghz_protocol_nice_flor_s.h"
#include "subghz_protocol_princeton.h"
#include "subghz_protocol_gate_tx.h"
#include "subghz_protocol_ido.h"
#include "subghz_protocol_faac_slh.h"
#include "subghz_protocol_nero_sketch.h"
#include "subghz_protocol_star_line.h"

#include "../subghz_keystore.h"

#include <furi.h>
#include <m-string.h>

struct SubGhzProtocol {
    SubGhzKeystore* keystore;

    SubGhzProtocolCame* came;
    SubGhzProtocolKeeloq* keeloq;
    SubGhzProtocolNiceFlo* nice_flo;
    SubGhzProtocolNiceFlorS* nice_flor_s;
    SubGhzProtocolPrinceton* princeton;
    SubGhzProtocolGateTX* gate_tx;
    SubGhzProtocolIDo* ido;
    SubGhzProtocolFaacSLH* faac_slh;
    SubGhzProtocolNeroSketch* nero_sketch;
    SubGhzProtocolStarLine* star_line;

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

    instance->keystore = subghz_keystore_alloc();

    instance->came = subghz_protocol_came_alloc();
    instance->keeloq = subghz_protocol_keeloq_alloc(instance->keystore);
    instance->princeton = subghz_protocol_princeton_alloc();
    instance->nice_flo = subghz_protocol_nice_flo_alloc();
    instance->nice_flor_s = subghz_protocol_nice_flor_s_alloc();
    instance->gate_tx = subghz_protocol_gate_tx_alloc();
    instance->ido = subghz_protocol_ido_alloc();
    instance->faac_slh = subghz_protocol_faac_slh_alloc();
    instance->nero_sketch = subghz_protocol_nero_sketch_alloc();
    instance->star_line = subghz_protocol_star_line_alloc(instance->keystore);

    return instance;
}

void subghz_protocol_free(SubGhzProtocol* instance) {
    furi_assert(instance);

    subghz_protocol_came_free(instance->came);
    subghz_protocol_keeloq_free(instance->keeloq);
    subghz_protocol_princeton_free(instance->princeton);
    subghz_protocol_nice_flo_free(instance->nice_flo);
    subghz_protocol_nice_flor_s_free(instance->nice_flor_s);
    subghz_protocol_gate_tx_free(instance->gate_tx);
    subghz_protocol_ido_free(instance->ido);
    subghz_protocol_faac_slh_free(instance->faac_slh);
    subghz_protocol_nero_sketch_free(instance->nero_sketch);
    subghz_protocol_star_line_free(instance->star_line);

    subghz_keystore_free(instance->keystore);

    free(instance);
}

void subghz_protocol_enable_dump_text(SubGhzProtocol* instance, SubGhzProtocolTextCallback callback, void* context) {
    furi_assert(instance);

    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->came, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->keeloq, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->princeton, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flo, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nice_flor_s, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->gate_tx, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->ido, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->faac_slh, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nero_sketch, subghz_protocol_text_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->star_line, subghz_protocol_text_rx_callback, instance);

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
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->gate_tx, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->ido, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->faac_slh, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->nero_sketch, subghz_protocol_parser_rx_callback, instance);
    subghz_protocol_common_set_callback((SubGhzProtocolCommon*)instance->star_line, subghz_protocol_parser_rx_callback, instance);
    
    instance->parser_callback = callback;
    instance->parser_callback_context = context;
}


void subghz_protocol_load_nice_flor_s_file(SubGhzProtocol* instance, const char* file_name) {
    subghz_protocol_nice_flor_s_name_file(instance->nice_flor_s, file_name);
}

void subghz_protocol_load_keeloq_file(SubGhzProtocol* instance, const char* file_name) {
    subghz_keystore_load(instance->keystore, file_name);
}

void subghz_protocol_reset(SubGhzProtocol* instance) {
    subghz_protocol_came_reset(instance->came);
    subghz_protocol_keeloq_reset(instance->keeloq);
    subghz_protocol_princeton_reset(instance->princeton);
    subghz_protocol_nice_flo_reset(instance->nice_flo);
    subghz_protocol_nice_flor_s_reset(instance->nice_flor_s);
    subghz_protocol_gate_tx_reset(instance->gate_tx);
    subghz_protocol_ido_reset(instance->ido);
    subghz_protocol_faac_slh_reset(instance->faac_slh);
    subghz_protocol_nero_sketch_reset(instance->nero_sketch);
    subghz_protocol_star_line_reset(instance->star_line);
}

void subghz_protocol_parse(SubGhzProtocol* instance, bool level, uint32_t duration) {
    subghz_protocol_came_parse(instance->came, level, duration);
    subghz_protocol_keeloq_parse(instance->keeloq, level, duration);
    subghz_protocol_princeton_parse(instance->princeton, level, duration);
    subghz_protocol_nice_flo_parse(instance->nice_flo, level, duration);
    subghz_protocol_nice_flor_s_parse(instance->nice_flor_s, level, duration);
    subghz_protocol_gate_tx_parse(instance->gate_tx, level, duration);
    subghz_protocol_ido_parse(instance->ido, level, duration);
    subghz_protocol_faac_slh_parse(instance->faac_slh, level, duration);
    subghz_protocol_nero_sketch_parse(instance->nero_sketch, level, duration);
    subghz_protocol_star_line_parse(instance->star_line, level, duration);
}
