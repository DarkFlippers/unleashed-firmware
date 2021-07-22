#include "irda-app-remote-manager.hpp"
#include "filesystem-api.h"
#include "furi.h"
#include "furi/check.h"
#include "gui/modules/button_menu.h"
#include "irda.h"
#include <cstdio>
#include <stdint.h>
#include <string>
#include <utility>
#include "irda-app-file-parser.hpp"

static const std::string default_remote_name = "remote";

std::string IrdaAppRemoteManager::find_vacant_remote_name(const std::string& name) {
    IrdaAppFileParser file_parser;
    bool exist = true;

    if(!file_parser.is_irda_file_exist(name.c_str(), &exist)) {
        return std::string();
    } else if(!exist) {
        return name;
    }

    uint32_t i = 1;
    /* if suggested name is occupied, try another one (name2, name3, etc) */
    while(file_parser.is_irda_file_exist((name + std::to_string(++i)).c_str(), &exist) && exist)
        ;

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
    IrdaAppFileParser file_parser;

    result = file_parser.remove_irda_file(remote->name.c_str());
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

    IrdaAppFileParser file_parser;
    bool result = file_parser.rename_irda_file(remote->name.c_str(), new_name.c_str());

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
    IrdaAppFileParser file_parser;
    bool result = true;

    if(!file_parser.open_irda_file_write(remote->name.c_str())) {
        return false;
    }

    for(const auto& button : remote->buttons) {
        bool result = file_parser.save_signal(button.signal, button.name.c_str());
        if(!result) {
            result = false;
            break;
        }
    }

    file_parser.close();

    return result;
}

bool IrdaAppRemoteManager::load(const std::string& name) {
    bool fs_res = false;
    IrdaAppFileParser file_parser;

    fs_res = file_parser.open_irda_file_read(name.c_str());
    if(!fs_res) {
        return false;
    }

    remote = std::make_unique<IrdaAppRemote>(name);

    while(1) {
        auto file_signal = file_parser.read_signal();
        if(!file_signal) {
            break;
        }
        remote->buttons.emplace_back(file_signal->name, file_signal->signal);
    }
    file_parser.close();

    return true;
}
