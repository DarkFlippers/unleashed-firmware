#include "irda-app-remote-manager.hpp"
#include "filesystem-api.h"
#include "furi.h"
#include "furi/check.h"
#include "gui/modules/button_menu.h"
#include "irda.h"
#include <cstdio>
#include <string>
#include <utility>
#include "irda-app-file-parser.hpp"

const char* IrdaAppRemoteManager::irda_directory = "irda";
const char* IrdaAppRemoteManager::irda_extension = ".ir";
static const std::string default_remote_name = "remote";

static bool find_string(const std::vector<std::string>& strings, const std::string& match_string) {
    for(const auto& string : strings) {
        if(!string.compare(match_string)) return true;
    }
    return false;
}

static std::string
    find_vacant_name(const std::vector<std::string>& strings, const std::string& name) {
    // if suggested name is occupied, try another one (name2, name3, etc)
    if(find_string(strings, name)) {
        int i = 1;
        while(find_string(strings, name + std::to_string(++i)))
            ;
        return name + std::to_string(i);
    } else {
        return name;
    }
}

bool IrdaAppRemoteManager::add_button(const char* button_name, const IrdaAppSignal& signal) {
    remote->buttons.emplace_back(button_name, signal);
    return store();
}

bool IrdaAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const IrdaAppSignal& signal) {
    furi_check(button_name != nullptr);

    std::vector<std::string> remote_list;
    bool result = get_remote_list(remote_list);
    if(!result) return false;

    auto new_name = find_vacant_name(remote_list, default_remote_name);

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

std::string IrdaAppRemoteManager::make_full_name(const std::string& remote_name) const {
    return std::string("/") + irda_directory + "/" + remote_name + irda_extension;
}

std::string IrdaAppRemoteManager::make_remote_name(const std::string& full_name) const {
    std::string str(full_name, full_name.find_last_of('/') + 1, full_name.size());
    str.erase(str.find_last_of('.'));

    return str;
}

bool IrdaAppRemoteManager::delete_remote() {
    FS_Error fs_res;
    IrdaAppFileParser file_parser;

    fs_res = file_parser.get_fs_api().common.remove(make_full_name(remote->name).c_str());
    if(fs_res != FSE_OK) {
        file_parser.get_sd_api().show_error(
            file_parser.get_sd_api().context, "Error deleting file");
        return false;
    }
    remote.reset();
    return true;
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
    return buttons[index].name;
}

std::string IrdaAppRemoteManager::get_remote_name() {
    furi_check(remote.get() != nullptr);
    return remote->name;
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

    if(!remote->name.compare(str)) return true;

    std::vector<std::string> remote_list;
    bool result = get_remote_list(remote_list);
    if(!result) return false;

    auto new_name = find_vacant_name(remote_list, str);
    IrdaAppFileParser file_parser;
    FS_Error fs_err = file_parser.get_fs_api().common.rename(
        make_full_name(remote->name).c_str(), make_full_name(new_name).c_str());
    remote->name = new_name;
    if(fs_err != FSE_OK) {
        file_parser.get_sd_api().show_error(
            file_parser.get_sd_api().context, "Error renaming\nremote file");
    }
    return fs_err == FSE_OK;
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
    File file;
    std::string dirname(std::string("/") + irda_directory);

    IrdaAppFileParser file_parser;
    FS_Error fs_err = file_parser.get_fs_api().common.mkdir(dirname.c_str());
    if((fs_err != FSE_OK) && (fs_err != FSE_EXIST)) {
        file_parser.get_sd_api().show_error(
            file_parser.get_sd_api().context, "Can't create directory");
        return false;
    }

    bool res = file_parser.get_fs_api().file.open(
        &file, make_full_name(remote->name).c_str(), FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(!res) {
        file_parser.get_sd_api().show_error(
            file_parser.get_sd_api().context, "Cannot create\nnew remote file");
        return false;
    }

    for(const auto& button : remote->buttons) {
        bool result = file_parser.store_signal(&file, button.signal, button.name.c_str());
        if(!result) {
            file_parser.get_sd_api().show_error(
                file_parser.get_sd_api().context, "Cannot write\nto key file");
            file_parser.get_fs_api().file.close(&file);
            return false;
        }
    }

    file_parser.get_fs_api().file.close(&file);
    file_parser.get_sd_api().check_error(file_parser.get_sd_api().context);

    return true;
}

bool IrdaAppRemoteManager::get_remote_list(std::vector<std::string>& remote_names) const {
    bool fs_res = false;
    char name[128];
    File dir;
    std::string dirname(std::string("/") + irda_directory);
    remote_names.clear();

    IrdaAppFileParser file_parser;
    fs_res = file_parser.get_fs_api().dir.open(&dir, dirname.c_str());
    if(!fs_res) {
        if(!check_fs()) {
            file_parser.get_sd_api().show_error(
                file_parser.get_sd_api().context, "Cannot open\napplication directory");
            return false;
        } else {
            return true; // SD ok, but no files written yet
        }
    }

    while(file_parser.get_fs_api().dir.read(&dir, nullptr, name, sizeof(name)) && strlen(name)) {
        std::string filename(name);
        auto extension_index = filename.rfind(irda_extension);
        if((extension_index == std::string::npos) ||
           (extension_index + strlen(irda_extension) != filename.size())) {
            continue;
        }
        remote_names.push_back(filename.erase(extension_index));
    }
    file_parser.get_fs_api().dir.close(&dir);

    return true;
}

bool IrdaAppRemoteManager::load(const std::string& name_arg, bool fullpath) {
    bool fs_res = false;
    IrdaAppFileParser file_parser;
    File file;
    std::string full_filename;
    std::string remote_name;

    if(fullpath) {
        full_filename = name_arg;
        remote_name = make_remote_name(name_arg);
    } else {
        full_filename = make_full_name(name_arg);
        remote_name = name_arg;
    }

    fs_res = file_parser.get_fs_api().file.open(
        &file, full_filename.c_str(), FSAM_READ, FSOM_OPEN_EXISTING);
    if(!fs_res) {
        file_parser.get_sd_api().show_error(
            file_parser.get_sd_api().context, "Error opening file");
        return false;
    }

    remote = std::make_unique<IrdaAppRemote>(remote_name);

    while(1) {
        auto file_signal = file_parser.read_signal(&file);
        if(!file_signal.get()) break;
        remote->buttons.emplace_back(file_signal->name, file_signal->signal);
    }
    file_parser.get_fs_api().file.close(&file);

    return true;
}

bool IrdaAppRemoteManager::check_fs() const {
    // TODO: [FL-1431] Add return value to file_parser.get_sd_api().check_error() and replace get_fs_info().
    IrdaAppFileParser file_parser;
    auto fs_err = file_parser.get_fs_api().common.get_fs_info(nullptr, nullptr);
    if(fs_err != FSE_OK)
        file_parser.get_sd_api().show_error(file_parser.get_sd_api().context, "SD card not found");
    return fs_err == FSE_OK;
}
