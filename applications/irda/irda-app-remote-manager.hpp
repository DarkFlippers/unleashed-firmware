#pragma once
#include "sys/_stdint.h"
#include <algorithm>
#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include <memory>
#include <irda.h>
#include <sd-card-api.h>
#include <filesystem-api.h>

class IrdaAppRemoteButton {
    friend class IrdaAppRemoteManager;
    std::string name;
    IrdaMessage message;
public:
    IrdaAppRemoteButton(const char* name, const IrdaMessage* message)
        : name(name), message (*message) {}
    ~IrdaAppRemoteButton() {}
};

class IrdaAppRemote {
    friend class IrdaAppRemoteManager;
    std::vector<IrdaAppRemoteButton> buttons;
    std::string name;
public:
    IrdaAppRemote(const std::string& name);
    IrdaAppRemote& operator=(std::string& new_name) noexcept
    {
        name = new_name;
        buttons.clear();
        return *this;
    }
};

class IrdaAppRemoteManager {
    static const char* irda_directory;
    static const char* irda_extension;
    std::unique_ptr<IrdaAppRemote> remote;
    // TODO: make FS_Api and SdCard_Api unique_ptr
    SdCard_Api* sd_ex_api;
    FS_Api* fs_api;
    void show_file_error_message(const char* error_text) const;
    bool parse_button(std::string& str);
    std::string make_filename(const std::string& name) const;
    char file_buf[48];
    size_t file_buf_cnt = 0;

public:
    bool add_remote_with_button(const char* button_name, const IrdaMessage* message);
    bool add_button(const char* button_name, const IrdaMessage* message);

    int find_remote_name(const std::vector<std::string>& strings);
    bool rename_button(uint32_t index, const char* str);
    bool rename_remote(const char* str);

    bool get_remote_list(std::vector<std::string>& remote_names) const;
    std::vector<std::string> get_button_list() const;
    std::string get_button_name(uint32_t index);
    std::string get_remote_name();
    size_t get_number_of_buttons();
    const IrdaMessage* get_button_data(size_t button_index) const;
    bool delete_button(uint32_t index);
    bool delete_remote();
    IrdaAppRemoteManager();
    ~IrdaAppRemoteManager();

    bool store();
    bool load(const std::string& name);
    bool check_fs() const;
};

