#include "../wifi_marauder_app_i.h"

// Scan
static void wifi_marauder_scene_script_stage_add_scan_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageScan* stage =
        (WifiMarauderScriptStageScan*)malloc(sizeof(WifiMarauderScriptStageScan));
    stage->type = WifiMarauderScriptScanTypeAp;
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SCAN;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeScan, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Select
static void wifi_marauder_scene_script_stage_add_select_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSelect* stage =
        (WifiMarauderScriptStageSelect*)malloc(sizeof(WifiMarauderScriptStageSelect));
    stage->type = WifiMarauderScriptSelectTypeAp;
    stage->filter = strdup("all");

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSelect, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Deauth
static void wifi_marauder_scene_script_stage_add_deauth_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageDeauth* stage =
        (WifiMarauderScriptStageDeauth*)malloc(sizeof(WifiMarauderScriptStageDeauth));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_DEAUTH;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeDeauth, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Probe
static void wifi_marauder_scene_script_stage_add_probe_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageProbe* stage =
        (WifiMarauderScriptStageProbe*)malloc(sizeof(WifiMarauderScriptStageProbe));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_PROBE;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeProbe, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff RAW
static void wifi_marauder_scene_script_stage_add_sniffraw_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffRaw* stage =
        (WifiMarauderScriptStageSniffRaw*)malloc(sizeof(WifiMarauderScriptStageSniffRaw));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffRaw, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff Beacon
static void
    wifi_marauder_scene_script_stage_add_sniffbeacon_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffBeacon* stage =
        (WifiMarauderScriptStageSniffBeacon*)malloc(sizeof(WifiMarauderScriptStageSniffBeacon));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffBeacon, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff Deauth
static void
    wifi_marauder_scene_script_stage_add_sniffdeauth_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffDeauth* stage =
        (WifiMarauderScriptStageSniffDeauth*)malloc(sizeof(WifiMarauderScriptStageSniffDeauth));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffDeauth, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff Esp
static void wifi_marauder_scene_script_stage_add_sniffesp_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffEsp* stage =
        (WifiMarauderScriptStageSniffEsp*)malloc(sizeof(WifiMarauderScriptStageSniffEsp));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffEsp, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff PMKID
static void
    wifi_marauder_scene_script_stage_add_sniffpmkid_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffPmkid* stage =
        (WifiMarauderScriptStageSniffPmkid*)malloc(sizeof(WifiMarauderScriptStageSniffPmkid));
    stage->channel = 0;
    stage->force_deauth = WifiMarauderScriptBooleanTrue;
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffPmkid, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Sniff Pwn
static void wifi_marauder_scene_script_stage_add_sniffpwn_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageSniffPwn* stage =
        (WifiMarauderScriptStageSniffPwn*)malloc(sizeof(WifiMarauderScriptStageSniffPwn));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_SNIFF;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeSniffPwn, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Beacon list
static void
    wifi_marauder_scene_script_stage_add_beaconlist_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageBeaconList* stage =
        (WifiMarauderScriptStageBeaconList*)malloc(sizeof(WifiMarauderScriptStageBeaconList));
    stage->ssids = NULL;
    stage->ssid_count = 0;
    stage->random_ssids = 0;
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_BEACON;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeBeaconList, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Beacon AP
static void wifi_marauder_scene_script_stage_add_beaconap_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageBeaconAp* stage =
        (WifiMarauderScriptStageBeaconAp*)malloc(sizeof(WifiMarauderScriptStageBeaconAp));
    stage->timeout = WIFI_MARAUDER_DEFAULT_TIMEOUT_BEACON;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeBeaconAp, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Exec
static void wifi_marauder_scene_script_stage_add_exec_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageExec* stage =
        (WifiMarauderScriptStageExec*)malloc(sizeof(WifiMarauderScriptStageExec));
    stage->command = NULL;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeExec, stage);
    scene_manager_previous_scene(app->scene_manager);
}

// Delay
static void wifi_marauder_scene_script_stage_add_delay_callback(void* context, uint32_t index) {
    UNUSED(index);
    WifiMarauderApp* app = context;

    WifiMarauderScriptStageDelay* stage =
        (WifiMarauderScriptStageDelay*)malloc(sizeof(WifiMarauderScriptStageDelay));
    stage->timeout = 0;

    wifi_marauder_script_add_stage(app->script, WifiMarauderScriptStageTypeDelay, stage);
    scene_manager_previous_scene(app->scene_manager);
}

void wifi_marauder_scene_script_stage_add_on_enter(void* context) {
    WifiMarauderApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_set_header(submenu, "Add stage");

    int menu_index = 0;
    submenu_add_item(
        submenu, "[+] Scan", menu_index++, wifi_marauder_scene_script_stage_add_scan_callback, app);
    submenu_add_item(
        submenu,
        "[+] Select",
        menu_index++,
        wifi_marauder_scene_script_stage_add_select_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Deauth",
        menu_index++,
        wifi_marauder_scene_script_stage_add_deauth_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Probe",
        menu_index++,
        wifi_marauder_scene_script_stage_add_probe_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff RAW",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffraw_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff Beacon",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffbeacon_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff Deauth",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffdeauth_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff Esp",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffesp_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff PMKID",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffpmkid_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Sniff Pwnagotchi",
        menu_index++,
        wifi_marauder_scene_script_stage_add_sniffpwn_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Beacon List",
        menu_index++,
        wifi_marauder_scene_script_stage_add_beaconlist_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Beacon AP",
        menu_index++,
        wifi_marauder_scene_script_stage_add_beaconap_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Custom command",
        menu_index++,
        wifi_marauder_scene_script_stage_add_exec_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] Delay",
        menu_index++,
        wifi_marauder_scene_script_stage_add_delay_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptEdit));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_stage_add_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_stage_add_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
