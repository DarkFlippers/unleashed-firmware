#include "m-string.h"
#include "storage/filesystem_api_defines.h"
#include <flipper_format/flipper_format.h>
#include "infrared_app_remote_manager.h"
#include "infrared/helpers/infrared_parser.h"
#include "infrared/infrared_app_signal.h"

#include <utility>

#include <infrared.h>
#include <cstdio>
#include <furi.h>
#include <gui/modules/button_menu.h>
#include <storage/storage.h>
#include "infrared_app.h"
#include <toolbox/path.h>

static const char* default_remote_name = "remote";

void InfraredAppRemoteManager::find_vacant_remote_name(string_t name, string_t path) {
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    string_t base_path;
    string_init_set(base_path, path);

    if(string_end_with_str_p(base_path, InfraredApp::infrared_extension)) {
        size_t filename_start = string_search_rchar(base_path, '/');
        string_left(base_path, filename_start);
    }

    string_printf(
        base_path,
        "%s/%s%s",
        string_get_cstr(path),
        string_get_cstr(name),
        InfraredApp::infrared_extension);

    FS_Error error = storage_common_stat(storage, string_get_cstr(base_path), NULL);

    if(error == FSE_OK) {
        /* if suggested name is occupied, try another one (name2, name3, etc) */
        size_t dot = string_search_rchar(base_path, '.');
        string_left(base_path, dot);

        string_t path_temp;
        string_init(path_temp);

        uint32_t i = 1;
        do {
            string_printf(
                path_temp,
                "%s%u%s",
                string_get_cstr(base_path),
                ++i,
                InfraredApp::infrared_extension);
            error = storage_common_stat(storage, string_get_cstr(path_temp), NULL);
        } while(error == FSE_OK);

        string_clear(path_temp);

        if(error == FSE_NOT_EXIST) {
            string_cat_printf(name, "%u", i);
        }
    }

    string_clear(base_path);
    furi_record_close("storage");
}

bool InfraredAppRemoteManager::add_button(const char* button_name, const InfraredAppSignal& signal) {
    remote->buttons.emplace_back(button_name, signal);
    return store();
}

bool InfraredAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const InfraredAppSignal& signal) {
    furi_check(button_name != nullptr);

    string_t new_name;
    string_init_set_str(new_name, default_remote_name);

    string_t new_path;
    string_init_set_str(new_path, InfraredApp::infrared_directory);

    find_vacant_remote_name(new_name, new_path);

    string_cat_printf(
        new_path, "/%s%s", string_get_cstr(new_name), InfraredApp::infrared_extension);

    remote = std::make_unique<InfraredAppRemote>(new_path);
    remote->name = std::string(string_get_cstr(new_name));

    string_clear(new_path);
    string_clear(new_name);

    return add_button(button_name, signal);
}

std::vector<std::string> InfraredAppRemoteManager::get_button_list(void) const {
    std::vector<std::string> name_vector;
    name_vector.reserve(remote->buttons.size());

    for(const auto& it : remote->buttons) {
        name_vector.emplace_back(it.name);
    }

    // copy elision
    return name_vector;
}

const InfraredAppSignal& InfraredAppRemoteManager::get_button_data(size_t index) const {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    return buttons.at(index).signal;
}

bool InfraredAppRemoteManager::delete_remote() {
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    FS_Error error = storage_common_remove(storage, string_get_cstr(remote->path));
    reset_remote();

    furi_record_close("storage");
    return (error == FSE_OK || error == FSE_NOT_EXIST);
}

void InfraredAppRemoteManager::reset_remote() {
    remote.reset();
}

bool InfraredAppRemoteManager::delete_button(uint32_t index) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    buttons.erase(buttons.begin() + index);
    return store();
}

std::string InfraredAppRemoteManager::get_button_name(uint32_t index) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());
    return buttons[index].name.c_str();
}

std::string InfraredAppRemoteManager::get_remote_name() {
    return remote.get() ? remote->name : std::string();
}

bool InfraredAppRemoteManager::rename_remote(const char* str) {
    furi_check(str != nullptr);
    furi_check(remote.get() != nullptr);
    furi_check(!string_empty_p(remote->path));

    if(!remote->name.compare(str)) {
        return true;
    }

    string_t new_name;
    string_init_set_str(new_name, str);
    find_vacant_remote_name(new_name, remote->path);

    string_t new_path;
    string_init_set(new_path, remote->path);
    if(string_end_with_str_p(new_path, InfraredApp::infrared_extension)) {
        size_t filename_start = string_search_rchar(new_path, '/');
        string_left(new_path, filename_start);
    }
    string_cat_printf(
        new_path, "/%s%s", string_get_cstr(new_name), InfraredApp::infrared_extension);

    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    FS_Error error =
        storage_common_rename(storage, string_get_cstr(remote->path), string_get_cstr(new_path));
    remote->name = std::string(string_get_cstr(new_name));

    string_clear(new_name);
    string_clear(new_path);

    furi_record_close("storage");
    return (error == FSE_OK || error == FSE_EXIST);
}

bool InfraredAppRemoteManager::rename_button(uint32_t index, const char* str) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    buttons[index].name = str;
    return store();
}

size_t InfraredAppRemoteManager::get_number_of_buttons() {
    furi_check(remote.get() != nullptr);
    return remote->buttons.size();
}

bool InfraredAppRemoteManager::store(void) {
    bool result = false;
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    if(!storage_simply_mkdir(storage, InfraredApp::infrared_directory)) return false;

    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FURI_LOG_I("RemoteManager", "store file: \'%s\'", string_get_cstr(remote->path));
    result = flipper_format_file_open_always(ff, string_get_cstr(remote->path));
    if(result) {
        result = flipper_format_write_header_cstr(ff, "IR signals file", 1);
    }
    if(result) {
        for(const auto& button : remote->buttons) {
            result = infrared_parser_save_signal(ff, button.signal, button.name.c_str());
            if(!result) {
                break;
            }
        }
    }

    flipper_format_free(ff);
    furi_record_close("storage");
    return result;
}

bool InfraredAppRemoteManager::load(string_t path) {
    bool result = false;
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FURI_LOG_I("RemoteManager", "load file: \'%s\'", string_get_cstr(path));
    result = flipper_format_file_open_existing(ff, string_get_cstr(path));
    if(result) {
        string_t header;
        string_init(header);
        uint32_t version;
        result = flipper_format_read_header(ff, header, &version);
        if(result) {
            result = !string_cmp_str(header, "IR signals file") && (version == 1);
        }
        string_clear(header);
    }
    if(result) {
        string_t new_name;
        string_init(new_name);

        remote = std::make_unique<InfraredAppRemote>(path);
        path_extract_filename(path, new_name, true);
        remote->name = std::string(string_get_cstr(new_name));

        string_clear(new_name);
        InfraredAppSignal signal;
        std::string signal_name;
        while(infrared_parser_read_signal(ff, signal, signal_name)) {
            remote->buttons.emplace_back(signal_name.c_str(), std::move(signal));
        }
    }

    flipper_format_free(ff);
    furi_record_close("storage");
    return result;
}
