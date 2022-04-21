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

static const char* default_remote_name = "remote";

std::string InfraredAppRemoteManager::make_full_name(
    const std::string& path,
    const std::string& remote_name) const {
    return std::string("") + path + "/" + remote_name + InfraredApp::infrared_extension;
}

std::string InfraredAppRemoteManager::find_vacant_remote_name(const std::string& name) {
    std::string result_name;
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    FS_Error error = storage_common_stat(
        storage, make_full_name(InfraredApp::infrared_directory, name).c_str(), NULL);

    if(error == FSE_NOT_EXIST) {
        result_name = name;
    } else if(error != FSE_OK) {
        result_name = std::string();
    } else {
        /* if suggested name is occupied, try another one (name2, name3, etc) */
        uint32_t i = 1;
        std::string new_name;
        do {
            new_name = make_full_name(InfraredApp::infrared_directory, name + std::to_string(++i));
            error = storage_common_stat(storage, new_name.c_str(), NULL);
        } while(error == FSE_OK);

        if(error == FSE_NOT_EXIST) {
            result_name = name + std::to_string(i);
        } else {
            result_name = std::string();
        }
    }

    furi_record_close("storage");
    return result_name;
}

bool InfraredAppRemoteManager::add_button(const char* button_name, const InfraredAppSignal& signal) {
    remote->buttons.emplace_back(button_name, signal);
    return store();
}

bool InfraredAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const InfraredAppSignal& signal) {
    furi_check(button_name != nullptr);

    auto new_name = find_vacant_remote_name(default_remote_name);
    if(new_name.empty()) {
        return false;
    }

    remote = std::make_unique<InfraredAppRemote>(InfraredApp::infrared_directory, new_name);
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

    FS_Error error =
        storage_common_remove(storage, make_full_name(remote->path, remote->name).c_str());
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

    if(!remote->name.compare(str)) {
        return true;
    }

    auto new_name = find_vacant_remote_name(str);
    if(new_name.empty()) {
        return false;
    }

    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));

    std::string old_filename = make_full_name(remote->path, remote->name);
    std::string new_filename = make_full_name(remote->path, new_name);
    FS_Error error = storage_common_rename(storage, old_filename.c_str(), new_filename.c_str());
    remote->name = new_name;

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

    FURI_LOG_I(
        "RemoteManager", "store file: \'%s\'", make_full_name(remote->path, remote->name).c_str());
    result =
        flipper_format_file_open_always(ff, make_full_name(remote->path, remote->name).c_str());
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

bool InfraredAppRemoteManager::load(const std::string& path, const std::string& remote_name) {
    bool result = false;
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FURI_LOG_I("RemoteManager", "load file: \'%s\'", make_full_name(path, remote_name).c_str());
    result = flipper_format_file_open_existing(ff, make_full_name(path, remote_name).c_str());
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
        remote = std::make_unique<InfraredAppRemote>(path, remote_name);
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
