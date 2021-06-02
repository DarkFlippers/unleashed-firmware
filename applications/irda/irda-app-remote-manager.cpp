#include "irda-app-remote-manager.hpp"
#include "furi.h"
#include <string>
#include <utility>

IrdaAppRemoteManager::IrdaAppRemoteManager() {
    // Read from api-hal-storage, and fill remotes
}

static const std::string default_remote_name = "remote";

void IrdaAppRemoteManager::add_button(const char* button_name, const IrdaMessage* message) {
    remotes[current_remote_index].buttons.emplace_back(button_name, message);
}

void IrdaAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const IrdaMessage* message) {
    bool found = true;
    int i = 0;

    // find first free common name for remote
    do {
        found = false;
        ++i;
        for(const auto& it : remotes) {
            if(it.name == (default_remote_name + std::to_string(i))) {
                found = true;
                break;
            }
        }
    } while(found);

    remotes.emplace_back(default_remote_name + std::to_string(i));
    current_remote_index = remotes.size() - 1;
    add_button(button_name, message);
}

IrdaAppRemote::IrdaAppRemote(std::string name)
    : name(name) {
}

std::vector<std::string> IrdaAppRemoteManager::get_button_list(void) const {
    std::vector<std::string> name_vector;
    auto remote = remotes[current_remote_index];
    name_vector.reserve(remote.buttons.size());

    for(const auto& it : remote.buttons) {
        name_vector.emplace_back(it.name);
    }

    // copy elision
    return name_vector;
}

std::vector<std::string> IrdaAppRemoteManager::get_remote_list() const {
    std::vector<std::string> name_vector;
    name_vector.reserve(remotes.size());

    for(const auto& it : remotes) {
        name_vector.push_back(it.name);
    }

    // copy elision
    return name_vector;
}

size_t IrdaAppRemoteManager::get_current_remote(void) const {
    return current_remote_index;
}

size_t IrdaAppRemoteManager::get_current_button(void) const {
    return current_button_index;
}

void IrdaAppRemote::add_button(
    size_t remote_index,
    const char* button_name,
    const IrdaMessage* message) {
    buttons.emplace_back(button_name, message);
}

const IrdaMessage* IrdaAppRemoteManager::get_button_data(size_t button_index) const {
    furi_check(remotes[current_remote_index].buttons.size() > button_index);
    auto& b = remotes[current_remote_index].buttons.at(button_index);
    return &b.message;
}

void IrdaAppRemoteManager::set_current_remote(size_t index) {
    furi_check(index < remotes.size());
    current_remote_index = index;
}

void IrdaAppRemoteManager::set_current_button(size_t index) {
    furi_check(current_remote_index < remotes.size());
    furi_check(index < remotes[current_remote_index].buttons.size());
    current_button_index = index;
}

void IrdaAppRemoteManager::delete_current_remote() {
    remotes.erase(remotes.begin() + current_remote_index);
    current_remote_index = 0;
}

void IrdaAppRemoteManager::delete_current_button() {
    auto& buttons = remotes[current_remote_index].buttons;
    buttons.erase(buttons.begin() + current_button_index);
    current_button_index = 0;
}

std::string IrdaAppRemoteManager::get_current_button_name() {
    auto buttons = remotes[current_remote_index].buttons;
    return buttons[current_button_index].name;
}

std::string IrdaAppRemoteManager::get_current_remote_name() {
    return remotes[current_remote_index].name;
}

void IrdaAppRemoteManager::rename_remote(const char* str) {
    remotes[current_remote_index].name = str;
}

void IrdaAppRemoteManager::rename_button(const char* str) {
    remotes[current_remote_index].buttons[current_button_index].name = str;
}

size_t IrdaAppRemoteManager::get_current_remote_buttons_number() {
    return remotes[current_remote_index].buttons.size();
}
