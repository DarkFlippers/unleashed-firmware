#include "nfc_supported_cards.h"
#include "../api/nfc_app_api_interface.h"

#include "../plugins/supported_cards/nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <flipper_application/plugins/composite_resolver.h>
#include <loader/firmware_api/firmware_api.h>

#include <furi.h>
#include <path.h>
#include <m-array.h>

#define TAG "NfcSupportedCards"

#define NFC_SUPPORTED_CARDS_PLUGINS_PATH  APP_DATA_PATH("plugins")
#define NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX "_parser.fal"

typedef enum {
    NfcSupportedCardsPluginFeatureHasVerify = (1U << 0),
    NfcSupportedCardsPluginFeatureHasRead = (1U << 1),
    NfcSupportedCardsPluginFeatureHasParse = (1U << 2),
} NfcSupportedCardsPluginFeature;

typedef struct {
    FuriString* name;
    NfcProtocol protocol;
    NfcSupportedCardsPluginFeature feature;
} NfcSupportedCardsPluginCache;

ARRAY_DEF(NfcSupportedCardsPluginCache, NfcSupportedCardsPluginCache, M_POD_OPLIST);

typedef enum {
    NfcSupportedCardsLoadStateIdle,
    NfcSupportedCardsLoadStateInProgress,
    NfcSupportedCardsLoadStateSuccess,
    NfcSupportedCardsLoadStateFail,
} NfcSupportedCardsLoadState;

typedef struct {
    Storage* storage;
    File* directory;
    char file_name[256];
    FlipperApplication* app;
} NfcSupportedCardsLoadContext;

struct NfcSupportedCards {
    CompositeApiResolver* api_resolver;
    NfcSupportedCardsPluginCache_t plugins_cache_arr;
    NfcSupportedCardsLoadState load_state;
    NfcSupportedCardsLoadContext* load_context;
};

NfcSupportedCards* nfc_supported_cards_alloc(void) {
    NfcSupportedCards* instance = malloc(sizeof(NfcSupportedCards));

    instance->api_resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(instance->api_resolver, firmware_api_interface);
    composite_api_resolver_add(instance->api_resolver, nfc_application_api_interface);

    NfcSupportedCardsPluginCache_init(instance->plugins_cache_arr);

    return instance;
}

void nfc_supported_cards_free(NfcSupportedCards* instance) {
    furi_assert(instance);

    NfcSupportedCardsPluginCache_it_t iter;
    for(NfcSupportedCardsPluginCache_it(iter, instance->plugins_cache_arr);
        !NfcSupportedCardsPluginCache_end_p(iter);
        NfcSupportedCardsPluginCache_next(iter)) {
        NfcSupportedCardsPluginCache* plugin_cache = NfcSupportedCardsPluginCache_ref(iter);
        furi_string_free(plugin_cache->name);
    }
    NfcSupportedCardsPluginCache_clear(instance->plugins_cache_arr);

    composite_api_resolver_free(instance->api_resolver);
    free(instance);
}

static NfcSupportedCardsLoadContext* nfc_supported_cards_load_context_alloc(void) {
    NfcSupportedCardsLoadContext* instance = malloc(sizeof(NfcSupportedCardsLoadContext));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->directory = storage_file_alloc(instance->storage);

    if(!storage_dir_open(instance->directory, NFC_SUPPORTED_CARDS_PLUGINS_PATH)) {
        FURI_LOG_D(TAG, "Failed to open directory: %s", NFC_SUPPORTED_CARDS_PLUGINS_PATH);
    }

    return instance;
}

static void nfc_supported_cards_load_context_free(NfcSupportedCardsLoadContext* instance) {
    if(instance->app) {
        flipper_application_free(instance->app);
    }

    storage_dir_close(instance->directory);
    storage_file_free(instance->directory);

    furi_record_close(RECORD_STORAGE);
    free(instance);
}

static const NfcSupportedCardsPlugin* nfc_supported_cards_get_plugin(
    NfcSupportedCardsLoadContext* instance,
    const char* name,
    const ElfApiInterface* api_interface) {
    furi_assert(instance);
    furi_assert(name);

    const NfcSupportedCardsPlugin* plugin = NULL;
    FuriString* plugin_path = furi_string_alloc_printf(
        "%s/%s%s", NFC_SUPPORTED_CARDS_PLUGINS_PATH, name, NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX);
    do {
        if(instance->app) flipper_application_free(instance->app);
        instance->app = flipper_application_alloc(instance->storage, api_interface);

        if(flipper_application_preload(instance->app, furi_string_get_cstr(plugin_path)) !=
           FlipperApplicationPreloadStatusSuccess)
            break;
        if(!flipper_application_is_plugin(instance->app)) break;
        if(flipper_application_map_to_memory(instance->app) != FlipperApplicationLoadStatusSuccess)
            break;
        const FlipperAppPluginDescriptor* descriptor =
            flipper_application_plugin_get_descriptor(instance->app);

        if(descriptor == NULL) break;

        if(strcmp(descriptor->appid, NFC_SUPPORTED_CARD_PLUGIN_APP_ID) != 0) break;
        if(descriptor->ep_api_version != NFC_SUPPORTED_CARD_PLUGIN_API_VERSION) break;

        plugin = descriptor->entry_point;
    } while(false);
    furi_string_free(plugin_path);

    return plugin;
}

static const NfcSupportedCardsPlugin* nfc_supported_cards_get_next_plugin(
    NfcSupportedCardsLoadContext* instance,
    const ElfApiInterface* api_interface) {
    const NfcSupportedCardsPlugin* plugin = NULL;

    do {
        if(!storage_file_is_open(instance->directory)) break;
        if(!storage_dir_read(
               instance->directory, NULL, instance->file_name, sizeof(instance->file_name)))
            break;

        const size_t suffix_len = strlen(NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX);
        const size_t file_name_len = strlen(instance->file_name);
        if(file_name_len <= suffix_len) break;

        size_t suffix_start_pos = file_name_len - suffix_len;
        if(memcmp(
               &instance->file_name[suffix_start_pos],
               NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX,
               suffix_len) != 0) //-V1051
            break;

        // Trim suffix from file_name to save memory. The suffix will be concatenated on plugin load.
        instance->file_name[suffix_start_pos] = '\0';

        plugin = nfc_supported_cards_get_plugin(instance, instance->file_name, api_interface);
    } while(plugin == NULL); //-V654

    return plugin;
}

void nfc_supported_cards_load_cache(NfcSupportedCards* instance) {
    furi_assert(instance);

    do {
        if((instance->load_state == NfcSupportedCardsLoadStateSuccess) ||
           (instance->load_state == NfcSupportedCardsLoadStateFail))
            break;

        instance->load_context = nfc_supported_cards_load_context_alloc();

        while(true) {
            const ElfApiInterface* api_interface =
                composite_api_resolver_get(instance->api_resolver);
            const NfcSupportedCardsPlugin* plugin =
                nfc_supported_cards_get_next_plugin(instance->load_context, api_interface);
            if(plugin == NULL) break; //-V547

            NfcSupportedCardsPluginCache plugin_cache = {}; //-V779
            plugin_cache.name = furi_string_alloc_set(instance->load_context->file_name);
            plugin_cache.protocol = plugin->protocol;
            if(plugin->verify) {
                plugin_cache.feature |= NfcSupportedCardsPluginFeatureHasVerify;
            }
            if(plugin->read) {
                plugin_cache.feature |= NfcSupportedCardsPluginFeatureHasRead;
            }
            if(plugin->parse) {
                plugin_cache.feature |= NfcSupportedCardsPluginFeatureHasParse;
            }
            NfcSupportedCardsPluginCache_push_back(instance->plugins_cache_arr, plugin_cache);
        }

        nfc_supported_cards_load_context_free(instance->load_context);

        size_t plugins_loaded = NfcSupportedCardsPluginCache_size(instance->plugins_cache_arr);
        if(plugins_loaded == 0) {
            FURI_LOG_D(TAG, "Plugins not found");
            instance->load_state = NfcSupportedCardsLoadStateFail;
        } else {
            FURI_LOG_D(TAG, "Loaded %zu plugins", plugins_loaded);
            instance->load_state = NfcSupportedCardsLoadStateSuccess;
        }

    } while(false);
}

bool nfc_supported_cards_read(NfcSupportedCards* instance, NfcDevice* device, Nfc* nfc) {
    furi_assert(instance);
    furi_assert(device);
    furi_assert(nfc);

    bool card_read = false;
    NfcProtocol protocol = nfc_device_get_protocol(device);

    do {
        if(instance->load_state != NfcSupportedCardsLoadStateSuccess) break;

        instance->load_context = nfc_supported_cards_load_context_alloc();

        NfcSupportedCardsPluginCache_it_t iter;
        for(NfcSupportedCardsPluginCache_it(iter, instance->plugins_cache_arr);
            !NfcSupportedCardsPluginCache_end_p(iter);
            NfcSupportedCardsPluginCache_next(iter)) {
            NfcSupportedCardsPluginCache* plugin_cache = NfcSupportedCardsPluginCache_ref(iter);
            if(plugin_cache->protocol != protocol) continue;
            if((plugin_cache->feature & NfcSupportedCardsPluginFeatureHasRead) == 0) continue;

            const ElfApiInterface* api_interface =
                composite_api_resolver_get(instance->api_resolver);
            const NfcSupportedCardsPlugin* plugin = nfc_supported_cards_get_plugin(
                instance->load_context, furi_string_get_cstr(plugin_cache->name), api_interface);
            if(plugin == NULL) continue;

            if(plugin->verify) {
                if(!plugin->verify(nfc)) continue;
            }

            if(plugin->read) {
                if(plugin->read(nfc, device)) {
                    card_read = true;
                    break;
                }
            }
        }

        nfc_supported_cards_load_context_free(instance->load_context);
    } while(false);

    return card_read;
}

bool nfc_supported_cards_parse(
    NfcSupportedCards* instance,
    NfcDevice* device,
    FuriString* parsed_data) {
    furi_assert(instance);
    furi_assert(device);
    furi_assert(parsed_data);

    bool card_parsed = false;
    NfcProtocol protocol = nfc_device_get_protocol(device);

    do {
        if(instance->load_state != NfcSupportedCardsLoadStateSuccess) break;

        instance->load_context = nfc_supported_cards_load_context_alloc();

        NfcSupportedCardsPluginCache_it_t iter;
        for(NfcSupportedCardsPluginCache_it(iter, instance->plugins_cache_arr);
            !NfcSupportedCardsPluginCache_end_p(iter);
            NfcSupportedCardsPluginCache_next(iter)) {
            NfcSupportedCardsPluginCache* plugin_cache = NfcSupportedCardsPluginCache_ref(iter);
            if(plugin_cache->protocol != protocol) continue;
            if((plugin_cache->feature & NfcSupportedCardsPluginFeatureHasParse) == 0) continue;

            const ElfApiInterface* api_interface =
                composite_api_resolver_get(instance->api_resolver);
            const NfcSupportedCardsPlugin* plugin = nfc_supported_cards_get_plugin(
                instance->load_context, furi_string_get_cstr(plugin_cache->name), api_interface);
            if(plugin == NULL) continue;

            if(plugin->parse) {
                if(plugin->parse(device, parsed_data)) {
                    card_parsed = true;
                    break;
                }
            }
        }

        nfc_supported_cards_load_context_free(instance->load_context);
    } while(false);

    return card_parsed;
}
