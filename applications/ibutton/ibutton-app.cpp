#include "ibutton-app.h"
#include <stdarg.h>
#include <callback-connector.h>
#include <m-string.h>

void iButtonApp::run(void) {
    iButtonEvent event;
    bool consumed;
    bool exit = false;

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view.receive_event(&event);

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == iButtonEvent::Type::EventTypeBack) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);
}

void iButtonApp::print_key_data(void) {
    uint8_t* key_data = key.get_data();
    switch(key.get_key_type()) {
    case iButtonKeyType::KeyDallas:
        printf(
            "Dallas %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3],
            key_data[4],
            key_data[5],
            key_data[6],
            key_data[7]);
        break;
    case iButtonKeyType::KeyCyfral:
        printf("Cyfral %02X %02X\r\n", key_data[0], key_data[1]);
        break;
    case iButtonKeyType::KeyMetakom:
        printf(
            "Metakom %02X %02X %02X %02X\r\n", key_data[0], key_data[1], key_data[2], key_data[3]);
        break;
    }
}

bool iButtonApp::read_hex_byte(string_t args, uint8_t* byte) {
    char* endptr;
    *byte = strtoul(string_get_cstr(args), &endptr, 16);
    if(*endptr == '\0') {
        return false;
    }
    size_t ws = string_search_char(args, ' ');
    if(ws != 2) {
        return false;
    }
    string_right(args, ws);
    string_strim(args);
    return true;
}

void iButtonApp::cli_cmd_callback(Cli* cli, string_t args, void* context) {
    iButtonApp::Scene scene;
    string_t cmd;
    string_init(cmd);
    if(!string_cmp_str(args, "read")) {
        scene = iButtonApp::Scene::SceneCliRead;
        printf("Reading key ...\r\n");
    } else {
        // Parse write / emulate commands
        size_t ws = string_search_char(args, ' ');
        if(ws == STRING_FAILURE) {
            printf("Incorrect input. Try tm <read | write | emulate> [key_type] [key_data]");
            string_clear(cmd);
            return;
        } else {
            string_set_n(cmd, args, 0, ws);
            string_right(args, ws);
            string_strim(args);
        }
        if(!string_cmp_str(cmd, "write")) {
            scene = iButtonApp::Scene::SceneCliWrite;
        } else if(!string_cmp_str(cmd, "emulate")) {
            scene = iButtonApp::Scene::SceneCliEmulate;
        } else {
            printf("Incorrect input. Try tm <write | emulate> <key_type> <key_data>");
            string_clear(cmd);
            return;
        }
        string_clear(cmd);
        // Parse key type
        string_t key_type;
        string_init(key_type);
        ws = string_search_char(args, ' ');
        string_set_n(key_type, args, 0, ws);
        uint8_t bytes_to_read = 0;
        if(!string_cmp_str(key_type, "0")) {
            key.set_type(iButtonKeyType::KeyDallas);
            bytes_to_read = 8;
        } else if(!string_cmp_str(key_type, "1")) {
            key.set_type(iButtonKeyType::KeyCyfral);
            bytes_to_read = 2;
        } else if(!string_cmp_str(key_type, "2")) {
            key.set_type(iButtonKeyType::KeyMetakom);
            bytes_to_read = 4;
        } else {
            printf("Incorrect key type. Try 0 - KeyDallas, 1 - KeyCyfral, 2 - KeyMetakom");
            string_clear(key_type);
            return;
        }
        string_clear(key_type);
        // Read key data
        string_right(args, 1);
        string_strim(args);
        uint8_t key_data[8] = {};
        uint8_t i = 0;
        bool ret = true;
        while((i < bytes_to_read) && ret) {
            ret = read_hex_byte(args, &key_data[i++]);
        }
        if(i != bytes_to_read) {
            printf("Incorrect key data. Type %d key data hex digits", bytes_to_read);
            return;
        }
        key.set_data(key_data, bytes_to_read);
        if(scene == iButtonApp::Scene::SceneCliWrite) {
            printf("Writing key ");
        } else {
            printf("Emulating key ");
        }
        print_key_data();
    }
    switch_to_next_scene(scene);
    // Wait return event
    iButtonApp::CliEvent result;
    if(osMessageQueueGet(cli_event_result, &result, NULL, osWaitForever) != osOK) {
        printf("Command execution error");
        return;
    }
    // Process return event
    switch(result) {
    case iButtonApp::CliEvent::CliReadSuccess:
    case iButtonApp::CliEvent::CliReadCRCError:
        print_key_data();
        if(result == iButtonApp::CliEvent::CliReadCRCError) {
            printf("Warning: invalid CRC");
        }
        break;
    case iButtonApp::CliEvent::CliReadNotKeyError:
        printf("Read error: not a key");
        break;
    case iButtonApp::CliEvent::CliTimeout:
        printf("Timeout error");
        break;
    case iButtonApp::CliEvent::CliInterrupt:
        printf("Command interrupted");
        break;
    case iButtonApp::CliEvent::CliWriteSuccess:
        printf("Write success");
        break;
    case iButtonApp::CliEvent::CliWriteFail:
        printf("Write fail");
        break;
    default:
        break;
    }
    return;
}

void iButtonApp::cli_send_event(iButtonApp::CliEvent scene) {
    osMessageQueuePut(cli_event_result, &scene, 0, osWaitForever);
}

iButtonApp::iButtonApp() {
    notify_init();
    api_hal_power_insomnia_enter();

    cli_event_result = osMessageQueueNew(1, sizeof(iButtonApp::Scene), NULL);
    key_worker = new KeyWorker(&ibutton_gpio);
    sd_ex_api = static_cast<SdCard_Api*>(furi_record_open("sdcard-ex"));
    fs_api = static_cast<FS_Api*>(furi_record_open("sdcard"));
    cli = static_cast<Cli*>(furi_record_open("cli"));
    auto callback = cbc::obtain_connector(this, &iButtonApp::cli_cmd_callback);
    cli_add_command(cli, "tm", callback, cli);

    // we need random
    srand(DWT->CYCCNT);
}

iButtonApp::~iButtonApp() {
    furi_record_close("sdcard-ex");
    furi_record_close("sdcard");
    cli_delete_command(cli, "tm");
    furi_record_close("cli");
    osMessageQueueDelete(cli_event_result);

    for(std::map<Scene, iButtonScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it) {
        delete it->second;
        scenes.erase(it);
    }

    api_hal_power_insomnia_exit();
}

iButtonAppViewManager* iButtonApp::get_view_manager() {
    return &view;
}

void iButtonApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);

    if(next_scene != Scene::SceneExit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
    }
}

void iButtonApp::search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list) {
    Scene previous_scene = Scene::SceneStart;
    bool scene_found = false;

    while(!scene_found) {
        previous_scene = get_previous_scene();
        for(Scene element : scenes_list) {
            if(previous_scene == element || previous_scene == Scene::SceneStart) {
                scene_found = true;
                break;
            }
        }
    }

    scenes[current_scene]->on_exit(this);
    current_scene = previous_scene;
    scenes[current_scene]->on_enter(this);
}

bool iButtonApp::switch_to_previous_scene(uint8_t count) {
    Scene previous_scene = Scene::SceneStart;

    for(uint8_t i = 0; i < count; i++) {
        previous_scene = get_previous_scene();
        if(previous_scene == Scene::SceneExit) break;
    }

    if(previous_scene == Scene::SceneExit) {
        return true;
    } else {
        scenes[current_scene]->on_exit(this);
        current_scene = previous_scene;
        scenes[current_scene]->on_enter(this);
        return false;
    }
}

iButtonApp::Scene iButtonApp::get_previous_scene() {
    Scene scene = previous_scenes_list.front();
    previous_scenes_list.pop_front();
    return scene;
}

const GpioPin* iButtonApp::get_ibutton_pin() {
    // TODO open record
    return &ibutton_gpio;
}

KeyWorker* iButtonApp::get_key_worker() {
    return key_worker;
}

iButtonKey* iButtonApp::get_key() {
    return &key;
}

SdCard_Api* iButtonApp::get_sd_ex_api() {
    return sd_ex_api;
}

FS_Api* iButtonApp::get_fs_api() {
    return fs_api;
}

char* iButtonApp::get_file_name() {
    return file_name;
}

uint8_t iButtonApp::get_file_name_size() {
    return file_name_size;
}

void iButtonApp::notify_init() {
    // TODO open record
    const GpioPin* vibro_record = &vibro_gpio;
    hal_gpio_init(vibro_record, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(vibro_record, false);
}

void iButtonApp::notify_green_blink() {
    notify_green_on();
    delay(10);
    notify_green_off();
}

void iButtonApp::notify_yellow_blink() {
    notify_red_on();
    notify_green_on();
    delay(10);
    notify_green_off();
    notify_red_off();
}

void iButtonApp::notify_red_blink() {
    notify_red_on();
    delay(10);
    notify_red_off();
}

void iButtonApp::notify_green_on() {
    api_hal_light_set(LightGreen, 0xFF);
}

void iButtonApp::notify_green_off() {
    api_hal_light_set(LightGreen, 0x00);
}

void iButtonApp::notify_red_on() {
    api_hal_light_set(LightRed, 0xFF);
}

void iButtonApp::notify_red_off() {
    api_hal_light_set(LightRed, 0x00);
}

void iButtonApp::notify_error() {
    notify_vibro_on();
    delay(50);
    notify_vibro_off();
    delay(100);
    notify_vibro_on();
    delay(50);
    notify_vibro_off();
}

void iButtonApp::notify_success() {
    notify_vibro_on();
    hal_pwm_set(0.5, 1760, &SPEAKER_TIM, SPEAKER_CH);
    delay(50);
    hal_pwm_stop(&SPEAKER_TIM, SPEAKER_CH);
    notify_vibro_off();
}

void iButtonApp::notify_vibro_on() {
    hal_gpio_write(&vibro_gpio, true);
}

void iButtonApp::notify_vibro_off() {
    hal_gpio_write(&vibro_gpio, false);
}

void iButtonApp::set_text_store(const char* text...) {
    va_list args;
    va_start(args, text);

    vsnprintf(text_store, text_store_size, text, args);

    va_end(args);
}

char* iButtonApp::get_text_store() {
    return text_store;
}

uint8_t iButtonApp::get_text_store_size() {
    return text_store_size;
}

void iButtonApp::generate_random_name(char* name, uint8_t max_name_size) {
    const uint8_t prefix_size = 9;
    const char* prefix[prefix_size] = {
        "ancient",
        "hollow",
        "strange",
        "disappeared",
        "unknown",
        "unthinkable",
        "unnamable",
        "nameless",
        "my",
    };

    const uint8_t suffix_size = 8;
    const char* suffix[suffix_size] = {
        "door",
        "entrance",
        "doorway",
        "entry",
        "portal",
        "entree",
        "opening",
        "crack",
    };

    sniprintf(
        name, max_name_size, "%s_%s", prefix[rand() % prefix_size], suffix[rand() % suffix_size]);
    // to upper
    name[0] = name[0] - 0x20;
}