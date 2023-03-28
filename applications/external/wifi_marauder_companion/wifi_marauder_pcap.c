#include "wifi_marauder_app_i.h"
#include "wifi_marauder_pcap.h"

void wifi_marauder_get_prefix_from_sniff_cmd(char* dest, const char* command) {
    int start, end, delta;
    start = strlen("sniff");
    end = strcspn(command, " ");
    delta = end - start;
    strncpy(dest, command + start, end - start);
    dest[delta] = '\0';
}

void wifi_marauder_get_prefix_from_cmd(char* dest, const char* command) {
    int end;
    end = strcspn(command, " ");
    strncpy(dest, command, end);
    dest[end] = '\0';
}

void wifi_marauder_create_pcap_file(WifiMarauderApp* app) {
    char prefix[10];
    char capture_file_path[100];
    wifi_marauder_get_prefix_from_sniff_cmd(prefix, app->selected_tx_string);

    int i = 0;
    do {
        snprintf(
            capture_file_path,
            sizeof(capture_file_path),
            "%s/%s_%d.pcap",
            MARAUDER_APP_FOLDER_PCAPS,
            prefix,
            i);
        i++;
    } while(storage_file_exists(app->storage, capture_file_path));

    if(!storage_file_open(app->capture_file, capture_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot open pcap file");
    }
}

void wifi_marauder_create_log_file(WifiMarauderApp* app) {
    char prefix[10];
    char log_file_path[100];
    wifi_marauder_get_prefix_from_cmd(prefix, app->selected_tx_string);

    int i = 0;
    do {
        snprintf(
            log_file_path,
            sizeof(log_file_path),
            "%s/%s_%d.log",
            MARAUDER_APP_FOLDER_LOGS,
            prefix,
            i);
        i++;
    } while(storage_file_exists(app->storage, log_file_path));

    if(!storage_file_open(app->log_file, log_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot open log file");
    } else {
        strcpy(app->log_file_path, log_file_path);
    }
}