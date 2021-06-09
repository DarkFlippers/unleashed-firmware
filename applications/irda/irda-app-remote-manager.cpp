#include "irda-app-remote-manager.hpp"
#include "filesystem-api.h"
#include "furi.h"
#include "furi/check.h"
#include "gui/modules/button_menu.h"
#include "irda.h"
#include "sys/_stdint.h"
#include <cstdio>
#include <string>
#include <utility>

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

IrdaAppRemoteManager::IrdaAppRemoteManager() {
    sd_ex_api = static_cast<SdCard_Api*>(furi_record_open("sdcard-ex"));
    fs_api = static_cast<FS_Api*>(furi_record_open("sdcard"));
}

IrdaAppRemoteManager::~IrdaAppRemoteManager() {
    furi_record_close("sdcard");
    furi_record_close("sdcard-ex");
}

bool IrdaAppRemoteManager::add_button(const char* button_name, const IrdaMessage* message) {
    remote->buttons.emplace_back(button_name, message);
    return store();
}

bool IrdaAppRemoteManager::add_remote_with_button(
    const char* button_name,
    const IrdaMessage* message) {
    furi_check(button_name != nullptr);
    furi_check(message != nullptr);

    std::vector<std::string> remote_list;
    bool result = get_remote_list(remote_list);
    if(!result) return false;

    auto new_name = find_vacant_name(remote_list, default_remote_name);

    remote = std::make_unique<IrdaAppRemote>(new_name);
    return add_button(button_name, message);
}

IrdaAppRemote::IrdaAppRemote(const std::string& name)
    : name(name) {
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

const IrdaMessage* IrdaAppRemoteManager::get_button_data(size_t index) const {
    furi_check(remote.get() != nullptr);
    auto& buttons = remote->buttons;
    furi_check(index < buttons.size());

    return &buttons.at(index).message;
}

std::string IrdaAppRemoteManager::make_filename(const std::string& name) const {
    return std::string("/") + irda_directory + "/" + name + irda_extension;
}

bool IrdaAppRemoteManager::delete_remote() {
    FS_Error fs_res;

    fs_res = fs_api->common.remove(make_filename(remote->name).c_str());
    if(fs_res != FSE_OK) {
        show_file_error_message("Error deleting file");
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
    FS_Error fs_err = fs_api->common.rename(
        make_filename(remote->name).c_str(), make_filename(new_name).c_str());
    remote->name = new_name;
    if(fs_err != FSE_OK) {
        show_file_error_message("Error renaming\nremote file");
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

void IrdaAppRemoteManager::show_file_error_message(const char* error_text) const {
    sd_ex_api->show_error(sd_ex_api->context, error_text);
}

bool IrdaAppRemoteManager::store(void) {
    File file;
    uint16_t write_count;
    std::string dirname(std::string("/") + irda_directory);

    FS_Error fs_err = fs_api->common.mkdir(dirname.c_str());
    if((fs_err != FSE_OK) && (fs_err != FSE_EXIST)) {
        show_file_error_message("Can't create directory");
        return false;
    }

    std::string filename = dirname + "/" + remote->name + irda_extension;
    bool res = fs_api->file.open(&file, filename.c_str(), FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(!res) {
        show_file_error_message("Cannot create\nnew remote file");
        return false;
    }

    char content[128];

    for(const auto& button : remote->buttons) {
        auto protocol = button.message.protocol;

        sniprintf(
            content,
            sizeof(content),
            "%.31s %.31s A:%0*lX C:%0*lX\n",
            button.name.c_str(),
            irda_get_protocol_name(protocol),
            irda_get_protocol_address_length(protocol),
            button.message.address,
            irda_get_protocol_command_length(protocol),
            button.message.command);

        auto content_len = strlen(content);
        write_count = fs_api->file.write(&file, content, content_len);
        if(file.error_id != FSE_OK || write_count != content_len) {
            show_file_error_message("Cannot write\nto key file");
            fs_api->file.close(&file);
            return false;
        }
    }

    fs_api->file.close(&file);
    sd_ex_api->check_error(sd_ex_api->context);

    return true;
}

bool IrdaAppRemoteManager::parse_button(std::string& str) {
    char button_name[32];
    char protocol_name[32];
    uint32_t address;
    uint32_t command;

    int parsed = std::sscanf(
        str.c_str(), "%31s %31s A:%lX C:%lX", button_name, protocol_name, &address, &command);

    if(parsed != 4) {
        return false;
    }

    IrdaProtocol protocol = irda_get_protocol_by_name(protocol_name);

    if(!irda_is_protocol_valid((IrdaProtocol)protocol)) {
        return false;
    }

    int address_length = irda_get_protocol_address_length(protocol);
    uint32_t address_mask = (1LU << (4 * address_length)) - 1;
    if(address != (address & address_mask)) {
        return false;
    }

    int command_length = irda_get_protocol_command_length(protocol);
    uint32_t command_mask = (1LU << (4 * command_length)) - 1;
    if(command != (command & command_mask)) {
        return false;
    }

    IrdaMessage irda_message = {
        .protocol = protocol,
        .address = address,
        .command = command,
        .repeat = false,
    };
    remote->buttons.emplace_back(button_name, &irda_message);

    return true;
}

std::string getline(
    const FS_Api* fs_api,
    File& file,
    char file_buf[],
    size_t file_buf_size,
    size_t& file_buf_cnt) {
    std::string str;
    size_t newline_index = 0;
    bool found_eol = false;

    while(1) {
        if(file_buf_cnt > 0) {
            size_t end_index = 0;
            char* endline_ptr = (char*)memchr(file_buf, '\n', file_buf_cnt);
            newline_index = endline_ptr - file_buf;

            if(endline_ptr == 0) {
                end_index = file_buf_cnt;
            } else if(newline_index < file_buf_cnt) {
                end_index = newline_index + 1;
                found_eol = true;
            } else {
                furi_assert(0);
            }

            str.append(file_buf, end_index);
            memmove(file_buf, &file_buf[end_index], file_buf_cnt - end_index);
            file_buf_cnt = file_buf_cnt - end_index;
            if(found_eol) break;
        }

        file_buf_cnt +=
            fs_api->file.read(&file, &file_buf[file_buf_cnt], file_buf_size - file_buf_cnt);
        if(file_buf_cnt == 0) {
            break; // end of reading
        }
    }

    return str;
}

bool IrdaAppRemoteManager::get_remote_list(std::vector<std::string>& remote_names) const {
    bool fs_res = false;
    char name[128];
    File dir;
    std::string dirname(std::string("/") + irda_directory);
    remote_names.clear();

    fs_res = fs_api->dir.open(&dir, dirname.c_str());
    if(!fs_res) {
        if(!check_fs()) {
            show_file_error_message("Cannot open\napplication directory");
            return false;
        } else {
            return true; // SD ok, but no files written yet
        }
    }

    while(fs_api->dir.read(&dir, nullptr, name, sizeof(name)) && strlen(name)) {
        std::string filename(name);
        auto extension_index = filename.rfind(irda_extension);
        if((extension_index == std::string::npos) ||
           (extension_index + strlen(irda_extension) != filename.size())) {
            continue;
        }
        remote_names.push_back(filename.erase(extension_index));
    }
    fs_api->dir.close(&dir);

    return true;
}

bool IrdaAppRemoteManager::load(const std::string& name) {
    bool fs_res = false;
    File file;

    fs_res = fs_api->file.open(&file, make_filename(name).c_str(), FSAM_READ, FSOM_OPEN_EXISTING);
    if(!fs_res) {
        show_file_error_message("Error opening file");
        return false;
    }

    remote = std::make_unique<IrdaAppRemote>(name);

    while(1) {
        auto str = getline(fs_api, file, file_buf, sizeof(file_buf), file_buf_cnt);
        if(str.empty()) break;
        parse_button(str);
    }
    fs_api->file.close(&file);

    return true;
}

bool IrdaAppRemoteManager::check_fs() const {
    // TODO: [FL-1431] Add return value to sd_ex_api->check_error() and replace get_fs_info().
    auto fs_err = fs_api->common.get_fs_info(nullptr, nullptr);
    if(fs_err != FSE_OK) show_file_error_message("SD card not found");
    return fs_err == FSE_OK;
}
