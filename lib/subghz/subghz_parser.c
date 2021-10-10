
#include "subghz_parser.h"
#include "protocols/subghz_protocol_came.h"
#include "protocols/subghz_protocol_came_twee.h"
#include "protocols/subghz_protocol_came_atomo.h"
#include "protocols/subghz_protocol_cfm.h"
#include "protocols/subghz_protocol_keeloq.h"
#include "protocols/subghz_protocol_nice_flo.h"
#include "protocols/subghz_protocol_nice_flor_s.h"
#include "protocols/subghz_protocol_princeton.h"
#include "protocols/subghz_protocol_gate_tx.h"
#include "protocols/subghz_protocol_ido.h"
#include "protocols/subghz_protocol_faac_slh.h"
#include "protocols/subghz_protocol_nero_sketch.h"
#include "protocols/subghz_protocol_star_line.h"
#include "protocols/subghz_protocol_nero_radio.h"
#include "protocols/subghz_protocol_scher_khan.h"
#include "protocols/subghz_protocol_kia.h"

#include "subghz_keystore.h"

#include <furi.h>
#include <m-string.h>

typedef enum {
    SubGhzProtocolTypeCame,
    SubGhzProtocolTypeCameTwee,
    SubGhzProtocolTypeCameAtomo,
    SubGhzProtocolTypeKeeloq,
    SubGhzProtocolTypeNiceFlo,
    SubGhzProtocolTypeNiceFlorS,
    SubGhzProtocolTypePrinceton,
    SubGhzProtocolTypeGateTX,
    SubGhzProtocolTypeIDo,
    SubGhzProtocolTypeFaacSLH,
    SubGhzProtocolTypeNeroSketch,
    SubGhzProtocolTypeStarLine,
    SubGhzProtocolTypeNeroRadio,
    SubGhzProtocolTypeScherKhan,
    SubGhzProtocolTypeKIA,

    SubGhzProtocolTypeMax,
} SubGhzProtocolType;

struct SubGhzParser {
    SubGhzKeystore* keystore;

    SubGhzProtocolCommon* protocols[SubGhzProtocolTypeMax];

    SubGhzProtocolTextCallback text_callback;
    void* text_callback_context;
    SubGhzProtocolCommonCallbackDump parser_callback;
    void* parser_callback_context;
};

static void subghz_parser_text_rx_callback(SubGhzProtocolCommon* parser, void* context) {
    SubGhzParser* instance = context;

    string_t output;
    string_init(output);
    subghz_protocol_common_to_str((SubGhzProtocolCommon*)parser, output);
    if(instance->text_callback) {
        instance->text_callback(output, instance->text_callback_context);
    } else {
        printf("%s", string_get_cstr(output));
    }
    string_clear(output);
}

static void subghz_parser_parser_rx_callback(SubGhzProtocolCommon* parser, void* context) {
    SubGhzParser* instance = context;
    if(instance->parser_callback) {
        instance->parser_callback(parser, instance->parser_callback_context);
    }
}

SubGhzParser* subghz_parser_alloc() {
    SubGhzParser* instance = furi_alloc(sizeof(SubGhzParser));

    instance->keystore = subghz_keystore_alloc();

    instance->protocols[SubGhzProtocolTypeCame] =
        (SubGhzProtocolCommon*)subghz_protocol_came_alloc();
    instance->protocols[SubGhzProtocolTypeCameTwee] =
        (SubGhzProtocolCommon*)subghz_protocol_came_twee_alloc();
    instance->protocols[SubGhzProtocolTypeCameAtomo] =
        (SubGhzProtocolCommon*)subghz_protocol_came_atomo_alloc();
    instance->protocols[SubGhzProtocolTypeKeeloq] =
        (SubGhzProtocolCommon*)subghz_protocol_keeloq_alloc(instance->keystore);
    instance->protocols[SubGhzProtocolTypePrinceton] =
        (SubGhzProtocolCommon*)subghz_decoder_princeton_alloc();
    instance->protocols[SubGhzProtocolTypeNiceFlo] =
        (SubGhzProtocolCommon*)subghz_protocol_nice_flo_alloc();
    instance->protocols[SubGhzProtocolTypeNiceFlorS] =
        (SubGhzProtocolCommon*)subghz_protocol_nice_flor_s_alloc();
    instance->protocols[SubGhzProtocolTypeGateTX] =
        (SubGhzProtocolCommon*)subghz_protocol_gate_tx_alloc();
    instance->protocols[SubGhzProtocolTypeIDo] =
        (SubGhzProtocolCommon*)subghz_protocol_ido_alloc();
    instance->protocols[SubGhzProtocolTypeFaacSLH] =
        (SubGhzProtocolCommon*)subghz_protocol_faac_slh_alloc();
    instance->protocols[SubGhzProtocolTypeNeroSketch] =
        (SubGhzProtocolCommon*)subghz_protocol_nero_sketch_alloc();
    instance->protocols[SubGhzProtocolTypeStarLine] =
        (SubGhzProtocolCommon*)subghz_protocol_star_line_alloc(instance->keystore);
    instance->protocols[SubGhzProtocolTypeNeroRadio] =
        (SubGhzProtocolCommon*)subghz_protocol_nero_radio_alloc();
    instance->protocols[SubGhzProtocolTypeScherKhan] =
        (SubGhzProtocolCommon*)subghz_protocol_scher_khan_alloc();
    instance->protocols[SubGhzProtocolTypeKIA] =
        (SubGhzProtocolCommon*)subghz_protocol_kia_alloc();

    return instance;
}

void subghz_parser_free(SubGhzParser* instance) {
    furi_assert(instance);

    subghz_protocol_came_free((SubGhzProtocolCame*)instance->protocols[SubGhzProtocolTypeCame]);
    subghz_protocol_came_twee_free(
        (SubGhzProtocolCameTwee*)instance->protocols[SubGhzProtocolTypeCameTwee]);
    subghz_protocol_came_atomo_free(
        (SubGhzProtocolCameAtomo*)instance->protocols[SubGhzProtocolTypeCameAtomo]);
    subghz_protocol_keeloq_free(
        (SubGhzProtocolKeeloq*)instance->protocols[SubGhzProtocolTypeKeeloq]);
    subghz_decoder_princeton_free(
        (SubGhzDecoderPrinceton*)instance->protocols[SubGhzProtocolTypePrinceton]);
    subghz_protocol_nice_flo_free(
        (SubGhzProtocolNiceFlo*)instance->protocols[SubGhzProtocolTypeNiceFlo]);
    subghz_protocol_nice_flor_s_free(
        (SubGhzProtocolNiceFlorS*)instance->protocols[SubGhzProtocolTypeNiceFlorS]);
    subghz_protocol_gate_tx_free(
        (SubGhzProtocolGateTX*)instance->protocols[SubGhzProtocolTypeGateTX]);
    subghz_protocol_ido_free((SubGhzProtocolIDo*)instance->protocols[SubGhzProtocolTypeIDo]);
    subghz_protocol_faac_slh_free(
        (SubGhzProtocolFaacSLH*)instance->protocols[SubGhzProtocolTypeFaacSLH]);
    subghz_protocol_nero_sketch_free(
        (SubGhzProtocolNeroSketch*)instance->protocols[SubGhzProtocolTypeNeroSketch]);
    subghz_protocol_star_line_free(
        (SubGhzProtocolStarLine*)instance->protocols[SubGhzProtocolTypeStarLine]);
    subghz_protocol_nero_radio_free(
        (SubGhzProtocolNeroRadio*)instance->protocols[SubGhzProtocolTypeNeroRadio]);
    subghz_protocol_scher_khan_free(
        (SubGhzProtocolScherKhan*)instance->protocols[SubGhzProtocolTypeScherKhan]);
    subghz_protocol_kia_free((SubGhzProtocolKIA*)instance->protocols[SubGhzProtocolTypeKIA]);

    subghz_keystore_free(instance->keystore);

    free(instance);
}

SubGhzProtocolCommon* subghz_parser_get_by_name(SubGhzParser* instance, const char* name) {
    SubGhzProtocolCommon* result = NULL;

    for(size_t i = 0; i < SubGhzProtocolTypeMax; i++) {
        if(strcmp(instance->protocols[i]->name, name) == 0) {
            result = instance->protocols[i];
            break;
        }
    }

    return result;
}

void subghz_parser_enable_dump_text(
    SubGhzParser* instance,
    SubGhzProtocolTextCallback callback,
    void* context) {
    furi_assert(instance);

    for(size_t i = 0; i < SubGhzProtocolTypeMax; i++) {
        subghz_protocol_common_set_callback(
            instance->protocols[i], subghz_parser_text_rx_callback, instance);
    }

    instance->text_callback = callback;
    instance->text_callback_context = context;
}

void subghz_parser_enable_dump(
    SubGhzParser* instance,
    SubGhzProtocolCommonCallbackDump callback,
    void* context) {
    furi_assert(instance);

    for(size_t i = 0; i < SubGhzProtocolTypeMax; i++) {
        subghz_protocol_common_set_callback(
            instance->protocols[i], subghz_parser_parser_rx_callback, instance);
    }

    instance->parser_callback = callback;
    instance->parser_callback_context = context;
}

void subghz_parser_load_nice_flor_s_file(SubGhzParser* instance, const char* file_name) {
    subghz_protocol_nice_flor_s_name_file(
        (SubGhzProtocolNiceFlorS*)instance->protocols[SubGhzProtocolTypeNiceFlorS], file_name);
}

void subghz_parser_load_keeloq_file(SubGhzParser* instance, const char* file_name) {
    subghz_keystore_load(instance->keystore, file_name);
}

void subghz_parser_reset(SubGhzParser* instance) {
    subghz_protocol_came_reset((SubGhzProtocolCame*)instance->protocols[SubGhzProtocolTypeCame]);
    subghz_protocol_came_twee_reset(
        (SubGhzProtocolCameTwee*)instance->protocols[SubGhzProtocolTypeCameTwee]);
    subghz_protocol_came_atomo_reset(
        (SubGhzProtocolCameAtomo*)instance->protocols[SubGhzProtocolTypeCameAtomo]);
    subghz_protocol_keeloq_reset(
        (SubGhzProtocolKeeloq*)instance->protocols[SubGhzProtocolTypeKeeloq]);
    subghz_decoder_princeton_reset(
        (SubGhzDecoderPrinceton*)instance->protocols[SubGhzProtocolTypePrinceton]);
    subghz_protocol_nice_flo_reset(
        (SubGhzProtocolNiceFlo*)instance->protocols[SubGhzProtocolTypeNiceFlo]);
    subghz_protocol_nice_flor_s_reset(
        (SubGhzProtocolNiceFlorS*)instance->protocols[SubGhzProtocolTypeNiceFlorS]);
    subghz_protocol_gate_tx_reset(
        (SubGhzProtocolGateTX*)instance->protocols[SubGhzProtocolTypeGateTX]);
    subghz_protocol_ido_reset((SubGhzProtocolIDo*)instance->protocols[SubGhzProtocolTypeIDo]);
    subghz_protocol_faac_slh_reset(
        (SubGhzProtocolFaacSLH*)instance->protocols[SubGhzProtocolTypeFaacSLH]);
    subghz_protocol_nero_sketch_reset(
        (SubGhzProtocolNeroSketch*)instance->protocols[SubGhzProtocolTypeNeroSketch]);
    subghz_protocol_star_line_reset(
        (SubGhzProtocolStarLine*)instance->protocols[SubGhzProtocolTypeStarLine]);
    subghz_protocol_nero_radio_reset(
        (SubGhzProtocolNeroRadio*)instance->protocols[SubGhzProtocolTypeNeroRadio]);
    subghz_protocol_scher_khan_reset(
        (SubGhzProtocolScherKhan*)instance->protocols[SubGhzProtocolTypeScherKhan]);
    subghz_protocol_kia_reset((SubGhzProtocolKIA*)instance->protocols[SubGhzProtocolTypeKIA]);
}

void subghz_parser_parse(SubGhzParser* instance, bool level, uint32_t duration) {
    subghz_protocol_came_parse(
        (SubGhzProtocolCame*)instance->protocols[SubGhzProtocolTypeCame], level, duration);
    subghz_protocol_came_twee_parse(
        (SubGhzProtocolCameTwee*)instance->protocols[SubGhzProtocolTypeCameTwee], level, duration);
    subghz_protocol_came_atomo_parse(
        (SubGhzProtocolCameAtomo*)instance->protocols[SubGhzProtocolTypeCameAtomo],
        level,
        duration);
    subghz_protocol_keeloq_parse(
        (SubGhzProtocolKeeloq*)instance->protocols[SubGhzProtocolTypeKeeloq], level, duration);
    subghz_decoder_princeton_parse(
        (SubGhzDecoderPrinceton*)instance->protocols[SubGhzProtocolTypePrinceton],
        level,
        duration);
    subghz_protocol_nice_flo_parse(
        (SubGhzProtocolNiceFlo*)instance->protocols[SubGhzProtocolTypeNiceFlo], level, duration);
    subghz_protocol_nice_flor_s_parse(
        (SubGhzProtocolNiceFlorS*)instance->protocols[SubGhzProtocolTypeNiceFlorS],
        level,
        duration);
    subghz_protocol_gate_tx_parse(
        (SubGhzProtocolGateTX*)instance->protocols[SubGhzProtocolTypeGateTX], level, duration);
    subghz_protocol_ido_parse(
        (SubGhzProtocolIDo*)instance->protocols[SubGhzProtocolTypeIDo], level, duration);
    subghz_protocol_faac_slh_parse(
        (SubGhzProtocolFaacSLH*)instance->protocols[SubGhzProtocolTypeFaacSLH], level, duration);
    subghz_protocol_nero_sketch_parse(
        (SubGhzProtocolNeroSketch*)instance->protocols[SubGhzProtocolTypeNeroSketch],
        level,
        duration);
    subghz_protocol_star_line_parse(
        (SubGhzProtocolStarLine*)instance->protocols[SubGhzProtocolTypeStarLine], level, duration);
    subghz_protocol_nero_radio_parse(
        (SubGhzProtocolNeroRadio*)instance->protocols[SubGhzProtocolTypeNeroRadio],
        level,
        duration);
    subghz_protocol_scher_khan_parse(
        (SubGhzProtocolScherKhan*)instance->protocols[SubGhzProtocolTypeScherKhan],
        level,
        duration);
    subghz_protocol_kia_parse(
        (SubGhzProtocolKIA*)instance->protocols[SubGhzProtocolTypeKIA], level, duration);
}
