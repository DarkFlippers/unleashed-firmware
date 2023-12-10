#include "nfc_supported_cards.h"
#include "../plugins/supported_cards/nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#include <furi.h>
#include <path.h>

#define TAG "NfcSupportedCards"

#define NFC_SUPPORTED_CARDS_PLUGINS_PATH APP_DATA_PATH("plugins")
#define NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX "_parser.fal"

typedef struct {
    Storage* storage;
    File* directory;
    FuriString* file_path;
    char file_name[256];
    FlipperApplication* app;
} NfcSupportedCards;

static NfcSupportedCards* nfc_supported_cards_alloc() {
    NfcSupportedCards* instance = malloc(sizeof(NfcSupportedCards));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->directory = storage_file_alloc(instance->storage);
    instance->file_path = furi_string_alloc();

    if(!storage_dir_open(instance->directory, NFC_SUPPORTED_CARDS_PLUGINS_PATH)) {
        FURI_LOG_D(TAG, "Failed to open directory: %s", NFC_SUPPORTED_CARDS_PLUGINS_PATH);
    }

    return instance;
}

static void nfc_supported_cards_free(NfcSupportedCards* instance) {
    if(instance->app) {
        flipper_application_free(instance->app);
    }

    furi_string_free(instance->file_path);

    storage_dir_close(instance->directory);
    storage_file_free(instance->directory);

    furi_record_close(RECORD_STORAGE);
    free(instance);
}

static const NfcSupportedCardsPlugin*
    nfc_supported_cards_get_next_plugin(NfcSupportedCards* instance) {
    const NfcSupportedCardsPlugin* plugin = NULL;

    do {
        if(!storage_file_is_open(instance->directory)) break;
        if(!storage_dir_read(
               instance->directory, NULL, instance->file_name, sizeof(instance->file_name)))
            break;

        furi_string_set(instance->file_path, instance->file_name);
        if(!furi_string_end_with_str(instance->file_path, NFC_SUPPORTED_CARDS_PLUGIN_SUFFIX))
            continue;

        path_concat(NFC_SUPPORTED_CARDS_PLUGINS_PATH, instance->file_name, instance->file_path);

        if(instance->app) flipper_application_free(instance->app);
        instance->app = flipper_application_alloc(instance->storage, firmware_api_interface);

        if(flipper_application_preload(instance->app, furi_string_get_cstr(instance->file_path)) !=
           FlipperApplicationPreloadStatusSuccess)
            continue;
        if(!flipper_application_is_plugin(instance->app)) continue;

        if(flipper_application_map_to_memory(instance->app) != FlipperApplicationLoadStatusSuccess)
            continue;

        const FlipperAppPluginDescriptor* descriptor =
            flipper_application_plugin_get_descriptor(instance->app);

        if(descriptor == NULL) continue;

        if(strcmp(descriptor->appid, NFC_SUPPORTED_CARD_PLUGIN_APP_ID) != 0) continue;
        if(descriptor->ep_api_version != NFC_SUPPORTED_CARD_PLUGIN_API_VERSION) continue;

        plugin = descriptor->entry_point;
    } while(plugin == NULL); //-V654

    return plugin;
}

bool nfc_supported_cards_read(NfcDevice* device, Nfc* nfc) {
    furi_assert(device);
    furi_assert(nfc);

    bool card_read = false;

    NfcSupportedCards* supported_cards = nfc_supported_cards_alloc();

    do {
        const NfcSupportedCardsPlugin* plugin =
            nfc_supported_cards_get_next_plugin(supported_cards);
        if(plugin == NULL) break; //-V547

        const NfcProtocol protocol = nfc_device_get_protocol(device); //-V779
        if(plugin->protocol != protocol) continue;

        if(plugin->verify) {
            if(!plugin->verify(nfc)) continue;
        }

        if(plugin->read) {
            card_read = plugin->read(nfc, device);
        }

    } while(!card_read);

    nfc_supported_cards_free(supported_cards);
    return card_read;
}

bool nfc_supported_cards_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = false;

    NfcSupportedCards* supported_cards = nfc_supported_cards_alloc();

    do {
        const NfcSupportedCardsPlugin* plugin =
            nfc_supported_cards_get_next_plugin(supported_cards);
        if(plugin == NULL) break; //-V547

        const NfcProtocol protocol = nfc_device_get_protocol(device); //-V779
        if(plugin->protocol != protocol) continue;

        if(plugin->parse) {
            parsed = plugin->parse(device, parsed_data);
        }

    } while(!parsed);

    nfc_supported_cards_free(supported_cards);
    return parsed;
}
