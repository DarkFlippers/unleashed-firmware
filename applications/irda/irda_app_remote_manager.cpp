#include <file_worker_cpp.h>
#include <flipper_file.h>
#include "irda_app_remote_manager.h"
#include "irda/helpers/irda_parser.h"
#include "irda/irda_app_signal.h"

#include <utility>

#include <irda.h>
#include <cstdio>
#include <furi.h>
#include <gui/modules/button_menu.h>
#include <storage/storage.h>
#include "irda_app.h"

static const std::string default_remote_name = "remote";

std::string IrdaAppRemoteManager::make_full_name(const std::string& remote_name) const {
    return std::string("") + IrdaApp::irda_directory + "/" + remote_name + IrdaApp::irda_extension;
}

std::string IrdaAppRemoteManager::find_vacant_remote_name(const std::string& name) {
    bool exist = true;
    FileWorkerCpp file_worker;

    if(!file_worker.is_file_exist(make_full_name(name).c_str(), &exist)) {
        return std::string();
    } else if(!exist) {
        return name;
    }

    /* if suggested name is occupied, try another one (name2, name3, etc) */
    uint32_t i = 1;
    bool file_worker_result = false;
    std::string new_name;
    do {
        new_name = make_full_name(name + std::to_string(++i));
        file_worker_result = file_worker.is_file_exist(new_name.c_str(), &exist);
    } while(file_worker_result && exist);

    return !exist ? name + std::to_string(i) : std::string();
}

bool IrdaAppRemoteManager::add_button(const char* button_name, const IrdaAppSignal& signal) {
    remote->buttons.emplace_back(button_name, signal);
    return store();
}

bool IrdaAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const IrdaAppSignal& signal) {
    furi_check(button_name != nullptr);

    auto new_name = find_vacant_remote_name(default_remote_name);
    if(new_name.empty()) {
        return false;
    }

    remote = std::make_unique<IrdaAppRemote>(new_name);
    return add_button(button_name, signal);
}

std::vector<std::string> IrdaAppRemoteManager::get_button_list(void) const {
    std::vector<std::string> name_vector;
    name_vector.reserve(remote->buttons.size());

    for(const auto& it : remote->buttons) {
        name_vector.emplace_back(it.name);
    }

    // copy elision
    return name_vector;
}

const IrdaAppSignal& IrdaAppRemoteManager::get_button_data(size_t index) const {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    return buttons.at(index).signal;
}

bool IrdaAppRemoteManager::delete_remote() {
    bool result;
    FileWorkerCpp file_worker;
    result = file_worker.remove(make_full_name(remote->name).c_str());

    reset_remote();
    return result;
}

void IrdaAppRemoteManager::reset_remote() {
    remote.reset();
}

bool IrdaAppRemoteManager::delete_button(uint32_t index) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    buttons.erase(buttons.begin() + index);
    return store();
}

std::string IrdaAppRemoteManager::get_button_name(uint32_t index) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());
    return buttons[index].name.c_str();
}

std::string IrdaAppRemoteManager::get_remote_name() {
    return remote ? remote->name : std::string();
}

int IrdaAppRemoteManager::find_remote_name(const std::vector<std::string>& strings) {
    int i = 0;
    for(const auto& str : strings) {
        if(!str.compare(remote->name)) {
            return i;
        }
        ++i;
    }
    return -1;
}

bool IrdaAppRemoteManager::rename_remote(const char* str) {
    furi_check(str != nullptr);
    furi_check(remote.get() != nullptr);

    if(!remote->name.compare(str)) {
        return true;
    }

    auto new_name = find_vacant_remote_name(str);
    if(new_name.empty()) {
        return false;
    }

    FileWorkerCpp file_worker;
    std::string old_filename = make_full_name(remote->name);
    std::string new_filename = make_full_name(new_name);
    bool result = file_worker.rename(old_filename.c_str(), new_filename.c_str());

    remote->name = new_name;

    return result;
}

bool IrdaAppRemoteManager::rename_button(uint32_t index, const char* str) {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    buttons[index].name = str;
    return store();
}

size_t IrdaAppRemoteManager::get_number_of_buttons() {
    furi_check(remote.get() != nullptr);
    return remote->buttons.size();
}

bool IrdaAppRemoteManager::store(void) {
    bool result = false;
    FileWorkerCpp file_worker;

    if(!file_worker.mkdir(IrdaApp::irda_directory)) return false;

    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
    FlipperFile* ff = flipper_file_alloc(storage);

    FURI_LOG_I("RemoteManager", "store file: \'%s\'", make_full_name(remote->name).c_str());
    result = flipper_file_open_always(ff, make_full_name(remote->name).c_str());
    if(result) {
        result = flipper_file_write_header_cstr(ff, "IR signals file", 1);
    }
    if(result) {
        for(const auto& button : remote->buttons) {
            result = irda_parser_save_signal(ff, button.signal, button.name.c_str());
            if(!result) {
                break;
            }
        }
    }

    flipper_file_close(ff);
    flipper_file_free(ff);
    furi_record_close("storage");
    return result;
}

bool IrdaAppRemoteManager::load(const std::string& remote_name) {
    bool result = false;
    Storage* storage = static_cast<Storage*>(furi_record_open("storage"));
    FlipperFile* ff = flipper_file_alloc(storage);

    FURI_LOG_I("RemoteManager", "load file: \'%s\'", make_full_name(remote_name).c_str());
    result = flipper_file_open_existing(ff, make_full_name(remote_name).c_str());
    if(result) {
        string_t header;
        string_init(header);
        uint32_t version;
        result = flipper_file_read_header(ff, header, &version);
        if(result) {
            result = !string_cmp_str(header, "IR signals file") && (version == 1);
        }
        string_clear(header);
    }
    if(result) {
        remote = std::make_unique<IrdaAppRemote>(remote_name);
        IrdaAppSignal signal;
        std::string signal_name;
        while(irda_parser_read_signal(ff, signal, signal_name)) {
            remote->buttons.emplace_back(signal_name.c_str(), std::move(signal));
        }
    }

    flipper_file_close(ff);
    flipper_file_free(ff);
    furi_record_close("storage");
    return result;
}
