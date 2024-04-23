/**
 * @file nfc_protocol.c
 * @brief Main protocol hierarchy definitions.
 *
 * To reduce code duplication, all NFC protocols are described as a tree, whose
 * structure is shown in the diagram below. The (Start) node is actually
 * nonexistent and is there only for clarity.
 *
 * All its child protocols are considered base protocols, which in turn serve
 * as parents to other, usually vendor-specific ones.
 *
 * ```
 * **************************** Protocol tree structure ***************************
 *
 *                                                  (Start)
 *                                                     |
 *                            +------------------------+-----------+---------+------------+
 *                            |                        |           |         |            |
 *                       ISO14443-3A              ISO14443-3B    Felica  ISO15693-3    ST25TB
 *                            |                        |                     |
 *            +---------------+-------------+     ISO14443-4B              SLIX
 *            |               |             |
 *       ISO14443-4A   Mf Ultralight   Mf Classic
 *            |
 *       Mf Desfire
 * ```
 *
 * When implementing a new protocol, its place in the tree must be determined first.
 * If no appropriate base protocols exists, then it must be a base protocol itself.
 *
 * This file is to be modified upon adding a new protocol (see below).
 *
 */
#include "nfc_protocol.h"

#include <furi/furi.h>

/**
 * @brief Tree node describing a protocol.
 *
 * All base protocols (see above) have NfcProtocolInvalid
 * in the parent_protocol field.
 */
typedef struct {
    NfcProtocol parent_protocol; /**< Parent protocol identifier. */
    size_t children_num; /** < Number of the child protocols. */
    const NfcProtocol* children_protocol; /**< Pointer to an array of child protocol identifiers. */
} NfcProtocolTreeNode;

/** List of ISO14443-3A child protocols. */
static const NfcProtocol nfc_protocol_iso14443_3a_children_protocol[] = {
    NfcProtocolIso14443_4a,
    NfcProtocolMfUltralight,
};

/** List of ISO14443-3B child protocols. */
static const NfcProtocol nfc_protocol_iso14443_3b_children_protocol[] = {
    NfcProtocolIso14443_4b,
};

/** List of ISO14443-4A child protocols. */
static const NfcProtocol nfc_protocol_iso14443_4a_children_protocol[] = {
    NfcProtocolMfDesfire,
    NfcProtocolMfPlus,
};

/** List of ISO115693-3 child protocols. */
static const NfcProtocol nfc_protocol_iso15693_3_children_protocol[] = {
    NfcProtocolSlix,
};

/* Add new child protocol lists here (if necessary) */

/**
 * @brief Flattened description of the NFC protocol tree.
 *
 * When implementing a new protocol, add the node here under its
 * own index defined in nfc_protocol.h.
 *
 * Additionally, if it has an already implemented protocol as a parent,
 * add its identifier to its respective list of child protocols (see above).
 */
static const NfcProtocolTreeNode nfc_protocol_nodes[NfcProtocolNum] = {
    [NfcProtocolIso14443_3a] =
        {
            .parent_protocol = NfcProtocolInvalid,
            .children_num = COUNT_OF(nfc_protocol_iso14443_3a_children_protocol),
            .children_protocol = nfc_protocol_iso14443_3a_children_protocol,
        },
    [NfcProtocolIso14443_3b] =
        {
            .parent_protocol = NfcProtocolInvalid,
            .children_num = COUNT_OF(nfc_protocol_iso14443_3b_children_protocol),
            .children_protocol = nfc_protocol_iso14443_3b_children_protocol,
        },
    [NfcProtocolIso14443_4a] =
        {
            .parent_protocol = NfcProtocolIso14443_3a,
            .children_num = COUNT_OF(nfc_protocol_iso14443_4a_children_protocol),
            .children_protocol = nfc_protocol_iso14443_4a_children_protocol,
        },
    [NfcProtocolIso14443_4b] =
        {
            .parent_protocol = NfcProtocolIso14443_3b,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolIso15693_3] =
        {
            .parent_protocol = NfcProtocolInvalid,
            .children_num = COUNT_OF(nfc_protocol_iso15693_3_children_protocol),
            .children_protocol = nfc_protocol_iso15693_3_children_protocol,
        },
    [NfcProtocolFelica] =
        {
            .parent_protocol = NfcProtocolInvalid,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolMfUltralight] =
        {
            .parent_protocol = NfcProtocolIso14443_3a,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolMfClassic] =
        {
            .parent_protocol = NfcProtocolIso14443_3a,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolMfPlus] =
        {
            .parent_protocol = NfcProtocolIso14443_4a,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolMfDesfire] =
        {
            .parent_protocol = NfcProtocolIso14443_4a,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolSlix] =
        {
            .parent_protocol = NfcProtocolIso15693_3,
            .children_num = 0,
            .children_protocol = NULL,
        },
    [NfcProtocolSt25tb] =
        {
            .parent_protocol = NfcProtocolInvalid,
            .children_num = 0,
            .children_protocol = NULL,
        },
    /* Add new protocols here */
};

NfcProtocol nfc_protocol_get_parent(NfcProtocol protocol) {
    furi_check(protocol < NfcProtocolNum);

    return nfc_protocol_nodes[protocol].parent_protocol;
}

bool nfc_protocol_has_parent(NfcProtocol protocol, NfcProtocol parent_protocol) {
    furi_check(protocol < NfcProtocolNum);
    furi_check(parent_protocol < NfcProtocolNum);

    bool parent_found = false;
    const NfcProtocolTreeNode* iter = &nfc_protocol_nodes[protocol];

    while(iter->parent_protocol != NfcProtocolInvalid) {
        if(iter->parent_protocol == parent_protocol) {
            parent_found = true;
            break;
        }
        iter = &nfc_protocol_nodes[iter->parent_protocol];
    }

    return parent_found;
}
