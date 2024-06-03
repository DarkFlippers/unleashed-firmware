#include "composite_resolver.h"

#include <furi.h>
#include <m-list.h>
#include <m-algo.h>

LIST_DEF(ElfApiInterfaceList, const ElfApiInterface*, M_POD_OPLIST) // NOLINT
#define M_OPL_ElfApiInterfaceList_t() LIST_OPLIST(ElfApiInterfaceList, M_POD_OPLIST)

struct CompositeApiResolver {
    ElfApiInterface api_interface;
    ElfApiInterfaceList_t interfaces;
};

static bool composite_api_resolver_callback(
    const ElfApiInterface* interface,
    uint32_t hash,
    Elf32_Addr* address) {
    CompositeApiResolver* resolver = (CompositeApiResolver*)interface;
    for
        M_EACH(interface, resolver->interfaces, ElfApiInterfaceList_t) {
            if((*interface)->resolver_callback(*interface, hash, address)) {
                return true;
            }
        }
    return false;
}

CompositeApiResolver* composite_api_resolver_alloc(void) {
    CompositeApiResolver* resolver = malloc(sizeof(CompositeApiResolver));

    resolver->api_interface.api_version_major = 0;
    resolver->api_interface.api_version_minor = 0;
    resolver->api_interface.resolver_callback = &composite_api_resolver_callback;
    ElfApiInterfaceList_init(resolver->interfaces);

    return resolver;
}

void composite_api_resolver_free(CompositeApiResolver* resolver) {
    furi_check(resolver);

    ElfApiInterfaceList_clear(resolver->interfaces);
    free(resolver);
}

void composite_api_resolver_add(CompositeApiResolver* resolver, const ElfApiInterface* interface) {
    furi_check(resolver);
    furi_check(interface);

    if(ElfApiInterfaceList_empty_p(resolver->interfaces)) {
        resolver->api_interface.api_version_major = interface->api_version_major;
        resolver->api_interface.api_version_minor = interface->api_version_minor;
    }
    ElfApiInterfaceList_push_back(resolver->interfaces, interface);
}

const ElfApiInterface* composite_api_resolver_get(CompositeApiResolver* resolver) {
    furi_check(resolver);
    return &resolver->api_interface;
}
