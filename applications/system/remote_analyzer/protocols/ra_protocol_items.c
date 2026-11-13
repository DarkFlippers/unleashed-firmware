#include "ra_protocol_items.h"

#include "crypt.h"
#include "hormann_bisecur.h"
#include "kce6xxx.h"
#include "life_fido.h"
#include "pc4.h"
#include "nero_intro_ii.h"

#include <lib/subghz/subghz_protocol_registry.h>
#include <furi.h>
#include <string.h>

/* subghz_receiver_alloc_init() allocates a decoder for every protocol it is handed whatever
 * the flags say, so these two must be kept out of the table rather than filtered later. */
static bool ra_protocol_registry_skip(const SubGhzProtocol* protocol) {
    return protocol == NULL || protocol->name == NULL || strcmp(protocol->name, "BinRAW") == 0 ||
           strcmp(protocol->name, "RAW") == 0;
}

/** Protocols this app adds on top of the firmware's set. Decoders only. */
static const SubGhzProtocol* const ra_protocol_registry_own[] = {
    &ra_protocol_crypt,
    &ra_protocol_crypt_lite,
    &ra_protocol_hormann_bisecur,
    &ra_protocol_kce6xxx,
    &ra_protocol_life_fido,
    &ra_protocol_nero_intro_ii,
    &ra_protocol_pc4,
};

/* Same layout as SubGhzProtocolRegistry, but writable - the public struct declares both
 * fields const, so there is no way to fill one in at runtime through it. */
typedef struct {
    const SubGhzProtocol* const* items;
    size_t size;
} RaProtocolRegistry;

_Static_assert(
    sizeof(RaProtocolRegistry) == sizeof(SubGhzProtocolRegistry),
    "registry layout drifted from SubGhzProtocolRegistry");

bool ra_protocol_is_own(const SubGhzProtocol* protocol) {
    for(size_t i = 0; i < COUNT_OF(ra_protocol_registry_own); i++) {
        if(ra_protocol_registry_own[i] == protocol) return true;
    }
    return false;
}

const SubGhzProtocolRegistry* ra_protocol_registry_alloc(void) {
    const size_t fw_count = subghz_protocol_registry_count(&subghz_protocol_registry);
    const size_t own_count = COUNT_OF(ra_protocol_registry_own);

    // Header and table in one block: one allocation, and the table cannot outlive the header.
    // Sized for the whole firmware set; the skipped entries just leave the tail unused.
    RaProtocolRegistry* registry = malloc(
        sizeof(RaProtocolRegistry) + (fw_count + own_count) * sizeof(const SubGhzProtocol*));
    const SubGhzProtocol** items = (const SubGhzProtocol**)(registry + 1);

    size_t count = 0;
    for(size_t i = 0; i < fw_count; i++) {
        const SubGhzProtocol* protocol =
            subghz_protocol_registry_get_by_index(&subghz_protocol_registry, i);
        if(ra_protocol_registry_skip(protocol)) continue;
        items[count++] = protocol;
    }
    for(size_t i = 0; i < own_count; i++) {
        items[count++] = ra_protocol_registry_own[i];
    }

    registry->items = items;
    registry->size = count;

    return (const SubGhzProtocolRegistry*)registry;
}

void ra_protocol_registry_free(const SubGhzProtocolRegistry* registry) {
    free((void*)registry);
}
