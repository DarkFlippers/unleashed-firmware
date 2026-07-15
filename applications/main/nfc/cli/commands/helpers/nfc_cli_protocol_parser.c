#include "nfc_cli_protocol_parser.h"

#include <m-bptree.h>

#define PROTOCOLS_TREE_RANK 4

BPTREE_DEF2(
    ProtocolTree,
    PROTOCOLS_TREE_RANK,
    FuriString*,
    FURI_STRING_OPLIST,
    NfcProtocol,
    M_POD_OPLIST);

#define M_OPL_ProtocolTree_t() BPTREE_OPLIST(ProtocolTree, M_POD_OPLIST)

struct NfcCliProtocolParser {
    ProtocolTree_t protocols;
};

NfcCliProtocolParser* nfc_cli_protocol_parser_alloc(
    const NfcProtocolNameValuePair* valid_protocols,
    const size_t valid_count) {
    furi_assert(valid_protocols);
    furi_assert(valid_count > 0);

    NfcCliProtocolParser* instance = malloc(sizeof(NfcCliProtocolParser));

    FuriString* name = furi_string_alloc();
    ProtocolTree_init(instance->protocols);
    for(size_t i = 0; i < valid_count; i++) {
        const NfcProtocolNameValuePair* item = &valid_protocols[i];
        furi_string_set_str(name, item->name);
        ProtocolTree_set_at(instance->protocols, name, item->value);
    }

    furi_string_free(name);
    return instance;
}

void nfc_cli_protocol_parser_free(NfcCliProtocolParser* instance) {
    furi_assert(instance);
    ProtocolTree_clear(instance->protocols);
    free(instance);
}

bool nfc_cli_protocol_parser_get(
    NfcCliProtocolParser* instance,
    FuriString* key,
    NfcProtocol* result) {
    furi_assert(instance);
    furi_assert(key);

    NfcProtocol* protocol = ProtocolTree_get(instance->protocols, key);
    if(protocol) *result = *protocol;

    return protocol != NULL;
}
