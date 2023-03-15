#include "wifi_marauder_app_i.h"
#include "wifi_marauder_pcap.h"

void wifi_marauder_get_prefix_from_cmd(char* dest, const char* command) {
    int start, end, delta;
    start = strlen("sniff");
    end = strcspn(command, " ");
    delta = end - start;
    strncpy(dest, command + start, end - start);
    dest[delta] = '\0';
}

void wifi_marauder_create_pcap_file(WifiMarauderApp* app) {
    char prefix[10];
    char capture_file_path[100];
    wifi_marauder_get_prefix_from_cmd(prefix, app->selected_tx_string);

    int i = 0;
    do {
        snprintf(
            capture_file_path,
            sizeof(capture_file_path),
            "%s/%s_%d.pcap",
            MARAUDER_APP_FOLDER,
            prefix,
            i);
        i++;
    } while(storage_file_exists(app->storage, capture_file_path));

    if(!storage_file_open(app->capture_file, capture_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot open pcap file");
    }
}