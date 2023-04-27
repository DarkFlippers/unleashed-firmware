#include "../wifi_marauder_app_i.h"
#include "wifi_marauder_script.h"

WifiMarauderScript* wifi_marauder_script_alloc() {
    WifiMarauderScript* script = (WifiMarauderScript*)malloc(sizeof(WifiMarauderScript));
    if(script == NULL) {
        return NULL;
    }
    script->name = NULL;
    script->description = NULL;
    script->first_stage = NULL;
    script->last_stage = NULL;
    script->enable_led = WifiMarauderScriptBooleanUndefined;
    script->save_pcap = WifiMarauderScriptBooleanUndefined;
    script->repeat = 1;
    return script;
}

WifiMarauderScript* wifi_marauder_script_create(const char* script_name) {
    WifiMarauderScript* script = wifi_marauder_script_alloc();
    script->name = strdup(script_name);
    return script;
}

void _wifi_marauder_script_load_meta(WifiMarauderScript* script, cJSON* meta_section) {
    if(meta_section != NULL) {
        // Script description
        cJSON* description = cJSON_GetObjectItem(meta_section, "description");
        if(description != NULL) {
            script->description = strdup(description->valuestring);
        }
        // Enable LED
        cJSON* enable_led_json = cJSON_GetObjectItem(meta_section, "enableLed");
        if(cJSON_IsBool(enable_led_json)) {
            script->enable_led = enable_led_json->valueint;
        }
        // Save PCAP
        cJSON* save_pcap_json = cJSON_GetObjectItem(meta_section, "savePcap");
        if(cJSON_IsBool(save_pcap_json)) {
            script->save_pcap = save_pcap_json->valueint;
        }
        // Times the script will be repeated
        cJSON* repeat = cJSON_GetObjectItem(meta_section, "repeat");
        if(repeat != NULL) {
            script->repeat = repeat->valueint;
        }
    }
    if(script->description == NULL) {
        script->description = strdup("My script");
    }
}

WifiMarauderScriptStageScan* _wifi_marauder_script_get_stage_scan(cJSON* stages) {
    cJSON* stage_scan = cJSON_GetObjectItem(stages, "scan");
    if(stage_scan == NULL) {
        return NULL;
    }
    cJSON* type = cJSON_GetObjectItem(stage_scan, "type");
    if(type == NULL) {
        return NULL;
    }
    WifiMarauderScriptScanType scan_type;
    if(strcmp(type->valuestring, "ap") == 0) {
        scan_type = WifiMarauderScriptScanTypeAp;
    } else if(strcmp(type->valuestring, "station") == 0) {
        scan_type = WifiMarauderScriptScanTypeStation;
    } else {
        return NULL;
    }
    cJSON* channel = cJSON_GetObjectItem(stage_scan, "channel");
    int scan_channel = channel != NULL ? (int)cJSON_GetNumberValue(channel) : 0;
    cJSON* timeout = cJSON_GetObjectItem(stage_scan, "timeout");
    int scan_timeout = timeout != NULL ? (int)cJSON_GetNumberValue(timeout) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SCAN;

    WifiMarauderScriptStageScan* scan_stage =
        (WifiMarauderScriptStageScan*)malloc(sizeof(WifiMarauderScriptStageScan));
    scan_stage->type = scan_type;
    scan_stage->channel = scan_channel;
    scan_stage->timeout = scan_timeout;

    return scan_stage;
}

WifiMarauderScriptStageSelect* _wifi_marauder_script_get_stage_select(cJSON* stages) {
    cJSON* select_stage_json = cJSON_GetObjectItemCaseSensitive(stages, "select");
    if(select_stage_json == NULL) {
        return NULL;
    }

    cJSON* type_json = cJSON_GetObjectItemCaseSensitive(select_stage_json, "type");
    cJSON* filter_json = cJSON_GetObjectItemCaseSensitive(select_stage_json, "filter");
    cJSON* indexes_json = cJSON_GetObjectItemCaseSensitive(select_stage_json, "indexes");
    cJSON* allow_repeat_json = cJSON_GetObjectItemCaseSensitive(select_stage_json, "allow_repeat");

    if(!cJSON_IsString(type_json)) {
        return NULL;
    }
    WifiMarauderScriptSelectType select_type;
    if(strcmp(type_json->valuestring, "ap") == 0) {
        select_type = WifiMarauderScriptSelectTypeAp;
    } else if(strcmp(type_json->valuestring, "station") == 0) {
        select_type = WifiMarauderScriptSelectTypeStation;
    } else if(strcmp(type_json->valuestring, "ssid") == 0) {
        select_type = WifiMarauderScriptSelectTypeSsid;
    } else {
        return NULL;
    }
    char* filter_str = cJSON_IsString(filter_json) ? strdup(filter_json->valuestring) : NULL;

    WifiMarauderScriptStageSelect* stage_select =
        (WifiMarauderScriptStageSelect*)malloc(sizeof(WifiMarauderScriptStageSelect));
    stage_select->type = select_type;
    stage_select->allow_repeat = cJSON_IsBool(allow_repeat_json) ? allow_repeat_json->valueint :
                                                                   true;
    stage_select->filter = filter_str;

    if(cJSON_IsArray(indexes_json)) {
        int indexes_size = cJSON_GetArraySize(indexes_json);
        int* indexes = (int*)malloc(indexes_size * sizeof(int));
        for(int i = 0; i < indexes_size; i++) {
            cJSON* index_item = cJSON_GetArrayItem(indexes_json, i);
            if(cJSON_IsNumber(index_item)) {
                indexes[i] = index_item->valueint;
            }
        }
        stage_select->indexes = indexes;
        stage_select->index_count = indexes_size;
    } else {
        stage_select->indexes = NULL;
        stage_select->index_count = 0;
    }

    return stage_select;
}

WifiMarauderScriptStageDeauth* _wifi_marauder_script_get_stage_deauth(cJSON* stages) {
    cJSON* deauth_stage_json = cJSON_GetObjectItemCaseSensitive(stages, "deauth");
    if(deauth_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout = cJSON_GetObjectItem(deauth_stage_json, "timeout");
    int deauth_timeout = timeout != NULL ? (int)cJSON_GetNumberValue(timeout) :
                                           WIFI_MARAUDER_DEFAULT_TIMEOUT_DEAUTH;

    WifiMarauderScriptStageDeauth* deauth_stage =
        (WifiMarauderScriptStageDeauth*)malloc(sizeof(WifiMarauderScriptStageDeauth));
    deauth_stage->timeout = deauth_timeout;

    return deauth_stage;
}

WifiMarauderScriptStageProbe* _wifi_marauder_script_get_stage_probe(cJSON* stages) {
    cJSON* probe_stage_json = cJSON_GetObjectItemCaseSensitive(stages, "probe");
    if(probe_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout = cJSON_GetObjectItem(probe_stage_json, "timeout");
    int probe_timeout = timeout != NULL ? (int)cJSON_GetNumberValue(timeout) :
                                          WIFI_MARAUDER_DEFAULT_TIMEOUT_PROBE;

    WifiMarauderScriptStageProbe* probe_stage =
        (WifiMarauderScriptStageProbe*)malloc(sizeof(WifiMarauderScriptStageProbe));
    probe_stage->timeout = probe_timeout;

    return probe_stage;
}

WifiMarauderScriptStageSniffRaw* _wifi_marauder_script_get_stage_sniff_raw(cJSON* stages) {
    cJSON* sniffraw_stage_json = cJSON_GetObjectItem(stages, "sniffraw");
    if(sniffraw_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(sniffraw_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    WifiMarauderScriptStageSniffRaw* sniff_raw_stage =
        (WifiMarauderScriptStageSniffRaw*)malloc(sizeof(WifiMarauderScriptStageSniffRaw));
    sniff_raw_stage->timeout = timeout;

    return sniff_raw_stage;
}

WifiMarauderScriptStageSniffBeacon* _wifi_marauder_script_get_stage_sniff_beacon(cJSON* stages) {
    cJSON* sniffbeacon_stage_json = cJSON_GetObjectItem(stages, "sniffbeacon");
    if(sniffbeacon_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(sniffbeacon_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    WifiMarauderScriptStageSniffBeacon* sniff_beacon_stage =
        (WifiMarauderScriptStageSniffBeacon*)malloc(sizeof(WifiMarauderScriptStageSniffBeacon));
    sniff_beacon_stage->timeout = timeout;

    return sniff_beacon_stage;
}

WifiMarauderScriptStageSniffDeauth* _wifi_marauder_script_get_stage_sniff_deauth(cJSON* stages) {
    cJSON* sniffdeauth_stage_json = cJSON_GetObjectItem(stages, "sniffdeauth");
    if(sniffdeauth_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(sniffdeauth_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    WifiMarauderScriptStageSniffDeauth* sniff_deauth_stage =
        (WifiMarauderScriptStageSniffDeauth*)malloc(sizeof(WifiMarauderScriptStageSniffDeauth));
    sniff_deauth_stage->timeout = timeout;

    return sniff_deauth_stage;
}

WifiMarauderScriptStageSniffEsp* _wifi_marauder_script_get_stage_sniff_esp(cJSON* stages) {
    cJSON* sniffesp_stage_json = cJSON_GetObjectItem(stages, "sniffesp");
    if(sniffesp_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(sniffesp_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    WifiMarauderScriptStageSniffEsp* sniff_esp_stage =
        (WifiMarauderScriptStageSniffEsp*)malloc(sizeof(WifiMarauderScriptStageSniffEsp));
    sniff_esp_stage->timeout = timeout;

    return sniff_esp_stage;
}

WifiMarauderScriptStageSniffPmkid* _wifi_marauder_script_get_stage_sniff_pmkid(cJSON* stages) {
    cJSON* sniffpmkid_stage_json = cJSON_GetObjectItem(stages, "sniffpmkid");
    if(sniffpmkid_stage_json == NULL) {
        return NULL;
    }

    cJSON* channel_json = cJSON_GetObjectItem(sniffpmkid_stage_json, "channel");
    int channel = channel_json != NULL ? (int)cJSON_GetNumberValue(channel_json) : 0;
    cJSON* timeout_json = cJSON_GetObjectItem(sniffpmkid_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;
    cJSON* force_deauth_json =
        cJSON_GetObjectItemCaseSensitive(sniffpmkid_stage_json, "forceDeauth");
    bool force_deauth = cJSON_IsBool(force_deauth_json) ? force_deauth_json->valueint : true;

    WifiMarauderScriptStageSniffPmkid* sniff_pmkid_stage =
        (WifiMarauderScriptStageSniffPmkid*)malloc(sizeof(WifiMarauderScriptStageSniffPmkid));
    sniff_pmkid_stage->channel = channel;
    sniff_pmkid_stage->timeout = timeout;
    sniff_pmkid_stage->force_deauth = force_deauth;

    return sniff_pmkid_stage;
}

WifiMarauderScriptStageSniffPwn* _wifi_marauder_script_get_stage_sniff_pwn(cJSON* stages) {
    cJSON* sniffpwn_stage_json = cJSON_GetObjectItem(stages, "sniffpwn");
    if(sniffpwn_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(sniffpwn_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    WifiMarauderScriptStageSniffPwn* sniff_pwn_stage =
        (WifiMarauderScriptStageSniffPwn*)malloc(sizeof(WifiMarauderScriptStageSniffPwn));
    sniff_pwn_stage->timeout = timeout;

    return sniff_pwn_stage;
}

WifiMarauderScriptStageBeaconList* _wifi_marauder_script_get_stage_beacon_list(cJSON* stages) {
    cJSON* stage_beaconlist = cJSON_GetObjectItem(stages, "beaconList");
    if(stage_beaconlist == NULL) {
        return NULL;
    }
    WifiMarauderScriptStageBeaconList* beaconlist_stage =
        (WifiMarauderScriptStageBeaconList*)malloc(sizeof(WifiMarauderScriptStageBeaconList));
    if(beaconlist_stage == NULL) {
        return NULL;
    }
    cJSON* ssids = cJSON_GetObjectItem(stage_beaconlist, "ssids");
    if(ssids == NULL) {
        return NULL;
    }
    // SSID count
    int ssid_count = cJSON_GetArraySize(ssids);
    if(ssid_count == 0) {
        return NULL;
    }
    beaconlist_stage->ssid_count = ssid_count;
    // SSIDs
    beaconlist_stage->ssids = (char**)malloc(sizeof(char*) * ssid_count);
    if(beaconlist_stage->ssids == NULL) {
        return NULL;
    }
    for(int i = 0; i < ssid_count; i++) {
        cJSON* ssid = cJSON_GetArrayItem(ssids, i);
        if(ssid == NULL) {
            continue;
        }
        char* ssid_string = cJSON_GetStringValue(ssid);
        if(ssid_string == NULL) {
            continue;
        }
        beaconlist_stage->ssids[i] = (char*)malloc(sizeof(char) * (strlen(ssid_string) + 1));
        strcpy(beaconlist_stage->ssids[i], ssid_string);
    }
    // Timeout
    cJSON* timeout = cJSON_GetObjectItem(stage_beaconlist, "timeout");
    beaconlist_stage->timeout = timeout != NULL ? (int)cJSON_GetNumberValue(timeout) :
                                                  WIFI_MARAUDER_DEFAULT_TIMEOUT_BEACON;
    // Random SSIDs
    cJSON* random_ssids = cJSON_GetObjectItem(stage_beaconlist, "generate");
    beaconlist_stage->random_ssids =
        random_ssids != NULL ? (int)cJSON_GetNumberValue(random_ssids) : 0;

    return beaconlist_stage;
}

WifiMarauderScriptStageBeaconAp* _wifi_marauder_script_get_stage_beacon_ap(cJSON* stages) {
    cJSON* beaconap_stage_json = cJSON_GetObjectItem(stages, "beaconAp");
    if(beaconap_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(beaconap_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) :
                                         WIFI_MARAUDER_DEFAULT_TIMEOUT_BEACON;

    WifiMarauderScriptStageBeaconAp* beacon_ap_stage =
        (WifiMarauderScriptStageBeaconAp*)malloc(sizeof(WifiMarauderScriptStageBeaconAp));
    beacon_ap_stage->timeout = timeout;

    return beacon_ap_stage;
}

WifiMarauderScriptStageExec* _wifi_marauder_script_get_stage_exec(cJSON* stages) {
    cJSON* exec_stage_json = cJSON_GetObjectItem(stages, "exec");
    if(exec_stage_json == NULL) {
        return NULL;
    }

    cJSON* command_json = cJSON_GetObjectItemCaseSensitive(exec_stage_json, "command");
    char* command_str = cJSON_IsString(command_json) ? strdup(command_json->valuestring) : NULL;

    WifiMarauderScriptStageExec* exec_stage =
        (WifiMarauderScriptStageExec*)malloc(sizeof(WifiMarauderScriptStageExec));
    exec_stage->command = command_str;

    return exec_stage;
}

WifiMarauderScriptStageDelay* _wifi_marauder_script_get_stage_delay(cJSON* stages) {
    cJSON* delay_stage_json = cJSON_GetObjectItem(stages, "delay");
    if(delay_stage_json == NULL) {
        return NULL;
    }

    cJSON* timeout_json = cJSON_GetObjectItem(delay_stage_json, "timeout");
    int timeout = timeout_json != NULL ? (int)cJSON_GetNumberValue(timeout_json) : 0;

    WifiMarauderScriptStageDelay* delay_stage =
        (WifiMarauderScriptStageDelay*)malloc(sizeof(WifiMarauderScriptStageDelay));
    delay_stage->timeout = timeout;

    return delay_stage;
}

WifiMarauderScriptStage*
    _wifi_marauder_script_create_stage(WifiMarauderScriptStageType type, void* stage_data) {
    WifiMarauderScriptStage* stage =
        (WifiMarauderScriptStage*)malloc(sizeof(WifiMarauderScriptStage));
    stage->type = type;
    stage->stage = stage_data;
    stage->next_stage = NULL;
    return stage;
}

void wifi_marauder_script_add_stage(
    WifiMarauderScript* script,
    WifiMarauderScriptStageType stage_type,
    void* stage_data) {
    if(script == NULL || stage_data == NULL) {
        return;
    }
    WifiMarauderScriptStage* stage = _wifi_marauder_script_create_stage(stage_type, stage_data);
    if(script->last_stage != NULL) {
        script->last_stage->next_stage = stage;
    } else {
        script->first_stage = stage;
    }
    script->last_stage = stage;
}

void _wifi_marauder_script_load_stages(WifiMarauderScript* script, cJSON* stages) {
    // Scan stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeScan, _wifi_marauder_script_get_stage_scan(stages));
    // Select stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeSelect, _wifi_marauder_script_get_stage_select(stages));
    // Deauth stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeDeauth, _wifi_marauder_script_get_stage_deauth(stages));
    // Probe stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeProbe, _wifi_marauder_script_get_stage_probe(stages));
    // Sniff raw stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffRaw,
        _wifi_marauder_script_get_stage_sniff_raw(stages));
    // Sniff beacon stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffBeacon,
        _wifi_marauder_script_get_stage_sniff_beacon(stages));
    // Sniff deauth stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffDeauth,
        _wifi_marauder_script_get_stage_sniff_deauth(stages));
    // Sniff esp stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffEsp,
        _wifi_marauder_script_get_stage_sniff_esp(stages));
    // Sniff PMKID stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffPmkid,
        _wifi_marauder_script_get_stage_sniff_pmkid(stages));
    // Sniff pwn stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeSniffPwn,
        _wifi_marauder_script_get_stage_sniff_pwn(stages));
    // Beacon List stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeBeaconList,
        _wifi_marauder_script_get_stage_beacon_list(stages));
    // Beacon Ap stage
    wifi_marauder_script_add_stage(
        script,
        WifiMarauderScriptStageTypeBeaconAp,
        _wifi_marauder_script_get_stage_beacon_ap(stages));
    // Exec stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeExec, _wifi_marauder_script_get_stage_exec(stages));
    // Delay stage
    wifi_marauder_script_add_stage(
        script, WifiMarauderScriptStageTypeDelay, _wifi_marauder_script_get_stage_delay(stages));
}

WifiMarauderScript* wifi_marauder_script_parse_raw(const char* json_raw) {
    WifiMarauderScript* script = wifi_marauder_script_alloc();
    if(script == NULL) {
        return NULL;
    }
    cJSON* json = cJSON_Parse(json_raw);
    if(json == NULL) {
        return NULL;
    }
    cJSON* meta = cJSON_GetObjectItem(json, "meta");
    _wifi_marauder_script_load_meta(script, meta);

    cJSON* stages = cJSON_GetObjectItem(json, "stages");
    if(cJSON_IsArray(stages)) {
        cJSON* stage_item = NULL;
        cJSON_ArrayForEach(stage_item, stages) {
            _wifi_marauder_script_load_stages(script, stage_item);
        }
    } else {
        _wifi_marauder_script_load_stages(script, stages);
    }

    return script;
}

WifiMarauderScript* wifi_marauder_script_parse_json(Storage* storage, const char* file_path) {
    WifiMarauderScript* script = NULL;
    File* script_file = storage_file_alloc(storage);
    FuriString* script_name = furi_string_alloc();
    path_extract_filename_no_ext(file_path, script_name);

    if(storage_file_open(script_file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint32_t file_size = storage_file_size(script_file);
        char* json_buffer = (char*)malloc(file_size + 1);
        uint16_t bytes_read = storage_file_read(script_file, json_buffer, file_size);
        json_buffer[bytes_read] = '\0';

        script = wifi_marauder_script_parse_raw(json_buffer);
    }
    if(script == NULL) {
        script = wifi_marauder_script_create(furi_string_get_cstr(script_name));
    }
    script->name = strdup(furi_string_get_cstr(script_name));

    furi_string_free(script_name);
    storage_file_close(script_file);
    storage_file_free(script_file);
    return script;
}

cJSON* _wifi_marauder_script_create_json_meta(WifiMarauderScript* script) {
    cJSON* meta_json = cJSON_CreateObject();
    if(script->description != NULL) {
        cJSON_AddStringToObject(meta_json, "description", script->description);
    } else {
        cJSON_AddStringToObject(meta_json, "description", "My Script");
    }
    if(script->enable_led != WifiMarauderScriptBooleanUndefined) {
        cJSON_AddBoolToObject(
            meta_json, "enableLed", (script->enable_led == WifiMarauderScriptBooleanTrue));
    }
    if(script->save_pcap != WifiMarauderScriptBooleanUndefined) {
        cJSON_AddBoolToObject(
            meta_json, "savePcap", (script->save_pcap == WifiMarauderScriptBooleanTrue));
    }
    cJSON_AddNumberToObject(meta_json, "repeat", script->repeat);
    return meta_json;
}

cJSON* _wifi_marauder_script_create_json_scan(WifiMarauderScriptStageScan* scan_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "scan", cJSON_CreateObject());
    cJSON* scan_json = cJSON_GetObjectItem(stage_json, "scan");
    // Scan type
    cJSON_AddStringToObject(
        scan_json, "type", scan_stage->type == WifiMarauderScriptScanTypeAp ? "ap" : "station");
    // Channel
    if(scan_stage->channel > 0) {
        cJSON_AddNumberToObject(scan_json, "channel", scan_stage->channel);
    }
    // Timeout
    if(scan_stage->timeout > 0) {
        cJSON_AddNumberToObject(scan_json, "timeout", scan_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_select(WifiMarauderScriptStageSelect* select_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "select", cJSON_CreateObject());
    cJSON* select_json = cJSON_GetObjectItem(stage_json, "select");
    // Select type
    cJSON_AddStringToObject(
        select_json,
        "type",
        select_stage->type == WifiMarauderScriptSelectTypeAp      ? "ap" :
        select_stage->type == WifiMarauderScriptSelectTypeStation ? "station" :
                                                                    "ssid");
    if(select_stage->filter != NULL) {
        cJSON_AddStringToObject(select_json, "filter", select_stage->filter);
    }
    // Indexes
    if(select_stage->indexes != NULL && select_stage->index_count > 0) {
        cJSON* indexes_json = cJSON_CreateArray();
        for(int i = 0; i < select_stage->index_count; i++) {
            cJSON_AddItemToArray(indexes_json, cJSON_CreateNumber(select_stage->indexes[i]));
        }
        cJSON_AddItemToObject(select_json, "indexes", indexes_json);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_deauth(WifiMarauderScriptStageDeauth* deauth_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "deauth", cJSON_CreateObject());
    cJSON* deauth_json = cJSON_GetObjectItem(stage_json, "deauth");
    // Timeout
    if(deauth_stage->timeout > 0) {
        cJSON_AddNumberToObject(deauth_json, "timeout", deauth_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_probe(WifiMarauderScriptStageProbe* probe_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "probe", cJSON_CreateObject());
    cJSON* probe_json = cJSON_GetObjectItem(stage_json, "probe");
    // Timeout
    if(probe_stage->timeout > 0) {
        cJSON_AddNumberToObject(probe_json, "timeout", probe_stage->timeout);
    }
    return stage_json;
}

cJSON*
    _wifi_marauder_script_create_json_sniffraw(WifiMarauderScriptStageSniffRaw* sniffraw_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffRaw", cJSON_CreateObject());
    cJSON* sniffraw_json = cJSON_GetObjectItem(stage_json, "sniffRaw");
    // Timeout
    if(sniffraw_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffraw_json, "timeout", sniffraw_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_sniffbeacon(
    WifiMarauderScriptStageSniffBeacon* sniffbeacon_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffBeacon", cJSON_CreateObject());
    cJSON* sniffbeacon_json = cJSON_GetObjectItem(stage_json, "sniffBeacon");
    // Timeout
    if(sniffbeacon_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffbeacon_json, "timeout", sniffbeacon_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_sniffdeauth(
    WifiMarauderScriptStageSniffDeauth* sniffdeauth_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffDeauth", cJSON_CreateObject());
    cJSON* sniffdeauth_json = cJSON_GetObjectItem(stage_json, "sniffDeauth");
    // Timeout
    if(sniffdeauth_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffdeauth_json, "timeout", sniffdeauth_stage->timeout);
    }
    return stage_json;
}

cJSON*
    _wifi_marauder_script_create_json_sniffesp(WifiMarauderScriptStageSniffEsp* sniffesp_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffEsp", cJSON_CreateObject());
    cJSON* sniffesp_json = cJSON_GetObjectItem(stage_json, "sniffEsp");
    // Timeout
    if(sniffesp_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffesp_json, "timeout", sniffesp_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_sniffpmkid(
    WifiMarauderScriptStageSniffPmkid* sniffpmkid_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffPmkid", cJSON_CreateObject());
    cJSON* sniffpmkid_json = cJSON_GetObjectItem(stage_json, "sniffPmkid");
    // Force deauth
    cJSON_AddBoolToObject(sniffpmkid_json, "forceDeauth", sniffpmkid_stage->force_deauth);
    // Channel
    if(sniffpmkid_stage->channel > 0) {
        cJSON_AddNumberToObject(sniffpmkid_json, "channel", sniffpmkid_stage->channel);
    }
    // Timeout
    if(sniffpmkid_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffpmkid_json, "timeout", sniffpmkid_stage->timeout);
    }
    return stage_json;
}

cJSON*
    _wifi_marauder_script_create_json_sniffpwn(WifiMarauderScriptStageSniffPwn* sniffpwn_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "sniffPwn", cJSON_CreateObject());
    cJSON* sniffpwn_json = cJSON_GetObjectItem(stage_json, "sniffPwn");
    // Timeout
    if(sniffpwn_stage->timeout > 0) {
        cJSON_AddNumberToObject(sniffpwn_json, "timeout", sniffpwn_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_beaconlist(
    WifiMarauderScriptStageBeaconList* beaconlist_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "beaconList", cJSON_CreateObject());
    cJSON* beaconlist_json = cJSON_GetObjectItem(stage_json, "beaconList");
    // SSIDs
    if(beaconlist_stage->ssids != NULL) {
        cJSON* ssids_json = cJSON_CreateStringArray(
            (const char**)beaconlist_stage->ssids, beaconlist_stage->ssid_count);
        cJSON_AddItemToObject(beaconlist_json, "ssids", ssids_json);
    }
    // Random SSIDs
    if(beaconlist_stage->random_ssids > 0) {
        cJSON_AddNumberToObject(beaconlist_json, "generate", beaconlist_stage->random_ssids);
    }
    // Timeout
    if(beaconlist_stage->timeout > 0) {
        cJSON_AddNumberToObject(beaconlist_json, "timeout", beaconlist_stage->timeout);
    }
    return stage_json;
}

cJSON*
    _wifi_marauder_script_create_json_beaconap(WifiMarauderScriptStageBeaconAp* beaconap_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "beaconAp", cJSON_CreateObject());
    cJSON* beaconap_json = cJSON_GetObjectItem(stage_json, "beaconAp");
    // Timeout
    if(beaconap_stage->timeout > 0) {
        cJSON_AddNumberToObject(beaconap_json, "timeout", beaconap_stage->timeout);
    }
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_exec(WifiMarauderScriptStageExec* exec_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "exec", cJSON_CreateObject());
    cJSON* exec_json = cJSON_GetObjectItem(stage_json, "exec");
    // Command
    cJSON_AddStringToObject(
        exec_json, "command", exec_stage->command != NULL ? exec_stage->command : "");
    return stage_json;
}

cJSON* _wifi_marauder_script_create_json_delay(WifiMarauderScriptStageDelay* delay_stage) {
    cJSON* stage_json = cJSON_CreateObject();
    cJSON_AddItemToObject(stage_json, "delay", cJSON_CreateObject());
    cJSON* delay_json = cJSON_GetObjectItem(stage_json, "delay");
    // Timeout
    if(delay_stage->timeout > 0) {
        cJSON_AddNumberToObject(delay_json, "timeout", delay_stage->timeout);
    }
    return stage_json;
}

void wifi_marauder_script_save_json(
    Storage* storage,
    const char* file_path,
    WifiMarauderScript* script) {
    File* script_file = storage_file_alloc(storage);

    if(storage_file_open(script_file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        cJSON* root_json = cJSON_CreateObject();

        // Meta info
        cJSON* meta_json = _wifi_marauder_script_create_json_meta(script);
        cJSON_AddItemToObject(root_json, "meta", meta_json);

        // Create array for stages
        cJSON* stages_array = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json, "stages", stages_array);

        // Iterate over each stage and create the corresponding JSON object
        WifiMarauderScriptStage* stage = script->first_stage;
        while(stage != NULL) {
            cJSON* stage_json = NULL;

            switch(stage->type) {
            case WifiMarauderScriptStageTypeScan: {
                WifiMarauderScriptStageScan* scan_stage =
                    (WifiMarauderScriptStageScan*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_scan(scan_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSelect: {
                WifiMarauderScriptStageSelect* select_stage =
                    (WifiMarauderScriptStageSelect*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_select(select_stage);
                break;
            }
            case WifiMarauderScriptStageTypeDeauth: {
                WifiMarauderScriptStageDeauth* deauth_stage =
                    (WifiMarauderScriptStageDeauth*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_deauth(deauth_stage);
                break;
            }
            case WifiMarauderScriptStageTypeProbe: {
                WifiMarauderScriptStageProbe* probe_stage =
                    (WifiMarauderScriptStageProbe*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_probe(probe_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffRaw: {
                WifiMarauderScriptStageSniffRaw* sniffraw_stage =
                    (WifiMarauderScriptStageSniffRaw*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffraw(sniffraw_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffBeacon: {
                WifiMarauderScriptStageSniffBeacon* sniffbeacon_stage =
                    (WifiMarauderScriptStageSniffBeacon*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffbeacon(sniffbeacon_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffDeauth: {
                WifiMarauderScriptStageSniffDeauth* sniffdeauth_stage =
                    (WifiMarauderScriptStageSniffDeauth*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffdeauth(sniffdeauth_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffEsp: {
                WifiMarauderScriptStageSniffEsp* sniffesp_stage =
                    (WifiMarauderScriptStageSniffEsp*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffesp(sniffesp_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffPmkid: {
                WifiMarauderScriptStageSniffPmkid* sniffpmkid_stage =
                    (WifiMarauderScriptStageSniffPmkid*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffpmkid(sniffpmkid_stage);
                break;
            }
            case WifiMarauderScriptStageTypeSniffPwn: {
                WifiMarauderScriptStageSniffPwn* sniffpwn_stage =
                    (WifiMarauderScriptStageSniffPwn*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_sniffpwn(sniffpwn_stage);
                break;
            }
            case WifiMarauderScriptStageTypeBeaconList: {
                WifiMarauderScriptStageBeaconList* beaconlist_stage =
                    (WifiMarauderScriptStageBeaconList*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_beaconlist(beaconlist_stage);
                break;
            }
            case WifiMarauderScriptStageTypeBeaconAp: {
                WifiMarauderScriptStageBeaconAp* beaconap_stage =
                    (WifiMarauderScriptStageBeaconAp*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_beaconap(beaconap_stage);
                break;
            }
            case WifiMarauderScriptStageTypeExec: {
                WifiMarauderScriptStageExec* exec_stage =
                    (WifiMarauderScriptStageExec*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_exec(exec_stage);
                break;
            }
            case WifiMarauderScriptStageTypeDelay: {
                WifiMarauderScriptStageDelay* delay_stage =
                    (WifiMarauderScriptStageDelay*)stage->stage;
                stage_json = _wifi_marauder_script_create_json_delay(delay_stage);
                break;
            }
            }

            // Add the stage JSON object to the "stages" array
            if(stage_json != NULL) {
                cJSON_AddItemToArray(stages_array, stage_json);
            }

            stage = stage->next_stage;
        }

        // Write JSON to file
        char* json_str = cJSON_Print(root_json);
        storage_file_write(script_file, json_str, strlen(json_str));

        //free(json_str);
        storage_file_close(script_file);
    }
    storage_file_free(script_file);
}

bool wifi_marauder_script_has_stage(
    WifiMarauderScript* script,
    WifiMarauderScriptStageType stage_type) {
    if(script == NULL) {
        return false;
    }
    WifiMarauderScriptStage* current_stage = script->first_stage;
    while(current_stage != NULL) {
        if(current_stage->type == stage_type) {
            return true;
        }
        current_stage = current_stage->next_stage;
    }
    return false;
}

void wifi_marauder_script_free(WifiMarauderScript* script) {
    if(script == NULL) {
        return;
    }
    WifiMarauderScriptStage* current_stage = script->first_stage;
    while(current_stage != NULL) {
        WifiMarauderScriptStage* next_stage = current_stage->next_stage;
        switch(current_stage->type) {
        case WifiMarauderScriptStageTypeScan:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSelect:
            if(((WifiMarauderScriptStageSelect*)current_stage->stage)->filter != NULL) {
                free(((WifiMarauderScriptStageSelect*)current_stage->stage)->filter);
            }
            if(((WifiMarauderScriptStageSelect*)current_stage->stage)->indexes != NULL) {
                free(((WifiMarauderScriptStageSelect*)current_stage->stage)->indexes);
            }
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeDeauth:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeProbe:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffRaw:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffBeacon:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffDeauth:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffEsp:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffPmkid:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeSniffPwn:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeBeaconList:
            for(int i = 0;
                i < ((WifiMarauderScriptStageBeaconList*)current_stage->stage)->ssid_count;
                i++) {
                free(((WifiMarauderScriptStageBeaconList*)current_stage->stage)->ssids[i]);
            }
            free(((WifiMarauderScriptStageBeaconList*)current_stage->stage)->ssids);
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeBeaconAp:
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeExec:
            if(((WifiMarauderScriptStageExec*)current_stage->stage)->command != NULL) {
                free(((WifiMarauderScriptStageExec*)current_stage->stage)->command);
            }
            free(current_stage->stage);
            break;
        case WifiMarauderScriptStageTypeDelay:
            free(current_stage->stage);
            break;
        }
        free(current_stage);
        current_stage = next_stage;
    }
    free(script->name);
    free(script->description);
    free(script);
}